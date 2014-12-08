#ifndef POR_EXPANSION_CORE_H
#define POR_EXPANSION_CORE_H

#include "por_method.h"

#include <vector>

class Operator;
class State;

namespace POR {
struct ExpansionCoreDTG {
    // Note: this class could be made private to ExpansionCore.
    struct Arc {
        Arc(int target_value_, int operator_no_)
            : target_value(target_value_),
              operator_no(operator_no_) {
        }
        int target_value;
        int operator_no;
    };

    struct Node {
        std::vector<Arc> outgoing;
        std::vector<Arc> incoming;
    };

    std::vector<Node> nodes;
    std::vector<bool> goal_values;

    void recurse_forwards(int value, std::vector<bool> &reachable) const;
    void recurse_backwards(int value, std::vector<bool> &relevant) const;
public:
    ExpansionCoreDTG();
    ~ExpansionCoreDTG();

    void forward_reachability_analysis(int start_value,
                                       std::vector<bool> &reachable) const;
    void backward_relevance_analysis(std::vector<bool> &relevant) const;
};


class ExpansionCore : public PORMethodWithStatistics {
    std::vector<ExpansionCoreDTG> dtgs;
    std::vector<std::vector<bool> > co_effects_arcs;
    std::vector<std::vector<bool> > pdg_arcs;

    void build_dtgs();
    void mark_co_effects();
    void mark_potential_preconditions(const State &state);
    void mark_potential_dependents(const State &state);
    void mark_pdg_arc(int from_var, int to_var);
    void compute_dependency_closure(
        const State &state, std::vector<bool> &dependency_closure) const;
    void recurse_dependency_closure(
        int var, std::vector<bool> &closure) const;

protected:
    virtual void do_pruning(const State &state,
                            std::vector<const Operator *> &ops);
public:
    ExpansionCore();
    ~ExpansionCore();

    virtual void dump_options() const;
};
}

#endif
