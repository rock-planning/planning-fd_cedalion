#include "por_method.h"
#include "../plugin.h"

// TODO: We can get rid of the following includes once we have the
//       plugin mechanism in place for this.
#include "expansion_core.h"
#include "simple_stubborn_sets.h"
#include "sss_expansion_core.h"
#include "../option_parser.h"

#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

namespace POR {
PORMethod::PORMethod() {}

PORMethod::~PORMethod() {}

NullPORMethod::NullPORMethod() {}

NullPORMethod::~NullPORMethod() {}

void NullPORMethod::dump_options() const {
    cout << "partial order reduction method: none" << endl;
}

PORMethodWithStatistics::PORMethodWithStatistics()
    : unpruned_successors_generated(0),
      pruned_successors_generated(0) {
}

PORMethodWithStatistics::~PORMethodWithStatistics() {
}

void PORMethodWithStatistics::prune_operators(
    const State &state, std::vector<const Operator *> &ops) {
    unpruned_successors_generated += ops.size();
    do_pruning(state, ops);
    pruned_successors_generated += ops.size();
}

void PORMethodWithStatistics::dump_statistics() const {
    cout << "total successors before partial-order reduction: "
         << unpruned_successors_generated << endl
         << "total successors after partial-order reduction: "
         << pruned_successors_generated << endl;
}

}

static inline OptionParser& add_mutex_parser_option(OptionParser& parser) {

    vector<string> mutex_options;
    mutex_options.push_back("none");
    mutex_options.push_back("fd"); // uses mutexes computed by Fast Downward
    
    parser.add_enum_option("mutexes", mutex_options,
			   "mutexes used for dependency relation",
			   "none");
    
    return parser;

}

static POR::PORMethod *_parse(OptionParser &parser) {
    if (parser.dry_run()) {
        return 0;
    }
    else {
	cout << "por method: none" << endl;
	return new POR::NullPORMethod;
    }
}

static POR::PORMethod *_parse_ec(OptionParser &parser) {
    if (parser.dry_run()) {
        return 0;
    }
    else {
	cout << "por method: expansion core" << endl;
	return new POR::ExpansionCore;
    }
}

static POR::PORMethod *_parse_sss(OptionParser &parser) {
    if (parser.dry_run()) {
        return 0;
    }
    else {
	cout << "por method: simple stubborn sets" << endl;
	
	parser.add_option<bool>("active_ops", "use active operators", "false");
	
	vector<string> precondition_choices;

	precondition_choices.push_back("forward");
	
	parser.add_enum_option("precond_choice", precondition_choices,
			       "selection of variable for NES",
			       "forward");

	parser = add_mutex_parser_option(parser);
	Options opts = parser.parse();
	
	return new POR::SimpleStubbornSets(opts);
    }
}

static POR::PORMethod *_parse_sss_ec(OptionParser &parser) {
    if (parser.dry_run()) {
        return 0;
    }
    else {
	cout << "por method: sss_ec" << endl;

	parser.add_option<bool>("active_ops", "use active operators", "false");
	parser = add_mutex_parser_option(parser);
	Options opts = parser.parse();

	return new POR::SSS_ExpansionCore(opts);
    }
}

static POR::PORMethod *_parse_sss_small(OptionParser &parser) {
    if (parser.dry_run()) {
        return 0;
    }
    else {
	cout << "por method: sss_small" << endl;

	parser.add_option<bool>("active_ops", "use active operators", "false");	
	
	vector<string> precondition_choices;
	precondition_choices.push_back("forward"); // the same as simple stubborn sets
	precondition_choices.push_back("backward");
	precondition_choices.push_back("small_static");
	precondition_choices.push_back("small_dynamic");
	precondition_choices.push_back("laarman_heuristic");
	precondition_choices.push_back("most_hitting");
	precondition_choices.push_back("c_forw_small"); // "combination of forward and small"
	precondition_choices.push_back("minimize_global_var_ordering");
	precondition_choices.push_back("random_dynamic");
	precondition_choices.push_back("minimize_random_global_var_ordering");

	parser.add_enum_option("precond_choice", precondition_choices,
			       "selection of variable for NES",
			       "forward");

	parser.add_option<int>("laarman_k", "operator weight for Laarman heuristic", "10");
	
	parser = add_mutex_parser_option(parser);

	Options opts = parser.parse();

	return new POR::SimpleStubbornSets(opts);
    }
}


static Plugin<POR::PORMethod> _plugin("none", _parse);
static Plugin<POR::PORMethod> _plugin_ec("expansion_core", _parse_ec);
static Plugin<POR::PORMethod> _plugin_sss("simple_stubborn_sets", _parse_sss);
static Plugin<POR::PORMethod> _plugin_sss_ec("sss_expansion_core", _parse_sss_ec);
static Plugin<POR::PORMethod> _plugin_sss_small("small_stubborn_sets", _parse_sss_small);

