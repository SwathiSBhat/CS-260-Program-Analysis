#pragma once

#include <deque>
#include <set>

#include "alpha.hpp"
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
            case InstructionType::ArithInstrType: {
                std::cout << "Encountered $arith " << __FILE_NAME__ << ":" << __LINE__ << std::endl;
                ArithInstruction *arith_instruction = (ArithInstruction *) instruction;

                /*
                 * Initialize op1.
                 */
                abstract_interval op1;
                if (arith_instruction->op1->IsConstInt()) {
                    op1 = alpha(arith_instruction->op1->val);
                } else {
                    op1 = get_val_from_store(sigma_prime, arith_instruction->op1->var->name);
                }

                /*
                 * Initialize op2.
                 */
                abstract_interval op2;
                if (arith_instruction->op2->IsConstInt()) {
                    op2 = alpha(arith_instruction->op1->val);
                } else {
                    op2 = get_val_from_store(sigma_prime, arith_instruction->op2->var->name);
                }

                /*
                 * Check if op1 and op2 are non-bottom, non-top intervals.
                 */
                if (std::holds_alternative<interval>(op1) && std::holds_alternative<interval>(op2)) {
                    if (arith_instruction->arith_op == "Add") {
                        std::cout << "Add " << __FILE_NAME__ << ":" << __LINE__ << std::endl;
                        sigma_prime[arith_instruction->lhs->name] = std::make_pair(std::get<interval>(op1).first + std::get<interval>(op2).first, std::get<interval>(op1).second + std::get<interval>(op2).second);
                    } else if (arith_instruction->arith_op == "Subtract") {
                        std::cout << "Subtract " << __FILE_NAME__ << ":" << __LINE__ << std::endl;
                        sigma_prime[arith_instruction->lhs->name] = std::make_pair(std::get<interval>(op1).first - std::get<interval>(op2).first, std::get<interval>(op1).second - std::get<interval>(op2).second);
                    } else if (arith_instruction->arith_op == "Multiply") {

                        /*
                         * TODO This is wrong! The semantics for multiplication
                         * TODO is a little bit more complicated than this.
                         */
                        std::cout << "Multiply " << __FILE_NAME__ << ":" << __LINE__ << std::endl;
                        sigma_prime[arith_instruction->lhs->name] = std::make_pair(std::get<interval>(op1).first * std::get<interval>(op2).first, std::get<interval>(op1).second * std::get<interval>(op2).second);
                    } else if (arith_instruction->arith_op == "Divide") {
                        std::cout << "Divide " << __FILE_NAME__ << ":" << __LINE__ << std::endl;

                        /*
                         * TODO I'm not sure how to handle the case where the
                         * TODO interval contains 0 but isn't [0, 0].
                         */
                    } else {
                        std::cout << "Unrecognized arithmetic operation " << __FILE_NAME__ << ":" << __LINE__ << std::endl;
                    }
                } else if ((std::holds_alternative<AbstractVals>(op1) && std::visit(IntervalVisitor{}, op1) == "Bottom") ||(std::holds_alternative<AbstractVals>(op2) && std::visit(IntervalVisitor{}, op2) == "Bottom")) {

                    /*
                     * Don't do anything because the result will always be
                     * bottom.
                     */
                } else if (false) {

                    /*
                     * If either op1 or op2 is [0, 0], then multiplication gives
                     * you [0, 0].
                     *
                     * TODO
                     */
                    sigma_prime[arith_instruction->lhs->name] = std::make_pair(0, 0);
                } else if (false) {

                    /*
                     * If op1 is [0, 0], then division will always give you
                     * [0, 0].
                     *
                     * TODO
                     */
                    sigma_prime[arith_instruction->lhs->name] = std::make_pair(0, 0);
                } else if (false) {

                    /*
                     * If op2 is [0, 0], then division will give you bottom.
                     */
                    if (sigma_prime.count(arith_instruction->lhs->name) != 0) {
                        sigma_prime.erase(arith_instruction->lhs->name);
                    }
                } else if (false) {

                    /*
                     * Handle the case where both op1 and op2 are top, so the
                     * result is top.
                     */
                    sigma_prime[arith_instruction->lhs->name] = AbstractVals::TOP;
                } else {
                    std::cout << "We shouldn't be here " << __FILE_NAME__ << ":" << __LINE__ << std::endl;
                }
                break;
            } case InstructionType::CmpInstrType: {
                std::cout << "Encountered $cmp " << __FILE_NAME__ << ":" << __LINE__ << std::endl;
                CmpInstruction *cmp_instruction = (CmpInstruction *) instruction;

                /*
                 * ?
                 */
                if (((cmp_instruction->op1->var) && (!(cmp_instruction->op1->var->isIntType()))) || ((cmp_instruction->op2->var) && (!(cmp_instruction->op2->var->isIntType())))) {
                    sigma_prime[cmp_instruction->lhs->name] = AbstractVals::TOP;
                } else {

                    /*
                     * Initialize op1.
                     */
                    abstract_interval op1;
                    if (cmp_instruction->op1->IsConstInt()) {
                        op1 = alpha(cmp_instruction->op1->val);
                    } else {
                        op1 = get_val_from_store(sigma_prime, cmp_instruction->op1->var->name);
                    }

                    /*
                     * Initialize op2.
                     */
                    abstract_interval op2;
                    if (cmp_instruction->op2->IsConstInt()) {
                        op2 = alpha(cmp_instruction->op2->val);
                    } else {
                        op2 = get_val_from_store(sigma_prime, cmp_instruction->op2->var->name);
                    }

                    /*
                     * Handle the case where both op1 and op2 are non-bottom and
                     * non-top.
                     */
                    if (std::holds_alternative<interval>(op1) && std::holds_alternative<interval>(op2)) {
                        interval op1_interval = std::get<interval>(op1);
                        interval op2_interval = std::get<interval>(op2);

                        /*
                         * TODO I have to figure out the semantics of this with
                         * TODO intervals.
                         */
                        if (cmp_instruction->cmp_op == "Eq") {

                        } else if (cmp_instruction->cmp_op == "Neq") {

                        } else if (cmp_instruction->cmp_op == "Less") {

                        } else if (cmp_instruction->cmp_op == "LessEq") {

                        } else if (cmp_instruction->cmp_op == "Greater") {

                        } else if (cmp_instruction->cmp_op == "GreaterEq") {

                        } else {
                            std::cout << "Unrecognized $cmp operation " << __FILE_NAME__ << ":" << __LINE__ << std::endl;
                        }
                    } else if (((std::holds_alternative<AbstractVals>(op1)) && (std::visit(IntervalVisitor{}, op1) == "Bottom")) || ((std::holds_alternative<AbstractVals>(op2)) && (std::visit(IntervalVisitor{}, op2) == "Bottom"))) {

                        /*
                         * If either operand is bottom, don't do anything.
                         */
                    } else {

                        /*
                         * If we got here, either op1 or op2 are TOP and neither
                         * of them are BOTTOM. Therefore, the result is TOP.
                         */
                        sigma_prime[cmp_instruction->lhs->name] = AbstractVals::TOP;
                    }
                }
                break;
            } case InstructionType::CopyInstrType: {
                std::cout << "Encountered $copy " << __FILE_NAME__ << ":" << __LINE__ << std::endl;
                CopyInstruction *copy_instruction = (CopyInstruction *) instruction;

                /*
                 * If the left-hand side isn't an int-typed variable, ignore the
                 * instruction.
                 */
                if (!copy_instruction->lhs->isIntType()) {
                    continue;
                }

                /*
                 * Copy over the value.
                 */
                abstract_interval op;
                if (copy_instruction->op->IsConstInt()) {
                    op = alpha(copy_instruction->op->val);
                } else {
                    op = get_val_from_store(sigma_prime, copy_instruction->op->var->name);
                }

                /*
                 * Handle the case where op is BOTTOM.
                 */
                if ((std::holds_alternative<AbstractVals>(op)) && (std::visit(IntervalVisitor{}, op) == "Bottom")) {
                    if (sigma_prime.count(copy_instruction->op->var->name) != 0) {
                        sigma_prime.erase(copy_instruction->op->var->name);
                    }
                } else {
                    sigma_prime[copy_instruction->lhs->name] = op;
                }
                break;
            } case InstructionType::LoadInstrType: {
                std::cout << "Encountered $load " << __FILE_NAME__ << ":" << __LINE__ << std::endl;
                break;
            } case InstructionType::StoreInstrType: {
                std::cout << "Encountered $store " << __FILE_NAME__ << ":" << __LINE__ << std::endl;
                break;
            } case InstructionType::CallExtInstrType: {
                std::cout << "Encountered $call_ext " << __FILE_NAME__ << ":" << __LINE__ << std::endl;
                break;
            } default: {
                std::cout << "Instruction not recognized " << __FILE_NAME__ << ":" << __LINE__ << std::endl;
            }
        } // End of switch-case statement.
    } // End of for-loop.

    std::cout << __FILE_NAME__ << ":" << __LINE__ << std::endl;

    /*
     * Now that we've executed the non-terminal instructions, let's look at the
     * terminals to see what to do next.
     */

    Instruction *terminal_instruction = bb->terminal;

    /*
     * TODO
     */

    return sigma_prime;
}