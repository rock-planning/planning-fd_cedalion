#include "sss_expansion_core.h"
#include "expansion_core.h"
#include "../globals.h"
#include "../operator.h"
#include "../successor_generator.h"
#include "../option_parser.h"
#include <cassert>
#include <algorithm>

using namespace std;
class Operator;
class State;

namespace POR {
std::vector<std::vector<FactEC> > operator_preconds;
std::vector<std::vector<FactEC> > operator_effects;

// Copied from SimpleStubbornSets
static inline int get_op_index(const Operator *op) {
    int op_index = op - &*g_operators.begin();
    assert(op_index >= 0 && op_index < g_operators.size());
    return op_index;
}

inline bool operator<(const Operator &lhs, const Operator &rhs) {
    return get_op_index(&lhs) < get_op_index(&rhs);
}

static inline bool is_v_applicable(int var, int op_no, const State &state, std::vector<std::vector<int> > &v_precond) {
    int vprecond = v_precond[op_no][var];
    return vprecond == -1 || vprecond == state[var];
}

// Copied from SimpleStubbornSets
static inline pair<int, int> find_unsatisfied_goal(const State &state) {
    for (size_t i = 0; i < g_goal.size(); ++i) {
        int goal_var = g_goal[i].first;
        int goal_value = g_goal[i].second;
        if (state[goal_var] != goal_value)
            return make_pair(goal_var, goal_value);
    }
    return make_pair(-1, -1);
}

static inline void get_disabled_vars(int op1_no, int op2_no, std::vector<int> &disabled_vars) {
    disabled_vars.clear();
    size_t i = 0;
    size_t j = 0;
    while (i < operator_preconds[op2_no].size() && j < operator_effects[op1_no].size()) {
        int read_var = operator_preconds[op2_no][i].var;
        int write_var = operator_effects[op1_no][j].var;
        if (read_var < write_var) {
            i++;
        } else {
            if (read_var == write_var) {
                int read_value = operator_preconds[op2_no][i].val;
                int write_value = operator_effects[op1_no][j].val;
                if (read_value != write_value) {
                    disabled_vars.push_back(read_var);
                }
                i++;
                j++;
            } else {
                // read_var > write_var
                j++;
            }
        }
    }
}

static inline bool can_disable(int op1_no, int op2_no) {
    std::vector<int> disabled_vars;
    get_disabled_vars(op1_no, op2_no, disabled_vars);
    return !disabled_vars.empty();
}

// Copied from SimpleStubbornSets
static inline bool can_conflict(int op1_no, int op2_no) {
    size_t i = 0;
    size_t j = 0;
    while (i < operator_effects[op1_no].size() && j < operator_effects[op2_no].size()) {
        int var1 = operator_effects[op1_no][i].var;
        int var2 = operator_effects[op2_no][j].var;
        if (var1 < var2)
            i++;
        else if (var1 == var2) {
            int value1 = operator_effects[op1_no][i].val;
            int value2 = operator_effects[op2_no][j].val;
            if (value1 != value2)
                return true;
            i++;
            j++;
        } else {
            // var1 > var2
            j++;
        }
    }
    return false;
}


SSS_ExpansionCore::SSS_ExpansionCore(const Options &opts) : 
    mutex_option(MutexOption(opts.get_enum("mutexes"))),
    use_active_ops(opts.get<bool>("active_ops"))
{
    verify_no_axioms_no_cond_effects();
    compute_sorted_operators();
    compute_v_precond();
    compute_achievers();
    compute_conflicts_and_disabling();
    size_t num_variables = g_variable_domain.size();
    reachability_map.resize(num_variables);
    build_dtgs();
    build_reachability_map();

    for (size_t i = 0; i < g_variable_domain.size(); i++) {
        nes_computed.push_back(vector<bool>(g_variable_domain[i], false));
    } 
   
}

SSS_ExpansionCore::~SSS_ExpansionCore() {}

void SSS_ExpansionCore::dump_options() const {
    cout << "partial order reduction method: sss_expansion core" << endl;
    
    cout << "mutexes for dependency relationship: ";
    switch (mutex_option) {
    case 0:
        cout << "none" << endl;
        break;
    case 1:
        cout << "Fast Downward mutexes" << endl;
        break;
    default:
	cout << "ERROR: unknown option for mutexes" << endl;
        exit(2);
    }   

    cout << "Active operators: ";

    if (use_active_ops)
	cout << "Enabled" << endl;
    else
	cout << "Disabled" << endl;
    
}

// Copied from SimpleStubbornSets
void SSS_ExpansionCore::compute_sorted_operators() {
    for (size_t op_no = 0; op_no < g_operators.size(); ++op_no) {
        Operator *op = &g_operators[op_no];
        const vector<PrePost> &pre_post = op->get_pre_post();
        const vector<Prevail> &prevail = op->get_prevail();

        vector<FactEC> pre;
        vector<FactEC> eff;

        for (size_t i = 0; i < pre_post.size(); i++) {
            const PrePost *pp = &pre_post[i];
            if (pp->pre != -1) {
                FactEC p(pp->var, pp->pre);
                pre.push_back(p);
            }
            FactEC e(pp->var, pp->post);
            eff.push_back(e);
        }
        for (size_t i = 0; i < prevail.size(); i++) {
            const Prevail *pp = &prevail[i];
            FactEC p(pp->var, pp->prev);
            pre.push_back(p);
        }
        if (pre.size() != 0) {
            sort(pre.begin(), pre.end());
            for (size_t i = 0; i < pre.size() - 1; ++i) {
                assert(pre[i].var < pre[i + 1].var);
            }
        }
        sort(eff.begin(), eff.end());
        for (size_t i = 0; i < eff.size() - 1; ++i) {
            assert(eff[i].var < eff[i + 1].var);
        }
        operator_preconds.push_back(pre);
        operator_effects.push_back(eff);
    }
}

// This code is copied from ExpansionCore
void SSS_ExpansionCore::build_dtgs() {
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

    cout << "[sss-expansion core] Building DTGs..." << flush;
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

void SSS_ExpansionCore::compute_v_precond() {
    v_precond.resize(g_operators.size());
    for (int op_no = 0; op_no < g_operators.size(); op_no++) {
        v_precond[op_no].resize(g_variable_name.size(), -1);
        for (int var = 0; var < g_variable_name.size(); var++) {
            const vector<Prevail> &prevail = g_operators[op_no].get_prevail();
            for (size_t i = 0; i < prevail.size(); ++i) {
                int prevar = prevail[i].var;
                if (prevar == var) {
                    v_precond[op_no][var] = prevail[i].prev;
                }
            }
            const vector<PrePost> &pre_post = g_operators[op_no].get_pre_post();
            for (size_t i = 0; i < pre_post.size(); ++i) {
                int prevar = pre_post[i].var;
                if (prevar == var) {
                    v_precond[op_no][var] = pre_post[i].pre;
                }
            }
        }
    }
}

void SSS_ExpansionCore::build_reachability_map() {
    size_t num_variables = g_variable_domain.size();
    for (int var_no = 0; var_no < num_variables; ++var_no) {
        ExpansionCoreDTG &dtg = dtgs[var_no];
        size_t num_values = dtg.nodes.size();
        reachability_map[var_no].resize(num_values);
        for (int val = 0; val < num_values; ++val) {
            reachability_map[var_no][val].assign(num_values, false);
        }
        for (int start_value = 0; start_value < g_variable_domain[var_no]; start_value++) {
            vector<bool> &reachable = reachability_map[var_no][start_value];
            recurse_forwards(var_no, start_value, start_value, reachable);
        }
    }
}


// Copied from ExpansionCore and adapted
void SSS_ExpansionCore::recurse_forwards(int var, int start_value, int current_value, std::vector<bool> &reachable) {
    ExpansionCoreDTG &dtg = dtgs[var];
    if (!reachable[current_value]) {
        reachable[current_value] = true;
        const vector<ExpansionCoreDTG::Arc> &outgoing = dtg.nodes[current_value].outgoing;
        for (int i = 0; i < outgoing.size(); ++i)
            recurse_forwards(var, start_value, outgoing[i].target_value, reachable);
    }
}

void SSS_ExpansionCore::compute_active_operators(const State &state) {
    for (size_t op_no = 0; op_no < g_operators.size(); ++op_no) {
        Operator &op = g_operators[op_no];
        bool all_preconditions_are_active = true;
        const vector<Prevail> &prevail = op.get_prevail();
        for (size_t i = 0; i < prevail.size(); ++i) {
            int var = prevail[i].var;
            int value = prevail[i].prev;
            int current_value = state[var];
            std::vector<bool> &reachable_values = reachability_map[var][current_value];
            if (!reachable_values[value]) {
                all_preconditions_are_active = false;
                break;
            }
        }
        if (all_preconditions_are_active) {
            const vector<PrePost> &pre_post = op.get_pre_post();
            for (size_t i = 0; i < pre_post.size(); ++i) {
                int var = pre_post[i].var;
                int value = pre_post[i].pre;
                if (value != -1) {
                    int current_value = state[var];
                    std::vector<bool> &reachable_values = reachability_map[var][current_value];
                    if (!reachable_values[value]) {
                        all_preconditions_are_active = false;
                        break;
                    }
                }
            }
        }
        if (all_preconditions_are_active) {
            active_ops[op_no] = true;
        }
    }
}

static inline void compute_precondition_facts(int op_no, vector<pair<int, int> >& op_preconds) {

    Operator *op = &g_operators[op_no];
    const vector<PrePost> &pre_post = op->get_pre_post();
    const vector<Prevail> &prevail = op->get_prevail();

    for (size_t i = 0; i < pre_post.size(); i++) {
	const PrePost *pp = &pre_post[i];
	if (pp->pre != -1) {
	    op_preconds.push_back(make_pair(pp->var, pp->pre));
	}
    }
    
    for (size_t i = 0; i < prevail.size(); i++) {
	const Prevail *pp = &prevail[i];
	op_preconds.push_back(make_pair(pp->var, pp->prev));
    }

}

static inline bool mutex_precond(int op1_no, int op2_no) {

    vector<pair<int, int> > op1_preconds;
    vector<pair<int, int> > op2_preconds;
    
    compute_precondition_facts(op1_no, op1_preconds);
    compute_precondition_facts(op2_no, op2_preconds);
    
    for (int i = 0; i < op1_preconds.size(); i++) {
	
	 int var1 = op1_preconds[i].first;
	 int val1 = op1_preconds[i].second;
	
	for (int j = 0; j < op2_preconds.size(); j++) {
	    
	    int var2 = op2_preconds[j].first;
	    int val2 = op2_preconds[j].second;
	    
	    if (are_mutex(make_pair(var1,val1), make_pair(var2, val2)))
		return true;
	}
    }
    
    return false;

}


void SSS_ExpansionCore::compute_conflicts_and_disabling() {
    size_t num_operators = g_operators.size();
    conflicting_and_disabling.resize(num_operators);
    disabled.resize(num_operators);

    for (size_t op1_no = 0; op1_no < num_operators; ++op1_no) {
        for (size_t op2_no = 0; op2_no < num_operators; ++op2_no) {
            if (op1_no != op2_no) {
		
		bool mutex_prec = false;
		switch (mutex_option) {
		case 0:
		    break;
		case 1:
		    mutex_prec = mutex_precond(op1_no, op2_no);
		    break;
		default:
		    break;
		}

		//		cout << "Mutex precond = " << mutex_prec << endl;
		
                bool conflict = can_conflict(op1_no, op2_no) && !mutex_prec;
                bool disable = can_disable(op2_no, op1_no) && !mutex_prec;
                if (conflict || disable) {
                    conflicting_and_disabling[op1_no].push_back(op2_no);
                }
                if (disable) {
                    disabled[op2_no].push_back(op1_no);
                }
            }
        }
    }

}

//Copied from SimpleStubbornSets
void SSS_ExpansionCore::compute_achievers() {
    size_t num_variables = g_variable_domain.size();
    achievers.resize(num_variables);

    for (int var_no = 0; var_no < num_variables; ++var_no)
        achievers[var_no].resize(g_variable_domain[var_no]);

    for (size_t op_no = 0; op_no < g_operators.size(); ++op_no) {
        const Operator &op = g_operators[op_no];
        const vector<PrePost> &pre_post = op.get_pre_post();
        for (size_t i = 0; i < pre_post.size(); ++i) {
            int var = pre_post[i].var;
            int value = pre_post[i].post;
            achievers[var][value].push_back(op_no);
        }
    }
}

//Copied from SimpleStubbornSets
inline void SSS_ExpansionCore::mark_as_stubborn(int op_no, const State &state) {
    if (!stubborn_ec[op_no]) {
        stubborn_ec[op_no] = true;
        stubborn_ec_queue.push_back(op_no);

        const Operator &op = g_operators[op_no];
        if (op.is_applicable(state)) {
            const vector<PrePost> &pre_post = op.get_pre_post();
            for (size_t i = 0; i < pre_post.size(); ++i) {
                int var = pre_post[i].var;
                written_vars[var] = true;
            }
        }
    }
}

//Copied from SimpleStubbornSets
void SSS_ExpansionCore::add_nes_for_fact(pair<int, int> fact, const State &state) {
    int var = fact.first;
    int value = fact.second;
    const vector<int> &op_nos = achievers[var][value];
    for (size_t i = 0; i < op_nos.size(); ++i) {
        if (active_ops[op_nos[i]]) {
            mark_as_stubborn(op_nos[i], state);
        }
    }

    nes_computed[var][value] = true;
}

void SSS_ExpansionCore::add_conflicting_and_disabling(int op_no, const State &state) {
    const vector<int> &conflict_and_disable = conflicting_and_disabling[op_no];

    for (size_t i = 0; i < conflict_and_disable.size(); ++i) {
        if (active_ops[conflict_and_disable[i]])
            mark_as_stubborn(conflict_and_disable[i], state);
    }
}


void SSS_ExpansionCore::apply_s5(const Operator &op, const State &state) {
    // Find a violated state variable and check if stubborn contains a writer for this variable.

    std::pair<int, int> violated_precondition = make_pair(-1, -1);
    std::pair<int, int> violated_pre_post = make_pair(-1, -1);

    const vector<Prevail> &prevail = op.get_prevail();
    for (size_t i = 0; i < prevail.size(); ++i) {
        int var = prevail[i].var;
        int value = prevail[i].prev;
        if (state[var] != value) {
            if (written_vars[var]) {
                if (!nes_computed[var][value]) {
                    std::pair<int, int> fact = make_pair(var, value);
                    add_nes_for_fact(fact, state);
                }
                return;
            }
            if (violated_precondition.first == -1) {
                violated_precondition = make_pair(var, value);
            }
        }
    }

    const vector<PrePost> &pre_post = op.get_pre_post();
    for (size_t i = 0; i < pre_post.size(); ++i) {
        int var = pre_post[i].var;
        int value = pre_post[i].pre;
        if (value != -1 && state[var] != value) {
            if (written_vars[var]) {
                if (!nes_computed[var][value]) {
                    std::pair<int, int> fact = make_pair(var, value);
                    add_nes_for_fact(fact, state);
                }
                return;
            }
            if (violated_pre_post.first == -1) {
                violated_pre_post = make_pair(var, value);
            }
        }
    }

    if (violated_pre_post.first != -1) {
        if (!nes_computed[violated_pre_post.first][violated_pre_post.second]) {
            add_nes_for_fact(violated_pre_post, state);
        }
        return;
    }

    assert(violated_precondition.first != -1);
    if (!nes_computed[violated_precondition.first][violated_precondition.second]) {
        add_nes_for_fact(violated_precondition, state);
    }

    return;
}


//Copied from SimpleStubbornSets and adapted
void SSS_ExpansionCore::do_pruning(const State &state, std::vector<const Operator *> &applicable_ops) {
    stubborn_ec.clear();
    stubborn_ec.assign(g_operators.size(), false);
    active_ops.clear();
    active_ops.assign(g_operators.size(), false);

    for (size_t i = 0; i < nes_computed.size(); i++) {
        nes_computed[i].clear();
        nes_computed[i].assign(g_variable_domain[i], false);
    }

    
    if (use_active_ops) {
	compute_active_operators(state);
    }
    else {
	active_ops.assign(g_operators.size(), true);
    }

    written_vars.assign(g_variable_domain.size(), false);
    std::vector<int> disabled_vars;

    //rule S1
    pair<int, int> goal_pair = find_unsatisfied_goal(state);
    assert(goal_pair.first != -1);
    add_nes_for_fact(goal_pair, state);     // active operators used

    while (!stubborn_ec_queue.empty()) {
        int op_no = stubborn_ec_queue.back();

        stubborn_ec_queue.pop_back();
        const Operator &op = g_operators[op_no];
        if (op.is_applicable(state)) {
            //Rule S2 & S3
            add_conflicting_and_disabling(op_no, state);     // active operators used
            //Rule S4'
            std::vector<int>::iterator op_it;
            for (op_it = disabled[op_no].begin(); op_it != disabled[op_no].end(); op_it++) {
                if (active_ops[*op_it]) {
                    const Operator &o = g_operators[*op_it];

                    bool v_applicable_op_found = false;
                    std::vector<int>::iterator var_it;
                    get_disabled_vars(op_no, *op_it, disabled_vars);
                    if (!disabled_vars.empty()) {     // == can_disable(op1_no, op2_no)
                        for (var_it = disabled_vars.begin(); var_it != disabled_vars.end(); var_it++) {
                            //First case: add o'
                            if (is_v_applicable(*var_it, *op_it, state, v_precond)) {
                                mark_as_stubborn(*op_it, state);
                                v_applicable_op_found = true;
                                break;
                            }
                        }

                        //Second case: add a necessray enabling set for o' following S5
                        if (!v_applicable_op_found) {
                            apply_s5(o, state);
                        }
                    }
                }
            }
        } else {     // op is inapplicable
            //S5
            apply_s5(op, state);
        }
    }

    // Now check which applicable operators are in the stubborn set.
    vector<const Operator *> pruned_ops;
    pruned_ops.reserve(applicable_ops.size());
    for (size_t i = 0; i < applicable_ops.size(); ++i) {
        const Operator *op = applicable_ops[i];
        int op_no = get_op_index(op);
        if (stubborn_ec[op_no])
            pruned_ops.push_back(op);
    }
    if (pruned_ops.size() != applicable_ops.size()) {
        applicable_ops.swap(pruned_ops);
        sort(applicable_ops.begin(), applicable_ops.end());
    }
}
}
