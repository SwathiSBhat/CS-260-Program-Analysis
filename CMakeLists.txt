CMAKE_MINIMUM_REQUIRED(VERSION 3.6)

project(CS260ProgramAnalysis)

SET(CMAKE_CXX_STANDARD 17)

#ADD_EXECUTABLE(assn0
#	assn0.cpp
#	headers/json.hpp
#)

#ADD_EXECUTABLE(test_abstract_store
#	tests/test_abstract_store.cpp
#	headers/abstract_store.hpp
#)

#ADD_EXECUTABLE(assn1_constant_analysis
#	constant-analysis/constant_analysis.cpp
#    headers/abstract_store.hpp
#    headers/datatypes.h
#	headers/execute.hpp
#)

#ADD_EXECUTABLE(assn1_interval_analysis
	#interval-analysis/interval_analysis.cpp
#	interval-analysis/new_interval_analysis.cpp
#	interval-analysis/interval_analysis.hpp
#	interval-analysis/alpha.hpp
#	interval-analysis/interval_execute.hpp
#	headers/datatypes.h
#)


#ADD_EXECUTABLE(assn2_control_flow_analysis
#	control-flow-analysis/control_flow_analysis.cpp
#	control-flow-analysis/cfa_utils.hpp
#	headers/datatypes.h
#)

#ADD_EXECUTABLE(assn2_reaching_def
#	reaching-defn/reachingdef.cpp
#	reaching-defn/rtype.hpp
#	reaching-defn/execute_rdef.hpp
#	headers/datatypes.h
#)

#ADD_EXECUTABLE(test_interval_join
#	interval-analysis/test_interval_join.cpp
#	interval-analysis/interval_analysis.hpp
#)

#ADD_EXECUTABLE(test_widen
#	interval-analysis/test_widen.cpp
#	interval-analysis/interval_analysis.hpp
#)

#ADD_EXECUTABLE(assn3-constraint-generator
#		constraint-generator/constraint-generator.cpp)

#ADD_EXECUTABLE(assn3_constraint_solver
#	pointer-analysis/constraint_solver.cpp
#	pointer-analysis/set_constraint_util.cpp
#	headers/tokenizer.hpp
#)

#ADD_EXECUTABLE(assn4_program_slicing
#	program-dependence-graph/program_slicing.cpp
#	program-dependence-graph/control_flow_analysis.hpp
#	program-dependence-graph/cfa_utils.hpp
#	program-dependence-graph/mod_ref_utils.hpp
#	program-dependence-graph/execute_rdef.hpp
#	program-dependence-graph/rtype.hpp
#	program-dependence-graph/reachingdef.hpp
#	headers/datatypes.h
#	headers/tokenizer.hpp
#)

ADD_EXECUTABLE(assn5_taint_analysis
	taint-analysis/taint_analysis.cpp
	taint-analysis/execute_taint.hpp
	headers/datatypes.h
	headers/tokenizer.hpp
)