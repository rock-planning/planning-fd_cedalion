#include "yahsp2.h"

#include "open_lists/open_list.h"
#include "search_space.h"
#include "search_progress.h"
#include "successor_generator.h"
#include "operator.h"
#include "option_parser.h"

using namespace std;

YAHSP2::YAHSP2(const Options &opts)
    : is_greedy(opts.get<bool>("la_greedy")),
      do_repair(opts.get<bool>("la_repair"))
{
}

YAHSP2::~YAHSP2()
{
}

State *YAHSP2::compute_node(const State &state,
                            SearchSpace &ssp,
                            OpenList<StateID> &open_list,
                            SearchProgress &search_progress,
                            Heuristic &heuristic,
                            int bound) {
    if (test_goal(state)) {
        return new State(state);
    }
    vector<const Operator *> relaxed_plan;
    heuristic.get_operator_bucket(relaxed_plan);
    vector<const Operator *> plan;
    StateID state_id = state.get_id();
    State lookahead_state = lookahead(plan, state, relaxed_plan);
    search_progress.inc_generated();
    StateID lookahead_state_id = lookahead_state.get_id();
    SearchNode lookahead_node = ssp.get_node(lookahead_state_id);

    int state_g = ssp.get_node(state_id).get_real_g();
    // lookahead_node.get_real_g() returns -1 if the search has not seen the state.
    int lookahead_g = state_g + calculate_plan_cost(plan);

    if (!lookahead_node.is_new() || lookahead_g >= bound) {
        return 0;
    }

    // code from eager_search.cc -->
    heuristic.evaluate(lookahead_state);
    lookahead_node.clear_h_dirty();
    search_progress.inc_evaluated_states();
    search_progress.inc_evaluations();
    // <--

    SearchNode node = ssp.get_node(state_id);
    open_list.evaluate(0, false); // g is not relevant for eager_greedy
    bool dead_end = open_list.is_dead_end();
    if (dead_end) {
        lookahead_node.mark_as_dead_end();
        search_progress.inc_dead_ends();
        return 0;
    }

    int h = heuristic.get_heuristic();

    if (!is_greedy) { // for shorter plans
        int special_h = node.get_length() + plan.size() + h;
        heuristic.set_evaluator_value(special_h);
        open_list.evaluate(0, false);
    }

    lookahead_node.open(h, node, plan);

    open_list.insert(lookahead_state_id);
    search_progress.check_h_progress(lookahead_node.get_g());

    return compute_node(lookahead_state, ssp, open_list, search_progress, heuristic, bound);
}

State YAHSP2::lookahead(vector<const Operator *> &plan, const State &state, vector<const Operator *> &relaxed_plan) {

    State s(state);
    int loop = true;
    while (loop) {
        loop = false;
        // determine a next applicable operator in the sorted list of operators
        vector<const Operator *>::iterator it;
        for (it = relaxed_plan.begin(); it != relaxed_plan.end(); it++) {
            const Operator *op = (*it);
            if (op->is_applicable(s)) {
                loop = true;
                s = g_state_registry->get_successor_state(s, *op);
                // TODO: Increase number of generated states?
                plan.push_back(op);
                relaxed_plan.erase(it);
                break;
            }
        }
        if (loop)
            continue;

        if (do_repair) {
            // determine a next applicable operator, which will replace one of the relaxed plan operator
            vector<const Operator *> applicable_ops_v;
            g_successor_generator->generate_applicable_ops(s, applicable_ops_v);
            // minimize costs
            multimap<int, const Operator *> applicable_ops;
            for (int i = 0; i < applicable_ops_v.size(); i++) {
                applicable_ops.insert(pair<int, const Operator *>(applicable_ops_v[i]->get_cost(), applicable_ops_v[i]));
            }

            vector<const Operator *>::iterator it_i = relaxed_plan.begin();
            vector<const Operator *>::iterator it_j = relaxed_plan.begin();
            while (!loop && it_i != relaxed_plan.end()) {
                while (!loop && it_j != relaxed_plan.end()) {
                    if (it_i != it_j) {
                        const Operator *op_i = (*it_i);
                        const Operator *op_j = (*it_j);
                        vector<const Operator *> ops_add;
                        ops_add.push_back(op_i);
                        vector<const Operator *> ops_pre;
                        ops_pre.push_back(op_j);
                        if (intersect(ops_add, ops_pre)) {
                            multimap<int, const Operator *>::iterator it_a;
                            for (it_a = applicable_ops.begin(); it_a != applicable_ops.end(); it_a++) {
                                const Operator *op_appl = (*it_a).second;
                                ops_add.push_back(op_appl);
                                if (intersect(ops_add, ops_pre)) {
                                    loop = true;
                                    relaxed_plan.erase(it_i);
                                    relaxed_plan.insert(it_i, op_appl);
                                    break;
                                }
                            }
                        }
                    }
                    it_j++;
                }
                it_i++;
            }
        }
    }

    return s;
}

void YAHSP2::trace_path(vector<const Operator *> &path, const State &state, SearchSpace &ssp) {
    StateID current_state_id = state.get_id();
    while (true) {
        const SearchNodeInfo &info = ssp.get_node(current_state_id).get_info();

        if (info.steps) { // add operator sequence to plan
            for (int i = info.steps->size() - 1; i >= 0; i--) {
                path.push_back((*info.steps)[i]);
            }
        } else { // add single operator to plan
            const Operator *op = info.creating_operator;
            if (op == 0) {
                assert(info.parent_state_id == StateID::no_state);
                break;
            }
            path.push_back(op);
        }
        current_state_id = info.parent_state_id;
    }
    reverse(path.begin(), path.end());
}

bool YAHSP2::intersect(vector<const Operator *> ops_add, vector<const Operator *> ops_pre) {
    vector<vector<int> > conditions;

    // effects (post-conditions)
    for (int i = 0; i < ops_add.size(); i++) {
        vector<int> vars(g_variable_name.size(), -1);
        const vector<PrePost> &pre_post = ops_add[i]->get_pre_post();
        for (int j = 0; j < pre_post.size(); j++) {
            vars[pre_post[j].var] = pre_post[j].post;
        }
        conditions.push_back(vars);
    }
    // conditions (pre-conditions)
    for (int i = 0; i < ops_pre.size(); i++) {
        vector<int> vars(g_variable_name.size(), -1);
        const vector<Prevail> &prevail = ops_pre[i]->get_prevail();
        for (int j = 0; j < prevail.size(); j++) {
            vars[prevail[j].var] = prevail[j].prev;
        }
        const vector<PrePost> &pre_post = ops_pre[i]->get_pre_post();
        for (int j = 0; j < pre_post.size(); j++) {
            vars[pre_post[j].var] = pre_post[j].pre;
        }
        conditions.push_back(vars);
    }


    for (int var_no = 0; var_no < g_variable_name.size(); var_no++) {
        int var0 = conditions[0][var_no];
        if (var0 == -1)
            continue;
        bool same_condition = true;
        for (int i = 1; i < conditions.size(); i++) {
           int var1 = conditions[i][var_no];
           if (var1 == -1 || var1 != var0) {
               same_condition = false;
               break;
           }
        }
        if (same_condition)
            return true;
    }

    return false;
}

void YAHSP2::add_options_to_parser(OptionParser &parser) {
    parser.add_option<bool>("lookahead",
                            "use lookahead as it is used in YAHSP2",
                            "false");
    parser.add_option<bool>("la_greedy",
                            "greedy option for lookahead (results in longer plans)",
                            "false");
    parser.add_option<bool>("la_repair",
                            "replace relaxed plan operators to get longer lookahead plans",
                            "true");
}

