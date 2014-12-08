#ifndef INCREMENTAL_LM_CUT_HEURISTIC_H
#define INCREMENTAL_LM_CUT_HEURISTIC_H

#include "lm_cut_heuristic.h"
#include "state_id.h"
#include "search_space.h"
#include "memory_tracking_allocator.h"

#include <tr1/memory>
#include <ext/hash_map>

// wraps vector<RelaxedOperator *> to add costs
// adding or removing operators after creation does *not* update costs (not implemented)
class Landmark : public std::vector<RelaxedOperator *, MemoryTrackingAllocator<RelaxedOperator *> > {
public:
    int cost;
    Landmark(std::vector<RelaxedOperator *> &operators, int cost);
};

typedef std::tr1::shared_ptr<Landmark> LandmarkPtr;
typedef std::vector<LandmarkPtr, MemoryTrackingAllocator<LandmarkPtr> > SavedLandmarks;



typedef __gnu_cxx::hash_map<StateID,
                            SavedLandmarks,
                            __gnu_cxx::hash<StateID>,
                            std::equal_to<StateID>,
                            MemoryTrackingAllocator<std::pair<const StateID,
                                                              SavedLandmarks> > >
        SavedLandmarksMap;

template<class T1, class T2>
struct ComparePairByFirstComponent {
    bool operator()(std::pair<T1, T2> const &lhs, std::pair<T1, T2> const &rhs) {
        return lhs.first < rhs.first;
    }
};

class IncrementalLandmarkCutHeuristic : public LandmarkCutHeuristic {
private:
    bool local_incremental_computation;
    bool landmarks_belong_to_parent;
    bool keep_frontier;
    bool reevaluate_parent;
    unsigned long memory_limit;
    StateID current_state_id;
    int current_landmarks_cost;
    SavedLandmarks current_landmarks;
    StateID current_parent_state_id;
    SavedLandmarks current_parent_landmarks;
    SavedLandmarksMap saved_landmarks_map;
    void remove_meta_information(StateID &id) {
        saved_landmarks_map.erase(id);
    }
    bool compute_parent_landmarks(const State &parent_state);
    int cache_hits;
    unsigned int min_cache_hits;
protected:
    virtual void initialize();
    virtual bool reach_state(const State &parent_state, const Operator &op,
                             const State &state);
    virtual void finished_state(const State &state, int f, bool is_on_frontier);

    virtual void discovered_landmark(const State &state, std::vector<RelaxedOperator *> &landmark, int cost);
    virtual void reset_operator_costs(const State &state);
    virtual int compute_heuristic(const State &state);
public:
    IncrementalLandmarkCutHeuristic(const Options &opts);
    virtual ~IncrementalLandmarkCutHeuristic();
    virtual void free_up_memory(SearchSpace &search_space);
};


#endif
