CMAKE_MINIMUM_REQUIRED(VERSION 3.6)

project(CS260ProgramAnalysis)

SET(CMAKE_CXX_STANDARD 17)

ADD_EXECUTABLE(assn0
	assn0.cpp
	headers/json.hpp
)

ADD_EXECUTABLE(test_abstract_store
	tests/test_abstract_store.cpp
	headers/abstract_store.hpp
)

ADD_EXECUTABLE(assn1_constant_analysis
	constant-analysis/constant_analysis.cpp
    headers/abstract_store.hpp
    headers/datatypes.h
	headers/execute.hpp
)

ADD_EXECUTABLE(assn1_interval_analysis
	#interval-analysis/interval_analysis.cpp
	interval-analysis/new_interval_analysis.cpp
	interval-analysis/interval_analysis.hpp
	interval-analysis/alpha.hpp
	interval-analysis/interval_execute.hpp
	headers/datatypes.h
)

ADD_EXECUTABLE(test_interval_join
	interval-analysis/test_interval_join.cpp
	interval-analysis/interval_analysis.hpp
)

ADD_EXECUTABLE(test_widen
	interval-analysis/test_widen.cpp
	interval-analysis/interval_analysis.hpp
)