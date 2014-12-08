#include "incremental_lm_cut_heuristic.h"

#include "globals.h"
#include "operator.h"
#include "option_parser.h"
#include "plugin.h"
#include "state.h"
#include "state_var_t.h"

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <vector>
using namespace std;

Landmark::Landmark(vector<RelaxedOperator *> &operators, int cost) :
    vector<RelaxedOperator *, MemoryTrackingAllocator<RelaxedOperator *> >(operators.begin(), operators.end()),
    cost(cost) {
}

// construction and destruction
IncrementalLandmarkCutHeuristic::IncrementalLandmarkCutHeuristic(const Options &opts)
    : LandmarkCutHeuristic(opts),
      local_incremental_computation(opts.get<bool>("local")),
      landmarks_belong_to_parent(false),
      keep_frontier(opts.get<bool>("keep_frontier")),
      reevaluate_parent(opts.get<bool>("reevaluate_parent")),
      memory_limit(opts.get<unsigned int>("memory_limit")),
      current_state_id(g_initial_state().get_id()),
      current_landmarks_cost(0),
      current_parent_state_id(g_initial_state().get_id()),
      cache_hits(0),
      min_cache_hits(opts.get<unsigned int>("min_cache_hits")) {
}

IncrementalLandmarkCutHeuristic::~IncrementalLandmarkCutHeuristic() {
}

void IncrementalLandmarkCutHeuristic::initialize() {
    LandmarkCutHeuristic::initialize();

    cout << "Landmarks for closed nodes on the search frontier will "
         << (keep_frontier ? "not " : "") << "be removed" << endl;

    cout << "For nodes without saved parent landmarks the parent node will "
         << (reevaluate_parent ? "" : "not ") << "be reevaluated" << endl;

    if (local_incremental_computation) {
        cout << "Incremental computation will only be used locally" << endl;
    }

    if (memory_limit == 0) {
        memory_limit = numeric_limits<unsigned long>::max();
        cout << "Using no memory limit" << endl;
    } else {
        // convert MB in byte
        cout << "Memory limit is " << memory_limit * 1024 * 1024 << " bytes (" << memory_limit << " MB). Meta information will be pruned if it takes up more space." << endl;
        memory_limit *= 1024 * 1024;
    }
}

void IncrementalLandmarkCutHeuristic::reset_operator_costs(const State &state) {
    // Redo cost changes of current landmarks (if there are no current landmarks,
    // this will just reset all costs to base costs)
    LandmarkCutHeuristic::reset_operator_costs(state);
    if (!landmarks_belong_to_parent) {
        assert(state.get_id() == current_state_id);
        for (SavedLandmarks::iterator it_landmark = current_landmarks.begin(); it_landmark != current_landmarks.end(); ++it_landmark) {
            LandmarkPtr landmark = *it_landmark;
            for (Landmark::iterator it_op = landmark->begin(); it_op != landmark->end(); ++it_op) {
                (*it_op)->cost -= landmark->cost;
            }
        }
    }
}

bool IncrementalLandmarkCutHeuristic::compute_parent_landmarks(const State &parent_state) {
    StateID new_parent_id = parent_state.get_id();
    StateID old_parent_id = current_parent_state_id;
    if (new_parent_id == old_parent_id) {
        return true;
    }
    current_parent_state_id = new_parent_id;

    if (local_incremental_computation) {
        current_parent_landmarks.clear();
    } else {
        saved_landmarks_map[old_parent_id].swap(current_parent_landmarks);
        assert(current_parent_landmarks.empty());
        SavedLandmarksMap::iterator parent_node_info_it = saved_landmarks_map.find(new_parent_id);
        if (parent_node_info_it != saved_landmarks_map.end()) {
            current_parent_landmarks.swap(parent_node_info_it->second);
            cache_hits++;
            return true;
        }
    }

    if (reevaluate_parent) {
        landmarks_belong_to_parent = true;
        compute_heuristic(parent_state);
        landmarks_belong_to_parent = false;
        return true;
    }
    return false;
}

