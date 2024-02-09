#pragma once

#include<map>
#include<set>
#include<iostream>
#include "../headers/datatypes.h"
#include <deque>
#include <unordered_set>
#include <unordered_map>
#include "rtype.hpp"

/*
 * Join is a union of the two abstract stores where each abstract store is a map of variable name to a set of pp where they are defined.
*/
bool joinAbsStore(std::map<std::string, std::set<std::string>>&curr_abs_store, std::map<std::string, std::set<std::string>>parent_bb_abs_store) {
    
    bool changed = false;
    std::map<std::string, std::set<std::string>> result_set;
    
    // For elements in curr_abs_store, do a set union with the parent_bb_abs_store
    // This also handles the case when the key is not present in parent_bb_abs_store but present in curr_abs_store
    for(auto it = curr_abs_store.begin(); it != curr_abs_store.end(); it++) {
        std::set<std::string> result_set;
        std::set_union(it->second.begin(), it->second.end(), parent_bb_abs_store[it->first].begin(), parent_bb_abs_store[it->first].end(), std::inserter(result_set, result_set.begin()));
        // We do not want to unset changed to false if alerady set to true
        if (!changed)
            changed = (result_set.size() != it->second.size());
        it->second = result_set;
    }

    // For elements in parent_bb_abs_store, if they are not in curr_abs_store, add them to curr_abs_store
    for(auto it = parent_bb_abs_store.begin(); it != parent_bb_abs_store.end(); it++) {
        if(curr_abs_store.count(it->first) == 0) {
            curr_abs_store[it->first] = it->second;
            changed = true;
        }
    }
    return changed;
}

bool joinSets(std::set<std::string> &s1, std::set<std::string> &s2) {
    bool changed = false;
    std::set<std::string> result_set;
    
    std::set_union(s1.begin(), s1.end(), s2.begin(), s2.end(), std::inserter(result_set, result_set.begin()));
    
    changed = (result_set.size() != s1.size());
    s1 = result_set;
    return changed;
}

