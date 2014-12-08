#ifndef YAHSP2_H
#define YAHSP2_H

#include <vector>

#include "state_id.h"
#include "state_var_t.h"

class State;
class SearchSpace;
class Operator;
class SearchProgress;
template<class Entry> class OpenList;
class Heuristic;
class OptionParser;
class Options;

class YAHSP2 {
public:
    YAHSP2(const Options &opts);
    ~YAHSP2();
    State *compute_node(const State &state, SearchSpace &ssp,
                        OpenList<StateID> &open_list,
                        SearchProgress &search_progress,
                        Heuristic &heuristic,
                        int bound);

    State lookahead(std::vector<const Operator *> &plan,
                   const State &state, std::vector<const Operator *> &relaxed_plan);
    void trace_path(std::vector<const Operator *> &plan, const State &state, SearchSpace &ssp);
    static void add_options_to_parser(OptionParser &parser);
    bool lookahead_is_greedy() const {return is_greedy; }

private:
    const bool is_greedy;
    const bool do_repair;
    bool intersect(std::vector<const Operator *> ops_add, std::vector<const Operator *> ops_pre);
};

#endif // YAHSP2_H
