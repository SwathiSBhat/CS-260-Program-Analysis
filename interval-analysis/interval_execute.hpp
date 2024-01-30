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
                        std::cout << "Multiply " << __FILE_NAME__ << ":" << __LINE__ << std::endl;

                        /*
                         * For multiplication, we have to use the bounds to
                         * compute every possible value and take the min/max of
                         * that.
                         */
                        std::multiset<int> possible_bounds;
                        possible_bounds.insert(std::get<interval>(op1).first * std::get<interval>(op2).first);
                        possible_bounds.insert(std::get<interval>(op1).first * std::get<interval>(op2).second);
                        possible_bounds.insert(std::get<interval>(op1).second * std::get<interval>(op2).first);
                        possible_bounds.insert(std::get<interval>(op1).second * std::get<interval>(op2).second);
                        sigma_prime[arith_instruction->lhs->name] = std::make_pair(*(possible_bounds.begin()), *(--possible_bounds.end()));
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

                            /*
                             * We will assign [1, 1] only if both intervals are
                             * a single value and those values equal each other.
                             * For example, [4, 4] eq [4, 4].
                             *
                             * We will assign [0, 0] only if the intervals don't
                             * overlap at all. In this case, there is no way
                             * they can be equal. For example, [1, 7] eq [9, 12].
                             *
                             * In every other case, we will assign [0, 1].
                             */
                            if ((op1_interval.first == op1_interval.second) && (op1_interval.first == op2_interval.first) && (op1_interval.first == op2_interval.second)) {
                                sigma_prime[cmp_instruction->lhs->name] = std::make_pair(1, 1);
                            } else if (op1_interval.second < op2_interval.first) {
                                sigma_prime[cmp_instruction->lhs->name] = std::make_pair(0, 0);
                            } else {
                                sigma_prime[cmp_instruction->lhs->name] = std::make_pair(0, 1);
                            }
                        } else if (cmp_instruction->cmp_op == "Neq") {

                            /*
                             * Assign [1, 1] if the intervals don't overlap.
                             *
                             * Assign [0, 0] if the intervals are both a single
                             * value and those values are equal.
                             *
                             * Assign [0, 1] otherwise.
                             */
                            if (op1_interval.second < op2_interval.first) {
                                sigma_prime[cmp_instruction->lhs->name] = std::make_pair(1, 1);
                            } else if ((op1_interval.first == op1_interval.second) && (op1_interval.first == op2_interval.first) && (op1_interval.first == op2_interval.second)) {
                                sigma_prime[cmp_instruction->lhs->name] = std::make_pair(0, 0);
                            } else {
                                sigma_prime[cmp_instruction->lhs->name] = std::make_pair(0, 1);
                            }

                        } else if (cmp_instruction->cmp_op == "Less") {

                            /*
                             * Return [1, 1] if op1_interval is strictly less
                             * than op2_interval. For example, [1, 4] and
                             * [6, 8].
                             *
                             * Return [0, 0] if op1_interval is strictly greater
                             * than op2_interval.
                             *
                             * Return [0, 1] otherwise.
                             */
                            if (op1_interval.second < op2_interval.first) {
                                sigma_prime[cmp_instruction->lhs->name] = std::make_pair(1, 1);
                            } else if (op1_interval.first > op2_interval.second) {
                                sigma_prime[cmp_instruction->lhs->name] = std::make_pair(0, 0);
                            } else {
                                sigma_prime[cmp_instruction->lhs->name] = std::make_pair(0, 1);
                            }
                        } else if (cmp_instruction->cmp_op == "LessEq") {

                            /*
                             * Assign [1, 1] if op1_interval is strictly less
                             * than op2_interval OR if both op1_interval and
                             * op2_interval are the same single value.
                             *
                             * Assign [0, 0] if op1_interval is strictly greater
                             * than op2_interval (with no overlap).
                             *
                             * Otherwise, assign [0, 1].
                             */
                            if ((op1_interval.second < op2_interval.first) || ((op1_interval.first == op1_interval.second) && (op1_interval.first == op2_interval.first) && (op1_interval.first == op2_interval.second))) {
                                sigma_prime[cmp_instruction->lhs->name] = std::make_pair(1, 1);
                            } else if (op1_interval.first > op2_interval.second) {
                                sigma_prime[cmp_instruction->lhs->name] = std::make_pair(0, 0);
                            } else {
                                sigma_prime[cmp_instruction->lhs->name] = std::make_pair(0, 1);
                            }
                        } else if (cmp_instruction->cmp_op == "Greater") {

                            /*
                             * Return [1, 1] if op1_interval is strictly greater
                             * than op2_interval (with no overlap).
                             *
                             * Return [0, 0] if op1_interval is strictly less
                             * than op2_interval (with no overlap).
                             *
                             * Return [0, 1] otherwise.
                             */
                            if (op1_interval.first > op2_interval.second) {
                                sigma_prime[cmp_instruction->lhs->name] = std::make_pair(1, 1);
                            } else if (op1_interval.second < op2_interval.first) {
                                sigma_prime[cmp_instruction->lhs->name] = std::make_pair(0, 0);
                            } else {
                                sigma_prime[cmp_instruction->lhs->name] = std::make_pair(0, 1);
                            }
                        } else if (cmp_instruction->cmp_op == "GreaterEq") {

                            /*
                             * Assign [1, 1] either if op1_interval is strictly
                             * greater than op2_interval or if op1_interval and
                             * op2_interval are both exactly the same single
                             * value.
                             *
                             * Assign [0, 0] if op1_interval is strictly less
                             * than op2_interval (with no overlap).
                             *
                             * Otherwise, assign [0, 1].
                             */
                            if ((op1_interval.first > op2_interval.second) || ((op1_interval.first == op1_interval.second) && (op1_interval.first == op2_interval.first) && (op1_interval.first == op2_interval.second))) {
                                sigma_prime[cmp_instruction->lhs->name] = std::make_pair(1, 1);
                            } else if (op1_interval.second < op2_interval.first) {
                                sigma_prime[cmp_instruction->lhs->name] = std::make_pair(0, 0);
                            } else {
                                sigma_prime[cmp_instruction->lhs->name] = std::make_pair(0, 1);
                            }
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
     *
     * TODO I have to modify this part to widen at loop headers.
     */

    Instruction *terminal_instruction = bb->terminal;
    if (!execute_post) {
        switch (terminal_instruction->instrType) {
            case InstructionType::BranchInstrType: {
                std::cout << "Encountered $branch " << __FILE_NAME__ << ":" << __LINE__ << std::endl;
                BranchInstruction *branch_instruction = (BranchInstruction *) terminal_instruction;

                /*
                 * If op is not 0, go to bb1. Otherwise, go to bb2. If op is
                 * TOP, then propagate to both.
                 */
                if (branch_instruction->condition->IsConstInt()) {
                    if (branch_instruction->condition->val != 0) {
                        bool store_changed_tt = join(bb2store[branch_instruction->tt], sigma_prime);
                        if (store_changed_tt || bbs_to_output.count(branch_instruction->tt) == 0) {
                            worklist.push_back(branch_instruction->tt);
                        }
                    } else {
                        bool store_changed_ff = join(bb2store[branch_instruction->ff], sigma_prime);
                        if (store_changed_ff || bbs_to_output.count(branch_instruction->ff) == 0) {
                            worklist.push_back(branch_instruction->ff);
                        }
                    }
                } else {
                    abstract_interval abs_val = get_val_from_store(sigma_prime, branch_instruction->condition->var->name);
                    if ((std::holds_alternative<AbstractVals>(abs_val)) && (std::visit(IntervalVisitor{}, abs_val) == TOP_STR)) {

                        /*
                         * Consider the "true" branch.
                         */
                        bool store_changed_tt = join(bb2store[branch_instruction->tt], sigma_prime);
                        if (store_changed_tt || bbs_to_output.count(branch_instruction->tt) == 0) {
                            worklist.push_back(branch_instruction->tt);
                        }

                        /*
                         * Consider the "false" branch.
                         */
                        bool store_changed_ff = join(bb2store[branch_instruction->ff], sigma_prime);
                        if (store_changed_ff || bbs_to_output.count(branch_instruction->ff) == 0) {
                            worklist.push_back(branch_instruction->ff);
                        }
                    } else if (std::holds_alternative<interval>(abs_val)) {

                        /*
                         * TODO I'm not sure about my logic here.
                         */
                        if ((std::get<interval>(abs_val).first != 0) || (std::get<interval>(abs_val).second != 0)) {
                            bool store_changed_tt = join(bb2store[branch_instruction->tt], sigma_prime);
                            if (store_changed_tt || bbs_to_output.count(branch_instruction->tt) == 0) {
                                worklist.push_back(branch_instruction->tt);
                            }
                        } else {
                            bool store_changed_ff = join(bb2store[branch_instruction->ff], sigma_prime);
                            if (store_changed_ff || bbs_to_output.count(branch_instruction->ff) == 0) {
                                worklist.push_back(branch_instruction->ff);
                            }
                        }
                    }
                }
                break;
            } case InstructionType::JumpInstrType: {
                std::cout << "Encountered $jump " << __FILE_NAME__ << ":" << __LINE__ << std::endl;
                JumpInstruction *jump_instruction = (JumpInstruction *) terminal_instruction;

                /*
                 * Join sigma_prime with the basic block's abstract store
                 * (updating the basic block's abstract store).
                 */
                bool store_changed = join(bb2store[jump_instruction->label], sigma_prime);
                if (store_changed || bbs_to_output.count(jump_instruction->label) == 0) {
                    worklist.push_back(jump_instruction->label);
                }
                break;
            } case InstructionType::RetInstrType: {
                std::cout << "Encountered $ret " << __FILE_NAME__ << ":" << __LINE__ << std::endl;
                break;
            } default: {
                std::cout << "Unrecognized terminal instruction " << __FILE_NAME__ << ":" << __LINE__ << std::endl;
                break;
            }
        }
    }

    return sigma_prime;
}