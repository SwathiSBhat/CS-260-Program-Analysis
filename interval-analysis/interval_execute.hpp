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
                                bool execute_post,
                                std::unordered_set<std::string> loop_headers) {

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
                //std::cout << "Encountered $arith " << __FILE_NAME__ << ":" << __LINE__ << std::endl;
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
                    op2 = alpha(arith_instruction->op2->val);
                } else {
                    op2 = get_val_from_store(sigma_prime, arith_instruction->op2->var->name);
                }

                /*
                 * Check if op1 and op2 are non-bottom, non-top intervals.
                 */
                if (std::holds_alternative<interval>(op1) && std::holds_alternative<interval>(op2)) {

                    /*
                     * For debugging.
                     */
                    if ((std::get<interval>(op1).first == INTERVAL_NEG_INFINITY) || (std::get<interval>(op1).first == INTERVAL_INFINITY) || (std::get<interval>(op1).second == INTERVAL_NEG_INFINITY) || (std::get<interval>(op1).second == INTERVAL_INFINITY)) {
                        //std::cout << "TO INFINITY AND BEYOND " __FILE_NAME__ << ":" << __LINE__ << std::endl;
                    }
                    if ((std::get<interval>(op2).first == INTERVAL_NEG_INFINITY) || (std::get<interval>(op2).first == INTERVAL_INFINITY) || (std::get<interval>(op2).second == INTERVAL_NEG_INFINITY) || (std::get<interval>(op2).second == INTERVAL_INFINITY)) {
                        //std::cout << "TO INFINITY AND BEYOND " __FILE_NAME__ << ":" << __LINE__ << std::endl;
                    }

                    if (arith_instruction->arith_op == "Add") {
                        //std::cout << "Add " << __FILE_NAME__ << ":" << __LINE__ << std::endl;

                        /*
                         * If one of the bounds are infinity/negative infinity,
                         * we don't want to add.
                         */
                        int lower_bound_result;
                        int upper_bound_result;
                        int op1_lower = std::get<interval>(op1).first;
                        int op1_upper = std::get<interval>(op1).second;
                        int op2_lower = std::get<interval>(op2).first;
                        int op2_upper = std::get<interval>(op2).second;

                        /*
                         * Calculate the result for the lower bound.
                         */
                        if ((op1_lower == INTERVAL_NEG_INFINITY) || (op2_lower == INTERVAL_NEG_INFINITY)) {
                            lower_bound_result = INTERVAL_NEG_INFINITY;
                        } else {
                            lower_bound_result = op1_lower + op2_lower;
                        }

                        /*
                         * Calculate the result for the upper bound.
                         */
                        if ((op1_upper == INTERVAL_INFINITY) || (op2_upper == INTERVAL_INFINITY)) {
                            upper_bound_result = INTERVAL_INFINITY;
                        } else {
                            upper_bound_result = op1_upper + op2_upper;
                        }

                        /*
                         * Update sigma_prime.
                         */
                        sigma_prime[arith_instruction->lhs->name] = std::make_pair(lower_bound_result, upper_bound_result);
                    } else if (arith_instruction->arith_op == "Subtract") {

                        int lower_bound_result;
                        int upper_bound_result;
                        int op1_lower = std::get<interval>(op1).first;
                        int op1_upper = std::get<interval>(op1).second;
                        int op2_lower = std::get<interval>(op2).first;
                        int op2_upper = std::get<interval>(op2).second;

                        /*
                         * Calculate the result for the lower bound. If
                         * op1_lower is negative infinity and op2_lower is NOT
                         * negative infinity, the result is negative infinity.
                         * If op1_lower is negative infinity AND op2_lower is
                         * negative infinity, the result is BOTTOM???. If
                         * op1_lower is NOT negative infinity and op2_lower IS
                         * negative infinity, the result is BOTTOM?
                         */
                        if ((op1_lower == INTERVAL_NEG_INFINITY) && (op2_lower != INTERVAL_NEG_INFINITY)) {
                            lower_bound_result = INTERVAL_NEG_INFINITY;
                        } else if ((op1_lower == INTERVAL_NEG_INFINITY) && (op2_lower == INTERVAL_NEG_INFINITY)) {
                            std::cout << __FILE__ << ":" << __LINE__ << std::endl;
                            std::cout << "Subtracting -INF from -INF" << std::endl;
                            lower_bound_result = INTERVAL_NEG_INFINITY;
                        } else if ((op1_lower != INTERVAL_NEG_INFINITY) && (op2_lower == INTERVAL_NEG_INFINITY)) {
                            std::cout << __FILE__ << ":" << __LINE__ << std::endl;
                            std::cout << "Subtracting -INF from an integer" << std::endl;
                        } else {
                            lower_bound_result = op1_lower - op2_lower;
                        }

                        /*
                         * Calculate the result for the upper bound. If
                         * op1_upper is infinity while op2_upper is NOT
                         * infinity, the result is infinity. If both op1_upper
                         * and op2_upper are infinity, I guess the result is
                         * BOTTOM? If op1_upper is NOT infinity and op2_upper IS
                         * infinity, I guess the result is also BOTTOM?
                         */
                        if ((op1_upper == INTERVAL_INFINITY) && (op2_upper != INTERVAL_INFINITY)) {
                            upper_bound_result = INTERVAL_INFINITY;
                        } else if ((op1_upper == INTERVAL_INFINITY) && (op2_upper == INTERVAL_INFINITY)) {
                            std::cout << __FILE__ << ":" << __LINE__ << std::endl;
                            std::cout << "Subtracting INF from INF" << std::endl;
                            upper_bound_result = INTERVAL_INFINITY;
                        } else if ((op1_upper != INTERVAL_INFINITY) && (op2_upper == INTERVAL_INFINITY)) {
                            std::cout << __FILE__ << ":" << __LINE__ << std::endl;
                            std::cout << "Subtracting INF from an integer" << std::endl;

                            /*
                             * TODO This is definitely wrong.
                             */
                            upper_bound_result = INTERVAL_INFINITY;
                        } else {
                            upper_bound_result = op1_upper - op2_upper;
                        }

                        /*
                         * Update sigma_prime.
                         */
                        //std::cout << "Subtracting " <<  std::visit(IntervalVisitor{}, op1) << " and " << std::visit(IntervalVisitor{}, op2) << " " << __FILE_NAME__ << ":" << __LINE__ << std::endl;
                        sigma_prime[arith_instruction->lhs->name] = std::make_pair(lower_bound_result, upper_bound_result);
                    } else if (arith_instruction->arith_op == "Multiply") {


                        /*
                         * For multiplication, we have to use the bounds to
                         * compute every possible value and use the min/max of
                         * that.
                         */
                        std::multiset<int> possible_bounds;
                        int op1_lower = std::get<interval>(op1).first;
                        int op1_upper = std::get<interval>(op1).second;
                        int op2_lower = std::get<interval>(op2).first;
                        int op2_upper = std::get<interval>(op2).second;

                        /*
                         * Handle the case where op1_lower or op2_lower are
                         * -INF. In that case, insert -INF into the multiset.
                         * Otherwise, insert op1_lower * op2_lower.
                         */
                        if ((op1_lower == INTERVAL_NEG_INFINITY) ||
                            (op2_lower == INTERVAL_NEG_INFINITY)) {
                            possible_bounds.insert(INTERVAL_NEG_INFINITY);
                        } else {
                            possible_bounds.insert(op1_lower * op2_lower);
                        }

                        /*
                         * Handle the case where op1_upper or op2_upper are INF.
                         * In that case, insert INF into the multiset.
                         * Otherwise, insert op1_upper * op2_upper.
                         */
                        if ((op1_upper == INTERVAL_INFINITY) ||
                            (op2_upper == INTERVAL_INFINITY)) {
                            possible_bounds.insert(INTERVAL_INFINITY);
                        } else {
                            possible_bounds.insert(op1_upper * op2_upper);
                        }

                        /*
                         * Handle the case where multiplying op1_lower with
                         * op2_upper produces a non-infinite bound.
                         */
                        if ((op1_lower != INTERVAL_NEG_INFINITY) &&
                            (op2_upper != INTERVAL_INFINITY)) {
                            possible_bounds.insert(op1_lower * op2_upper);
                        }

                        /*
                         * Handle the case where multiplying op1_upper with
                         * op2_lower produces a bound.
                         */
                        if ((op1_upper != INTERVAL_INFINITY) &&
                            (op2_lower != INTERVAL_NEG_INFINITY)) {
                            possible_bounds.insert(op1_upper * op2_lower);
                        }

                        /*
                         * Update sigma_prime with the true min and max.
                         */
                        sigma_prime[arith_instruction->lhs->name] = std::make_pair(*(possible_bounds.begin()), *(--possible_bounds.end()));
                    } else if (arith_instruction->arith_op == "Divide") {

                        /*
                         * TODO account for infinities?
                         */
                        //std::cout << "Divide " << __FILE_NAME__ << ":" << __LINE__ << std::endl;

                        /*
                         * Get the actual intervals for the operands.
                         */
                        interval op1_interval = std::get<interval>(op1);
                        interval op2_interval = std::get<interval>(op2);

                        /*
                         * This is what we're doing for op1 / op2:
                         *
                         * If op2 is [0, 0], the answer is BOTTOM.
                         *
                         * If op2.lower is negative and op2.upper is positive,
                         * compute op1 / [-1, 1] using the min/max method.
                         *
                         * If op2.lower is 0, compute op1 / [1, op2.upper] using
                         * the min/max method.
                         *
                         * If op2.upper is 0, compute op1 / [op2.lower, -1]
                         * using the min/max method.
                         */
                        if ((op2_interval.first == 0) && (op2_interval.second == 0)) {
                            sigma_prime[arith_instruction->lhs->name] = AbstractVals::BOTTOM;
                        } else if ((op2_interval.first < 0) && (op2_interval.second > 0)) {
                            std::multiset<int> possible_bounds;
                            possible_bounds.insert((op1_interval.first) / (-1));
                            possible_bounds.insert((op1_interval.first) / (1));
                            possible_bounds.insert((op1_interval.second) / (-1));
                            possible_bounds.insert((op1_interval.second) / (1));
                            sigma_prime[arith_instruction->lhs->name] = std::make_pair(*(possible_bounds.begin()), *(--possible_bounds.end()));
                        } else if (op2_interval.first == 0) {
                            std::multiset<int> possible_bounds;
                            possible_bounds.insert((op1_interval.first) / (1));
                            possible_bounds.insert((op1_interval.first) / (op2_interval.second));
                            possible_bounds.insert((op1_interval.second) / (1));
                            possible_bounds.insert((op1_interval.second) / (op2_interval.second));
                            sigma_prime[arith_instruction->lhs->name] = std::make_pair(*(possible_bounds.begin()), *(--possible_bounds.end()));
                        } else if (op2_interval.second == 0) {
                            std::multiset<int> possible_bounds;
                            possible_bounds.insert((op1_interval.first) / (op2_interval.first));
                            possible_bounds.insert((op1_interval.first) / (-1));
                            possible_bounds.insert((op1_interval.second) / (op2_interval.first));
                            possible_bounds.insert((op1_interval.second) / (-1));
                            sigma_prime[arith_instruction->lhs->name] = std::make_pair(*(possible_bounds.begin()), *(--possible_bounds.end()));
                        } else {

                            /*
                             * Just use the min/max method directly.
                             */
                            std::multiset<int> possible_bounds;
                            possible_bounds.insert((op1_interval.first) / (op2_interval.first));
                            possible_bounds.insert((op1_interval.first) / (op2_interval.second));
                            possible_bounds.insert((op1_interval.second) / (op2_interval.first));
                            possible_bounds.insert((op1_interval.second) / (op2_interval.second));
                            sigma_prime[arith_instruction->lhs->name] = std::make_pair(*(possible_bounds.begin()), *(--possible_bounds.end()));
                        }
                    } else {
                        //std::cout << "Unrecognized arithmetic operation " << __FILE_NAME__ << ":" << __LINE__ << std::endl;
                    }
                } else if ((std::holds_alternative<AbstractVals>(op1) && std::visit(IntervalVisitor{}, op1) == "Bottom") ||(std::holds_alternative<AbstractVals>(op2) && std::visit(IntervalVisitor{}, op2) == "Bottom")) {

                    /*
                     * If either op1 or op2 are BOTTOM, set the left-hand side
                     * to BOTTOM (erasing it from the store).
                     */
                    if (sigma_prime.count(arith_instruction->lhs->name) != 0) {
                        sigma_prime.erase(arith_instruction->lhs->name);
                    }
                } else if (((std::holds_alternative<AbstractVals>(op1)) && (std::visit(IntervalVisitor{}, op1) == TOP_STR)) || ((std::holds_alternative<AbstractVals>(op2)) && (std::visit(IntervalVisitor{}, op2) == TOP_STR))) {

                    /*
                     * Handle the case where one or both of op1 or op2 are TOP.
                     */

                    //std::cout << "Something is TOP " << __FILE_NAME__ << ":" << __LINE__ << std::endl;

                    /*
                     * If either op1 or op2 is [0, 0], then multiplication gives
                     * you [0, 0].
                     *
                     * TODO
                     */
                    sigma_prime[arith_instruction->lhs->name] = std::make_pair(0, 0);
                } else {
                    //std::cout << "BLAH BLAH BLAH NO NO NO " << __FILE_NAME__ << ":" << __LINE__ << std::endl;

                }
                break;
            } case InstructionType::CmpInstrType: {
                //std::cout << "Encountered $cmp " << __FILE_NAME__ << ":" << __LINE__ << std::endl;
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
                         * Handle each type of $cmp instruction differently.
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
                             * than op2_interval or if (op1_lower = op2_lower
                             * and op1_upper <= op2_upper). See the example
                             * below.
                             *
                             * [4,4] lt [4, 10] should return [0, 0].
                             *
                             * Return [0, 1] otherwise.
                             */
                            if (op1_interval.second < op2_interval.first) {
                                sigma_prime[cmp_instruction->lhs->name] = std::make_pair(1, 1);
                            } else if ((op1_interval.first > op2_interval.second) || ((op1_interval.first == op2_interval.first) && (op1_interval.second <= op2_interval.second))) {
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
                             * than op2_interval (with no overlap) or if (op1's
                             * lower <= op2's lower and op1's upper = op2's
                             * upper).
                             *
                             * [4, 10] gt [10, 10] should return [0, 0].
                             *
                             * Return [0, 1] otherwise.
                             */
                            if (op1_interval.first > op2_interval.second) {
                                sigma_prime[cmp_instruction->lhs->name] = std::make_pair(1, 1);
                            } else if ((op1_interval.second < op2_interval.first) || ((op1_interval.first <= op2_interval.first) && (op1_interval.second == op2_interval.second))) {
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
                            //std::cout << "Unrecognized $cmp operation " << __FILE_NAME__ << ":" << __LINE__ << std::endl;
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
                //std::cout << "Encountered $copy " << __FILE_NAME__ << ":" << __LINE__ << std::endl;
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
                if (bb->label == "bb1") {
                    //std::cout << "Inside bb1, value of op inside copy\n";
                    //std::cout << std::visit(IntervalVisitor{}, op) << std::endl;
                    //if (!copy_instruction->op->IsConstInt())
                    //std::cout << copy_instruction->op->var->name << std::endl;
                }
                /*
                 * Handle the case where op is BOTTOM.
                 */
                if ((std::holds_alternative<AbstractVals>(op)) && (std::visit(IntervalVisitor{}, op) == "Bottom")) {
                    if (bb->label == "bb1") {
                        //std::cout << "Count of " << copy_instruction->op->var->name << std::endl;
                        //std::cout << sigma_prime.count(copy_instruction->op->var->name) << std::endl;
                    }
                    if (sigma_prime.count(copy_instruction->lhs->name) != 0) {
                        sigma_prime.erase(copy_instruction->lhs->name);
                    }
                } else {
                    sigma_prime[copy_instruction->lhs->name] = op;
                }
                break;
            } case InstructionType::LoadInstrType: {
                //std::cout << "Encountered $load " << __FILE_NAME__ << ":" << __LINE__ << std::endl;
                break;
            } case InstructionType::StoreInstrType: {
                //std::cout << "Encountered $store " << __FILE_NAME__ << ":" << __LINE__ << std::endl;
                break;
            } case InstructionType::CallExtInstrType: {
                //std::cout << "Encountered $call_ext " << __FILE_NAME__ << ":" << __LINE__ << std::endl;
                break;
            } default: {
                //std::cout << "Instruction not recognized " << __FILE_NAME__ << ":" << __LINE__ << std::endl;
            }
        } // End of switch-case statement.
    } // End of for-loop.

    //std::cout << __FILE_NAME__ << ":" << __LINE__ << std::endl;

    /*
     * Now that we've executed the non-terminal instructions, let's look at the
     * terminals to see what to do next.
     */

    Instruction *terminal_instruction = bb->terminal;
    if (!execute_post) {
        //std::cout << "Considering terminals now " << __FILE_NAME__ << ":" << __LINE__ << std::endl;
        switch (terminal_instruction->instrType) {
            case InstructionType::BranchInstrType: {

                /*
                 * TODO We might need to get the branch name from the store dynamically!!!
                 */
                //std::cout << "Encountered $branch " << __FILE_NAME__ << ":" << __LINE__ << std::endl;
                BranchInstruction *branch_instruction = (BranchInstruction *) terminal_instruction;
                //std::cout << "This is the branch value =" << branch_instruction->condition->val << std::endl;

                /*
                 * If op is not 0, go to bb1. Otherwise, go to bb2. If op is
                 * TOP, then propagate to both.
                 */
                if (branch_instruction->condition->IsConstInt()) {
                    if (branch_instruction->condition->val != 0) {

                        /*
                         * TODO I'm just assuming that we widen when the current
                         * TODO bb is a loop header, but this might be wrong.
                         */
                        bool store_changed_tt;
                        if (loop_headers.count(branch_instruction->tt) != 0) {
                            store_changed_tt = widen(bb2store[branch_instruction->tt], sigma_prime);
                        } else {
                            store_changed_tt = join(bb2store[branch_instruction->tt], sigma_prime);
                        }
                        if (store_changed_tt || bbs_to_output.count(branch_instruction->tt) == 0) {
                            //std::cout << "Pushing " << branch_instruction->tt << " onto the worklist " << __FILE_NAME__ << ":" << __LINE__ << std::endl;
                            worklist.push_back(branch_instruction->tt);
                        }
                    } else {
                        /*
                         * TODO This logic might be wrong for the same reason.
                         */
                        bool store_changed_ff;
                        if (loop_headers.count(branch_instruction->ff) != 0) {
                            store_changed_ff = widen(bb2store[branch_instruction->ff], sigma_prime);
                        } else {
                            store_changed_ff = join(bb2store[branch_instruction->ff], sigma_prime);
                        }
                        if (store_changed_ff || bbs_to_output.count(branch_instruction->ff) == 0) {
                            //std::cout << "Pushing " << branch_instruction->ff << " onto the worklist " << __FILE_NAME__ << ":" << __LINE__ << std::endl;
                            worklist.push_back(branch_instruction->ff);
                        }
                    }
                } else {
                    abstract_interval abs_val = get_val_from_store(sigma_prime, branch_instruction->condition->var->name);
                    //std::cout << "This is our branch abstract interval!!!! =" << std::visit(IntervalVisitor{}, abs_val) << std::endl;
                    //std::cout << std::visit(IntervalVisitor{}, abs_val) << std::endl;
                    if (((std::holds_alternative<AbstractVals>(abs_val)) && (std::visit(IntervalVisitor{}, abs_val) == TOP_STR)) || ((std::holds_alternative<interval>(abs_val)) && (((std::get<interval>(abs_val).first < 0) && (std::get<interval>(abs_val).second >= 0)) || ((std::get<interval>(abs_val).first <= 0) && (std::get<interval>(abs_val).second > 0))))) {

                        /*
                         * Consider the "true" branch.
                         */
                        bool store_changed_tt;
                        if (loop_headers.count(branch_instruction->tt) != 0) {
                            store_changed_tt = widen(bb2store[branch_instruction->tt], sigma_prime);
                        } else {
                            store_changed_tt = join(bb2store[branch_instruction->tt], sigma_prime);
                        }
                        if (store_changed_tt || bbs_to_output.count(branch_instruction->tt) == 0) {
                            //std::cout << "Pushing " << branch_instruction->tt << " onto the worklist " << __FILE_NAME__ << ":" << __LINE__ << std::endl;
                            worklist.push_back(branch_instruction->tt);
                        }

                        /*
                         * Consider the "false" branch.
                         */
                        bool store_changed_ff;
                        if (loop_headers.count(branch_instruction->ff) != 0) {
                            store_changed_ff = widen(bb2store[branch_instruction->ff], sigma_prime);
                        } else {
                            store_changed_ff = join(bb2store[branch_instruction->ff], sigma_prime);
                        }
                        if (store_changed_ff || bbs_to_output.count(branch_instruction->ff) == 0) {
                            //std::cout << "Pushing " << branch_instruction->ff << " onto the worklist " << __FILE_NAME__ << ":" << __LINE__ << std::endl;
                            worklist.push_back(branch_instruction->ff);
                        }
                    } else if (std::holds_alternative<interval>(abs_val)) {

                        /*
                         * TODO I'm not sure about my logic here.
                         */
                        if ((std::get<interval>(abs_val).first != 0) || (std::get<interval>(abs_val).second != 0)) {
                            bool store_changed_tt;
                            if (loop_headers.count(branch_instruction->tt) != 0) {
                                store_changed_tt = widen(bb2store[branch_instruction->tt], sigma_prime);
                            } else {
                                store_changed_tt = join(bb2store[branch_instruction->tt], sigma_prime);
                            }
                            if (store_changed_tt || bbs_to_output.count(branch_instruction->tt) == 0) {
                                //std::cout << "Pushing " << branch_instruction->tt << " onto the worklist " << __FILE_NAME__ << ":" << __LINE__ << std::endl;
                                worklist.push_back(branch_instruction->tt);
                            }
                        } else {
                            bool store_changed_ff;
                            if (loop_headers.count(branch_instruction->ff) != 0) {
                                store_changed_ff = widen(bb2store[branch_instruction->ff], sigma_prime);
                            } else {
                                store_changed_ff = join(bb2store[branch_instruction->ff], sigma_prime);
                            }
                            if (store_changed_ff || bbs_to_output.count(branch_instruction->ff) == 0) {
                                //std::cout << "Pushing " << branch_instruction->ff << " onto the worklist " << __FILE_NAME__ << ":" << __LINE__ << std::endl;
                                worklist.push_back(branch_instruction->ff);
                            }
                        }
                    }
                }
                break;
            } case InstructionType::JumpInstrType: {
                //std::cout << "Encountered $jump " << __FILE__ << ":" << __LINE__ << std::endl;
                JumpInstruction *jump_instruction = (JumpInstruction *) terminal_instruction;
                //std::cout << "Current: " << bb->label << " target: " << jump_instruction->label << " " << __FILE__ << ":" << __LINE__ << std::endl;

                          /*
                           * Join sigma_prime with the basic block's abstract store
                           * (updating the basic block's abstract store).
                           */
                bool store_changed;
                if (loop_headers.count(jump_instruction->label) != 0) {
                    //std::cout << "Going to widen " << __FILE__ << ":" << __LINE__ << std::endl;
                    //std::cout << "bb2store[jump_instruction->label] BEFORE!" << std::endl;
                    //print(bb2store[jump_instruction->label]);
                    store_changed = widen(bb2store[jump_instruction->label],
                                          sigma_prime);
                    //std::cout << "sigma_prime" << std::endl;
                    //print(sigma_prime);
                    //std::cout << "bb2store[jump_instruction->label] after" << std::endl;
                    //print(bb2store[jump_instruction->label]);
                } else {
                    //std::cout << "Going to join " << __FILE__ << ":" << __LINE__ << std::endl;
                    store_changed = join(bb2store[jump_instruction->label],
                                         sigma_prime);
                }
                if (store_changed || (bbs_to_output.count(jump_instruction->label) == 0)) {
                    //std::cout << "Pushing " << jump_instruction->label << " onto the worklist (from $jump) " << __FILE_NAME__ << ":" << __LINE__ << std::endl;
                    worklist.push_back(jump_instruction->label);
                }
                break;
            } case InstructionType::RetInstrType: {
                //std::cout << "Encountered $ret " << __FILE_NAME__ << ":" << __LINE__ << std::endl;
                break;
            } default: {
                //std::cout << "Unrecognized terminal instruction " << __FILE_NAME__ << ":" << __LINE__ << std::endl;
                break;
            }
        }
    }

    if (execute_post) {
        //std::cout << "Basic block name:" << bb->label << std::endl;
        //print(sigma_prime);
    }
    return sigma_prime;
}