#pragma once

#include "abstract_store.hpp"
#include "datatypes.h"
#include <queue>
#include <variant>

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
        std::deque<std::string> &worklist,
        std::unordered_set<std::string> addr_of_int_types,
        std::set<std::string> bbs_to_output) {

    /*
     * Make a copy of sigma that we'll return at the end of this function.
     */
    AbstractStore sigma_prime = sigma;

    /*
     * Iterate through each instruction in bb.
     *
     * TODO I guess we shouldn't care about terminals?
     */
    for (const Instruction *inst : bb->instructions) {

        //std::cout << "This is the instruction type: " << inst->instrType << std::endl;

        if ((*inst).instrType == InstructionType::ArithInstrType) {

            /*
             * Cast it.
             */
            ArithInstruction *arith_inst = (ArithInstruction *) inst;

            /*
             * Since $arith is only done on ints, we know that op1 and op2 are ints
             * the operands can be int typed variables or direct int constants
             */
            std::variant<int, AbstractVal> op1;
            std::variant<int, AbstractVal> op2;

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
            // If either op1 or op2 is 0, then the result is 0 for multiply and divide
            else if ((arith_inst->arith_op == "Multiply" || arith_inst->arith_op == "Divide") && 
            ((arith_inst->op1->IsConstInt() && arith_inst->op1->val == 0) || 
            (arith_inst->op2->IsConstInt() && arith_inst->op2->val == 0) ||
            (std::holds_alternative<int>(op1) && std::get<int>(op1) == 0) ||
            (std::holds_alternative<int>(op2) && std::get<int>(op2) == 0)))
            {
                sigma_prime.abstract_store[arith_inst->lhs->name] = 0;
            }
            /*
            * This means that either op1 or op2 is TOP and neither of them is BOTTOM => result is TOP
            */
            else
            {
                sigma_prime.abstract_store[arith_inst->lhs->name] = AbstractVal::TOP;
            } 

        } else if ((*inst).instrType == InstructionType::CmpInstrType) {
            
            CmpInstruction *cmp_inst = (CmpInstruction *) inst;
            //std::cout << "Inside $cmp" << std::endl;
            if ((cmp_inst->op1->var && !(cmp_inst->op1->var->isIntType())) || (cmp_inst->op2->var && !(cmp_inst->op2->var->isIntType()))) {
                sigma_prime.abstract_store[cmp_inst->lhs->name] = AbstractVal::TOP;
                //std::cout << cmp_inst->op1->var->name << std::endl;
            }
            else {
            
            std::variant<int, AbstractVal> op1;
            std::variant<int, AbstractVal> op2;
            
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
                if (cmp_inst->cmp_op == "Eq") {
                    sigma_prime.abstract_store[cmp_inst->lhs->name] = (op1_val == op2_val);
                }
                else if (cmp_inst->cmp_op == "Neq") {
                    sigma_prime.abstract_store[cmp_inst->lhs->name] = (op1_val != op2_val);
                }
                else if (cmp_inst->cmp_op == "Less") {
                    sigma_prime.abstract_store[cmp_inst->lhs->name] = (op1_val < op2_val);
                }
                else if (cmp_inst->cmp_op == "LessEq") {
                    sigma_prime.abstract_store[cmp_inst->lhs->name] = (op1_val <= op2_val);
                }
                else if (cmp_inst->cmp_op == "Greater") {
                    sigma_prime.abstract_store[cmp_inst->lhs->name] = (op1_val > op2_val);
                }
                else if (cmp_inst->cmp_op == "GreaterEq") {
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
            } }

        } else if ((*inst).instrType == InstructionType::CopyInstrType) {

            //std::cout << "Encountered $copy" << std::endl;

            /*
             * Cast it.
             */
            CopyInstruction *copy_inst = (CopyInstruction *) inst;
            //std::cout << copy_inst->lhs->name << std::endl;

            /*
             * If the lhs isn't an int-typed variable, ignore instruction.
             */
            if (!copy_inst->lhs->isIntType()) {
                continue;
            }

            //std::cout << "lhs is definitely int-typed" << std::endl;
            //copy_inst->pretty_print();

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
            if (std::holds_alternative<AbstractVal>(op) && std::get<AbstractVal>(op) == AbstractVal::BOTTOM)
            {
                if (sigma_prime.abstract_store.count(copy_inst->lhs->name) != 0) {
                    sigma_prime.abstract_store.erase(copy_inst->lhs->name);
                }
            }
            else {
                sigma_prime.abstract_store[copy_inst->lhs->name] = op;
            }
        } else if ((*inst).instrType == InstructionType::LoadInstrType) {
            /*
             * Cast it.
             */
            LoadInstruction *load_inst = (LoadInstruction *) inst;

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
        } else if ((*inst).instrType == InstructionType::StoreInstrType) {
            /*
             * Cast it.
             */
            StoreInstruction *store_inst = (StoreInstruction *) inst;
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
        }
            else if ((*inst).instrType == InstructionType::CallExtInstrType) {
                CallExtInstruction *call_inst = (CallExtInstruction *) inst;
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
                    // Break out of the loop once we've set all addr_of_int_types to TOP once
                    break;
                }
            }
            else {

                //std::cout << "Found an instruction we don't recognize :(" << std::endl;
                /*
                * This is a catch-all for instructions we don't have to do anything about for constant analysis.
                */
                continue;
            }
    }

    /*
     * Ok, now we've executed the instructions. Now let's look at the terminal
     * to see what to push onto the worklist and such.
     */

    Instruction *terminal_instruction = bb->terminal;
    if (terminal_instruction->instrType == InstructionType::BranchInstrType) {

        //std::cout << "Breakpoint 5?" << std::endl;

        /*
         * Cast it.
         */
        BranchInstruction *branch_inst = (BranchInstruction *) terminal_instruction;


        /*
             * If op is not 0, go to bb1. Otherwise, go to bb2. If op is TOP, then
             * propagate to both.
             */
        if (branch_inst->condition->IsConstInt()) {
            if (branch_inst->condition->val != 0) {
                bool store_changed_tt = bb2store[branch_inst->tt].join(sigma_prime);
                // If store has changed or this is the first time the block is being visited, push it into worklist
                if (store_changed_tt || bbs_to_output.count(branch_inst->tt) == 0)
                    worklist.push_back(branch_inst->tt);
            } else {
                bool store_changed_ff = bb2store[branch_inst->ff].join(sigma_prime);
                if (store_changed_ff || bbs_to_output.count(branch_inst->ff) == 0)
                    worklist.push_back(branch_inst->ff);
            }
        }
        else {
            std::variant<int,AbstractVal> absVal = sigma_prime.GetValFromStore(branch_inst->condition->var->name);
            if (std::holds_alternative<AbstractVal>(absVal) && std::get<AbstractVal>(absVal) == AbstractVal::TOP){
                    bool store_changed_tt = bb2store[branch_inst->tt].join(sigma_prime);
                    bool store_changed_ff = bb2store[branch_inst->ff].join(sigma_prime);

                    if (store_changed_tt || bbs_to_output.count(branch_inst->tt) == 0)
                        worklist.push_back(branch_inst->tt);
                    if (store_changed_ff || bbs_to_output.count(branch_inst->ff) == 0)
                        worklist.push_back(branch_inst->ff);

            }
            else if (std:: holds_alternative<int>(absVal))
            {
                if (std::get<int>(absVal) != 0) {
                    bool store_changed_tt = bb2store[branch_inst->tt].join(sigma_prime);
                    if (store_changed_tt || bbs_to_output.count(branch_inst->tt) == 0)
                        worklist.push_back(branch_inst->tt);
                } else {
                    bool store_changed_ff = bb2store[branch_inst->ff].join(sigma_prime);
                    if (store_changed_ff || bbs_to_output.count(branch_inst->ff) == 0)
                        worklist.push_back(branch_inst->ff);
                }
            }
        }
    } else if (terminal_instruction->instrType == InstructionType::JumpInstrType) {
        // std::cout << "Encountered $jump" << std::endl;

        /*
             * Cast it.
             */
        JumpInstruction *jump_inst = (JumpInstruction *) terminal_instruction;

        /*
         * Join sigma_prime with the basic block's abstract store (updating
         * the basic block's abstract store).
         */
            bool store_changed = bb2store[jump_inst->label].join(sigma_prime);

            if (store_changed || bbs_to_output.count(jump_inst->label) == 0)
            {
                // If the basic block's abstract store changed, add the basic block to the worklist
                worklist.push_back(jump_inst->label);
            }
    } else if (terminal_instruction->instrType == InstructionType::RetInstrType) {
        //std::cout << "Encountered $ret" << std::endl;

        /*
         * No-op. We don't have to do anything here.
         */
    } else if (terminal_instruction->instrType == InstructionType::CallDirInstrType) {
        //std::cout << "Encountered $call_dir" << std::endl;

        // For all ints in globals_ints, update sigma_primt to TOP
        // TODO: Ignoring global variables for assignment 1

        CallDirInstruction *call_inst = (CallDirInstruction *) terminal_instruction;
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
                // Break out of the loop once we've set all addr_of_int_types to TOP once
                break;
            }
        }

        // If abstract store of next_bb has changed, push it into worklist
        if (bb2store[call_inst->next_bb].join(sigma_prime) || bbs_to_output.count(call_inst->next_bb) == 0) {
            worklist.push_back(call_inst->next_bb);
        }

    } else if (terminal_instruction->instrType == InstructionType::CallIdrInstrType ) {
            CallIdrInstruction *call_inst = (CallIdrInstruction *) terminal_instruction;
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
                    // Break out of the loop once we've set all addr_of_int_types to TOP once
                    break;
                }
            }

            // If abstract store of next_bb has changed, push it into worklist
            if (bb2store[call_inst->next_bb].join(sigma_prime) || bbs_to_output.count(call_inst->next_bb) == 0) {
                worklist.push_back(call_inst->next_bb);
            }
        }
    else {
        std::cout << "Found a terminal we don't recognize :(" << std::endl;
        /*
         * This is a catch-all for instructions we don't have to do anything about for constant analysis.
         */
    }
    return sigma_prime;
}