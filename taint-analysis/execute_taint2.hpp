#pragma once

#include<map>
#include<set>
#include<iostream>
#include "../headers/datatypes.h"
#include <deque>
#include <queue>
#include <unordered_set>
#include <unordered_map>

using AbsStore = std::map<std::string, std::set<std::string>>; // abs store mapping from variable name to sources that taint them
using SolnStore = std::map<std::string, std::set<std::string>>;  // sink -> sources

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

// Check if 2 abstract stores are equal. Ignore keys that are empty and not present in either store
// Example store1 = {a: {}, b: {1,2}, c:{3,4}} store2 = {b:{1,2}, c:{3,4},d:{}} = true
bool isAbsStoreEqual(AbsStore store1, AbsStore store2)
{
    for (auto it = store1.begin(); it != store1.end(); it++)
    {
        if (it->second.size() == 0 && store2.count(it->first) == 0)
            continue;
        else if (store2.count(it->first) == 0)
            return false;
        else if (store2[it->first] != it->second)
            return false;
    }
    for (auto it = store2.begin(); it != store2.end(); it++)
    {
        if (it->second.size() == 0 && store1.count(it->first) == 0)
            continue;
        else if (store1.count(it->first) == 0)
            return false;
        else if (store1[it->first] != it->second)
            return false;
    }
    return true;
}

bool isGlobalVar(Variable *var, Program *program, std::string func_name) {
    
    if (program->funcs[func_name]->locals.count(var->name) > 0)
        return false;

    for (auto gl : program->globals) {
        if (gl->globalVar->name == var->name)
            return true;
    }
    return false;
}

/*
* Get the key for the abstract store
*/
std::string GetKey(Program* program, Function* func, Variable *var) {
    if (isGlobalVar(var, program, func->name))
        return var->name;
    else
        return func->name + "." + var->name;
}

std::string GetKey(Program* program, Function* func, Operand *op) {
    if (op->IsConstInt())
        return "";
    else
        return GetKey(program, func, op->var);
}

/*
* Determine if an extern is a source or a sink
*/
// Sink = Any extern function whose name starts with sink_prefix
bool isSink(Program *program, ExternalFunction *extern_func) {
    if (extern_func->name.find("snk") != std::string::npos)
        return true;
    return false;
}

// Source = Any extern function whose name starts with source_prefix
bool isSource(Program *program, ExternalFunction *extern_func) {
    if (extern_func->name.find("src") != std::string::npos)
        return true;
    return false;
}

std::set<std::string> GetReachable(std::vector<Operand*> args, std::unordered_map<std::string, std::set<std::string>> pointsTo, Program *program, Function* curr_func) {
    std::set<std::string> reachable;
    std::queue<std::string> q;
    std::set<std::string> visited;

    for(const auto& op: args) {
        if(!op->IsConstInt()) {
            std::string var = op->var->name;
            std::string pointsToVarName = isGlobalVar(op->var, program, curr_func->name) ? var : curr_func->name + "." + var;

            if(pointsTo.count(pointsToVarName)) {
                for(auto v: pointsTo[pointsToVarName]) {
                    
                    if (visited.count(v) == 0) {
                        visited.insert(v);
                        q.push(v);
                    }
                    reachable.insert(v);

                }

                while(!q.empty()) {

                    std::string tmp = q.front();
                    q.pop();
                    
                    if(pointsTo.count(tmp)) {
                        for(auto v: pointsTo[tmp]) {
                            
                            if (visited.count(v) == 0) {
                                visited.insert(v);
                                q.push(v);
                            }
                            reachable.insert(v);
                        }
                    }
                }
            }
        }
    }
    return reachable;
}

std::set<std::string> GetReachable(std::vector<Variable*> params, std::unordered_map<std::string, std::set<std::string>> pointsTo, Program *program, Function* curr_func) {
    std::set<std::string> reachable;
    std::queue<std::string> q;
    std::set<std::string> visited;

    for(const auto& param: params) {
        std::string var = param->name;
        std::string pointsToVarName = isGlobalVar(param, program, curr_func->name) ? var : curr_func->name + "." + var;

        if(pointsTo.count(pointsToVarName)) {
            for(auto v: pointsTo[pointsToVarName]) {
                
                if (visited.count(v) == 0) {
                    visited.insert(v);
                    q.push(v);
                }
                reachable.insert(v);

            }

            while(!q.empty()) {

                std::string tmp = q.front();
                q.pop();
                
                if(pointsTo.count(tmp)) {
                    for(auto v: pointsTo[tmp]) {
                        
                        if (visited.count(v) == 0) {
                            visited.insert(v);
                            q.push(v);
                        }
                        reachable.insert(v);
                    }
                }
            }
        }
    }
    return reachable;
}

/*
* Helper functions for intraprocedural taint analysis
*/
/*
* taint(op) =   {
                    store[op] if op is a variable
                    {} if op is a constant
                }
*/
std::set<std::string> taint(Operand *op, AbsStore &store, std::string key) {
    if (op->IsConstInt())
        return {};
    else
    {
        return store[key];
    }
}

