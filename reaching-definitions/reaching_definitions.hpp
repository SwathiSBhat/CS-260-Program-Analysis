#pragma once

#include <iostream>
#include <map>
#include <set>
#include <string>

#include "../headers/datatypes.h"

/*
 * This is helpful for debugging.
 */
#define print_debug() (std::cout << __FILE__ << ":" << __LINE__ << std::endl)

/*
 * Our abstract domain is sets of program points. Each program point is
 * uniquely identified by its basic block and instruction index in that basic
 * block. Therefore, our abstract value is a pair<string, int>.
 */
typedef std::pair<std::string, int> program_point;

/*
 * Our analysis will eventually give us a mapping from program points to sets of
 * program points that specify the set of reaching definitions for that line.
 */
std::map<program_point, std::set<program_point>> solution;

/*
 * Utility function to get the DEF and USE sets for any instruction.
 */
void get_def_use_sets(Instruction *instruction,
                      std::set<std::string> &def,
                      std::set<std::string> &use) {
    switch (instruction->instrType) {
        case InstructionType::AllocInstrType: {
            AllocInstruction *alloc = (AllocInstruction *) instruction;
            def.insert(alloc->lhs->name);
            break;
        }
        case InstructionType::ArithInstrType: {
            ArithInstruction *arith = (ArithInstruction *) instruction;
            def.insert(arith->lhs->name);
            if (!arith->op1->IsConstInt()) {
                use.insert(arith->op1->var->name);
            }
            if (!arith->op2->IsConstInt()) {
                use.insert(arith->op2->var->name);
            }
            break;
        }
        case InstructionType::CmpInstrType: {
            CmpInstruction *cmp = (CmpInstruction *) instruction;
            def.insert(cmp->lhs->name);
            if (!cmp->op1->IsConstInt()) {
                use.insert(cmp->op1->var->name);
            }
            if (!cmp->op2->IsConstInt()) {
                use.insert(cmp->op2->var->name);
            }
            break;
        }
        case InstructionType::CopyInstrType: {
            CopyInstruction *copy = (CopyInstruction *) instruction;
            def.insert(copy->lhs->name);
            if (!copy->op->IsConstInt()) {
                use.insert(copy->op->var->name);
            }
            break;
        }
        case InstructionType::GepInstrType: {
            GepInstruction *gep = (GepInstruction *) instruction;
            def.insert(gep->lhs->name);

            /*
             * TODO Update USE set.
             */
            break;
        }
        case InstructionType::GfpInstrType: {
            GfpInstruction *gfp = (GfpInstruction *) instruction;
            def.insert(gfp->lhs->name);

            /*
             * TODO Update USE set.
             */
            break;
        }
        default: {
            print_debug();
            std::cout << "Unrecognized instruction type" << std::endl;
            break;
        }
    }
}