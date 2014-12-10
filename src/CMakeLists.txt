include_directories(${CMAKE_CURRENT_SOURCE_DIR}/search/ext)
rock_library(${PROJECT_NAME}-search
    SOURCES 
        search/additive_heuristic.cc 
        search/axioms.cc 
        search/blind_search_heuristic.cc 
        search/causal_graph.cc 
        search/cea_heuristic.cc 
        search/cg_cache.cc 
        search/cg_heuristic.cc 
        search/combining_evaluator.cc 
        search/conditional_effect_incremental_lm_cut_heuristic.cc 
        search/conditional_effect_lm_cut_heuristic.cc 
        search/domain_transition_graph.cc 
        search/eager_search.cc 
        search/enforced_hill_climbing_search.cc 
        search/exact_timer.cc 
        search/ff_heuristic.cc 
        search/g_evaluator.cc 
        search/globals.cc 
        search/goal_count_heuristic.cc 
        search/heuristic.cc 
        search/hm_heuristic.cc 
        search/incremental_lm_cut_heuristic.cc 
        search/ipc_max_heuristic.cc 
        search/iterated_search.cc 
        search/lazy_search.cc 
        search/legacy_causal_graph.cc 
        search/lm_cut_heuristic.cc 
        search/max_evaluator.cc 
        search/max_heuristic.cc 
        search/memory_tracking_allocator.cc 
        search/operator.cc 
        search/operator_cost.cc 
        search/option_parser.cc 
        search/option_parser_util.cc 
        search/per_state_information.cc 
        search/planner.cc 
        search/pref_evaluator.cc 
        search/relaxation_heuristic.cc 
        search/rng.cc 
        search/search_engine.cc 
        search/search_node_info.cc 
        search/search_progress.cc 
        search/search_space.cc 
        search/segmented_vector.cc 
        search/state.cc 
        search/state_id.cc 
        search/state_registry.cc 
        search/successor_generator.cc 
        search/sum_evaluator.cc 
        search/timer.cc 
        search/utilities.cc 
        search/weighted_evaluator.cc 
        search/yahsp2.cc
        # Landmarks
        search/landmarks/exploration.cc
        search/landmarks/h_m_landmarks.cc
        search/landmarks/lama_ff_synergy.cc
        search/landmarks/landmark_cost_assignment.cc
        search/landmarks/landmark_count_heuristic.cc
        search/landmarks/landmark_factory.cc
        search/landmarks/landmark_factory_rpg_exhaust.cc
        search/landmarks/landmark_factory_rpg_sasp.cc
        search/landmarks/landmark_factory_zhu_givan.cc
        search/landmarks/landmark_graph.cc
        search/landmarks/landmark_graph_merged.cc
        search/landmarks/landmark_status_manager.cc
        search/landmarks/util.cc
        # Learning
        #search/learning/AODE.cc
        #search/learning/classifier.cc
        #search/learning/composite_feature_extractor.cc
        #search/learning/feature_extractor.cc
        #search/learning/maximum_heuristic.cc
        #search/learning/naive_bayes_classifier.cc
        #search/learning/PDB_state_space_sample.cc
        #search/learning/probe_state_space_sample.cc
        #search/learning/selective_max_heuristic.cc
        #search/learning/state_space_sample.cc
        #search/learning/state_vars_feature_extractor.cc
        # Merge and Shrink
        search/merge_and_shrink/abstraction.cc
        search/merge_and_shrink/shrink_fh.cc
        search/merge_and_shrink/label_reducer.cc
        search/merge_and_shrink/shrink_random.cc
        search/merge_and_shrink/merge_and_shrink_heuristic.cc
        search/merge_and_shrink/shrink_strategy.cc
        search/merge_and_shrink/shrink_bisimulation.cc
        search/merge_and_shrink/variable_order_finder.cc
        search/merge_and_shrink/shrink_bucket_based.cc
        # Open lists
        search/open_lists/alternation_open_list.cc
        search/open_lists/open_list_buckets.cc
        search/open_lists/pareto_open_list.cc
        search/open_lists/standard_scalar_open_list.cc
        search/open_lists/tiebreaking_open_list.cc
        # PDBS 
        search/pdbs/canonical_pdbs_heuristic.cc
        search/pdbs/dominance_pruner.cc
        search/pdbs/match_tree.cc
        search/pdbs/max_cliques.cc
        search/pdbs/pattern_generation_edelkamp.cc
        search/pdbs/pattern_generation_haslum.cc
        search/pdbs/pdb_heuristic.cc
        search/pdbs/util.cc
        search/pdbs/zero_one_pdbs_heuristic.cc
        # Cegar
        search/cegar/abstraction.cc
        search/cegar/abstract_state.cc
        search/cegar/cegar_heuristic.cc
        search/cegar/split_tree.cc
        search/cegar/task.cc
        search/cegar/utils.cc
        search/cegar/values.cc
        # Por
        search/por/expansion_core.cc
        search/por/por_method.cc
        search/por/simple_stubborn_sets.cc
        search/por/sss_expansion_core.cc
    HEADERS  
        search/additive_heuristic.h 
        search/axioms.h 
        search/blind_search_heuristic.h 
        search/causal_graph.h 
        search/cea_heuristic.h 
        search/cg_cache.h 
        search/cg_heuristic.h 
        search/combining_evaluator.h 
        search/conditional_effect_incremental_lm_cut_heuristic.h 
        search/conditional_effect_lm_cut_heuristic.h 
        search/doc.h 
        search/domain_transition_graph.h 
        search/eager_search.h 
        search/enforced_hill_climbing_search.h 
        search/evaluator.h 
        search/exact_timer.h 
        search/ff_heuristic.h 
        search/g_evaluator.h 
        search/globals.h 
        search/goal_count_heuristic.h 
        search/heuristic.h 
        search/hm_heuristic.h 
        search/incremental_lm_cut_heuristic.h 
        search/ipc_max_heuristic.h 
        search/iterated_search.h 
        search/lazy_search.h 
        search/legacy_causal_graph.h 
        search/lm_cut_heuristic.h 
        search/max_evaluator.h 
        search/max_heuristic.h 
        search/memory_tracking_allocator.h 
        search/operator_cost.h 
        search/operator.h 
        search/option_parser.h 
        search/option_parser_util.h 
        search/per_state_information.h 
        search/plugin.h 
        search/pref_evaluator.h 
        search/priority_queue.h 
        search/relaxation_heuristic.h 
        search/rng.h 
        search/scalar_evaluator.h 
        search/search_engine.h 
        search/search_node_info.h 
        search/search_progress.h 
        search/search_space.h 
        search/segmented_vector.h 
        search/state.h 
        search/state_id.h 
        search/state_registry.h 
        search/state_var_t.h 
        search/successor_generator.h 
        search/sum_evaluator.h 
        search/timer.h 
        search/utilities.h 
        search/weighted_evaluator.h 
        search/yahsp2.h
        # Landmarks
        search/landmarks/exploration.h
        search/landmarks/h_m_landmarks.h
        search/landmarks/lama_ff_synergy.h
        search/landmarks/landmark_cost_assignment.h
        search/landmarks/landmark_count_heuristic.h
        search/landmarks/landmark_factory.h
        search/landmarks/landmark_factory_rpg_exhaust.h
        search/landmarks/landmark_factory_rpg_sasp.h
        search/landmarks/landmark_factory_zhu_givan.h
        search/landmarks/landmark_graph.h
        search/landmarks/landmark_graph_merged.h
        search/landmarks/landmark_status_manager.h
        search/landmarks/landmark_types.h
        search/landmarks/util.h
        # Learning
        #search/learning/AODE.h
        #search/learning/classifier.h
        #search/learning/composite_feature_extractor.h
        #search/learning/feature_extractor.h
        #search/learning/maximum_heuristic.h
        #search/learning/naive_bayes_classifier.h
        #search/learning/PDB_state_space_sample.h
        #search/learning/probe_state_space_sample.h
        #search/learning/selective_max_heuristic.h
        #search/learning/state_space_sample.h
        #search/learning/state_vars_feature_extractor.h
        # Merge and Shrink
        search/merge_and_shrink/abstraction.h
        search/merge_and_shrink/shrink_fh.h
        search/merge_and_shrink/label_reducer.h
        search/merge_and_shrink/shrink_random.h
        search/merge_and_shrink/merge_and_shrink_heuristic.h
        search/merge_and_shrink/shrink_strategy.h
        search/merge_and_shrink/shrink_bisimulation.h
        search/merge_and_shrink/variable_order_finder.h
        search/merge_and_shrink/shrink_bucket_based.h
        # Open lists
        search/open_lists/alternation_open_list.h
        search/open_lists/open_list_buckets.h
        search/open_lists/open_list.h
        search/open_lists/pareto_open_list.h
        search/open_lists/standard_scalar_open_list.h
        search/open_lists/tiebreaking_open_list.h
        # PDBS
        search/pdbs/canonical_pdbs_heuristic.h
        search/pdbs/dominance_pruner.h
        search/pdbs/match_tree.h
        search/pdbs/max_cliques.h
        search/pdbs/pattern_generation_edelkamp.h
        search/pdbs/pattern_generation_haslum.h
        search/pdbs/pdb_heuristic.h
        search/pdbs/util.h
        search/pdbs/zero_one_pdbs_heuristic.h
        # Cegar
        search/cegar/abstraction.h
        search/cegar/abstract_state.h
        search/cegar/cegar_heuristic.h
        search/cegar/split_tree.h
        search/cegar/task.h
        search/cegar/utils.h
        search/cegar/values.h
        # Por
        search/por/expansion_core.h
        search/por/por_method.h
        search/por/simple_stubborn_sets.h
        search/por/sss_expansion_core.h
    DEPS_PKGCONFIG coinutils clp osi-clp
)