/*
* Helper functions:
* GetCalleeStore : (pointoTo, curr abs store, callee, args) -> abs store
*       func = func where the call to GetCalleeStore is made
* GetReturnedStore : (pointoTo, curr abs store, curr func, ret op? if present) -> abs store
* GetCallerStore : (store, call lhs if present) -> abs store
*/
AbsStore GetCalleeStore(Program *program, std::unordered_map<std::string, std::set<std::string>> pointsTo, AbsStore curr_store, std::string callee, std::vector<Operand*> args, Function* func) {

    // 1. Map each callee parameter to abstract store of the corresponding argumentg
    AbsStore callee_store = {};
    Function *callee_func = program->funcs[callee];
    for (int i = 0; i < args.size(); i++) {
        // TODO - How to handle constant arguments?
        std::string key = GetKey(program, func, args[i]);
        if (!args[i]->IsConstInt() && curr_store[key].size() > 0)
            callee_store[callee + "." + callee_func->params[i]->name] = curr_store[key];
    }
    // 2. Copy each element reachable from args
    std::set<std::string> reachable = GetReachable(args, pointsTo, program, func);
    for (auto it = reachable.begin(); it != reachable.end(); it++) {
        // TODO - Ensure that the weak update here is correct
        joinSets(callee_store[*it], curr_store[*it]);
    }

    return callee_store;
}

AbsStore GetReturnedStore(Program *program, std::unordered_map<std::string, std::set<std::string>> pointsTo, AbsStore curr_store, Function *curr_func, Operand* ret_op) {
    
    AbsStore returned_store = {};

    // 1. Add all elements of current store reachable from a parameter
    std::set<std::string> reachable = GetReachable(curr_func->params, pointsTo, program, curr_func);
    for (auto it = reachable.begin(); it != reachable.end(); it++) {
        if (curr_store.find(*it) != curr_store.end()) {
            returned_store[*it] = curr_store[*it];
        }
    }

    // 2. Add all elements of current store reachable from the return operand
    if (ret_op != nullptr && !ret_op->IsConstInt()) {
        std::string ret_op_key = GetKey(program, curr_func, ret_op->var);
        for (auto points_to: pointsTo[ret_op_key]) {
            returned_store[points_to] = curr_store[points_to];
        }
        
        // TODO - Check if anything needs to be done if constant is returned
        // 3. Placeholde FAKE for lhs of return instruction
        if (curr_store.find(ret_op_key) != curr_store.end()) {
            returned_store["FAKE"] = curr_store[ret_op_key];
        }
    }

    return returned_store;
}

AbsStore GetCallerStore(Program *program, AbsStore curr_store, Variable* call_lhs, Function *caller_func) {

    AbsStore caller_store = curr_store;
    if (call_lhs != nullptr) {
        std::string call_lhs_key = GetKey(program, caller_func, call_lhs);
        if (curr_store.find("FAKE") != curr_store.end()) {
            caller_store[call_lhs_key] = curr_store["FAKE"];
            caller_store.erase("FAKE");
        }
    }
    else {
        if (caller_store.find("FAKE") != caller_store.end())
            caller_store.erase("FAKE");
    }
    return caller_store;
}


void PrintAbsStore(AbsStore store)
{
    for (auto it = store.begin(); it != store.end(); it++)
    {
        std::cout << it->first << " : ";
        for (auto it2 = it->second.begin(); it2 != it->second.end(); it2++)
        {
            std::cout << *it2 << ",";
        }
        std::cout << std::endl;
    }
}

// The callstring stack is a string of the form "func1.bbx#func2.bby#func3.bbz#func1.bba...."
// The first function is top of the stack and the last function is the bottom of the stack

// Pop off callsite from the callstring stack
std::string PopFromCurrentContext(int sensitivity, std::string curr_context) {
    std::string callstring;

    if (sensitivity == 1)
    {
        callstring = "";
    }
    else if (sensitivity == 2)
    {
        int index = curr_context.find("#");
        if (index != std::string::npos)
        {
            callstring = curr_context.substr(index + 1);
        }
        else
        {
            callstring = "";
        }
    }
    return callstring;
}

std::string AddToCurrentContext(std::string callsite, int sensitivity, std::string curr_context) {
    
    std::string callstring;

    if (sensitivity == 1)
    {
        // For callstring-1 sensitivity, we need to maintain a callstring stack of size 1.
        callstring = callsite;
    }
    else if (sensitivity == 2)
    {
        // For callstring-2 sensitity, we need to maintain a callstring stack of size 2.
        // IF the size is already 2, then we drop the bottom of the stack and add the new function to the top of the stack
        if (curr_context.size() > 0)
        {
            int index = curr_context.find("#");
            if (index != std::string::npos)
            {
                std::string first_callsite = curr_context.substr(0, index);
                callstring = callsite + "#" + first_callsite;
            }
            else
            {
                callstring = callsite + "#" + curr_context;
            }
        }
    }
    return callstring;
}

