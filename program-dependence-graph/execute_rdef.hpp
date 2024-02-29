#pragma once

#include<map>
#include<set>
#include<iostream>
#include "../headers/datatypes.h"
#include <deque>
#include <unordered_set>
#include <unordered_map>
#include "rtype.hpp"
#include <queue>
#include "mod_ref_utils.hpp"

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

std::set<std::string> GetReachable(std::vector<Operand*> args, std::unordered_map<std::string, std::set<std::string>> pointsTo, Program *program) {
    // TODO - Need to handle name of pointsTo - for locals - funcname.argname
    std::set<std::string> reachable;
    std::queue<std::string> q;
    std::set<std::string> visited;

    for(const auto& op: args) {
        if(!op->IsConstInt()) {
            std::string var = op->var->name;
            if(pointsTo.count(var)) {
                for(const auto& v: pointsTo[var]) {
                    reachable.insert(v);
                    if (visited.count(v) == 0) {
                        visited.insert(v);
                        q.push(v);
                    }
                }
                while(!q.empty()) {
                    std::string tmp = q.front();
                    q.pop();
                    if(pointsTo.count(tmp)) {
                        for(const auto& v: pointsTo[tmp]) {
                            reachable.insert(v);
                            if (visited.count(v) == 0) {
                                visited.insert(v);
                                q.push(v);
                            }
                        }
                    }
                }
            }
        }
    }
    // Globals + All objects reachable from globals

    std::set<std::string> visited_globals;

    for (auto gl : program->globals) {
        reachable.insert(gl->globalVar->name);

        if (!pointsTo.count(gl->globalVar->name))
            continue;

        for (auto v : pointsTo[gl->globalVar->name]) {
            reachable.insert(v);
            if (visited_globals.count(v) == 0) {
                visited_globals.insert(v);
                q.push(v);
            }
        }
        while (!q.empty()) {
            std::string tmp = q.front();
            q.pop();
            if (pointsTo.count(tmp)) {
                for (const auto &v : pointsTo[tmp]) {
                    reachable.insert(v);
                    if (visited_globals.count(v) == 0) {
                        visited_globals.insert(v);
                        q.push(v);
                    }
                }
            }
        }
    }
    return reachable;
}

std::set<std::string> GetRefs(std::set<std::string> callees, std::set<std::string> reachable, std::map<std::string, ModRefInfo> modRefInfo_) {
    std::set<std::string> refs, union_refs;
    for(const auto& callee: callees) {
        if(modRefInfo_.count(callee)) {
            ModRefInfo tmp = modRefInfo_[callee];
            union_refs.insert(tmp.ref.begin(), tmp.ref.end());
        }
    }
    std::set_intersection(union_refs.begin(), union_refs.end(), reachable.begin(), reachable.end(),
                                                std::inserter(refs, refs.begin()));
    return refs;
}

std::set<std::string> GetDefs(std::set<std::string> callees, std::set<std::string> reachable, std::map<std::string, ModRefInfo> modRefInfo_) {
    std::set<std::string> defs, union_defs;
    for(const auto& callee: callees) {
        if(modRefInfo_.count(callee)) {
            ModRefInfo tmp = modRefInfo_[callee];
            union_defs.insert(tmp.mod.begin(), tmp.mod.end());
        }
    }
    std::set_intersection(union_defs.begin(), union_defs.end(), reachable.begin(), reachable.end(),
                                                std::inserter(defs, defs.begin()));
    return defs;
}

