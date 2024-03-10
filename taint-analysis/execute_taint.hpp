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
    // TODO - Verify whole function

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
    // TODO - Verify whole function
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
    // TODO - Verify whole function
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

void execute(
    Program *program,
    Function* func,
    BasicBlock *bb,
    std::map<std::string, std::map<std::string, AbsStore>> &bb2store, // function -> bb -> variable -> set of sources
    std::deque<std::pair<std::string, std::string>> &worklist,
    std::set<std::string> &bbs_to_output,
    std::map<std::string, std::set<std::string>> &soln,
    std::unordered_map<std::string, std::set<std::string>> pointsTo,
    std::map<std::pair<std::string, std::string>, std::set<std::string>> &call_edges,
    std::map<std::pair<std::string, std::string>, AbsStore> &call_returned,
    std::map<std::string, RetInstruction*> func_ret_op
)
{
    std::map<std::string, std::map<std::string, AbsStore>> sigma_prime = bb2store;  
    
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
            std::set<std::string> taint_op1 = taint(arith_inst->op1, sigma_prime[func->name][bb->label], GetKey(program, func, arith_inst->op1)); // TODO - Verify that is sigma_prime and not bb2store 
            std::set<std::string> taint_op2 = taint(arith_inst->op2, sigma_prime[func->name][bb->label], GetKey(program, func, arith_inst->op2));
            joinSets(taint_op1, taint_op2);
            sigma_prime[func->name][bb->label][GetKey(program, func, arith_inst->lhs)] = taint_op1;

        }
        else if ((*inst).instrType == InstructionType::CmpInstrType)
        {
             CmpInstruction *cmp_inst = (CmpInstruction *) inst;
             
            /*
             * x = $cmp gt y z
             * sigma_prime[x] = taint(op1) U taint(op2)
            */
            std::set<std::string> taint_op1 = taint(cmp_inst->op1, sigma_prime[func->name][bb->label], GetKey(program, func, cmp_inst->op1));
            std::set<std::string> taint_op2 = taint(cmp_inst->op2, sigma_prime[func->name][bb->label], GetKey(program, func, cmp_inst->op2));
            
            joinSets(taint_op1, taint_op2);
            
            sigma_prime[func->name][bb->label][GetKey(program, func, cmp_inst->lhs)] = taint_op1;
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
            sigma_prime[func->name][bb->label][GetKey(program, func, copy_inst->lhs)] = taint(copy_inst->op, sigma_prime[func->name][bb->label], GetKey(program, func, copy_inst->op));

        }
        else if ((*inst).instrType == InstructionType::AllocInstrType)
        {
             AllocInstruction *alloc_inst = (AllocInstruction *) inst;

            /*
             * x = $alloc y [id]
             * sigma_prime[x] = {}
            */
            sigma_prime[func->name][bb->label][GetKey(program, func, alloc_inst->lhs)] = {};

        }
        else if ((*inst).instrType == InstructionType::GepInstrType)
        {
            GepInstruction *gep_inst = (GepInstruction *) inst;
            
            /*
             * x = $gep y op
             * sigma_prime[x] = taint(op) U taint(y)
            */
            std::set<std::string> taint_y = sigma_prime[func->name][bb->label][GetKey(program, func, gep_inst->src)];
            std::set<std::string> taint_op = taint(gep_inst->idx, sigma_prime[func->name][bb->label], GetKey(program, func, gep_inst->idx));
            
            joinSets(taint_op, taint_y);
            
            sigma_prime[func->name][bb->label][GetKey(program, func, gep_inst->lhs)] = taint_op;

        }
        else if ((*inst).instrType == InstructionType::GfpInstrType)
        {
             GfpInstruction *gfp_inst = (GfpInstruction *) inst;

            /*
             * x = $gfp y id
             * sigma_prime[x] = taint(y)
            */
            sigma_prime[func->name][bb->label][GetKey(program, func, gfp_inst->lhs)] = sigma_prime[func->name][bb->label][GetKey(program, func, gfp_inst->src)];

        }
        else if ((*inst).instrType == InstructionType::AddrofInstrType)
        {
             AddrofInstruction *addrof_inst = (AddrofInstruction *) inst;
            /*
             * x = $addrof y
             * sigma_prime[x] = {}
            */
            sigma_prime[func->name][bb->label][GetKey(program, func, addrof_inst->lhs)] = {};

        }
        else if ((*inst).instrType == InstructionType::LoadInstrType)
        {
            LoadInstruction *load_inst = (LoadInstruction *) inst;
            /*
             * x = $load y
             * sigma_prime[x] = taint(y) U (for all v in ptsto(y): taint(v))
            */
            std::string pointsToKey = GetKey(program, func, load_inst->src);
            std::set<std::string> taint_y = sigma_prime[func->name][bb->label][pointsToKey];
            
            for (auto pointed_to: pointsTo[pointsToKey]) {
                joinSets(taint_y, sigma_prime[func->name][bb->label][pointed_to]);
            }
            
            sigma_prime[func->name][bb->label][GetKey(program, func, load_inst->lhs)] = taint_y;
        }
        else if ((*inst).instrType == InstructionType::StoreInstrType)
        {
            StoreInstruction *store_inst = (StoreInstruction *) inst;
            /*
             * $store x op
             * for all v in ptsto(x): sigma_prime[v] = sigma_prime[v] U (taint(op) U taint(x))
            */
            std::string pointsToKey = GetKey(program, func, store_inst->dst);
            std::set<std::string> taint_op = taint(store_inst->op, sigma_prime[func->name][bb->label], GetKey(program, func, store_inst->op));
            std::set<std::string> taint_x = sigma_prime[func->name][bb->label][pointsToKey];
            
            joinSets(taint_op, taint_x);

            for (auto pointed_to: pointsTo[pointsToKey]) {
                joinSets(sigma_prime[func->name][bb->label][pointed_to], taint_op);
            }
        }
        else if ((*inst).instrType == InstructionType::CallExtInstrType)
        {
            // TODO - Need to verify whole logic
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
                    sigma_prime[func->name][bb->label][lhsKey] = {callext_inst->extFuncName};
                }

                std::set<std::string> reachable = GetReachable(callext_inst->args, pointsTo, program, func);
                for (std::string v : reachable) {
                    //std::cout << "Setting " << v << " to {" << callext_inst->extFuncName << "}" << std::endl;
                    // TODO - Reachable can have pts to from different funcs. Handle the naming in sigma_prime
                    
                    std::set<std::string> sources_set = {callext_inst->extFuncName};

                    joinSets(sigma_prime[func->name][bb->label][v], sources_set);

                    /*
                    std::string func_name = v.find(".") == std::string::npos ? "" : v.substr(0, v.find("."));
                    std::cout << "Func name: " << func_name << std::endl;
                    if (func_name == "")
                        joinSets(sigma_prime[func->name][bb->label][v], sources_set);
                    else if (func_name == func->name)
                        joinSets(sigma_prime[func->name][bb->label][v.substr(func_name.size() + 1)], sources_set);
                    */
                    // TODO - How to know bb label when it is from a different function?
                    // TODO - Or every local variable should be prefixed by the corresponding func name when adding to store
                    /*else
                        joinSets(sigma_prime[func->name][bb->label][v.substr(func_name.size() + 1)], sources_set);*/
                }
            }
            else if (program->ext_funcs.find(callext_inst->extFuncName) != program->ext_funcs.end() && 
                isSink(program, program->ext_funcs[callext_inst->extFuncName])) {
                // std::cout << "Sink " << callext_inst->extFuncName << std::endl;
                std::set<std::string> reachable = GetReachable(callext_inst->args, pointsTo, program, func);
                
                for (std::string v : reachable) {
                    joinSets(soln[callext_inst->extFuncName], sigma_prime[func->name][bb->label][v]);
                }

                for (auto v: callext_inst->args) {
                    if (v->IsConstInt())
                        continue;
                    std::string v_key = GetKey(program, func, v->var);
                    joinSets(soln[callext_inst->extFuncName], sigma_prime[func->name][bb->label][v_key]);
                }

                if (callext_inst->lhs) {
                    std::string lhsKey = GetKey(program, func, callext_inst->lhs);
                    sigma_prime[func->name][bb->label][lhsKey] = {};
                }
            }
            else {
                std::cout << "Neither source nor sink " << callext_inst->extFuncName << std::endl;
                if (callext_inst->lhs) {
                    std::string lhsKey = GetKey(program, func, callext_inst->lhs);
                    sigma_prime[func->name][bb->label][lhsKey] = {};
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
        // Propagate store to jump label
        if (joinAbsStore(bb2store[func->name][jump_inst->label], sigma_prime[func->name][bb->label]) || 
            bbs_to_output.count(func->name + "." + jump_inst->label) == 0)
        {
            bbs_to_output.insert(func->name + "." + jump_inst->label);
            worklist.push_back({func->name, jump_inst->label});
        }
    }
    else if ((*terminal_instruction).instrType == InstructionType::BranchInstrType)
    {
        BranchInstruction *branch_inst = (BranchInstruction *) terminal_instruction;

        if (joinAbsStore(bb2store[func->name][branch_inst->tt], sigma_prime[func->name][bb->label]) ||
            bbs_to_output.count(func->name + "." + branch_inst->tt) == 0)
        {
            bbs_to_output.insert(func->name + "." + branch_inst->tt);
            worklist.push_back({func->name, branch_inst->tt});
        }

        if (joinAbsStore(bb2store[func->name][branch_inst->ff], sigma_prime[func->name][bb->label]) ||
            bbs_to_output.count(func->name + "." + branch_inst->ff) == 0)
        {
            bbs_to_output.insert(func->name + "." + branch_inst->ff);
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
            
            AbsStore ret_store = GetReturnedStore(program, pointsTo, sigma_prime[func->name][bb->label], func, ret_inst->op);

            call_returned[{func->name, func->name}] = ret_store;

            // Print ret store
            //std::cout << "Ret store for " << func->name << " is: " << std::endl;
            //PrintAbsStore(ret_store);

            for (auto it = call_edges[{func->name, func->name}].begin(); it != call_edges[{func->name, func->name}].end(); it++) {
                // TODO - Handle different contexts differently based on sensitivity
                std::string caller_bb = it->substr(it->find(".") + 1);
                std::string caller_func = it->substr(0, it->find("."));
                
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
                if (joinAbsStore(bb2store[caller_func][next_bb], caller_store) ||
                    bbs_to_output.count(caller_func + "." + next_bb) == 0)
                {
                    bbs_to_output.insert(caller_func + "." + next_bb);
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

        // TODO - Add check for sensitivity here and modify context accordingly
        std::string curr_context = func->name + "." + bb->label;
        // For context insentive, func_name = context_id
        call_edges[{calldir_inst->callee, calldir_inst->callee}].insert(curr_context);

        //std::cout << "Inside calldir for context: " << curr_context << std::endl;

        AbsStore callee_store = GetCalleeStore(program, pointsTo, sigma_prime[func->name][bb->label], calldir_inst->callee, calldir_inst->args, func);
    
        //std::cout << "Callee store for " << calldir_inst->callee << " is: " << std::endl;
        //PrintAbsStore(callee_store);

        bool callee_store_changed = joinAbsStore(bb2store[calldir_inst->callee]["entry"], callee_store);

        // Print bb2store
        //std::cout << "bb2store[" << calldir_inst->callee << ".entry]" << std::endl;
        //PrintAbsStore(bb2store[calldir_inst->callee]["entry"]);

        // Propagate callee store to (<func>, entry), if changed add to worklist
        if (bbs_to_output.count(calldir_inst->callee + ".entry") == 0 || callee_store_changed)
        {
            bbs_to_output.insert(calldir_inst->callee + ".entry");
            worklist.push_back({calldir_inst->callee, "entry"});
            //std::cout << "Pushed to worklist: " << calldir_inst->callee + ".entry" << std::endl;
        }

        // store[x] = bottom
        // TODO - Check if this has to be removed or made an empty set
        if (calldir_inst->lhs) {
            std::string lhsKey = GetKey(program, func, calldir_inst->lhs);
            sigma_prime[func->name][bb->label][lhsKey] = {};
        }

        // Propagate store to next bb
        if (joinAbsStore(bb2store[func->name][calldir_inst->next_bb], sigma_prime[func->name][bb->label]) ||
            bbs_to_output.count(func->name + "." + calldir_inst->next_bb) == 0)
        {
            bbs_to_output.insert(func->name + "." + calldir_inst->next_bb);
            worklist.push_back({func->name, calldir_inst->next_bb});
            //std::cout << "Pushed to worklist: " << func->name + "." + calldir_inst->next_bb << std::endl;
        }

        // if call_returned[<func>] = ret_store then
        // let caller_store = get_caller_store(ret_store, x)
        // propagate caller_store to bb

        AbsStore returned_store = call_returned[{calldir_inst->callee, calldir_inst->callee}];
        Operand *callee_ret_op = func_ret_op[calldir_inst->callee]->op; 
        AbsStore ret_store = GetReturnedStore(program, pointsTo, sigma_prime[func->name][bb->label], func, callee_ret_op);

        /*std::cout << "Returned store for " << calldir_inst->callee << " is: " << std::endl;
        PrintAbsStore(returned_store);
        std::cout << "Ret store for " << calldir_inst->callee << " is: " << std::endl;
        PrintAbsStore(ret_store);*/

        // TODO - Check if this equality check works as expected
        if (returned_store == ret_store) {
            AbsStore caller_store = GetCallerStore(program, sigma_prime[func->name][bb->label], calldir_inst->lhs, func);
            if (joinAbsStore(bb2store[func->name][calldir_inst->next_bb], caller_store) ||
                bbs_to_output.count(func->name + "." + calldir_inst->next_bb) == 0)
            {
                bbs_to_output.insert(func->name + "." + calldir_inst->next_bb);
                worklist.push_back({func->name, calldir_inst->next_bb});
                //std::cout << "Pushed to worklist: " << func->name + "." + calldir_inst->next_bb << std::endl;
            }
        }

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
            // For context insentive, func_name = context_id
            call_edges[{points_to, points_to}].insert(curr_context);

            //std::cout << "Inside callidir for context: " << curr_context << std::endl;

            AbsStore callee_store = GetCalleeStore(program, pointsTo, sigma_prime[func->name][bb->label], points_to, callidir_inst->args, func);
        
            //std::cout << "Callee store for " << points_to << " is: " << std::endl;
            //PrintAbsStore(callee_store);

            bool callee_store_changed = joinAbsStore(bb2store[points_to]["entry"], callee_store);

            // Print bb2store
            //std::cout << "bb2store[" << points_to << ".entry]" << std::endl;
            //PrintAbsStore(bb2store[points_to]["entry"]);

            // Propagate callee store to (<func>, entry), if changed add to worklist
            if (bbs_to_output.count(points_to + ".entry") == 0 || callee_store_changed)
            {
                bbs_to_output.insert(points_to + ".entry");
                worklist.push_back({points_to, "entry"});
                //std::cout << "Pushed to worklist: " << points_to + ".entry" << std::endl;
            }

            // if call_returned[<func>] = ret_store then
            // let caller_store = get_caller_store(ret_store, x)
            // propagate caller_store to bb

            AbsStore returned_store = call_returned[{points_to, points_to}];
            Operand *callee_ret_op = func_ret_op[points_to]->op; 
            AbsStore ret_store = GetReturnedStore(program, pointsTo, sigma_prime[func->name][bb->label], func, callee_ret_op);

            /*std::cout << "Returned store for " << points_to << " is: " << std::endl;
            PrintAbsStore(returned_store);
            std::cout << "Ret store for " << points_to << " is: " << std::endl;
            PrintAbsStore(ret_store);

            // Print bbs_to_output
            std::cout << "bbs_to_output: " << std::endl;
            for (auto it = bbs_to_output.begin(); it != bbs_to_output.end(); it++) {
                std::cout << *it << std::endl;
            }*/

            // TODO - Check if this equality check works as expected
            if (returned_store == ret_store) {
                AbsStore caller_store = GetCallerStore(program, sigma_prime[func->name][bb->label], callidir_inst->lhs, func);
                if (joinAbsStore(bb2store[func->name][callidir_inst->next_bb], caller_store) ||
                    bbs_to_output.count(func->name + "." + callidir_inst->next_bb) == 0)
                {
                    bbs_to_output.insert(func->name + "." + callidir_inst->next_bb);
                    worklist.push_back({func->name, callidir_inst->next_bb});
                    //std::cout << "Pushed to worklist: " << func->name + "." + callidir_inst->next_bb << std::endl;
                }
            }

            // store[x] = bottom
            // TODO - Check if this has to be removed or made an empty set
            if (callidir_inst->lhs) {
                std::string lhsKey = GetKey(program, func, callidir_inst->lhs);
                sigma_prime[func->name][bb->label][lhsKey] = {};
            }

            // Propagate store to next bb
            if (joinAbsStore(bb2store[func->name][callidir_inst->next_bb], sigma_prime[func->name][bb->label]) ||
                bbs_to_output.count(func->name + "." + callidir_inst->next_bb) == 0)
            {
                bbs_to_output.insert(func->name + "." + callidir_inst->next_bb);
                worklist.push_back({func->name, callidir_inst->next_bb});
                //std::cout << "Pushed to worklist: " << func->name + "." + callidir_inst->next_bb << std::endl;
            }

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
    for (auto it = sigma_prime[func->name][bb->label].begin(); it != sigma_prime[func->name][bb->label].end(); it++) {
        std::cout << it->first << " -> {";
        for (auto it2 = it->second.begin(); it2 != it->second.end(); it2++) {
            std::cout << *it2 << ",";
        }
        std::cout << "}" << std::endl;
    }
    std::cout << std::endl;*/
    
    return;
}