void execute(
    Program *program,
    Function* func,
    std::string curr_cid,
    BasicBlock *bb,
    std::map<std::string, std::map<std::string, AbsStore>> &bb2store, // function -> bb -> variable -> set of sources
    std::deque<std::pair<std::string, std::string>> &worklist,
    std::set<std::string> &bbs_to_output,
    std::map<std::string, std::set<std::string>> &soln,
    std::unordered_map<std::string, std::set<std::string>> pointsTo,
    std::map<std::pair<std::string, std::string>, std::set<std::pair<std::string,std::string>>> &call_edges,
    std::map<std::pair<std::string, std::string>, AbsStore> &call_returned,
    std::map<std::string, RetInstruction*> func_ret_op,
    int sensitivity
)
{
    AbsStore sigma_prime = bb2store[func->name][bb->label]; 
    if (sensitivity == 1 || sensitivity == 2)
        sigma_prime = bb2store[func->name + "|" + curr_cid][bb->label]; // For callstring and functional sensitivity, we need to maintain a separate store for each context (cid
    
    int index = 0; // To help build program point name

    /*
     * Iterate through each instruction in bb.
     */
    for (const Instruction *inst : bb->instructions) {

        std::string pp = func->name + "." + bb->label + "." + std::to_string(index);

        if ((*inst).instrType == InstructionType::ArithInstrType) 
        {
             ArithInstruction *arith_inst = (ArithInstruction *) inst;

            /*
             * x = $arith add y z
             * sigma_prime[x] = taint(op1) U taint(op2)
            */
            std::set<std::string> taint_op1 = taint(arith_inst->op1, sigma_prime, GetKey(program, func, arith_inst->op1)); // TODO - Verify that is sigma_prime and not bb2store 
            std::set<std::string> taint_op2 = taint(arith_inst->op2, sigma_prime, GetKey(program, func, arith_inst->op2));
            joinSets(taint_op1, taint_op2);
            sigma_prime[GetKey(program, func, arith_inst->lhs)] = taint_op1;

        }
        else if ((*inst).instrType == InstructionType::CmpInstrType)
        {
             CmpInstruction *cmp_inst = (CmpInstruction *) inst;
             
            /*
             * x = $cmp gt y z
             * sigma_prime[x] = taint(op1) U taint(op2)
            */
            std::set<std::string> taint_op1 = taint(cmp_inst->op1, sigma_prime, GetKey(program, func, cmp_inst->op1));
            std::set<std::string> taint_op2 = taint(cmp_inst->op2, sigma_prime, GetKey(program, func, cmp_inst->op2));
            
            joinSets(taint_op1, taint_op2);
            
            sigma_prime[GetKey(program, func, cmp_inst->lhs)] = taint_op1;
        }
        else if ((*inst).instrType == InstructionType::CopyInstrType)
        {
            CopyInstruction *copy_inst = (CopyInstruction *) inst;
            std::set<Variable*> DEF;
            std::set<Variable*> USE;
            /*
             * x = $copy y
             * sigma_prime[x] = taint(y)
            */
            sigma_prime[GetKey(program, func, copy_inst->lhs)] = taint(copy_inst->op, sigma_prime, GetKey(program, func, copy_inst->op));

        }
        else if ((*inst).instrType == InstructionType::AllocInstrType)
        {
             AllocInstruction *alloc_inst = (AllocInstruction *) inst;

            /*
             * x = $alloc y [id]
             * sigma_prime[x] = {}
            */
            sigma_prime[GetKey(program, func, alloc_inst->lhs)] = {};

        }
        else if ((*inst).instrType == InstructionType::GepInstrType)
        {
            GepInstruction *gep_inst = (GepInstruction *) inst;
            
            /*
             * x = $gep y op
             * sigma_prime[x] = taint(op) U taint(y)
            */
            std::set<std::string> taint_y = sigma_prime[GetKey(program, func, gep_inst->src)];
            std::set<std::string> taint_op = taint(gep_inst->idx, sigma_prime, GetKey(program, func, gep_inst->idx));
            
            joinSets(taint_op, taint_y);
            
            sigma_prime[GetKey(program, func, gep_inst->lhs)] = taint_op;

        }
        else if ((*inst).instrType == InstructionType::GfpInstrType)
        {
             GfpInstruction *gfp_inst = (GfpInstruction *) inst;

            /*
             * x = $gfp y id
             * sigma_prime[x] = taint(y)
            */
            sigma_prime[GetKey(program, func, gfp_inst->lhs)] = sigma_prime[GetKey(program, func, gfp_inst->src)];

        }
        else if ((*inst).instrType == InstructionType::AddrofInstrType)
        {
             AddrofInstruction *addrof_inst = (AddrofInstruction *) inst;
            /*
             * x = $addrof y
             * sigma_prime[x] = {}
            */
            sigma_prime[GetKey(program, func, addrof_inst->lhs)] = {};

        }
        else if ((*inst).instrType == InstructionType::LoadInstrType)
        {
            LoadInstruction *load_inst = (LoadInstruction *) inst;
            /*
             * x = $load y
             * sigma_prime[x] = taint(y) U (for all v in ptsto(y): taint(v))
            */
            std::string pointsToKey = GetKey(program, func, load_inst->src);
            std::set<std::string> taint_y = sigma_prime[pointsToKey];
            
            for (auto pointed_to: pointsTo[pointsToKey]) {
                joinSets(taint_y, sigma_prime[pointed_to]);
            }
            
            sigma_prime[GetKey(program, func, load_inst->lhs)] = taint_y;
        }
        else if ((*inst).instrType == InstructionType::StoreInstrType)
        {
            StoreInstruction *store_inst = (StoreInstruction *) inst;
            /*
             * $store x op
             * for all v in ptsto(x): sigma_prime[v] = sigma_prime[v] U (taint(op) U taint(x))
            */
            std::string pointsToKey = GetKey(program, func, store_inst->dst);
            std::set<std::string> taint_op = taint(store_inst->op, sigma_prime, GetKey(program, func, store_inst->op));
            std::set<std::string> taint_x = sigma_prime[pointsToKey];
            
            joinSets(taint_op, taint_x);

            for (auto pointed_to: pointsTo[pointsToKey]) {
                joinSets(sigma_prime[pointed_to], taint_op);
            }
        }
        else if ((*inst).instrType == InstructionType::CallExtInstrType)
        {
            CallExtInstruction *callext_inst = (CallExtInstruction *) inst;
            /*
             * [x=] $call_ext f(args...)
             * if f = source :
             *    sigma_prime[x] = {<source>} 
             *    for all v in reachable(args), sigma_prime[v] = sigma_prime[v] U {<source>}
             * else if f = sink :
             *    for all v in reachable(args), soln[<sink>] = soln[<sink>] U sigma_prime[v] - Can be done once in the end
             *    sigma_prime[x] = {}
             * else:
             *    sigma_prime[x] = {}
            */
           //std::cout << "Inside call ext for callee: " << callext_inst->extFuncName << std::endl;
            if (program->ext_funcs.find(callext_inst->extFuncName) != program->ext_funcs.end() && 
                isSource(program, program->ext_funcs[callext_inst->extFuncName]))
            {
                if (callext_inst->lhs) {
                    //std::cout << "Setting lhs to " << callext_inst->extFuncName << std::endl;
                    std::string lhsKey = GetKey(program, func, callext_inst->lhs);
                    sigma_prime[lhsKey] = {callext_inst->extFuncName};
                }

                std::set<std::string> reachable = GetReachable(callext_inst->args, pointsTo, program, func);
                for (std::string v : reachable) {
                    //std::cout << "Setting " << v << " to {" << callext_inst->extFuncName << "}" << std::endl;
                    
                    std::set<std::string> sources_set = {callext_inst->extFuncName};
                    joinSets(sigma_prime[v], sources_set);

                }
            }
            else if (program->ext_funcs.find(callext_inst->extFuncName) != program->ext_funcs.end() && 
                isSink(program, program->ext_funcs[callext_inst->extFuncName])) {
                // std::cout << "Sink " << callext_inst->extFuncName << std::endl;
                std::set<std::string> reachable = GetReachable(callext_inst->args, pointsTo, program, func);
                
                for (std::string v : reachable) {
                    joinSets(soln[callext_inst->extFuncName], sigma_prime[v]);
                }

                for (auto v: callext_inst->args) {
                    if (v->IsConstInt())
                        continue;
                    std::string v_key = GetKey(program, func, v->var);
                    joinSets(soln[callext_inst->extFuncName], sigma_prime[v_key]);
                }

                if (callext_inst->lhs) {
                    std::string lhsKey = GetKey(program, func, callext_inst->lhs);
                    sigma_prime[lhsKey] = {};
                }
            }
            else {
                std::cout << "Neither source nor sink " << callext_inst->extFuncName << std::endl;
                if (callext_inst->lhs) {
                    std::string lhsKey = GetKey(program, func, callext_inst->lhs);
                    sigma_prime[lhsKey] = {};
                }
            }
        }

        index += 1;
    }

    Instruction *terminal_instruction = bb->terminal;

    std::string pp = bb->label + "." + "term";
    
    if ((*terminal_instruction).instrType == InstructionType::JumpInstrType)
    {
        JumpInstruction *jump_inst = (JumpInstruction *) terminal_instruction;

        std::string bbs_to_output_key = func->name + "|" + jump_inst->label;
        std::string context = func->name;

        // Propagate store to jump label
        if (sensitivity == 1 || sensitivity == 2) {
            context = func->name + "|" + curr_cid;
            bbs_to_output_key = func->name + "|" + curr_cid + "|" + jump_inst->label;
        }
        
        if (joinAbsStore(bb2store[context][jump_inst->label], sigma_prime) || 
            bbs_to_output.count(bbs_to_output_key) == 0)
        {
            bbs_to_output.insert(bbs_to_output_key);
            if (sensitivity > 0)
                worklist.push_back({func->name + "|" + curr_cid, jump_inst->label});
            else
                worklist.push_back({func->name, jump_inst->label});
        }
    }
    else if ((*terminal_instruction).instrType == InstructionType::BranchInstrType)
    {
        BranchInstruction *branch_inst = (BranchInstruction *) terminal_instruction;

        std::string bbs_to_output_key_tt = func->name + "|" + branch_inst->tt;
        std::string bbs_to_output_key_ff = func->name + "|" + branch_inst->ff;

        if (sensitivity == 1 || sensitivity == 2) {
            bbs_to_output_key_tt = func->name + "|" + curr_cid + "|" + branch_inst->tt;
            bbs_to_output_key_ff = func->name + "|" + curr_cid + "|" + branch_inst->ff;
        }

        std::string context = func->name;
        if (sensitivity == 1 || sensitivity == 2)
            context = func->name + "|" + curr_cid;
        if (joinAbsStore(bb2store[context][branch_inst->tt], sigma_prime) ||
            bbs_to_output.count(bbs_to_output_key_tt) == 0)
        {
            bbs_to_output.insert(bbs_to_output_key_tt);
            if (sensitivity == 1 || sensitivity == 2)
            {
                worklist.push_back({func->name + "|" + curr_cid, branch_inst->tt});
                //std::cout << "Adding to worklist: " << func->name + "|" + curr_cid << " " << branch_inst->tt << std::endl;
            }
            else
                worklist.push_back({func->name, branch_inst->tt});
        }

        if (joinAbsStore(bb2store[context][branch_inst->ff], sigma_prime) ||
            bbs_to_output.count(bbs_to_output_key_ff) == 0)
        {
            bbs_to_output.insert(bbs_to_output_key_ff);
            if (sensitivity == 1 || sensitivity == 2) {
                worklist.push_back({func->name + "|" + curr_cid, branch_inst->ff});
                //std::cout << "Adding to worklist: " << func->name + "|" + curr_cid << " " << branch_inst->ff << std::endl;
            }
            else
                worklist.push_back({func->name, branch_inst->ff});
        }
    }
    else if ((*terminal_instruction).instrType == InstructionType::RetInstrType)
    {
        RetInstruction *ret_inst = (RetInstruction *) terminal_instruction;
        /*
        * If we are in main, then no-op
        * Let ret_store = GetReturnedStore(...)
        * call_returned[<curr func>] = ret_store
        * for all (func,bb) in call_edges[<curr func>], where terminal of bb is a call_(dir/idr) instruction:
        *    let caller_store = get_caller_store(ret_store, x)
        *    propagate caller_store to (func, next_bb)
        */

        if (func->name != "main") {

            AbsStore ret_store = GetReturnedStore(program, pointsTo, sigma_prime, func, ret_inst->op);

            // for ci, curr_context = func_name
            std::string curr_context = func->name;
            if (sensitivity == 1 || sensitivity == 2) {
                // Popping off from callstring stack is not needed since that is handled by k-limiting the callstring
                curr_context = curr_cid;
            }

            call_returned[{func->name, curr_context}] = ret_store;

            //std::cout << "Inside ret instruction for " << func->name << std::endl;

            // Print ret store
            //std::cout << "Ret store for (" << func->name << " , " << curr_context << ") is: " << std::endl;
            //PrintAbsStore(ret_store);

            for (auto it = call_edges[{func->name, curr_context}].begin(); it != call_edges[{func->name, curr_context}].end(); it++) {
                
                std::string caller_bb = it->first.substr(it->first.find(".") + 1);
                std::string caller_func = it->first.substr(0, it->first.find("."));
                std::string caller_context = it->second;
                
                Variable *caller_lhs = nullptr;
                Instruction *instr = program->funcs[caller_func]->bbs[caller_bb]->terminal;
                std::string next_bb;
                
                if (instr->instrType == InstructionType::CallDirInstrType)
                {
                    CallDirInstruction *calldir_inst = (CallDirInstruction*) instr;
                    caller_lhs = calldir_inst->lhs;
                    next_bb = calldir_inst->next_bb;
                }
                else if (instr->instrType == InstructionType::CallIdrInstrType)
                {
                    CallIdrInstruction *callidr_inst = (CallIdrInstruction*) instr;
                    caller_lhs = callidr_inst->lhs;
                    next_bb = callidr_inst->next_bb;
                }
                else
                {
                    std::cout << "Unknown call_edges terminal instruction type" << std::endl;
                }
                
                AbsStore caller_store = GetCallerStore(program, ret_store, caller_lhs, program->funcs[caller_func]);

                // Print caller store
                //std::cout << "Caller store for " << caller_func << " is: " << std::endl;
                //PrintAbsStore(caller_store);

                // Propagate caller store to (func, next_bb)
                std::string bbs_to_output_key = caller_func + "|" + next_bb;
                std::string bb2store_key = caller_func;
                if (sensitivity == 1 || sensitivity == 2) {
                    bbs_to_output_key = caller_func + "|" + caller_context + "|" + next_bb;
                    bb2store_key = caller_func + "|" + caller_context;
                }
                if (joinAbsStore(bb2store[bb2store_key][next_bb], caller_store) ||
                    bbs_to_output.count(bbs_to_output_key) == 0)
                {
                    bbs_to_output.insert(bbs_to_output_key);
                    if (sensitivity == 1 || sensitivity == 2) {
                        worklist.push_back({caller_func + "|" + caller_context, next_bb});
                        //std::cout << "Pushed to worklist: " << caller_func + "|" + caller_context << " , " << next_bb << std::endl;
                    }
                    else
                        worklist.push_back({caller_func, next_bb});
                }
            }
        }
    }
    else if ((*terminal_instruction).instrType == InstructionType::CallDirInstrType)
    {
        CallDirInstruction *calldir_inst = (CallDirInstruction *) terminal_instruction;
        /*
        * 1. Update call_edges to include new call site. Populate it with correct context id based on sensitivity
        * 2. Get callee store
        * 3. Propagate callee store to (<func>, entry), if changed add to worklist
        * 4. store[x] = bottom
        * 5. Propagate store to next bb
        * 6. if call_returned[<func>] = ret_store then
                let caller_store = get_caller_store(ret_store, x)
                propagate caller_store to bb
        */
        //std::cout << "Inside calldir in callsite: " << func->name + "." + bb->label << std::endl;

        std::string curr_context = func->name + "." + bb->label;
        if (sensitivity == 0) {
            // For context insentive, func_name = context_id
            call_edges[{calldir_inst->callee, calldir_inst->callee}].insert({curr_context, curr_context});
        }
        else if (sensitivity == 1 || sensitivity == 2) {
            /* Add callsite to stack of call-sites which is our context
             * callsite = func.bb
            */
            std::string callee_context = AddToCurrentContext(func->name + "." + bb->label, sensitivity, curr_cid);
            /*
            * For callstring-1 and callstring-2 sensitivity, context_id = callee_context
            * Map <callee func, callee context> = { set of <caller func, caller context> pairs }
            */ 
            call_edges[{calldir_inst->callee, callee_context}].insert({curr_context, curr_cid});
            //std::cout << "Added to call_edges: " << calldir_inst->callee << " : " << callee_context << " -> {" << curr_context << " , " << curr_cid <<  std::endl;
        }

        AbsStore callee_store = GetCalleeStore(program, pointsTo, sigma_prime, calldir_inst->callee, calldir_inst->args, func);
    
        //std::cout << "Callee store for (" << calldir_inst->callee << ") is: " << std::endl;
        //PrintAbsStore(callee_store);

        std::string bbs_to_output_key = calldir_inst->callee + "|entry";
        std::string bb2store_key = calldir_inst->callee;
        if (sensitivity == 1 || sensitivity == 2) {
            std::string callee_context = AddToCurrentContext(func->name + "." + bb->label, sensitivity, curr_cid);
            bbs_to_output_key = calldir_inst->callee + "|" + callee_context + "|entry";
            bb2store_key = calldir_inst->callee + "|" + callee_context;
        }
        bool callee_store_changed = joinAbsStore(bb2store[bb2store_key]["entry"], callee_store);

        // Print bb2store
        //std::cout << "bb2store[" << bb2store_key << "][entry]" << std::endl;
        //PrintAbsStore(bb2store[bb2store_key]["entry"]);

        // Propagate callee store to (<func>, entry), if changed add to worklist
        if (bbs_to_output.count(bbs_to_output_key) == 0 || callee_store_changed)
        {
            bbs_to_output.insert(bbs_to_output_key);
            if (sensitivity == 1 || sensitivity == 2) {
                std::string callee_context = AddToCurrentContext(func->name + "." + bb->label, sensitivity, curr_cid);
                worklist.push_back({calldir_inst->callee + "|" + callee_context, "entry"});
                //std::cout << "Pushed to worklist: " << calldir_inst->callee + "|" + callee_context << " , entry" << std::endl;
            }
            else
                worklist.push_back({calldir_inst->callee, "entry"});
        }

        std::string bbs_to_output_key_next = func->name + "|" + calldir_inst->next_bb;
        bb2store_key = func->name;
        if (sensitivity == 1 || sensitivity == 2) {
            bbs_to_output_key_next = func->name + "|" + curr_cid + "|" + calldir_inst->next_bb;
            bb2store_key = func->name + "|" + curr_cid;
        }

        // store[x] = bottom
        if (calldir_inst->lhs) {
            std::string lhsKey = GetKey(program, func, calldir_inst->lhs);
            sigma_prime[lhsKey] = {};
        }

        // Propagate store to next bb
        if (joinAbsStore(bb2store[bb2store_key][calldir_inst->next_bb], sigma_prime) ||
            bbs_to_output.count(bbs_to_output_key_next) == 0)
        {
            bbs_to_output.insert(bbs_to_output_key_next);
            if (sensitivity == 1 || sensitivity == 2) {
                worklist.push_back({func->name + "|" + curr_cid, calldir_inst->next_bb});
            }
            else if (sensitivity == 0)
                worklist.push_back({func->name, calldir_inst->next_bb});
            //std::cout << "Pushed to worklist: " << func->name + "." + calldir_inst->next_bb << std::endl;
        }

        /* if call_returned[<func>] = ret_store then
         * let caller_store = get_caller_store(ret_store, x)
         * propagate caller_store to bb
        */

        std::string context = calldir_inst->callee;
        if (sensitivity == 1 || sensitivity == 2)
            context = AddToCurrentContext(func->name + "." + bb->label, sensitivity, curr_cid);

        AbsStore returned_store = call_returned[{calldir_inst->callee, context}];
        Operand *callee_ret_op = func_ret_op[calldir_inst->callee]->op; 
        AbsStore ret_store;
        if (sensitivity == 0)
            ret_store = GetReturnedStore(program, pointsTo, sigma_prime, func, callee_ret_op);
        else if (sensitivity == 1 || sensitivity == 2)
        {
            ret_store = GetReturnedStore(program, pointsTo, bb2store[calldir_inst->callee + "|" + context]["entry"], program->funcs[calldir_inst->callee], callee_ret_op);
        }

        /*std::cout << "Returned store for (" << calldir_inst->callee << " , " << context << ") is: " << std::endl;
        PrintAbsStore(returned_store);
        std::cout << "Ret store for " << calldir_inst->callee << " is: " << std::endl;
        PrintAbsStore(ret_store);*/

        if (returned_store.size() > 0)
        {
            //std::cout << "Returned store is not empty" << std::endl;
            AbsStore caller_store = GetCallerStore(program, returned_store, calldir_inst->lhs, func);
            //std::cout << "Caller store for " << func->name << " is: " << std::endl;
            //PrintAbsStore(caller_store);
            std::string bb2store_key = func->name;
            if (sensitivity == 1 || sensitivity == 2)
                bb2store_key = func->name + "|" + curr_cid;
            if (joinAbsStore(bb2store[bb2store_key][calldir_inst->next_bb], caller_store) ||
                bbs_to_output.count(bbs_to_output_key_next) == 0)
            {
                bbs_to_output.insert(bbs_to_output_key_next);
                if (sensitivity == 1 || sensitivity == 2){
                    worklist.push_back({func->name + "|" + curr_cid, calldir_inst->next_bb});
                    //std::cout << "Pushed to worklist: " << func->name + "|" + curr_cid << " , " << calldir_inst->next_bb << std::endl;
                }
                else
                    worklist.push_back({func->name, calldir_inst->next_bb});
                //std::cout << "Pushed to worklist: " << func->name + "." + calldir_inst->next_bb << std::endl;
            }
        }

        // TODO - Check if this equality check works as expected
        /*if (isAbsStoreEqual(returned_store, ret_store)) {
            std::cout << "Returned store and ret store are equal" << std::endl;
            AbsStore caller_store = GetCallerStore(program, sigma_prime, calldir_inst->lhs, func);
            std::string bb2store_key = func->name;
            if (sensitivity == 1 || sensitivity == 2)
                bb2store_key = func->name + "|" + curr_cid;
            if (joinAbsStore(bb2store[bb2store_key][calldir_inst->next_bb], caller_store) ||
                bbs_to_output.count(bbs_to_output_key_next) == 0)
            {
                bbs_to_output.insert(bbs_to_output_key_next);
                if (sensitivity == 1 || sensitivity == 2){
                    worklist.push_back({func->name + "|" + curr_cid, calldir_inst->next_bb});
                    std::cout << "Pushed to worklist: " << func->name + "|" + curr_cid << " , " << calldir_inst->next_bb << std::endl;
                }
                else
                    worklist.push_back({func->name, calldir_inst->next_bb});
                //std::cout << "Pushed to worklist: " << func->name + "." + calldir_inst->next_bb << std::endl;
            }
        }*/

    }
    else if ((*terminal_instruction).instrType == InstructionType::CallIdrInstrType)
    {
        CallIdrInstruction *callidir_inst = (CallIdrInstruction *) terminal_instruction;

        /*
        * Run the below logic for each pointsTo of fp
        * 1. Update call_edges to include new call site. Populate it with correct context id based on sensitivity
        * 2. Get callee store
        * 3. Propagate callee store to (<func>, entry), if changed add to worklist
        * 4. store[x] = bottom
        * 5. Propagate store to next bb
        * 6. if call_returned[<func>] = ret_store then
                let caller_store = get_caller_store(ret_store, x)
                propagate caller_store to bb
        */

        std::string pointoToKey = isGlobalVar(callidir_inst->fp, program, func->name) ? callidir_inst->fp->name : func->name + "." + callidir_inst->fp->name;

        for(auto points_to: pointsTo[pointoToKey])
        {
            std::string curr_context = func->name + "." + bb->label;
            if (sensitivity == 0) {
                // For context insentive, func_name = context_id
                call_edges[{points_to, points_to}].insert({curr_context, curr_context});
            }
            else if (sensitivity == 1 || sensitivity == 2) {
                std::string callee_context = AddToCurrentContext(func->name + "." + bb->label, sensitivity, curr_cid);
                call_edges[{points_to, callee_context}].insert({curr_context, curr_cid});
            }

            //std::cout << "Inside callidir for context: " << curr_context << std::endl;

            AbsStore callee_store = GetCalleeStore(program, pointsTo, sigma_prime, points_to, callidir_inst->args, func);
        
            //std::cout << "Callee store for " << points_to << " is: " << std::endl;
            //PrintAbsStore(callee_store);

            std::string bb2store_key = points_to;
            std::string bbs_to_output_key = points_to + "|entry";

            if (sensitivity == 1 || sensitivity == 2) {
                std::string callee_context = AddToCurrentContext(func->name + "." + bb->label, sensitivity, curr_cid);
                bb2store_key = points_to + "|" + callee_context;
                bbs_to_output_key = points_to + "|" + callee_context + "|entry";
            }

            bool callee_store_changed = joinAbsStore(bb2store[bb2store_key]["entry"], callee_store);

            // Print bb2store
            //std::cout << "bb2store[" << points_to << ".entry]" << std::endl;
            //PrintAbsStore(bb2store[points_to]["entry"]);

            // Propagate callee store to (<func>, entry), if changed add to worklist

            if (bbs_to_output.count(bbs_to_output_key) == 0 || callee_store_changed)
            {
                bbs_to_output.insert(bbs_to_output_key);
                if (sensitivity == 1 || sensitivity == 2) {
                    std::string callee_context = AddToCurrentContext(func->name + "." + bb->label, sensitivity, curr_cid);
                    worklist.push_back({points_to + "|" + callee_context, "entry"});
                }
                else if (sensitivity == 0)
                    worklist.push_back({points_to, "entry"});
                //std::cout << "Pushed to worklist: " << points_to + ".entry" << std::endl;
            }

            /* if call_returned[<func>] = ret_store then
             * let caller_store = get_caller_store(ret_store, x)
             * propagate caller_store to bb
            */

            std::string context = points_to;
            if (sensitivity == 1 || sensitivity == 2)
                context = AddToCurrentContext(func->name + "." + bb->label, sensitivity, curr_cid);

            AbsStore returned_store = call_returned[{points_to, context}];
            Operand *callee_ret_op = func_ret_op[points_to]->op; 
            AbsStore ret_store;

            if (sensitivity == 0)
                ret_store = GetReturnedStore(program, pointsTo, sigma_prime, func, callee_ret_op);
            else if (sensitivity == 1 || sensitivity == 2)
            {
                ret_store = GetReturnedStore(program, pointsTo, bb2store[points_to + "|" + context]["entry"], program->funcs[points_to], callee_ret_op);
            }

            std::cout << "Returned store for " << points_to << " is: " << std::endl;
            PrintAbsStore(returned_store);
            std::cout << "Ret store for " << points_to << " is: " << std::endl;
            PrintAbsStore(ret_store);

            if (returned_store.size() > 0)
            {
                std::string bbs_to_output_key_next = func->name + "|" + callidir_inst->next_bb;
                bb2store_key = func->name;
                if (sensitivity == 1 || sensitivity == 2) {
                    bbs_to_output_key_next = func->name + "|" + curr_cid + "|" + callidir_inst->next_bb;
                    bb2store_key = func->name + "|" + curr_cid;
                }

                //std::cout << "Returned store is not empty" << std::endl;
                AbsStore caller_store = GetCallerStore(program, returned_store, callidir_inst->lhs, func);
                //std::cout << "Caller store for " << func->name << " is: " << std::endl;
                //PrintAbsStore(caller_store);
                std::string bb2store_key = func->name;
                if (sensitivity == 1 || sensitivity == 2)
                    bb2store_key = func->name + "|" + curr_cid;
                if (joinAbsStore(bb2store[bb2store_key][callidir_inst->next_bb], caller_store) ||
                    bbs_to_output.count(bbs_to_output_key_next) == 0)
                {
                    bbs_to_output.insert(bbs_to_output_key_next);
                    if (sensitivity == 1 || sensitivity == 2){
                        worklist.push_back({func->name + "|" + curr_cid, callidir_inst->next_bb});
                        //std::cout << "Pushed to worklist: " << func->name + "|" + curr_cid << " , " << calldir_inst->next_bb << std::endl;
                    }
                    else
                        worklist.push_back({func->name, callidir_inst->next_bb});
                    //std::cout << "Pushed to worklist: " << func->name + "." + calldir_inst->next_bb << std::endl;
                }
            }

            // TODO - Check if this equality check works as expected
            /*std::string bbs_to_output_key_next = func->name + "." + callidir_inst->next_bb;
            if (sensitivity > 0)
                bbs_to_output_key_next = func->name + "." + callstring_stack + "." + callidir_inst->next_bb;
            if (isAbsStoreEqual(returned_store, ret_store)) {
                AbsStore caller_store = GetCallerStore(program, sigma_prime, callidir_inst->lhs, func);
                std::string bb2store_key = func->name;
                if (sensitivity == 1 || sensitivity == 2)
                    bb2store_key = func->name + "|" + callstring_stack;
                if (joinAbsStore(bb2store[bb2store_key][callidir_inst->next_bb], caller_store) ||
                    bbs_to_output.count(bbs_to_output_key_next) == 0)
                {
                    bbs_to_output.insert(bbs_to_output_key_next);
                    if (sensitivity > 0)
                        worklist.push_back({func->name + "|" + callstring_stack, callidir_inst->next_bb});
                    else
                        worklist.push_back({func->name, callidir_inst->next_bb});
                    //std::cout << "Pushed to worklist: " << func->name + "." + callidir_inst->next_bb << std::endl;
                }
            }*/
        }

        std::string bbs_to_output_key_next = func->name + "|" + callidir_inst->next_bb;
        std::string bb2store_key = func->name;
        if (sensitivity == 1 || sensitivity == 2) {
            bbs_to_output_key_next = func->name + "|" + curr_cid + "|" + callidir_inst->next_bb;
            bb2store_key = func->name + "|" + curr_cid;
        }

        // store[x] = bottom
        if (callidir_inst->lhs) {
            std::string lhsKey = GetKey(program, func, callidir_inst->lhs);
            sigma_prime[lhsKey] = {};
        }

        // Propagate store to next bb
        if (joinAbsStore(bb2store[bb2store_key][callidir_inst->next_bb], sigma_prime) ||
            bbs_to_output.count(bbs_to_output_key_next) == 0)
        {
            bbs_to_output.insert(bbs_to_output_key_next);
            if (sensitivity == 1 || sensitivity == 2) {
                worklist.push_back({func->name + "|" + curr_cid, callidir_inst->next_bb});
            }
            else if (sensitivity == 0)
                worklist.push_back({func->name, callidir_inst->next_bb});
            //std::cout << "Pushed to worklist: " << func->name + "." + callidir_inst->next_bb << std::endl;
        }
    }
    else
    {
        std::cout << "Unknown terminal instruction type" << std::endl;
    }

    /*
    * Print sigma prime
    */
    /*std::cout << "sigma_prime[" << func->name << "." << bb->label << "]:" << std::endl;
    for (auto it = sigma_prime.begin(); it != sigma_prime.end(); it++) {
        std::cout << it->first << " -> {";
        for (auto it2 = it->second.begin(); it2 != it->second.end(); it2++) {
            std::cout << *it2 << ",";
        }
        std::cout << "}" << std::endl;
    }
    std::cout << std::endl;*/
    
    return;
}