rock_executable(${PROJECT_NAME}-planner-search
        search/planner.cc
        DEPS ${PROJECT_NAME}-search
        NOINSTALL)
        
rock_library(${PROJECT_NAME}-preprocess
    SOURCES
        preprocess/axiom.cc
        preprocess/causal_graph.cc
        preprocess/domain_transition_graph.cc
        preprocess/helper_functions.cc
        preprocess/max_dag.cc
        preprocess/mutex_group.cc
        preprocess/operator.cc
        preprocess/planner.cc
        preprocess/scc.cc
        preprocess/state.cc
        preprocess/successor_generator.cc
        preprocess/variable.cc
    HEADERS
        preprocess/axiom.h
        preprocess/causal_graph.h
        preprocess/domain_transition_graph.h
        preprocess/helper_functions.h
        preprocess/max_dag.h
        preprocess/mutex_group.h
        preprocess/operator.h
        preprocess/scc.h
        preprocess/state.h
        preprocess/successor_generator.h
        preprocess/variable.h
)
rock_executable(${PROJECT_NAME}-planner-preprocess
        preprocess/planner.cc
        DEPS ${PROJECT_NAME}-preprocess
        NOINSTALL)

# Installation of the driver directory
# Python wrapper to facilitate running the system
install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E copy_directory
\"${CMAKE_CURRENT_SOURCE_DIR}/translate\"
\"${CMAKE_INSTALL_PREFIX}/bin/${PROJECT_NAME}/translate\") ")

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/search/ DESTINATION ${CMAKE_INSTALL_PREFIX}/bin/${PROJECT_NAME}/search/ 
          FILES_MATCHING PATTERN "*.py")

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-planner-preprocess
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
    DESTINATION bin/${PROJECT_NAME}/preprocess
    RENAME preprocess)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-planner-search
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
    DESTINATION bin/${PROJECT_NAME}/search
    RENAME downward-1)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/search/downward
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
    DESTINATION bin/${PROJECT_NAME}/search)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/search/dispatch
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
    DESTINATION bin/${PROJECT_NAME}/search)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/search/unitcost
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
    DESTINATION bin/${PROJECT_NAME}/search)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/plan-ipc
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
    DESTINATION bin
    RENAME cedalion-planner-ipc)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/plan
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
    DESTINATION bin
    RENAME cedalion-planner)
