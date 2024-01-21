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

ADD_EXECUTABLE(assn1_signanalysis
    signanalysis/signanalysis.cpp
    headers/abstract_store.hpp
    headers/datatypes.h
	headers/execute.hpp
)

ADD_EXECUTABLE(test_mfp_worklist
	headers/MFPWorklistAlgorithm.hpp
	tests/test_worklist.cpp
	headers/json.hpp
)