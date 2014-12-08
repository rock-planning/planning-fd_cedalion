#! /usr/bin/env python
# -*- coding: utf-8 -*-

import sys

import portfolio
import requirements

ARGS = sys.argv[1:]

if not ARGS or not ARGS[0].startswith("downward.tmp."):
    print "Error: First argument to portfolio must be the path to the SAS-file: %s" % ARGS

SAS_FILE = ARGS[0]

ADL_CONFIGS = [
    [
        1,
        [
            "--heuristic",
            "hBlind=blind()",
            "--heuristic",
            "hcond_eff_incremental_lmcut=cond_eff_incremental_lmcut(local=false,memory_limit=46,keep_frontier=false,reevaluate_parent=false,min_cache_hits=4385)",
            "--heuristic",
            "hCombinedMax=max([hBlind,hcond_eff_incremental_lmcut])",
            "--search",
            "astar(hCombinedMax,mpd=false,pathmax=true,cost_type=0)"
        ]
    ],
    [
        99,
        [
            "--heuristic",
            "hMas=merge_and_shrink(reduce_labels=true,merge_strategy=MERGE_LINEAR_GOAL_CG_LEVEL,shrink_strategy=shrink_bisimulation(max_states=731,max_states_before_merge=-1,greedy=true,threshold=329,group_by_h=true,at_limit=RETURN))",
            "--heuristic",
            "hHMax=hmax()",
            "--heuristic",
            "hCombinedMax=max([hMas,hHMax])",
            "--search",
            "astar(hCombinedMax,mpd=false,pathmax=false,cost_type=0)"
        ]
    ],
    [
        38,
        [
            "--heuristic",
            "hcond_eff_incremental_lmcut=cond_eff_incremental_lmcut(local=false,memory_limit=94,keep_frontier=false,reevaluate_parent=false,min_cache_hits=9175)",
            "--heuristic",
            "hHMax=hmax()",
            "--heuristic",
            "hCombinedMax=max([hcond_eff_incremental_lmcut,hHMax])",
            "--search",
            "astar(hCombinedMax,mpd=false,pathmax=false,cost_type=0)"
        ]
    ],
    [
        463,
        [
            "--heuristic",
            "hcond_eff_incremental_lmcut=cond_eff_incremental_lmcut(local=false,memory_limit=36,keep_frontier=true,reevaluate_parent=false,min_cache_hits=9943)",
            "--search",
            "astar(hcond_eff_incremental_lmcut,mpd=false,pathmax=true,cost_type=0)"
        ]
    ]
]

