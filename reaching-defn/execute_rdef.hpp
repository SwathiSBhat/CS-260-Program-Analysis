#pragma once

#include<map>
#include<set>
#include<iostream>
#include "../headers/datatypes.h"

/*
 * Join is a union of the two abstract stores where each abstract store is a map of variable name to a set of pp where they are defined.
*/
bool joinAbsStore(std::map<std::string, std::set<std::string>>curr_abs_store, std::map<std::string, std::set<std::string>>parent_bb_abs_store) {
    
    bool changed = false;
    std::map<std::string, std::set<std::string>> result_set;
    
    // For elements in curr_abs_store, do a set union with the parent_bb_abs_store
    // This also handles the case when the key is not present in parent_bb_abs_store but present in curr_abs_store
    for(auto it = curr_abs_store.begin(); it != curr_abs_store.end(); it++) {
        std::set<std::string> result_set;
        std::set_union(it->second.begin(), it->second.end(), parent_bb_abs_store[it->first].begin(), parent_bb_abs_store[it->first].end(), std::inserter(result_set, result_set.begin()));
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
    std::set<Variable*> DEF;
    std::set<Variable*> USE;
    int index = 0; // To help build program point name

    /*
     * Iterate through each instruction in bb.
     */
    for (const Instruction *inst : bb->instructions) {

        std::string pp = bb->label + "." + std::to_string(index);

        // arith, cmp, alloc, copy, gep, gfp work the same way
        if ((*inst).instrType == InstructionType::ArithInstrType) {

            /*
             * Cast it.
             */
             ArithInstruction *arith_inst = (ArithInstruction *) inst;

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
                for (Variable *v : USE) {
                    // soln[pp] = soln[pp] U sigma_prime[v]
                    joinSets(soln[pp], sigma_prime[v->name]);
                }
            }

            sigma_prime[arith_inst->lhs->name] = {pp};
        }
        else if ((*inst).instrType == InstructionType::CmpInstrType)
        {
            /*
             * Cast it.
             */
             CmpInstruction *cmp_inst = (CmpInstruction *) inst;

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
                for (Variable *v : USE) {
                    // soln[pp] = soln[pp] U sigma_prime[v]
                    joinSets(soln[pp], sigma_prime[v->name]);
                }
            }

            sigma_prime[cmp_inst->lhs->name] = {pp};
        }
        else if ((*inst).instrType == InstructionType::CopyInstrType)
        {
            /*
             * Cast it.
             */
             CopyInstruction *copy_inst = (CopyInstruction *) inst;

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
                for (Variable *v : USE) {
                    // soln[pp] = soln[pp] U sigma_prime[v]
                    joinSets(soln[pp], sigma_prime[v->name]);
                }
            }

            sigma_prime[copy_inst->lhs->name] = {pp};
        }
        else if ((*inst).instrType == InstructionType::AllocInstrType)
        {
            /*
             * Cast it.
             */
             AllocInstruction *alloc_inst = (AllocInstruction *) inst;

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
                for (Variable *v : USE) {
                    // soln[pp] = soln[pp] U sigma_prime[v]
                    joinSets(soln[pp], sigma_prime[v->name]);
                }
            }

            sigma_prime[alloc_inst->lhs->name] = {pp};
        }
        else if ((*inst).instrType == InstructionType::GepInstrType)
        {
            /*
             * Cast it.
             */
             GepInstruction *gep_inst = (GepInstruction *) inst;

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

            if (execute_final) {
                for (Variable *v : USE) {
                    // soln[pp] = soln[pp] U sigma_prime[v]
                    joinSets(soln[pp], sigma_prime[v->name]);
                }
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

            /*
             * x = $gfp id id
             * DEF = {x}
             * USE = { op | op is a variable }
             * for all v in USE: soln[pp] = soln[pp] U sigma_prime[v]
             * sigma_prime[x] = { pp }
            */
            DEF.insert(gfp_inst->lhs);
            
            if (execute_final) {
                for (Variable *v : USE) {
                    // soln[pp] = soln[pp] U sigma_prime[v]
                    joinSets(soln[pp], sigma_prime[v->name]);
                }
            }

            sigma_prime[gfp_inst->lhs->name] = {pp};
        }
        else if ((*inst).instrType == InstructionType::AddrofInstrType)
        {
            /*
             * Cast it.
             */
             AddrofInstruction *addrof_inst = (AddrofInstruction *) inst;

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
            
            // TODO : Need to do extra handling for structs here
            /*
             * DEF = {x}
             * USE = {y} U { v in addr_taken | type(v) = type(x) }
             * for all v in USE: soln[pp] = soln[pp] U sigma_prime[v]
             * sigma_prime[x] = { pp }
            */
            DEF.insert(load_inst->lhs);
            USE.insert(load_inst->src);
            for (auto v : addr_taken) {
                if (Type::isEqualType(v->type, load_inst->lhs->type)) {
                    USE.insert(v);
                }
            }

            if (execute_final) {
                for (Variable *v : USE) {
                    // soln[pp] = soln[pp] U sigma_prime[v]
                    joinSets(soln[pp], sigma_prime[v->name]);
                }
            }

            sigma_prime[load_inst->lhs->name] = {pp};
        }
        else if ((*inst).instrType == InstructionType::StoreInstrType)
        {
            /*
             * Cast it
            */
            StoreInstruction *store_inst = (StoreInstruction *) inst;

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
                if (store_inst->op->IsConstInt() && v->isIntType())
                    DEF.insert(v);
                else if (Type::isEqualType(v->type, store_inst->op->var->type))
                    DEF.insert(v);
            }

            USE.insert(store_inst->dst);
            if (!store_inst->op->IsConstInt())
                USE.insert(store_inst->op->var);

            if (execute_final) {
                for (Variable *v : USE) {
                    // soln[pp] = soln[pp] U sigma_prime[v]
                    joinSets(soln[pp], sigma_prime[v->name]);
                }
            }

            for (Variable *v : DEF) {
                sigma_prime[v->name].insert(pp);
            }
        }
        else if ((*inst).instrType == InstructionType::CallExtInstrType)
        {
            // TODO - Yet to implement
        }

        index += 1;
    }

    return;
}