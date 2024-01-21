#include "abstract_store.hpp"
#include "datatypes.h"

/*
 * Execute a given BasicBlock against a given AbstractStore. This is a helper
 * function to be used in the MFP worklist algorithm. I figure this is important
 * and complicated enough to be given its own file, but we can refactor later as
 * needed.
 */
AbstractStore execute(BasicBlock *bb, AbstractStore sigma) {

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
        if (dynamic_cast<ArithInstruction*>(*inst) != nullptr) {

            /*
             * Cast it.
             */
            ArithInstruction *arith_inst = dynamic_cast<ArithInstruction*>(*inst);
            std::cout << "Executing ArithInstruction: " << std::endl;
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
                op1 = sigma.GetValFromStore(arith_inst->op1->var->name);
            }
            if (arith_inst->op2->IsConstInt()) {
                op2 = arith_inst->op2->val;
            }
            else {
                op2 = sigma.GetValFromStore(arith_inst->op2->var->name);
            }
            // TODO: What about division by zero? - it is undefined right so BOTTOM?
            if (std::holds_alternative<int>(op1) && std::holds_alternative<int>(op2))
            {
                int op1_val = std::get<int>(op1);
                int op2_val = std::get<int>(op2);
                if (arith_inst->arith_op == "add")
                {
                    sigma_prime.abstract_store[arith_inst->lhs->name] = op1_val + op2_val;
                }
                else if (arith_inst->arith_op == "sub")
                {
                    sigma_prime.abstract_store[arith_inst->lhs->name] = op1_val - op2_val;
                }
                else if (arith_inst->arith_op == "mul")
                {
                    sigma_prime.abstract_store[arith_inst->lhs->name] = op1_val * op2_val;
                }
                else if (arith_inst->arith_op == "div")
                {
                    sigma_prime.abstract_store[arith_inst->lhs->name] = op1_val / op2_val;
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

        } else if (dynamic_cast<CmpInstruction*>(*inst) != nullptr) {

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
                op1 = sigma.GetValFromStore(cmp_inst->op1->var->name);
            }
            if (cmp_inst->op2->IsConstInt()) {
                op2 = cmp_inst->op2->val;
            }
            else {
                op2 = sigma.GetValFromStore(cmp_inst->op2->var->name);
            }
            // TODO: What about division by zero? - it is undefined right so BOTTOM?
            if (std::holds_alternative<int>(op1) && std::holds_alternative<int>(op2))
            {
                int op1_val = std::get<int>(op1);
                int op2_val = std::get<int>(op2);
                if (cmp_inst->cmp_op == "eq")
                {
                    sigma_prime.abstract_store[cmp_inst->lhs->name] = (op1_val == op2_val);
                }
                else if (cmp_inst->cmp_op == "neq")
                {
                    sigma_prime.abstract_store[cmp_inst->lhs->name] = (op1_val != op2_val);
                }
                else if (cmp_inst->cmp_op == "lt")
                {
                    sigma_prime.abstract_store[cmp_inst->lhs->name] = (op1_val < op2_val);
                }
                else if (cmp_inst->cmp_op == "lte")
                {
                    sigma_prime.abstract_store[cmp_inst->lhs->name] = (op1_val <= op2_val);
                }
                else if (cmp_inst->cmp_op == "gt")
                {
                    sigma_prime.abstract_store[cmp_inst->lhs->name] = (op1_val > op2_val);
                }
                else if (cmp_inst->cmp_op == "gte")
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

        } else if (dynamic_cast<CopyInstruction*>(*inst) != nullptr) {

            /*
             * Cast it.
             */
            CopyInstruction *copy_inst = dynamic_cast<CopyInstruction*>(*inst);

        } else if (dynamic_cast<BranchInstruction*>(*inst) != nullptr) {

            /*
             * Cast it.
             */
            BranchInstruction *branch_inst = dynamic_cast<BranchInstruction*>(*inst);

        } else if (dynamic_cast<JumpInstruction*>(*inst) != nullptr) {

            /*
             * Cast it.
             */
            JumpInstruction *jump_inst = dynamic_cast<JumpInstruction*>(*inst);

        }
    }
    return sigma_prime;
}