void execute(
    Program *program,
    std::unordered_map<std::string, std::set<std::string>> &pointsTo,
    std::map<std::string, ModRefInfo> &modRefInfo,
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
                for (Variable *v : USE) {
                    // soln[pp] = soln[pp] U sigma_prime[v]
                    joinSets(soln[pp], sigma_prime[v->name]);
                }
            }

            sigma_prime[gep_inst->lhs->name] = {pp};
        }
        else if ((*inst).instrType == InstructionType::GfpInstrType)
        {
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
            std::set<std::string> DEF;
            std::set<std::string> USE;

            /*
             * DEF = {x}
             * USE = {y} U pointsTo[y]
             * for all v in USE: soln[pp] = soln[pp] U sigma_prime[v]
             * sigma_prime[x] = { pp }
            */
            DEF.insert(load_inst->lhs->name);
            USE.insert(load_inst->src->name);

            if (pointsTo.count(load_inst->src->name)) {
                for (auto pts_to : pointsTo[load_inst->src->name]) {
                    USE.insert(pts_to);
                }
            }

            if (execute_final) {
                for (std::string v : USE) {
                    // soln[pp] = soln[pp] U sigma_prime[v]
                    joinSets(soln[pp], sigma_prime[v]);
                }
            }

            sigma_prime[load_inst->lhs->name] = {pp};
        }
        else if ((*inst).instrType == InstructionType::StoreInstrType)
        {
            StoreInstruction *store_inst = (StoreInstruction *) inst;
            std::set<std::string> DEF;
            std::set<std::string> USE;

            /*
             * $store x op
             * DEF = pointsTo[x]
             * USE = {x} U { op | op is a variable }
             * for all v in USE: soln[pp] = soln[pp] U sigma_prime[v]
             * for all v in DEF: sigma_prime[v] = sigma_prime[v] U { pp }
            */
            if (pointsTo.count(store_inst->dst->name)) {
                for (auto pts_to : pointsTo[store_inst->dst->name]) {
                    DEF.insert(pts_to);
                }
            }

            USE.insert(store_inst->dst->name);
            if (!store_inst->op->IsConstInt())
                USE.insert(store_inst->op->var->name);

            if (execute_final) {
                for (std::string v : USE) {
                    // soln[pp] = soln[pp] U sigma_prime[v]
                    joinSets(soln[pp], sigma_prime[v]);
                }
            }

            for (std::string v : DEF) {
                sigma_prime[v].insert(pp);
            }
        }
        else if ((*inst).instrType == InstructionType::CallExtInstrType)
        {
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
            for (Variable *v : USE) {
                // soln[pp] = soln[pp] U sigma_prime[v]
                joinSets(soln[pp], sigma_prime[v->name]);
            }
        }

        // For target basic block, join bb2store[target] with sigma_prime and add target to worklist if changed
        if (!execute_final) {
            
            bool store_changed_tt = joinAbsStore(bb2store[branch_inst->tt], sigma_prime);
            bool store_changed_ff = joinAbsStore(bb2store[branch_inst->ff], sigma_prime);

            if (store_changed_tt || bbs_to_output.count(branch_inst->tt) == 0) {
                worklist.push_back(branch_inst->tt);
                bbs_to_output.insert(branch_inst->tt);
            }

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
            for (Variable *v : USE) {
                // soln[pp] = soln[pp] U sigma_prime[v]
                joinSets(soln[pp], sigma_prime[v->name]);
            }
        }
    }
    else if ((*terminal_instruction).instrType == InstructionType::CallDirInstrType)
    {
        CallDirInstruction *calldir_inst = (CallDirInstruction *) terminal_instruction;

        std::set<std::string> CALLEES, REFS, WDEF, REACHABLE, USE;
        /*
        * CALLEES = {id} 
        * REACHABLE = globals U all objects reachable from globals or arguments (using points to solution)
        * WDEF = (U mod(c) for all mods of c in CALLEES) ^ REACHABLE
        * USE = {fp} U {arg | arg is a variable} U ((U ref(c) for all mods of c in CALLEES) ^ REACHABLE)
        */
       // TODO - Check logic for each function and need to add logic for globals and pointsTo globals
        CALLEES.insert(calldir_inst->callee);

        REACHABLE = GetReachable(calldir_inst->args, pointsTo, program);

        REFS = GetRefs(CALLEES, REACHABLE, modRefInfo);
		
        USE.insert(REFS.begin(), REFS.end());

        for(const auto& arg: calldir_inst->args) {
            if(!arg->IsConstInt()) {
                USE.insert(arg->var->name);
            }
        }
        WDEF = GetDefs(CALLEES, REACHABLE, modRefInfo);

        if (execute_final) {
            for (std::string v : USE) {
                // soln[pp] = soln[pp] U sigma_prime[v]
                joinSets(soln[pp], sigma_prime[v]);
            }
        }

        for (std::string v : WDEF) {
            // sigma_prime[v] = sigma_prime[v] U { pp }
            sigma_prime[v].insert(pp);
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

        std::set<std::string> CALLEES, REFS, WDEF, REACHABLE, USE;
        /*
        * CALLEES = pointsTo[fp]
        * REACHABLE = globals U all objects reachable from globals or arguments (using points to solution)
        * WDEF = (U mod(c) for all mods of c in CALLEES) ^ REACHABLE
        * USE = {fp} U {arg | arg is a variable} U ((U ref(c) for all mods of c in CALLEES) ^ REACHABLE)
        */

        for(const auto& pts_to: pointsTo[callidir_inst->fp->name]) {
            CALLEES.insert(pts_to);
        }

        REACHABLE = GetReachable(callidir_inst->args, pointsTo, program);

        REFS = GetRefs(CALLEES, REACHABLE, modRefInfo);
		
        USE.insert(REFS.begin(), REFS.end());

        for(const auto& arg: callidir_inst->args) {
            if(!arg->IsConstInt()) {
                USE.insert(arg->var->name);
            }
        }
        WDEF = GetDefs(CALLEES, REACHABLE, modRefInfo);

        if (execute_final) {
            for (std::string v : USE) {
                // soln[pp] = soln[pp] U sigma_prime[v]
                joinSets(soln[pp], sigma_prime[v]);
            }
        }

        for (std::string v : WDEF) {
            // sigma_prime[v] = sigma_prime[v] U { pp }
            sigma_prime[v].insert(pp);
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
    return;
}