bool IncrementalLandmarkCutHeuristic::reach_state(const State &parent_state, const Operator &op, const State &state) {
    current_state_id = state.get_id();
    current_landmarks_cost = 0;
    current_landmarks.clear();
    assert(local_incremental_computation || saved_landmarks_map[current_state_id].empty());
    bool h_dirty = false;

    if (!compute_parent_landmarks(parent_state)) {
        return h_dirty;
    }

    // Copy all landmarks that do not mention op
    for (size_t i = 0; i < current_parent_landmarks.size(); ++i) {
        LandmarkPtr landmark = current_parent_landmarks[i];
        bool copy_landmark = true;
        for (Landmark::iterator it_op = landmark->begin(); it_op != landmark->end(); ++it_op) {
            const Operator *base_operator = (*it_op)->op;
            if (base_operator == &op) {
                copy_landmark = false;
                h_dirty = true;
                break;
            }
        }
        if (copy_landmark) {
            current_landmarks.push_back(landmark);
            current_landmarks_cost += landmark->cost;
        }
    }
    return h_dirty;
}

void IncrementalLandmarkCutHeuristic::finished_state(const State &state, int /*f*/,
                                                     bool is_on_frontier) {
    StateID id = state.get_id();
    if (is_on_frontier && keep_frontier) {
        // TODO could also keep only a fixed amount of states (keep lower f values)
        return;
    } else {
        remove_meta_information(id);
    }
}

void IncrementalLandmarkCutHeuristic::discovered_landmark(const State &state, vector<RelaxedOperator *> &landmark, int cost) {
    ((void) state); // Avoid 'unused parameter' warning in release build
    LandmarkPtr createdLandmark(new Landmark(landmark, cost));
    if (landmarks_belong_to_parent) {
        assert(state.get_id() == current_parent_state_id);
        current_parent_landmarks.push_back(createdLandmark);
    } else {
        assert(state.get_id() == current_state_id);
        current_landmarks_cost += cost;
        current_landmarks.push_back(createdLandmark);
    }
}

int IncrementalLandmarkCutHeuristic::compute_heuristic(const State &state) {
    assert(state.get_id() == current_state_id);
    if (LandmarkCutHeuristic::compute_heuristic(state) == DEAD_END) {
        remove_meta_information(current_state_id);
        return DEAD_END;
    }
    if (!local_incremental_computation) {
        saved_landmarks_map[current_state_id].swap(current_landmarks);
        assert(current_landmarks.empty());
    }
    return current_landmarks_cost;
}

void IncrementalLandmarkCutHeuristic::free_up_memory(SearchSpace &/*search_space*/) {
    if (local_incremental_computation || g_memory_tracking_allocated < memory_limit) {
        return;
    }
    int n = saved_landmarks_map.size();
    saved_landmarks_map.clear();
    cout << "Cache hits since last reset: " << cache_hits << endl;
    if (cache_hits < min_cache_hits) {
        cout << "Switching to h^iLM-cut_local." << endl;
        local_incremental_computation = true;
        reevaluate_parent = true;
    } else {
        cout << "Freeing memory by deleting meta information for all " << n << " states." << endl;
    }
    cache_hits = 0;
}


static Heuristic *_parse(OptionParser &parser) {
    parser.add_option<unsigned int>("memory_limit",
                                    "Maximum amount of memory (in MB) used for storing meta information. Use 0 (default) for no limit.",
                                    "0");
    parser.add_option<bool>("keep_frontier",
                            "Landmarks for nodes on the search frontier are not removed if the node is closed.",
                            "false");
    parser.add_option<bool>("reevaluate_parent",
                            "If a node is encountered where the parent node has no saved landmarks, the heuristic for the parent is recomputed. This is based on the assumption that siblings of this node will be expanded soon and will profit from the calculated parent info",
                            "false");
    parser.add_option<bool>("local",
                            "Only use incremental computation locally, i.e. reevaluate the parent and then incrementally compute the child nodes. Automatically enables reevaluate_parent",
                            "false");
    parser.add_option<unsigned int>("min_cache_hits",
                                    "Switch from memory bounded computation to local incremental computation if there are not enough cache hits, i.e. states where the landmarks of the parent state are known. A value of 0 (default) never switches, a value of max int switches the first time memory is exhausted, a value of 2 switches when only the parent state is in the hash map.",
                                    "0");
    Heuristic::add_options_to_parser(parser);
    Options opts = parser.parse();
    if (parser.help_mode())
        return 0;

    if (opts.get<bool>("local")) {
        opts.set<bool>("reevaluate_parent", true);
    }

    if (parser.dry_run())
        return 0;
    else
        return new IncrementalLandmarkCutHeuristic(opts);
}


static Plugin<Heuristic> _plugin("incremental_lmcut", _parse);
