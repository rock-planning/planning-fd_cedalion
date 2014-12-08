#ifndef POR_SSS_EXPANSION_CORE_H
#define POR_SSS_EXPANSION_CORE_H

#include "por_method.h"
#include "expansion_core.h"
#include <vector>

class Operator;
class State;
class Options;

namespace POR {
struct FactEC {
    int var;
    int val;
    FactEC(int v, int d) : var(v), val(d) {}
};

inline bool operator<(const FactEC &lhs, const FactEC &rhs) {
    return lhs.var < rhs.var;
}

class SSS_ExpansionCore : public PORMethodWithStatistics {
private:
    std::vector<ExpansionCoreDTG> dtgs;
    std::vector<std::vector<std::vector<bool> > > reachability_map;
    std::vector<std::vector<int> > v_precond;
    std::vector<bool> stubborn_ec;
    std::vector<bool> active_ops;
    std::vector<int> stubborn_ec_queue;
    std::vector<std::vector<std::vector<int> > > achievers;
    std::vector<std::vector<int> > conflicting_and_disabling;
    std::vector<std::vector<int> > disabled;
    std::vector<bool> written_vars;
    std::vector<std::vector<bool> > nes_computed;
    
    const MutexOption mutex_option;
    bool use_active_ops;
    
    void build_dtgs();
    void build_reachability_map();
    void compute_v_precond();
    void compute_achievers();
    void compute_conflicts_and_disabling();
    void compute_disabled_by_o();
    void add_conflicting_and_disabling(int op_no, const State &state);
    void compute_sorted_operators();
    void recurse_forwards(int var, int start_value, int current_value, std::vector<bool> &reachable);
    void compute_active_operators(const State &state);
    void mark_as_stubborn(int op_no, const State &state);
    void add_nes_for_fact(std::pair<int, int> fact, const State &state);
    void apply_s5(const Operator &op, const State &state);
protected:
    void do_pruning(const State &state, std::vector<const Operator *> &ops);
public:
    SSS_ExpansionCore(const Options &opts);
    ~SSS_ExpansionCore();
    virtual void dump_options() const;
};
}
#endif
