#pragma once

#include "abstract_store.hpp"
#include "datatypes.h"
#include<queue>
#include<variant>

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
        std::map<std::string, AbstractStore> &bb2store,
        std::queue<std::string> &worklist,
        std::unordered_set<std::string> addr_of_int_types
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
             * If the lhs isn't an int-typed variable, ignore instruction.
             */
            if (!copy_inst->lhs->isIntType()) {
                continue;
            }

            /*
             * Copy over the value. We can just do a simple integer copy because
             * of our constant domain. If we had some other domain, we would
             * have an alpha function here.
             */
            std::variant<int, AbstractVal> op;
            if (copy_inst->op->IsConstInt()) {
                op = copy_inst->op->val;
            }
            else {
                op = sigma_prime.GetValFromStore(copy_inst->op->var->name);
            }
            sigma_prime.abstract_store[copy_inst->lhs->name] = op;

        } else if ((*inst)->instrType == InstructionType::BranchInstrType) {
            /*
             * Cast it.
             */
            BranchInstruction *branch_inst = dynamic_cast<BranchInstruction*>(*inst);

            /*
             * If op is not 0, go to bb1. Otherwise, go to bb2. If op is TOP, then
             * propagate to both.
             */
            if (branch_inst->condition->IsConstInt()) {
                if (branch_inst->condition->val != 0) {
                    worklist.push(branch_inst->tt);
                } else {
                    worklist.push(branch_inst->ff);
                }
            }
            else {
                // TODO: Check if the bb has to be pushed to worklist even if the store doesn't change
                std::variant<int,AbstractVal> absVal = sigma_prime.GetValFromStore(branch_inst->condition->var->name);
                if (std::holds_alternative<AbstractVal>(absVal) && std::get<AbstractVal>(absVal) == AbstractVal::TOP) {
                    worklist.push(branch_inst->tt);
                    worklist.push(branch_inst->ff);
                }
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
            if (bb2store[jump_inst->label].join(bb2store[bb->label]))
            {
                // If the basic block's abstract store changed, add the basic block to the worklist
                worklist.push(jump_inst->label);
            }
        } else if ((*inst)->instrType == InstructionType::LoadInstrType) {
            /*
             * Cast it.
             */
            LoadInstruction *load_inst = dynamic_cast<LoadInstruction*>(*inst);

            /*
             * If the lhs isn't an int-typed variable, ignore instruction.
             */
            if (!load_inst->lhs->isIntType()) {
                continue;
            }
            /*
             * We don't know what the value of the rhs is, so we just set the
             * lhs to TOP.
            */
            sigma_prime.abstract_store[load_inst->lhs->name] = AbstractVal::TOP;
        } else if ((*inst)->instrType == InstructionType::StoreInstrType) {
            /*
             * Cast it.
             */
            StoreInstruction *store_inst = dynamic_cast<StoreInstruction*>(*inst);
            /*
             * If the lhs isn't an int-typed variable, ignore instruction.
             */
            if (!store_inst->dst->isIntType()) {
                continue;
            }

            // Get abstract domain value from op
            std::variant<int, AbstractVal> op;
            if (store_inst->op->IsConstInt()) {
                op = store_inst->op->val;
            }
            else {
                op = sigma_prime.GetValFromStore(store_inst->op->var->name);
            }
            // For every entry in addr-of-ints, join with op value to get new sigma_prime
            for(auto addr_of_int : addr_of_int_types) {
                AbstractStore opStore = AbstractStore();
                opStore.abstract_store[addr_of_int] = op;
                sigma_prime.join(opStore);
            }
        } else if ((*inst)->instrType == InstructionType::CallDirInstrType)
            {
                // For all ints in globals_ints, update sigma_primt to TOP
                // TODO: Ignoring global variables for assignment 1

                CallDirInstruction *call_inst = dynamic_cast<CallDirInstruction*>(*inst);
                // If function returns something and it is of int type, update sigma_prime to TOP
                if (call_inst->lhs && call_inst->lhs->isIntType()) {
                    sigma_prime.abstract_store[call_inst->lhs->name] = AbstractVal::TOP;
                }

                // If any argument is a pointer to an int then for all variables in addr_of_int_types, update sigma_prime to TOP
                for (auto arg : call_inst->args) {
                    if (arg->var && arg->var->type->indirection > 0 && arg->var->type->type == DataType::IntType){
                        for(auto addr_of_int : addr_of_int_types) {
                            sigma_prime.abstract_store[addr_of_int] = AbstractVal::TOP;
                        }
                    }
                }

                // If abstract store of next_bb has changed, push it into worklist
                if (bb2store[call_inst->next_bb].join(sigma_prime)) {
                    worklist.push(call_inst->next_bb);
                }
            } else if ((*inst)->instrType == InstructionType::CallIdrInstrType ) {
                CallIdrInstruction *call_inst = dynamic_cast<CallIdrInstruction*>(*inst);
                // If function returns something and it is of int type, update sigma_prime to TOP
                if (call_inst && call_inst->lhs && call_inst->lhs->isIntType()) {
                    sigma_prime.abstract_store[call_inst->lhs->name] = AbstractVal::TOP;
                }

                // If any argument is a pointer to an int then for all variables in addr_of_int_types, update sigma_prime to TOP
                for (auto arg : call_inst->args) {
                    if (arg->var && arg->var->type->indirection > 0 && arg->var->type->type == DataType::IntType){
                        for(auto addr_of_int : addr_of_int_types) {
                            sigma_prime.abstract_store[addr_of_int] = AbstractVal::TOP;
                        }
                    }
                }

                // If abstract store of next_bb has changed, push it into worklist
                if (bb2store[call_inst->next_bb].join(sigma_prime)) {
                    worklist.push(call_inst->next_bb);
                }
            }
            else if ((*inst)->instrType == InstructionType::CallExtInstrType) {
                CallExtInstruction *call_inst = dynamic_cast<CallExtInstruction*>(*inst);
                // If function returns something and it is of int type, update sigma_prime to TOP
                if (call_inst && call_inst->lhs && call_inst->lhs->isIntType()) {
                    sigma_prime.abstract_store[call_inst->lhs->name] = AbstractVal::TOP;
                }

                // If any argument is a pointer to an int then for all variables in addr_of_int_types, update sigma_prime to TOP
                for (auto arg : call_inst->args) {
                    if (arg->var && arg->var->type->indirection > 0 && arg->var->type->type == DataType::IntType){
                        for(auto addr_of_int : addr_of_int_types) {
                            sigma_prime.abstract_store[addr_of_int] = AbstractVal::TOP;
                        }
                    }
                }
            }
            else {
                /*
                * This is a catch-all for instructions we don't have to do anything about for constant analysis.
                */
                continue;
            }
    }
    return sigma_prime;
}