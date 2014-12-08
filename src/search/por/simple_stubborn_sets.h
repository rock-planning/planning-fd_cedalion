#ifndef POR_SIMPLE_STUBBORN_SETS_H
#define POR_SIMPLE_STUBBORN_SETS_H

#include "por_method.h"
#include "../globals.h"
#include "../operator.h"
#include "../successor_generator.h"
#include "expansion_core.h" // HACK

#include <vector>
#include <map>
#include <cassert>
#include <algorithm>
using namespace std;

class Operator;
class State;
class Options;

namespace POR {
struct Fact {
    int var;
    int val;
    Fact(int v, int d) : var(v), val(d) {}
};

inline bool operator<(const Fact &lhs, const Fact &rhs) {
    return lhs.var < rhs.var;
}
////////////////////////////////////////////

enum PreconditionChoice {
    forward,
    backward, 
    small_static,
    small_dynamic,
    laarman_heuristic,
    most_hitting,
    c_forw_small,
    minimize_global_var_ordering,
    random_dynamic,
    minimize_random_global_var_ordering
};


class SimpleStubbornSets : public PORMethodWithStatistics {
    // achievers[var][value] contains all operator indices of
    // operators that achieve the fact (var, value).
    std::vector<std::vector<std::vector<int> > > achievers;

    // interference_relation[op1_no] contains all operator indices
    // of operators that interfere with op1.
    std::vector<std::vector<int> > interference_relation;

    // stubborn[op_no] is true iff the operator with operator index
    // op_no is contained in the stubborn set
    std::vector<bool> stubborn;

    // stubborn_queue contains the operator indices of operators that
    // have been marked and stubborn but have not yet been processed
    // (i.e. more operators might need to be added to stubborn because
    // of the operators in the queue).
    std::vector<int> stubborn_queue;

    std::vector<POR::ExpansionCoreDTG> dtgs;
    std::vector<std::vector<std::vector<bool> > > reachability_map;
    std::vector<bool> active_ops;
    std::map<int, int> global_variable_ordering;

    const PreconditionChoice precond_choice;
    const MutexOption mutex_option;
    bool use_active_ops;
    int laarman_k; // K parameter for laarman heuristic for penalizing
		   // applicable ops

    std::map<const Operator*, std::vector<std::pair<int, int> > > ordering_map;
    std::vector<std::pair<int, int> > goal_ordering;

    std::map<const Operator*, int> last_variables;
    
    void mark_as_stubborn(int op_no);
    void add_nes_for_fact(std::pair<int, int> fact);
    void add_interfering(int op_no);

    void compute_interference_relation();
    void compute_achievers();
    void compute_sorted_operators();
    void build_dtgs();
    void build_reachability_map();
    
    void recurse_forwards(int var, int start_value, int current_value, std::vector<bool> &reachable);

    void compute_active_operators(const State &state);

    pair<int, int> find_unsatisfied_goal(const State& state);
    pair<int, int> find_unsatisfied_precondition(const Operator &op, const State &state);
    
    pair<int, int> find_unsatisfied_goal_small_static(const State &state);
    pair<int, int> find_unsatisfied_goal_small_dynamic(const State &state);
    pair<int, int> find_unsatisfied_goal_most_hitting(const State &state);
    pair<int, int> find_unsatisfied_goal_global_variable_ordering(const State& state);
    pair<int, int> find_unsatisfied_goal_random_dynamic(const State &state);
	
    pair<int, int> find_unsatisfied_precondition_dynamic(const Operator &op, const State &state);
    pair<int, int> find_unsatisfied_precondition_small_static(const Operator &op, const State &state);
    pair<int, int> find_unsatisfied_precondition_small_dynamic(const State &state, 
							      const std::vector<std::pair<int, int> >& precondition_facts);
    pair<int, int> find_unsatisfied_precondition_laarman(const State &state, 
							 const std::vector<std::pair<int, int> >& precondition_facts);
    pair<int, int> find_unsatisfied_precondition_most_hitting(const State &state,
							      const vector<pair<int, int> >& precondition_facts);
    pair<int, int> find_unsatisfied_precondition_forward_small(const Operator &op, const State &state);
    pair<int, int> find_unsatisfied_precondition_global_variable_ordering(const Operator& op, const State& state);
    pair<int, int> find_unsatisfied_precondition_random_dynamic(const State &state, 
                                                                const std::vector<std::pair<int, int> >& precondition_facts);
    
    void compute_static_precondition_orderings();
    void compute_static_goal_ordering();

    void compute_static_variable_ordering();
    void compute_random_static_variable_ordering();

    inline bool interfere(int op1_no, int op2_no);

    void dump_precondition_choice(); 
    void dump_mutex_option();
    void dump_active_ops_option();
    
protected:
    virtual void do_pruning(const State &state,
                            std::vector<const Operator *> &ops);
public:
    SimpleStubbornSets(const Options &opts);
    ~SimpleStubbornSets();

    virtual void dump_options() const;
};
}



#endif