STRIPS_CONFIGS = [
    [
        1,
        [
            "--heuristic",
            "hincremental_lmcut=incremental_lmcut(local=false,memory_limit=1000,keep_frontier=false,reevaluate_parent=true,min_cache_hits=4294967295)",
            "--heuristic",
            "hcpdbs=cpdbs()",
            "--heuristic",
            "hCombinedMax=max([hcpdbs,hincremental_lmcut])",
            "--search",
            "astar(hCombinedMax,partial_order_reduction=sss_expansion_core(active_ops=false,mutexes=none),mpd=false,pathmax=false,cost_type=0)"
        ]
    ],
    [
        538,
        [
            "--landmarks",
            "lmg=lm_zg(only_causal_landmarks=false,disjunctive_landmarks=true,conjunctive_landmarks=true,no_orders=true)",
            "--heuristic",
            "hincremental_lmcut=incremental_lmcut(local=false,memory_limit=200,keep_frontier=false,reevaluate_parent=true,min_cache_hits=4294967295)",
            "--heuristic",
            "hLMCut=lmcut()",
            "--heuristic",
            "hipdb=ipdb(pdb_max_size=87443270,collection_max_size=182173479,num_samples=94,min_improvement=7,max_time=48)",
            "--heuristic",
            "hLM=lmcount(lmg,admissible=true)",
            "--heuristic",
            "hCombinedMax=max([hipdb,hLM,hLMCut,hincremental_lmcut])",
            "--search",
            "astar(hCombinedMax,mpd=false,pathmax=true,cost_type=0)"
        ]
    ],
    [
        338,
        [
            "--heuristic",
            "hHMax=hmax()",
            "--heuristic",
            "hCegar=cegar(max_states=6072054,max_time=74,pick=max_constrained,fact_order=original,decomposition=goal_leaves,max_abstractions=5143)",
            "--heuristic",
            "hpdb=pdb(max_states=88665)",
            "--heuristic",
            "hincremental_lmcut=incremental_lmcut(local=false,memory_limit=100,keep_frontier=true,reevaluate_parent=true,min_cache_hits=4294967295)",
            "--heuristic",
            "hLMCut=lmcut()",
            "--heuristic",
            "hCombinedMax=max([hCegar,hpdb,hLMCut,hincremental_lmcut,hHMax])",
            "--search",
            "astar(hCombinedMax,partial_order_reduction=small_stubborn_sets(active_ops=false,precond_choice=minimize_global_var_ordering,mutexes=fd),mpd=false,pathmax=true,cost_type=0)"
        ]
    ],
    [
        340,
        [
            "--heuristic",
            "hBlind=blind()",
            "--heuristic",
            "hpdb=pdb(max_states=61783637)",
            "--heuristic",
            "hCegar=cegar(max_states=1052487,max_time=289,pick=max_constrained,fact_order=hadd_down,decomposition=goal_leaves,max_abstractions=6563)",
            "--heuristic",
            "hCombinedMax=max([hCegar,hpdb,hBlind])",
            "--search",
            "astar(hCombinedMax,mpd=false,pathmax=false,cost_type=0)"
        ]
    ],
    [
        392,
        [
            "--landmarks",
            "lmg=lm_rhw(only_causal_landmarks=false,disjunctive_landmarks=true,conjunctive_landmarks=true,no_orders=false)",
            "--heuristic",
            "hMas=merge_and_shrink(reduce_labels=true,merge_strategy=MERGE_LINEAR_REVERSE_LEVEL,shrink_strategy=shrink_fh(max_states=10188,max_states_before_merge=-1,shrink_f=HIGH,shrink_h=HIGH))",
            "--heuristic",
            "hpdb=pdb(max_states=958787)",
            "--heuristic",
            "hincremental_lmcut=incremental_lmcut(local=false,memory_limit=2000,keep_frontier=false,reevaluate_parent=true,min_cache_hits=100)",
            "--heuristic",
            "hCegar=cegar(max_states=2970840,max_time=361,pick=max_hadd,fact_order=original,decomposition=landmarks,max_abstractions=473)",
            "--heuristic",
            "hcpdbs=cpdbs()",
            "--heuristic",
            "hLM=lmcount(lmg,admissible=true)",
            "--heuristic",
            "hzopdbs=zopdbs()",
            "--heuristic",
            "hCombinedMax=max([hMas,hCegar,hzopdbs,hcpdbs,hpdb,hLM,hincremental_lmcut])",
            "--search",
            "astar(hCombinedMax,partial_order_reduction=sss_expansion_core(active_ops=true,mutexes=fd),mpd=true,pathmax=false,cost_type=0)"
        ]
    ],
    [
        113,
        [
            "--heuristic",
            "hcpdbs=cpdbs()",
            "--heuristic",
            "hpdb=pdb(max_states=282621)",
            "--heuristic",
            "hMas=merge_and_shrink(reduce_labels=true,merge_strategy=MERGE_LINEAR_REVERSE_LEVEL,shrink_strategy=shrink_bisimulation(max_states=6447701,max_states_before_merge=-1,greedy=false,threshold=4577868,group_by_h=false,at_limit=RETURN))",
            "--heuristic",
            "hLMCut=lmcut()",
            "--heuristic",
            "hincremental_lmcut=incremental_lmcut(local=true)",
            "--heuristic",
            "hBlind=blind()",
            "--heuristic",
            "hipdb=ipdb(pdb_max_size=494641,collection_max_size=2355433,num_samples=1135,min_improvement=11,max_time=21)",
            "--heuristic",
            "hCombinedMax=max([hMas,hipdb,hcpdbs,hpdb,hBlind,hLMCut,hincremental_lmcut])",
            "--search",
            "astar(hCombinedMax,partial_order_reduction=small_stubborn_sets(active_ops=true,precond_choice=forward,mutexes=fd),mpd=false,pathmax=true,cost_type=0)"
        ]
    ],
    [
        11,
        [
            "--landmarks",
            "lmg=lm_rhw(only_causal_landmarks=false,disjunctive_landmarks=true,conjunctive_landmarks=true,no_orders=false)",
            "--heuristic",
            "hLM=lmcount(lmg,admissible=true)",
            "--heuristic",
            "hLMCut=lmcut()",
            "--heuristic",
            "hcpdbs=cpdbs()",
            "--heuristic",
            "hCegar=cegar(max_states=8603232,max_time=6,pick=max_refined,fact_order=hadd_up,decomposition=none,max_abstractions=4284)",
            "--heuristic",
            "hHm=hm(m=1)",
            "--heuristic",
            "hBlind=blind()",
            "--heuristic",
            "hMas=merge_and_shrink(reduce_labels=true,merge_strategy=MERGE_LINEAR_GOAL_CG_LEVEL,shrink_strategy=shrink_bisimulation(max_states=3766,max_states_before_merge=-1,greedy=false,threshold=2900,group_by_h=true,at_limit=RETURN))",
            "--heuristic",
            "hCombinedMax=max([hMas,hCegar,hcpdbs,hBlind,hHm,hLM,hLMCut])",
            "--search",
            "astar(hCombinedMax,mpd=false,pathmax=true,cost_type=0)"
        ]
    ]
]

if requirements.has_conditional_effects(SAS_FILE):
    CONFIGS = ADL_CONFIGS
else:
    CONFIGS = STRIPS_CONFIGS

if __name__ == '__main__':
    portfolio.run(CONFIGS, optimal=True)
