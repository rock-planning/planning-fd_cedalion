#ifndef CONDITIONAL_EFFECT_LM_CUT_HEURISTIC_H
#define CONDITIONAL_EFFECT_LM_CUT_HEURISTIC_H

#include "heuristic.h"
#include "priority_queue.h"

#include <algorithm>
#include <cassert>
#include <vector>

// TODO: Fix duplication with the other relaxation heuristics.

class Operator;
class State;

class CERelaxedProposition;
class CERelaxedOperator;

class Options;
/* TODO: Check the impact of using unary relaxed operators instead of
   multi-effect ones.

   Pros:
   * potentially simpler code
   * easy integration of conditional effects

   Cons:
   * potentially worse landmark extraction because unary operators
     for the same SAS+ operator are not tied to each other, and hence
     might choose different hmax supporters, leading to unnecessarily
     many paths in the justification graph. (However, if we use a
     systematic method for choosing hmax supporters, this might be
     a non-issue.)

   It's unclear which approach would be faster. Compiling to unary
   operators should be faster in unary domains because we lose the
   vector overhead. In non-unary domains, we have less overhead, but
   more operators.
*/

enum CEPropositionStatus {
    UNREACHED = 0,
    REACHED = 1,
    GOAL_ZONE = 2,
    BEFORE_GOAL_ZONE = 3
};

const int COST_MULTIPLIER = 1;
// Choose 1 for maximum speed, larger values for possibly better
// heuristic accuracy. Heuristic computation time should increase
// roughly linearly with the multiplier.

/* TODO: In some very preliminary tests in the IPC-2008 Elevators
   domain (more precisely, on Elevators-01), a larger cost multiplier
   (I tried 10) reduced expansion count quite a bit, although not
   enough to balance the extra runtime. Worth experimenting a bit to
   see the effect, though.
 */

struct CERelaxedOperatorGroup {
    const Operator *op;
    int base_cost;
    int cost : 31;
    bool marked : 1;
    // NOTE: Mixed type bit fields are not guarantueed to be compact in memory.
    //       See the comment for SearchNodeInfo.
    std::vector<CERelaxedOperator> relaxed_operators;

    size_t size() const {
        return relaxed_operators.size();
    }

    CERelaxedOperator &operator[](size_t i) {
        return relaxed_operators[i];
    }

    const CERelaxedOperator &operator[](size_t i) const {
        return relaxed_operators[i];
    }

    CERelaxedOperatorGroup(const Operator *op_, int base_cost_)
        : op(op_), base_cost(base_cost_), marked(false) {
    }
};

struct CERelaxedOperator {
    CERelaxedOperatorGroup *group;
    std::vector<CERelaxedProposition *> precondition;
    std::vector<CERelaxedProposition *> effects;

    int unsatisfied_preconditions;
    int h_max_supporter_cost; // h_max_cost of h_max_supporter
    CERelaxedProposition *h_max_supporter;
    CERelaxedOperator(const std::vector<CERelaxedProposition *> &pre,
                    const std::vector<CERelaxedProposition *> &eff,
                    CERelaxedOperatorGroup *group_)
        : group(group_), precondition(pre), effects(eff) {
    }

    inline void update_h_max_supporter();
};

struct CERelaxedProposition {
    std::vector<CERelaxedOperator *> precondition_of;
    std::vector<CERelaxedOperator *> effect_of;

    CEPropositionStatus status;
    int h_max_cost;
    /* TODO: Also add the rpg depth? The Python implementation used
       this for tie breaking, and it led to better landmark extraction
       than just using the cost. However, the Python implementation
       used a heap for the priority queue whereas we use a bucket
       implementation [NOTE: no longer true], which automatically gets
       a lot of tie-breaking by depth anyway (although not complete
       tie-breaking on depth -- if we add a proposition from
       cost/depth (4, 9) with (+1,+1), we'll process it before one
       which is added from cost/depth (5,5) with (+0,+1). The
       disadvantage of using depth is that we would need a more
       complicated open queue implementation -- however, in the unit
       action cost case, we might exploit that we never need to keep
       more than the current and next cost layer in memory, and simply
       use two bucket vectors (for two costs, and arbitrarily many
       depths). See if the init h values degrade compared to Python
       without explicit depth tie-breaking, then decide.
    */

    CERelaxedProposition() {
    }
};

class ConditionalEffectLandmarkCutHeuristic : public Heuristic {
    AdaptiveQueue<CERelaxedProposition *> priority_queue;

    void build_relaxed_operator(const Operator &op,
                                CERelaxedOperatorGroup &group);
    void add_relaxed_operator(const std::vector<CERelaxedProposition *> &precondition,
                              const std::vector<CERelaxedProposition *> &effects,
                              CERelaxedOperatorGroup &group);
    void setup_exploration_queue();
    void setup_exploration_queue_state(const State &state);
    void first_exploration(const State &state);
    void first_exploration_incremental(std::vector<CERelaxedOperator *> &cut);
    void second_exploration(const State &state, std::vector<CERelaxedProposition *> &queue,
                            std::vector<CERelaxedOperator *> &cut);

    void enqueue_if_necessary(CERelaxedProposition *prop, int cost) {
        assert(cost >= 0);
        if (prop->status == UNREACHED || prop->h_max_cost > cost) {
            prop->status = REACHED;
            prop->h_max_cost = cost;
            priority_queue.push(cost, prop);
        }
    }

    void mark_goal_plateau(CERelaxedProposition *subgoal);
    void validate_h_max() const;
    void reduce_operator_costs(const std::vector<CERelaxedOperator *> &cut, int cut_cost);
protected:
    // CERelaxedOperators are grouped by the Operator that induced them
    std::vector<CERelaxedOperatorGroup> relaxed_operator_groups;
    std::vector<std::vector<CERelaxedProposition> > propositions;
    CERelaxedProposition artificial_precondition;
    CERelaxedProposition artificial_goal;
    int num_propositions;
    virtual void initialize();
    virtual void discovered_landmark(const State & /*state*/, std::vector<CERelaxedOperator *> & /*landmark*/, int /*cost*/) {
    }
    virtual void reset_operator_costs(const State &state);
    virtual int compute_heuristic(const State &state);
public:
    ConditionalEffectLandmarkCutHeuristic(const Options &opts);
    virtual ~ConditionalEffectLandmarkCutHeuristic();
};

inline void CERelaxedOperator::update_h_max_supporter() {
    assert(!unsatisfied_preconditions);
    for (int i = 0; i < precondition.size(); i++)
        if (precondition[i]->h_max_cost > h_max_supporter->h_max_cost)
            h_max_supporter = precondition[i];
    h_max_supporter_cost = h_max_supporter->h_max_cost;
}

#endif
