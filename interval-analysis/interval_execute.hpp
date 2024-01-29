#pragma once

#include <deque>
#include <set>

#include "interval_analysis.hpp"
#include "../headers/datatypes.h"

/*
 * Execute a BasicBlock against an interval_abstract_store.
 */
interval_abstract_store execute(BasicBlock *bb,
                                interval_abstract_store sigma,
                                std::map<std::string, interval_abstract_store> &bb2store,
                                std::deque<std::string> &worklist,
                                std::unordered_set<std::string> addrof_ints,
                                std::set<std::string> bbs_to_output,
                                bool execute_post = false) {

    /*
     * Make a copy of sigma that we'll return at the end of this function.
     */
    interval_abstract_store sigma_prime = sigma;

    /*
     * Iterate through each instruction in the basic block. This doesn't include
     * terminal instructions.
     */
    for (const Instruction *instruction : bb->instructions) {
        switch (instruction->instrType) {
            case InstructionType::ArithInstrType:
                break;
            case InstructionType::CmpInstrType:
                break;
            case InstructionType::CopyInstrType:
                break;
            case InstructionType::LoadInstrType:
                break;
            case InstructionType::StoreInstrType:
                break;
            case InstructionType::CallExtInstrType:
                break;
            default:
                std::cout << "DEBUG " << __FILE_NAME__ << ":" << __LINE__ << std::endl;
                std::cout << "Instruction not recognized" << std::endl;
        }
    }
}