void execute(
    Program *program,
    BasicBlock *bb,
    std::map<std::string, std::map<std::string, std::set<std::string>>> &bb2store,
    std::deque<std::string> &worklist,
    std::unordered_set<Variable*> &addr_taken,
    std::set<std::string> &bbs_to_output,
    std::map<std::string, std::set<std::string>> &soln,
    bool execute_final = false
)
{
    std::map<std::string, std::set<std::string>> sigma_prime = bb2store[bb->label];

    /*
     * We define DEF for definitions of the variable and USE for the uses of the variable.
     * DEF = set of variables
     * USE = set of variables
    */
    
    int index = 0; // To help build program point name

    /*
     * Iterate through each instruction in bb.
     */
    for (const Instruction *inst : bb->instructions) {

        std::string pp = bb->label + "." + std::to_string(index);
        //std::cout << "pp: " << pp << std::endl;

        // arith, cmp, alloc, copy, gep, gfp work the same way
        if ((*inst).instrType == InstructionType::ArithInstrType) {

            /*
             * Cast it.
             */
             ArithInstruction *arith_inst = (ArithInstruction *) inst;
             std::set<Variable*> DEF;
             std::set<Variable*> USE;

            /*
             * x = $arith add y z
             * DEF = {x}
             * USE = { op | op is a variable }
             * for all v in USE: soln[pp] = soln[pp] U sigma_prime[v]
             * sigma_prime[x] = { pp }
            */
            DEF.insert(arith_inst->lhs);
            if (!arith_inst->op1->IsConstInt())
                USE.insert(arith_inst->op1->var);
            if (!arith_inst->op2->IsConstInt())
                USE.insert(arith_inst->op2->var);

            if (execute_final) {
                /*std::cout << "pp: " << pp << " Arith inst " << std::endl;
                std::cout << "Before joining soln[pp]: " << std::endl;
                for(auto it = soln[pp].begin(); it != soln[pp].end(); it++) {
                    std::cout << *it << " ";
                }
                std::cout << "USE: " <<std::endl;
                for (Variable *v : USE) {
                    std::cout << "Joining for variable: " << v->name << std::endl;
                    std::cout << "Sigma prime " << std::endl;
                    for (auto it = sigma_prime[v->name].begin(); it != sigma_prime[v->name].end(); it++) {
                        std::cout << *it << " ";
                    }
                    std::cout << std::endl;
                }*/

                for (Variable *v : USE) {
                    // soln[pp] = soln[pp] U sigma_prime[v]
                    joinSets(soln[pp], sigma_prime[v->name]);
                }

                /*std::cout << "After joining soln[pp]: " << std::endl;
                for(auto it = soln[pp].begin(); it != soln[pp].end(); it++) {
                    std::cout << *it << " ";
                }
                std::cout << std::endl;*/
            }

            sigma_prime[arith_inst->lhs->name] = {pp};
        }
        else if ((*inst).instrType == InstructionType::CmpInstrType)
        {
            /*
             * Cast it.
             */
             CmpInstruction *cmp_inst = (CmpInstruction *) inst;
             std::set<Variable*> DEF;
             std::set<Variable*> USE;
            /*
             * x = $cmp gt y z
             * DEF = {x}
             * USE = { op | op is a variable }
             * for all v in USE: soln[pp] = soln[pp] U sigma_prime[v]
             * sigma_prime[x] = { pp }
            */
            DEF.insert(cmp_inst->lhs);
            if (!cmp_inst->op1->IsConstInt())
                USE.insert(cmp_inst->op1->var);
            if (!cmp_inst->op2->IsConstInt())
                USE.insert(cmp_inst->op2->var);

            if (execute_final) {
                /*std::cout << "pp: " << pp << " Cmp inst " << std::endl;
                std::cout << "Before joining soln[pp]: " << std::endl;
                for(auto it = soln[pp].begin(); it != soln[pp].end(); it++) {
                    std::cout << *it << " ";
                }
                std::cout << "USE: " <<std::endl;
                for (Variable *v : USE) {
                    std::cout << "Joining for variable: " << v->name << std::endl;
                    std::cout << "Sigma prime " << std::endl;
                    for (auto it = sigma_prime[v->name].begin(); it != sigma_prime[v->name].end(); it++) {
                        std::cout << *it << " ";
                    }
                    std::cout << std::endl;
                }*/

                for (Variable *v : USE) {
                    // soln[pp] = soln[pp] U sigma_prime[v]
                    joinSets(soln[pp], sigma_prime[v->name]);
                }

                /*std::cout << "After joining soln[pp]: " << std::endl;
                for(auto it = soln[pp].begin(); it != soln[pp].end(); it++) {
                    std::cout << *it << " ";
                }
                std::cout << std::endl;*/
            }

            sigma_prime[cmp_inst->lhs->name] = {pp};
        }
        else if ((*inst).instrType == InstructionType::CopyInstrType)
        {
            /*
             * Cast it.
             */
             CopyInstruction *copy_inst = (CopyInstruction *) inst;
             std::set<Variable*> DEF;
             std::set<Variable*> USE;
            /*
             * x = $copy y
             * DEF = {x}
             * USE = { op | op is a variable }
             * for all v in USE: soln[pp] = soln[pp] U sigma_prime[v]
             * sigma_prime[x] = { pp }
            */
            DEF.insert(copy_inst->lhs);
            if (!copy_inst->op->IsConstInt())
                USE.insert(copy_inst->op->var);

            if (execute_final) {
                /*std::cout << "pp: " << pp << " Copy inst " << std::endl;
                std::cout << "Before joining soln[pp]: " << std::endl;
                for(auto it = soln[pp].begin(); it != soln[pp].end(); it++) {
                    std::cout << *it << " ";
                }
                std::cout << "USE: " <<std::endl;
                for (Variable *v : USE) {
                    std::cout << "Joining for variable: " << v->name << std::endl;
                    std::cout << "Sigma prime " << std::endl;
                    for (auto it = sigma_prime[v->name].begin(); it != sigma_prime[v->name].end(); it++) {
                        std::cout << *it << " ";
                    }
                    std::cout << std::endl;
                }*/

                for (Variable *v : USE) {
                    // soln[pp] = soln[pp] U sigma_prime[v]
                    joinSets(soln[pp], sigma_prime[v->name]);
                }

                /*std::cout << "After joining soln[pp]: " << std::endl;
                for(auto it = soln[pp].begin(); it != soln[pp].end(); it++) {
                    std::cout << *it << " ";
                }
                std::cout << std::endl;*/
            }

            sigma_prime[copy_inst->lhs->name] = {pp};
        }
        else if ((*inst).instrType == InstructionType::AllocInstrType)
        {
            /*
             * Cast it.
             */
             AllocInstruction *alloc_inst = (AllocInstruction *) inst;
             std::set<Variable*> DEF;
             std::set<Variable*> USE;
            /*
             * x = $alloc y [id]
             * DEF = {x}
             * USE = { op | op is a variable }
             * for all v in USE: soln[pp] = soln[pp] U sigma_prime[v]
             * sigma_prime[x] = { pp }
            */
            DEF.insert(alloc_inst->lhs);
            if (!alloc_inst->num->IsConstInt())
                USE.insert(alloc_inst->num->var);

            if (execute_final) {
                /*std::cout << "pp: " << pp << " Alloc inst " << std::endl;
                std::cout << "Before joining soln[pp]: " << std::endl;
                for(auto it = soln[pp].begin(); it != soln[pp].end(); it++) {
                    std::cout << *it << " ";
                }
                std::cout << "USE: " <<std::endl;
                for (Variable *v : USE) {
                    std::cout << "Joining for variable: " << v->name << std::endl;
                    std::cout << "Sigma prime " << std::endl;
                    for (auto it = sigma_prime[v->name].begin(); it != sigma_prime[v->name].end(); it++) {
                        std::cout << *it << " ";
                    }
                    std::cout << std::endl;
                }*/

                for (Variable *v : USE) {
                    // soln[pp] = soln[pp] U sigma_prime[v]
                    joinSets(soln[pp], sigma_prime[v->name]);
                }

                /*std::cout << "After joining soln[pp]: " << std::endl;
                for(auto it = soln[pp].begin(); it != soln[pp].end(); it++) {
                    std::cout << *it << " ";
                }
                std::cout << std::endl;*/
            }

            sigma_prime[alloc_inst->lhs->name] = {pp};
        }
        else if ((*inst).instrType == InstructionType::GepInstrType)
        {
            /*
             * Cast it.
             */
             GepInstruction *gep_inst = (GepInstruction *) inst;
             std::set<Variable*> DEF;
             std::set<Variable*> USE;
            /*
             * x = $gep id op
             * DEF = {x}
             * USE = { op | op is a variable }
             * for all v in USE: soln[pp] = soln[pp] U sigma_prime[v]
             * sigma_prime[x] = { pp }
            */
            DEF.insert(gep_inst->lhs);
            if (!gep_inst->idx->IsConstInt())
                USE.insert(gep_inst->idx->var);
            // TODO - Confirm this with Ben
            USE.insert(gep_inst->src);

            if (execute_final) {
                /*std::cout << "pp: " << pp << " Gep inst " << std::endl;
                std::cout << "Before joining soln[pp]: " << std::endl;
                for(auto it = soln[pp].begin(); it != soln[pp].end(); it++) {
                    std::cout << *it << " ";
                }
                std::cout << "USE: " <<std::endl;
                for (Variable *v : USE) {
                    std::cout << "Joining for variable: " << v->name << std::endl;
                    std::cout << "Sigma prime " << std::endl;
                    for (auto it = sigma_prime[v->name].begin(); it != sigma_prime[v->name].end(); it++) {
                        std::cout << *it << " ";
                    }
                    std::cout << std::endl;
                }*/

                for (Variable *v : USE) {
                    // soln[pp] = soln[pp] U sigma_prime[v]
                    joinSets(soln[pp], sigma_prime[v->name]);
                }

                /*std::cout << "After joining soln[pp]: " << std::endl;
                for(auto it = soln[pp].begin(); it != soln[pp].end(); it++) {
                    std::cout << *it << " ";
                }
                std::cout << std::endl;*/
            }

            sigma_prime[gep_inst->lhs->name] = {pp};
        }
        else if ((*inst).instrType == InstructionType::GfpInstrType)
        {
            /*
             * Cast it.
             */
            // TODO - Check if gfp and gep require the id of variable types also included
             GfpInstruction *gfp_inst = (GfpInstruction *) inst;
             std::set<Variable*> DEF;
             std::set<Variable*> USE;
            /*
             * x = $gfp id id
             * DEF = {x}
             * USE = { op | op is a variable }
             * for all v in USE: soln[pp] = soln[pp] U sigma_prime[v]
             * sigma_prime[x] = { pp }
            */
            DEF.insert(gfp_inst->lhs);
            USE.insert(gfp_inst->src);
            //USE.insert(gfp_inst->field);
            
            if (execute_final) {
                /*std::cout << "pp: " << pp << " Gfp inst " << std::endl;
                std::cout << "Before joining soln[pp]: " << std::endl;
                for(auto it = soln[pp].begin(); it != soln[pp].end(); it++) {
                    std::cout << *it << " ";
                }
                std::cout << "USE: " <<std::endl;
                for (Variable *v : USE) {
                    std::cout << "Joining for variable: " << v->name << std::endl;
                    std::cout << "Sigma prime " << std::endl;
                    for (auto it = sigma_prime[v->name].begin(); it != sigma_prime[v->name].end(); it++) {
                        std::cout << *it << " ";
                    }
                    std::cout << std::endl;
                }*/


                for (Variable *v : USE) {
                    // soln[pp] = soln[pp] U sigma_prime[v]
                    joinSets(soln[pp], sigma_prime[v->name]);
                }

                /*std::cout << "After joining soln[pp]: " << std::endl;
                for(auto it = soln[pp].begin(); it != soln[pp].end(); it++) {
                    std::cout << *it << " ";
                }
                std::cout << std::endl;*/
            }

            sigma_prime[gfp_inst->lhs->name] = {pp};
        }
        else if ((*inst).instrType == InstructionType::AddrofInstrType)
        {
            /*
             * Cast it.
             */
             AddrofInstruction *addrof_inst = (AddrofInstruction *) inst;
             std::set<Variable*> DEF;
             std::set<Variable*> USE;
            /*
             * x = $addrof y
             * DEF = {x}
             * USE = {}
             * No update to soln required
             * sigma_prime[x] = { pp }
            */

            DEF.insert(addrof_inst->lhs);
            USE = {};

            sigma_prime[addrof_inst->lhs->name] = { pp };
        }
        else if ((*inst).instrType == InstructionType::LoadInstrType)
        {
            /*
             * Cast it.
             */
            LoadInstruction *load_inst = (LoadInstruction *) inst;
            std::set<Variable*> DEF;
            std::set<Variable*> USE;

            // TODO : Need to do extra handling for structs here
            /*
             * DEF = {x}
             * USE = {y} U { v in addr_taken | type(v) = type(x) }
             * for all v in USE: soln[pp] = soln[pp] U sigma_prime[v]
             * sigma_prime[x] = { pp }
            */
            DEF.insert(load_inst->lhs);
            USE.insert(load_inst->src);

            //std::cout << "Load instruction: " << load_inst->lhs->name << " = $load " << load_inst->src->name << std::endl;
            for (auto v : addr_taken) {
                if (Type::isEqualType(v->type, load_inst->lhs->type)) {
                    USE.insert(v);
                }
            }

            if (execute_final) {
                /*std::cout << "pp: " << pp << " Load inst " << std::endl;
                std::cout << "Before joining soln[pp]: " << std::endl;
                for(auto it = soln[pp].begin(); it != soln[pp].end(); it++) {
                    std::cout << *it << " ";
                }
                std::cout << "USE: " <<std::endl;
                for (Variable *v : USE) {
                    std::cout << "Joining for variable: " << v->name << std::endl;
                    std::cout << "Sigma prime " << std::endl;
                    for (auto it = sigma_prime[v->name].begin(); it != sigma_prime[v->name].end(); it++) {
                        std::cout << *it << " ";
                    }
                    std::cout << std::endl;
                }*/

                for (Variable *v : USE) {
                    // soln[pp] = soln[pp] U sigma_prime[v]
                    joinSets(soln[pp], sigma_prime[v->name]);
                }

                /*std::cout << "After joining soln[pp]: " << std::endl;
                for(auto it = soln[pp].begin(); it != soln[pp].end(); it++) {
                    std::cout << *it << " ";
                }
                std::cout << std::endl;*/
            }

            sigma_prime[load_inst->lhs->name] = {pp};
        }
        else if ((*inst).instrType == InstructionType::StoreInstrType)
        {
            /*
             * Cast it
            */
            StoreInstruction *store_inst = (StoreInstruction *) inst;
            std::set<Variable*> DEF;
            std::set<Variable*> USE;

            // TODO : Need to do extra handling for structs here
            /*
             * $store x op
             * DEF = { v in addr_taken | type(v) = type(op) }
             * USE = {x} U { op | op is a variable }
             * for all v in USE: soln[pp] = soln[pp] U sigma_prime[v]
             * for all v in DEF: sigma_prime[v] = sigma_prime[v] U { pp }
            */
            for (auto v : addr_taken) {
                // To handle the case when op is a constant int vs variable
                if (store_inst->op->IsConstInt())
                {
                    if (v->isIntType())
                        DEF.insert(v);
                }

                else if (Type::isEqualType(v->type, store_inst->op->var->type))
                    DEF.insert(v);
            }

            USE.insert(store_inst->dst);
            if (!store_inst->op->IsConstInt())
                USE.insert(store_inst->op->var);

            if (execute_final) {
                /*std::cout << "pp: " << pp << " Store inst " << std::endl;
                std::cout << "Before joining soln[pp]: " << std::endl;
                for(auto it = soln[pp].begin(); it != soln[pp].end(); it++) {
                    std::cout << *it << " ";
                }
                std::cout << "USE: " <<std::endl;
                for (Variable *v : USE) {
                    std::cout << "Joining for variable: " << v->name << std::endl;
                    std::cout << "Sigma prime " << std::endl;
                    for (auto it = sigma_prime[v->name].begin(); it != sigma_prime[v->name].end(); it++) {
                        std::cout << *it << " ";
                    }
                    std::cout << std::endl;
                }*/

                for (Variable *v : USE) {
                    // soln[pp] = soln[pp] U sigma_prime[v]
                    joinSets(soln[pp], sigma_prime[v->name]);
                }

                /*std::cout << "After joining soln[pp]: " << std::endl;
                for(auto it = soln[pp].begin(); it != soln[pp].end(); it++) {
                    std::cout << *it << " ";
                }
                std::cout << std::endl;*/
            }

            for (Variable *v : DEF) {
                sigma_prime[v->name].insert(pp);
            }
        }
        else if ((*inst).instrType == InstructionType::CallExtInstrType)
        {
            /*
             * Cast it
            */
            CallExtInstruction *callext_inst = (CallExtInstruction *) inst;
            std::set<Variable*> USE;
            /*
             * SDEF = {x} - Strong defs - definitely updating the variable
             * WDEF = { globals } U { v in addr_taken | type(v) in reachable_types(globals) } U { v in addr_taken | type(v) in reachable_types(args) } - Weak defs - may be updating the variable
             * USE = { fp } U { arg | arg is a variable } U WDEF // add fp only in case of call_idr
             * for all v in USE: soln[pp] = soln[pp] U sigma_prime[v]
             * for all v in WDEF: sigma_prime[v] = sigma_prime[v] U { pp }
             * sigma_prime[x] = { pp }
            */
            std::set<Variable*> SDEF;
            std::set<Variable*> WDEF;

            if (callext_inst->lhs)
                SDEF.insert(callext_inst->lhs);
            
            // Add all globals to WDEF
            for (auto it = program->globals.begin(); it != program->globals.end(); it++) {
                WDEF.insert((*it)->globalVar);
            }

            // Get reachable types for all globals
            std::unordered_set<ReachableType*> global_reachable_types;
            for (auto gl : program->globals) {
                ReachableType *var_type = new ReachableType(gl->globalVar->type);
                ReachableType::GetReachableType(program, var_type, global_reachable_types);
            }

            // Get reachable types for all args
            std::unordered_set<ReachableType*> args_reachable_types;
            for (auto arg : callext_inst->args) {
                if (arg->IsConstInt())
                    continue;
                ReachableType *var_type = new ReachableType(arg->var->type);
                ReachableType::GetReachableType(program, var_type, args_reachable_types);
            }

            for (auto v : addr_taken) {
                if (ReachableType::isPresentInSet(global_reachable_types, new ReachableType(v->type)))
                    WDEF.insert(v);
            }

            for (auto v : addr_taken) {
                if (ReachableType::isPresentInSet(args_reachable_types, new ReachableType(v->type)))
                    WDEF.insert(v);
            }

            std::copy(WDEF.begin(), WDEF.end(), std::inserter(USE, USE.end()));
            
            for(auto arg : callext_inst->args) {
                if (!arg->IsConstInt())
                    USE.insert(arg->var);
            }

            if (execute_final) {
                for (Variable *v : USE) {
                    // soln[pp] = soln[pp] U sigma_prime[v]
                    joinSets(soln[pp], sigma_prime[v->name]);
                }
            }

            for (Variable *v : WDEF) {
                // sigma_prime[v] = sigma_prime[v] U { pp }
                sigma_prime[v->name].insert(pp);
            }

            if (callext_inst->lhs)
                sigma_prime[callext_inst->lhs->name] = {pp};

        }
        index += 1;
    }

    Instruction *terminal_instruction = bb->terminal;

    std::string pp = bb->label + "." + "term";
    //std::cout << "pp: " << pp << std::endl;
    
    if ((*terminal_instruction).instrType == InstructionType::JumpInstrType)
    {
        JumpInstruction *jump_inst = (JumpInstruction *) terminal_instruction;
        std::set<Variable*> USE;
        std::set<Variable*> DEF;

        /*
         * DEF = {}
         * USE = {}
         * for all v in USE: soln[pp] = soln[pp] U sigma_prime[v] => since USE is empty, no update to soln
        */

        // For target basic block, join bb2store[target] with sigma_prime and add target to worklist if changed
        // TODO - Check if usage of bbs_to_output can be removed
        if (!execute_final) {
            bool store_changed = joinAbsStore(bb2store[jump_inst->label], sigma_prime);
            if (store_changed || bbs_to_output.count(jump_inst->label) == 0) {
                worklist.push_back(jump_inst->label);
                bbs_to_output.insert(jump_inst->label);
            }
        }
    }
    else if ((*terminal_instruction).instrType == InstructionType::BranchInstrType)
    {
        BranchInstruction *branch_inst = (BranchInstruction *) terminal_instruction;

        /*
         * DEF = {}
         * USE = { op | op is a variable }
         * for all v in USE: soln[pp] = soln[pp] U sigma_prime[v]
        */
        std::set<Variable*> USE;
        std::set<Variable*> DEF;

        if (!branch_inst->condition->IsConstInt())
            USE.insert(branch_inst->condition->var);

        if (execute_final) {
            /*std::cout << "pp: " << pp << " Branch inst " << std::endl;
            std::cout << "Before joining soln[pp]: " << std::endl;
            for(auto it = soln[pp].begin(); it != soln[pp].end(); it++) {
                std::cout << *it << " ";
            }
            std::cout << "USE: " <<std::endl;
            for (Variable *v : USE) {
                std::cout << "Joining for variable: " << v->name << std::endl;
                std::cout << "Sigma prime " << std::endl;
                for (auto it = sigma_prime[v->name].begin(); it != sigma_prime[v->name].end(); it++) {
                    std::cout << *it << " ";
                }
                std::cout << std::endl;
            }*/

            for (Variable *v : USE) {
                // soln[pp] = soln[pp] U sigma_prime[v]
                joinSets(soln[pp], sigma_prime[v->name]);
            }

            /*std::cout << "After joining soln[pp]: " << std::endl;
            for(auto it = soln[pp].begin(); it != soln[pp].end(); it++) {
                std::cout << *it << " ";
            }
            std::cout << std::endl;*/
        }

        // For target basic block, join bb2store[target] with sigma_prime and add target to worklist if changed
        // TODO - Check if usage of bbs_to_output can be removed
        if (!execute_final) {
            
            bool store_changed_tt = joinAbsStore(bb2store[branch_inst->tt], sigma_prime);
            bool store_changed_ff = joinAbsStore(bb2store[branch_inst->ff], sigma_prime);
            // std::cout << "Joining for branch true : " << branch_inst->tt << std::endl;
            if (store_changed_tt || bbs_to_output.count(branch_inst->tt) == 0) {
                worklist.push_back(branch_inst->tt);
                bbs_to_output.insert(branch_inst->tt);
            }

            // std::cout << "Joining for branch false: " << branch_inst->ff << std::endl;
            if (store_changed_ff || bbs_to_output.count(branch_inst->ff) == 0) {
                worklist.push_back(branch_inst->ff);
                bbs_to_output.insert(branch_inst->ff);
            }
        }
    }
    else if ((*terminal_instruction).instrType == InstructionType::RetInstrType)
    {
        RetInstruction *ret_inst = (RetInstruction *) terminal_instruction;

        /*
         * DEF = {}
         * USE = { op | op is a variable }
         * for all v in USE: soln[pp] = soln[pp] U sigma_prime[v]
        */
        std::set<Variable*> USE;
        std::set<Variable*> DEF;

        if (ret_inst->op && !(ret_inst->op->IsConstInt()))
            USE.insert(ret_inst->op->var);
        
        if (execute_final) {
            /*std::cout << "pp: " << pp << " Ret inst " << std::endl;
            std::cout << "Before joining soln[pp]: " << std::endl;
            for(auto it = soln[pp].begin(); it != soln[pp].end(); it++) {
                std::cout << *it << " ";
            }
            std::cout << "USE: " <<std::endl;
            for (Variable *v : USE) {
                std::cout << "Joining for variable: " << v->name << std::endl;
                std::cout << "Sigma prime " << std::endl;
                for (auto it = sigma_prime[v->name].begin(); it != sigma_prime[v->name].end(); it++) {
                    std::cout << *it << " ";
                }
                std::cout << std::endl;
            }*/

            for (Variable *v : USE) {
                // soln[pp] = soln[pp] U sigma_prime[v]
                joinSets(soln[pp], sigma_prime[v->name]);
            }

            /*std::cout << "After joining soln[pp]: " << std::endl;
            for(auto it = soln[pp].begin(); it != soln[pp].end(); it++) {
                std::cout << *it << " ";
            }
            std::cout << std::endl;*/
        }
    }
    else if ((*terminal_instruction).instrType == InstructionType::CallDirInstrType)
    {
        CallDirInstruction *calldir_inst = (CallDirInstruction *) terminal_instruction;

        std::set<Variable*> USE;
        // TODO: This needs to be updated with pointer information
        /*
            * SDEF = {x} - Strong defs - definitely updating the variable
            * WDEF = { globals } U { v in addr_taken | type(v) in reachable_types(globals) } U { v in addr_taken | type(v) in reachable_types(args) } - Weak defs - may be updating the variable
            * USE = { fp } U { arg | arg is a variable } U WDEF // add fp only in case of call_idr
            * for all v in USE: soln[pp] = soln[pp] U sigma_prime[v]
            * for all v in WDEF: sigma_prime[v] = sigma_prime[v] U { pp }
            * sigma_prime[x] = { pp }
        */
        std::set<Variable*> SDEF;
        std::set<Variable*> WDEF;

        if (calldir_inst->lhs)
            SDEF.insert(calldir_inst->lhs);
        
        // Add all globals to WDEF
        for (auto it = program->globals.begin(); it != program->globals.end(); it++) {
            WDEF.insert((*it)->globalVar);
        }

        // Get reachable types for all globals
            std::unordered_set<ReachableType*> global_reachable_types;
            for (auto gl : program->globals) {
                ReachableType *var_type = new ReachableType(gl->globalVar->type);
                ReachableType::GetReachableType(program, var_type, global_reachable_types);
            }

            // Get reachable types for all args
            std::unordered_set<ReachableType*> args_reachable_types;
            for (auto arg : calldir_inst->args) {
                if (arg->IsConstInt())
                    continue;
                ReachableType *var_type = new ReachableType(arg->var->type);
                ReachableType::GetReachableType(program, var_type, args_reachable_types);
            }

            for (auto v : addr_taken) {
                if (ReachableType::isPresentInSet(global_reachable_types, new ReachableType(v->type)))
                    WDEF.insert(v);
            }

            for (auto v : addr_taken) {
                if (ReachableType::isPresentInSet(args_reachable_types, new ReachableType(v->type)))
                    WDEF.insert(v);
            }

        // Add WDEF to USE
        std::copy(WDEF.begin(), WDEF.end(), std::inserter(USE, USE.end()));
        
        for(auto arg : calldir_inst->args) {
            if (!arg->IsConstInt())
                USE.insert(arg->var);
        }

        if (execute_final) {
            for (Variable *v : USE) {
                // soln[pp] = soln[pp] U sigma_prime[v]
                joinSets(soln[pp], sigma_prime[v->name]);
            }
        }

        for (Variable *v : WDEF) {
            // sigma_prime[v] = sigma_prime[v] U { pp }
            sigma_prime[v->name].insert(pp);
        }

        if (calldir_inst->lhs)
            sigma_prime[calldir_inst->lhs->name] = {pp};

        if (!execute_final) {
            if (joinAbsStore(bb2store[calldir_inst->next_bb], sigma_prime) || bbs_to_output.count(calldir_inst->next_bb) == 0) {
                worklist.push_back(calldir_inst->next_bb);
                bbs_to_output.insert(calldir_inst->next_bb);
            }
        }

    }
    else if ((*terminal_instruction).instrType == InstructionType::CallIdrInstrType)
    {
        CallIdrInstruction *callidir_inst = (CallIdrInstruction *) terminal_instruction;

        std::set<Variable*> USE;
        // TODO: This needs to be updated with pointer information
        /*
            * SDEF = {x} - Strong defs - definitely updating the variable
            * WDEF = { globals } U { v in addr_taken | type(v) in reachable_types(globals) } U { v in addr_taken | type(v) in reachable_types(args) } - Weak defs - may be updating the variable
            * USE = { fp } U { arg | arg is a variable } U WDEF // add fp only in case of call_idr
            * for all v in USE: soln[pp] = soln[pp] U sigma_prime[v]
            * for all v in WDEF: sigma_prime[v] = sigma_prime[v] U { pp }
            * sigma_prime[x] = { pp }
        */
        std::set<Variable*> SDEF;
        std::set<Variable*> WDEF;

        if (callidir_inst->lhs)
            SDEF.insert(callidir_inst->lhs);
        
        // Add all globals to WDEF
        for (auto it = program->globals.begin(); it != program->globals.end(); it++) {
            WDEF.insert((*it)->globalVar);
        }
        // Get reachable types for all globals
        std::unordered_set<ReachableType*> global_reachable_types;
        for (auto gl : program->globals) {
            ReachableType *var_type = new ReachableType(gl->globalVar->type);
            ReachableType::GetReachableType(program, var_type, global_reachable_types);
        }

        // Get reachable types for all args
        std::unordered_set<ReachableType*> args_reachable_types;
        for (auto arg : callidir_inst->args) {
            if (arg->IsConstInt())
                continue;
            ReachableType *var_type = new ReachableType(arg->var->type);
            ReachableType::GetReachableType(program, var_type, args_reachable_types);
        }

        for (auto v : addr_taken) {
            if (ReachableType::isPresentInSet(global_reachable_types, new ReachableType(v->type)))
                WDEF.insert(v);
        }

        for (auto v : addr_taken) {
            if (ReachableType::isPresentInSet(args_reachable_types, new ReachableType(v->type)))
                WDEF.insert(v);
        }

        std::copy(WDEF.begin(), WDEF.end(), std::inserter(USE, USE.end()));

        for(auto arg : callidir_inst->args) {
            if (!arg->IsConstInt())
                USE.insert(arg->var);
        }
        USE.insert(callidir_inst->fp);

        if (execute_final) {
            for (Variable *v : USE) {
                // soln[pp] = soln[pp] U sigma_prime[v]
                joinSets(soln[pp], sigma_prime[v->name]);
            }
        }

        for (Variable *v : WDEF) {
            // sigma_prime[v] = sigma_prime[v] U { pp }
            sigma_prime[v->name].insert(pp);
        }

        if (callidir_inst->lhs)
            sigma_prime[callidir_inst->lhs->name] = {pp};

        if (!execute_final) {
            if (joinAbsStore(bb2store[callidir_inst->next_bb], sigma_prime) || bbs_to_output.count(callidir_inst->next_bb) == 0) {
                worklist.push_back(callidir_inst->next_bb);
                bbs_to_output.insert(callidir_inst->next_bb);
            }
        }

    }
    else
    {
        std::cout << "Unknown terminal instruction type" << std::endl;
    }
    
    /*if (!execute_final) {
        std::cout << "Sigma prime for bb: " << bb->label << std::endl;
        for (auto it = sigma_prime.begin(); it != sigma_prime.end(); it++) {
            std::cout << it->first << " -> {";
            for (auto it2 = it->second.begin(); it2 != it->second.end(); it2++) {
                std::cout << *it2 << " ";
            }
            std::cout << "}" << std::endl;
        } 
    }*/
    
    return;
}