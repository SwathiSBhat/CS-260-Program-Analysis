CMAKE_MINIMUM_REQUIRED(VERSION 3.6)

project(CS260ProgramAnalysis)

SET(CMAKE_CXX_STANDARD 17)

ADD_EXECUTABLE(assn0
	assn0.cpp
	json.hpp
)

ADD_EXECUTABLE(test_abstract_store
	test_abstract_store.cpp
	headers/abstract_store.hpp
)

ADD_EXECUTABLE(signanalysis
    signanalysis/signanalysis.cpp
    abstract_store.hpp
    datatypes.h
)