#ifndef EAGER_SEARCH_H
#define EAGER_SEARCH_H

#include <vector>

#include "open_lists/open_list.h"
#include "search_engine.h"
#include "search_space.h"
#include "state.h"
#include "timer.h"
#include "evaluator.h"
#include "search_progress.h"

class Heuristic;
class Operator;
class ScalarEvaluator;
class Options;

namespace POR {
class PORMethod;
}

class YAHSP2;
class EagerSearch : public SearchEngine {
protected: // TODO: Remove this hack (added for POR integration)
    // Search Behavior parameters
    bool reopen_closed_nodes; // whether to reopen closed nodes upon finding lower g paths
    bool do_pathmax; // whether to use pathmax correction
    bool use_multi_path_dependence;

    OpenList<StateID> *open_list;
    ScalarEvaluator *f_evaluator;

    POR::PORMethod *partial_order_reduction_method;
protected:
    int step();
    std::pair<SearchNode, bool> fetch_next_node();
    bool check_goal(const SearchNode &node);
    void update_jump_statistic(const SearchNode &node);
    void print_heuristic_values(const std::vector<int> &values) const;
    void reward_progress();

    std::vector<Heuristic *> heuristics;
    std::vector<Heuristic *> preferred_operator_heuristics;
    std::vector<Heuristic *> estimate_heuristics;
    // TODO: in the long term this
    // should disappear into the open list

    YAHSP2 *yahsp2;
    virtual void initialize();

public:
    EagerSearch(const Options &opts);
    ~EagerSearch();
    void statistics() const;

    void dump_search_space();
};

#endif
