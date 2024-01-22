#pragma once

#include "abstract_store.hpp"
#include "datatypes.h"

/*
 * Execute a given BasicBlock against a given AbstractStore. This is a helper
 * function to be used in the MFP worklist algorithm. I figure this is important
 * and complicated enough to be given its own file, but we can refactor later as
 * needed.
 *
 * Our bb2store is an argument because the $jmp and $branch instructions involve
 * updating the bb2store. Same with the worklist.
 */
AbstractStore execute(
        BasicBlock *bb,
        AbstractStore sigma,
        std::map<std::string,
        AbstractStore> &bb2store,
        std::queue<std::string> &worklist
        ) {

    /*
     * Make a copy of sigma that we'll return at the end of this function.
     */
    AbstractStore sigma_prime = sigma;

    /*
     * Iterate through each instruction in bb.
     */
    for (auto inst = bb->instructions.begin(); inst != bb->instructions.end(); ++inst) {

        /*
         * This is the weirdest way of checking the instruction type ever, but
         * it works for now. I would like to figure out a more elegant solution.
         *
         * TODO These are only the instructions needed for the very first test
         * TODO suite. We will need to add more later.
         */
        if ((*inst)->instrType == InstructionType::ArithInstrType) {

            /*
             * Cast it.
             */
            ArithInstruction *arith_inst = dynamic_cast<ArithInstruction*>(*inst);
            /*
             * Since $arith is only done on ints, we know that op1 and op2 are ints
             * the operands can be int typed variables or direct int constants
             */
            std::variant<int, AbstractVal> op1;
            std::variant<int, AbstractVal> op2;
            int op1_val, op2_val;
            if (arith_inst->op1->IsConstInt()) {
                op1 = arith_inst->op1->val;
            }
            else {
                op1 = sigma_prime.GetValFromStore(arith_inst->op1->var->name);
            }
            if (arith_inst->op2->IsConstInt()) {
                op2 = arith_inst->op2->val;
            }
            else {
                op2 = sigma_prime.GetValFromStore(arith_inst->op2->var->name);
            }

            if (std::holds_alternative<int>(op1) && std::holds_alternative<int>(op2))
            {
                int op1_val = std::get<int>(op1);
                int op2_val = std::get<int>(op2);
                if (arith_inst->arith_op == "Add")
                {
                    sigma_prime.abstract_store[arith_inst->lhs->name] = op1_val + op2_val;
                }
                else if (arith_inst->arith_op == "Subtract")
                {
                    sigma_prime.abstract_store[arith_inst->lhs->name] = op1_val - op2_val;
                }
                else if (arith_inst->arith_op == "Multiply")
                {
                    sigma_prime.abstract_store[arith_inst->lhs->name] = op1_val * op2_val;
                }
                else if (arith_inst->arith_op == "Divide")
                {
                    if (op2_val == 0) {
                        if (sigma_prime.abstract_store.count(arith_inst->lhs->name) != 0) {
                            sigma_prime.abstract_store.erase(arith_inst->lhs->name);
                        }
                    }
                    else {
                        sigma_prime.abstract_store[arith_inst->lhs->name] = (int)(op1_val / op2_val);
                    }
                }
            }
            else if ((std::holds_alternative<AbstractVal>(op1) && std::get<AbstractVal>(op1) == AbstractVal::BOTTOM) || 
                    (std::holds_alternative<AbstractVal>(op2) && std::get<AbstractVal>(op2) == AbstractVal::BOTTOM))
            {
                // No-op since the result will always be BOTTOM
            }
            /*
            * This means that either op1 or op2 is TOP and neither of them is BOTTOM => result is TOP
            */
            else
            {
                sigma_prime.abstract_store[arith_inst->lhs->name] = AbstractVal::TOP;
            } 

        } else if ((*inst)->instrType == InstructionType::CmpInstrType) {

            /*
             * Cast it.
             */
            CmpInstruction *cmp_inst = dynamic_cast<CmpInstruction*>(*inst);
            /*
             * Since $cmp is only done on ints, we know that op1 and op2 are ints
             * the operands can be int typed variables or direct int constants
             */
            std::variant<int, AbstractVal> op1;
            std::variant<int, AbstractVal> op2;
            int op1_val, op2_val;
            if (cmp_inst->op1->IsConstInt()) {
                op1 = cmp_inst->op1->val;
            }
            else {
                op1 = sigma_prime.GetValFromStore(cmp_inst->op1->var->name);
            }
            if (cmp_inst->op2->IsConstInt()) {
                op2 = cmp_inst->op2->val;
            }
            else {
                op2 = sigma_prime.GetValFromStore(cmp_inst->op2->var->name);
            }

            if (std::holds_alternative<int>(op1) && std::holds_alternative<int>(op2))
            {
                int op1_val = std::get<int>(op1);
                int op2_val = std::get<int>(op2);
                if (cmp_inst->cmp_op == "Eq")
                {
                    sigma_prime.abstract_store[cmp_inst->lhs->name] = (op1_val == op2_val);
                }
                else if (cmp_inst->cmp_op == "Neq")
                {
                    sigma_prime.abstract_store[cmp_inst->lhs->name] = (op1_val != op2_val);
                }
                else if (cmp_inst->cmp_op == "Less")
                {
                    sigma_prime.abstract_store[cmp_inst->lhs->name] = (op1_val < op2_val);
                }
                else if (cmp_inst->cmp_op == "LessEq")
                {
                    sigma_prime.abstract_store[cmp_inst->lhs->name] = (op1_val <= op2_val);
                }
                else if (cmp_inst->cmp_op == "Greater")
                {
                    sigma_prime.abstract_store[cmp_inst->lhs->name] = (op1_val > op2_val);
                }
                else if (cmp_inst->cmp_op == "GreaterEq")
                {
                    sigma_prime.abstract_store[cmp_inst->lhs->name] = (op1_val >= op2_val);
                }
            }
            else if ((std::holds_alternative<AbstractVal>(op1) && std::get<AbstractVal>(op1) == AbstractVal::BOTTOM) || 
                    (std::holds_alternative<AbstractVal>(op2) && std::get<AbstractVal>(op2) == AbstractVal::BOTTOM))
            {
                // No-op since the result will always be BOTTOM
            }
            /*
            * This means that either op1 or op2 is TOP and neither of them is BOTTOM => result is TOP
            */
            else
            {
                sigma_prime.abstract_store[cmp_inst->lhs->name] = AbstractVal::TOP;
            } 

        } else if ((*inst)->instrType == InstructionType::CopyInstrType) {

            /*
             * Cast it.
             */
            CopyInstruction *copy_inst = dynamic_cast<CopyInstruction*>(*inst);

            /*
             * If the lhs isn't an int-typed variable, return immediately.
             */
            if (!copy_inst->lhs->isIntType()) {
                return sigma_prime;
            }

            /*
             * Copy over the value. We can just do a simple integer copy because
             * of our constant domain. If we had some other domain, we would
             * have an alpha function here.
             */
            sigma_prime.abstract_store[copy_inst->lhs->name] = copy_inst->op->val;

        } else if ((*inst)->instrType == InstructionType::BranchInstrType) {

            /*
             * Cast it.
             */
            BranchInstruction *branch_inst = dynamic_cast<BranchInstruction*>(*inst);

            /*
             * If op is 0, go to bb1. Otherwise, go to bb2. If op is TOP, then
             * propagate to both.
             */
            if (branch_inst->condition->IsConstInt() && branch_inst->condition->val == 0) {
                worklist.push(branch_inst->tt);
            } else if (branch_inst->condition->IsConstInt() && branch_inst->condition->val != 0) {
                worklist.push(branch_inst->ff);
            } else {
                worklist.push(branch_inst->tt);
                worklist.push(branch_inst->ff);
            }
        } else if ((*inst)->instrType == InstructionType::JumpInstrType) {

            /*
             * Cast it.
             */
            JumpInstruction *jump_inst = dynamic_cast<JumpInstruction*>(*inst);

            /*
             * Join sigma_prime with the basic block's abstract store (updating
             * the basic block's abstract store).
             */
            bb2store[jump_inst->label].join(bb2store[bb->label]);

            /*
             * Push the basic block referred to by the $jmp instruction onto the
             * worklist.
             */
            worklist.push(jump_inst->label);
        }
    }
    return sigma_prime;
}