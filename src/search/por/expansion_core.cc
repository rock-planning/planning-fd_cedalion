#include "expansion_core.h"

#include "../globals.h"
#include "../operator.h"

#include <cassert>
#include <iostream>
#include <algorithm>
#include <vector>
using namespace std;


namespace POR {
static inline int get_op_index(const Operator *op) {
    int op_index = op - &*g_operators.begin();
    assert(op_index >= 0 && op_index < g_operators.size());
    return op_index;
}

inline bool operator<(const Operator &lhs, const Operator &rhs) {
    return get_op_index(&lhs) < get_op_index(&rhs);
}

ExpansionCore::ExpansionCore() {
    verify_no_axioms_no_cond_effects();
    build_dtgs();
    mark_co_effects();
}

ExpansionCore::~ExpansionCore() {
}

void ExpansionCore::dump_options() const {
    cout << "partial order reduction method: expansion core" << endl;
}

void ExpansionCore::mark_co_effects() {
    size_t num_variables = g_variable_domain.size();
    co_effects_arcs.resize(num_variables, vector<bool>(num_variables));

    size_t num_operators = g_operators.size();
    for (size_t op_no = 0; op_no < num_operators; ++op_no) {
        const Operator &op = g_operators[op_no];
        const vector<PrePost> &pre_post = op.get_pre_post();
        for (int i = 0; i < pre_post.size(); ++i) {
            int eff1_var = pre_post[i].var;
            for (int j = 0; j < pre_post.size(); ++j) {
                int eff2_var = pre_post[j].var;
                if (eff1_var != eff2_var)
                    co_effects_arcs[eff1_var][eff2_var] = true;
            }
        }
    }
}

void ExpansionCore::mark_potential_preconditions(const State &state) {
    size_t num_variables = g_variable_domain.size();
    size_t num_operators = g_operators.size();

    /*
      Step 1:

      For all variables v', mark all operators in DTG(v') that are
      backward-reachable from the goal (should be *all* operators in
      the DTG, since we do a relevance analysis in the preprocessor)
      and forward-reachable from the current value of v'.
    */

    // DTG operators are marked by variable index and operator index.

    /*
      TODO: It might be more efficient to only set up this vector
      once and then reuse it.
    */
    vector<vector<bool> > marked_dtg_operators(
        num_variables, vector<bool>(num_operators));

    for (int var_no = 0; var_no < num_variables; ++var_no) {
        const ExpansionCoreDTG &dtg = dtgs[var_no];
        vector<bool> forward_reachable;
        dtg.forward_reachability_analysis(state[var_no], forward_reachable);
        vector<bool> backward_reachable;
        dtg.backward_relevance_analysis(backward_reachable);

        int num_values = g_variable_domain[var_no];
        for (int value = 0; value < num_values; ++value) {
            if (forward_reachable[value] && backward_reachable[value]) {
                const vector<ExpansionCoreDTG::Arc> &arcs =
                    dtg.nodes[value].outgoing;
                for (int i = 0; i < arcs.size(); ++i) {
                    int target_value = arcs[i].target_value;
                    int op_no = arcs[i].operator_no;
                    if (backward_reachable[target_value]) {
                        marked_dtg_operators[var_no][op_no] = true;
                    }
                }
            }
        }
    }

    /*
      Step 2:

      Iterate over all operators o. For each precondition variable v
      and effect variable v', report the arc (v, v') if the
      precondition on v is satisfied and o is marked in DTG(v').

      Note that we don't care about variables v which do not have a
      proper precondition (i.e., a pre_post entry with pre = -1).
    */

    for (size_t op_no = 0; op_no < num_operators; ++op_no) {
        const Operator &op = g_operators[op_no];

        const vector<Prevail> &prevail = op.get_prevail();
        const vector<PrePost> &pre_post = op.get_pre_post();

        vector<int> satisfied_pre_vars;

        for (int i = 0; i < prevail.size(); ++i) {
            int pre_var = prevail[i].var;
            int pre_value = prevail[i].prev;
            if (state[pre_var] == pre_value)
                satisfied_pre_vars.push_back(pre_var);
        }

        for (int i = 0; i < pre_post.size(); ++i) {
            int pre_var = pre_post[i].var;
            int pre_value = pre_post[i].pre;
            if (pre_value != -1 && state[pre_var] == pre_value)
                satisfied_pre_vars.push_back(pre_var);
        }

        for (int i = 0; i < pre_post.size(); ++i) {
            int post_var = pre_post[i].var;
            if (marked_dtg_operators[post_var][op_no]) {
                for (int j = 0; j < satisfied_pre_vars.size(); ++j) {
                    int pre_var = satisfied_pre_vars[j];
                    if (pre_var != post_var) {
                        mark_pdg_arc(pre_var, post_var);
                    }
                }
            }
        }
    }
}


void ExpansionCore::mark_potential_dependents(const State &state) {
    size_t num_variables = g_variable_domain.size();
    size_t num_operators = g_operators.size();

    /*
      Step 1:

      For all variables v', mark all values in DTG(v') which are
      backward-reachable from the goal (should be *all* value in the
      DTG, since we do a relevance analysis in the preprocessor) and
      forward-reachable from the current value of v'.
    */

    /*
      TODO: It might be more efficient to only set up this vector
      once and then reuse it.
    */
    vector<vector<bool> > marked_dtg_nodes(num_variables);
    for (int var_no = 0; var_no < num_variables; ++var_no) {
        vector<bool> &marked_nodes = marked_dtg_nodes[var_no];
        int num_values = g_variable_domain[var_no];
        marked_nodes.resize(num_values, false);

        const ExpansionCoreDTG &dtg = dtgs[var_no];
        vector<bool> forward_reachable;
        dtg.forward_reachability_analysis(state[var_no], forward_reachable);
        vector<bool> backward_reachable;
        dtg.backward_relevance_analysis(backward_reachable);

        for (int value = 0; value < num_values; ++value)
            if (forward_reachable[value] && backward_reachable[value])
                marked_nodes[value] = true;
    }

    /*
      Step 2:

      Iterate over all operators o. For each precondition variable v'
      and effect variable v, report the arc (v, v') if
      1) the precondition on v is satisfied or there is no
         precondition on v, and
      2) operator o is marked in DTG(v').
    */

    for (size_t op_no = 0; op_no < num_operators; ++op_no) {
        const Operator &op = g_operators[op_no];

        const vector<Prevail> &prevail = op.get_prevail();
        const vector<PrePost> &pre_post = op.get_pre_post();

        vector<int> pre_vars;

        for (int i = 0; i < prevail.size(); ++i) {
            int pre_var = prevail[i].var;
            int pre_value = prevail[i].prev;
            if (marked_dtg_nodes[pre_var][pre_value])
                pre_vars.push_back(pre_var);
        }

        for (int i = 0; i < pre_post.size(); ++i) {
            int pre_var = pre_post[i].var;
            int pre_value = pre_post[i].pre;
            if (pre_value != -1 && marked_dtg_nodes[pre_var][pre_value])
                pre_vars.push_back(pre_var);
        }

        for (int i = 0; i < pre_post.size(); ++i) {
            int eff_var = pre_post[i].var;
            int pre_value = pre_post[i].pre;
            if (pre_value == -1 || state[eff_var] == pre_value) {
                for (int j = 0; j < pre_vars.size(); ++j) {
                    int pre_var = pre_vars[j];
                    if (pre_var != eff_var) {
                        mark_pdg_arc(eff_var, pre_var);
                    }
                }
            }
        }
    }
}

void ExpansionCore::do_pruning(const State &state,
                               vector<const Operator *> &ops) {
    /*
      Expansion core pseudo-code:
      0. (preprocessing) Compute variables which co-occur in effects.
      1. Set potential dependency graph to arcs precomputed in step 0.
      2. Compute potential preconditions.
      3. Compute potential dependents.
      4. Compute dependency closure.
      5. Prune operators based on dependency closure.
    */

    pdg_arcs = co_effects_arcs;
    mark_potential_preconditions(state);
    mark_potential_dependents(state);
    vector<bool> dependency_closure;
    compute_dependency_closure(state, dependency_closure);

    vector<const Operator *> pruned_ops;
    pruned_ops.reserve(ops.size());

    for (int i = 0; i < ops.size(); ++i) {
        const Operator *op = ops[i];

        const vector<PrePost> &pre_post = op->get_pre_post();
        for (int j = 0; j < pre_post.size(); ++j) {
            int var = pre_post[j].var;
            if (dependency_closure[var]) {
                pruned_ops.push_back(op);
                break;
            }
        }
    }

    if (pruned_ops.size() != ops.size()) {
        ops.swap(pruned_ops);
        sort(ops.begin(), ops.end());
    }
}

void ExpansionCore::compute_dependency_closure(
    const State &state, vector<bool> &dependency_closure) const {
    int unsatisfied_goal_var = -1;
    for (int i = 0; i < g_goal.size(); ++i) {
        int var = g_goal[i].first;
        int value = g_goal[i].second;
        if (state[var] != value) {
            unsatisfied_goal_var = var;
            break;
        }
    }
    assert(unsatisfied_goal_var != -1);

    size_t num_variables = g_variable_domain.size();
    dependency_closure.assign(num_variables, false);
    recurse_dependency_closure(unsatisfied_goal_var, dependency_closure);
}

void ExpansionCore::recurse_dependency_closure(
    int var, vector<bool> &closure) const {
    if (!closure[var]) {
        closure[var] = true;
        size_t num_variables = g_variable_domain.size();
        for (int other_var = 0; other_var < num_variables; ++other_var)
            if (pdg_arcs[var][other_var])
                recurse_dependency_closure(other_var, closure);
    }
}

void ExpansionCore::build_dtgs() {
    /*
      NOTE: Code lifted and adapted from M&S atomic abstraction code.

      We need a more general mechanism for creating data structures of
      this kind.
    */

    /*
      NOTE: for expansion core, the DTG for v *does* include
      self-loops from d to d if there is an operator that sets the
      value of v to d and has no precondition on v. This is different
      from the usual DTG definition.
    */

    cout << "[expansion core] Building DTGs..." << flush;
    assert(dtgs.empty());
    size_t num_variables = g_variable_domain.size();

    dtgs.resize(num_variables);

    // Step 1: Create the empty DTG nodes.
    for (int var_no = 0; var_no < num_variables; ++var_no) {
        size_t var_size = g_variable_domain[var_no];
        dtgs[var_no].nodes.resize(var_size);
        dtgs[var_no].goal_values.resize(var_size, true);
    }

    // Step 2: Mark goal values in each DTG. Variables that are not in
    //         the goal have previously been set to "all are goals".
    for (int i = 0; i < g_goal.size(); ++i) {
        int var_no = g_goal[i].first;
        int goal_value = g_goal[i].second;
        vector<bool> &goal_values = dtgs[var_no].goal_values;
        size_t var_size = g_variable_domain[var_no];
        goal_values.clear();
        goal_values.resize(var_size, false);
        goal_values[goal_value] = true;
    }

    // Step 3: Add DTG arcs.
    for (int op_no = 0; op_no < g_operators.size(); ++op_no) {
        const Operator &op = g_operators[op_no];
        const vector<PrePost> &pre_post = op.get_pre_post();
        for (int i = 0; i < pre_post.size(); ++i) {
            int var = pre_post[i].var;
            int pre_value = pre_post[i].pre;
            int post_value = pre_post[i].post;

            ExpansionCoreDTG &dtg = dtgs[var];
            int pre_value_min, pre_value_max;
            if (pre_value == -1) {
                pre_value_min = 0;
                pre_value_max = g_variable_domain[var];
            } else {
                pre_value_min = pre_value;
                pre_value_max = pre_value + 1;
            }
            for (int value = pre_value_min; value < pre_value_max; ++value) {
                dtg.nodes[value].outgoing.push_back(
                    ExpansionCoreDTG::Arc(post_value, op_no));
                dtg.nodes[post_value].incoming.push_back(
                    ExpansionCoreDTG::Arc(value, op_no));
            }
        }
    }
    cout << " done!" << endl;
}

void ExpansionCore::mark_pdg_arc(int from_var, int to_var) {
    pdg_arcs[from_var][to_var] = true;
}


ExpansionCoreDTG::ExpansionCoreDTG() {
}


ExpansionCoreDTG::~ExpansionCoreDTG() {
}

void ExpansionCoreDTG::forward_reachability_analysis(
    int start_value, vector<bool> &reachable) const {
    size_t num_values = nodes.size();
    reachable.clear();
    reachable.resize(num_values, false);
    recurse_forwards(start_value, reachable);
}

void ExpansionCoreDTG::backward_relevance_analysis(
    vector<bool> &relevant) const {
    size_t num_values = nodes.size();
    relevant.clear();
    relevant.resize(num_values, false);
    for (int value = 0; value < num_values; ++value) {
        if (goal_values[value]) {
            recurse_backwards(value, relevant);
        }
    }
}

void ExpansionCoreDTG::recurse_forwards(
    int value, vector<bool> &reachable) const {
    if (!reachable[value]) {
        reachable[value] = true;
        const vector<Arc> &outgoing = nodes[value].outgoing;
        for (int i = 0; i < outgoing.size(); ++i)
            recurse_forwards(outgoing[i].target_value, reachable);
    }
}

void ExpansionCoreDTG::recurse_backwards(
    int value, vector<bool> &relevant) const {
    if (!relevant[value]) {
        relevant[value] = true;
        const vector<Arc> &incoming = nodes[value].incoming;
        for (int i = 0; i < incoming.size(); ++i)
            recurse_backwards(incoming[i].target_value, relevant);
    }
}
}
