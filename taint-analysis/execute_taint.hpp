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

void execute(
    Program *program,
    Function* func,
    BasicBlock *bb,
    std::map<std::string, std::map<std::string, AbsStore>> &bb2store, // function -> bb -> variable -> set of sources
    std::deque<std::pair<std::string, std::string>> &worklist,
    std::set<std::string> &bbs_to_output,
    std::map<std::string, std::set<std::string>> &soln,
    std::unordered_map<std::string, std::set<std::string>> pointsTo,
    bool execute_final = false
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
           std::cout << "Inside call ext for callee: " << callext_inst->extFuncName << std::endl;
            if (program->ext_funcs.find(callext_inst->extFuncName) != program->ext_funcs.end() && 
                isSource(program, program->ext_funcs[callext_inst->extFuncName]))
            {
                if (callext_inst->lhs) {
                    std::cout << "Setting lhs to " << callext_inst->extFuncName << std::endl;
                    std::string lhsKey = GetKey(program, func, callext_inst->lhs);
                    sigma_prime[func->name][bb->label][lhsKey] = {callext_inst->extFuncName};
                }

                std::set<std::string> reachable = GetReachable(callext_inst->args, pointsTo, program, func);
                for (std::string v : reachable) {
                    std::cout << "Setting " << v << " to {" << callext_inst->extFuncName << "}" << std::endl;
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
                std::cout << "Sink " << callext_inst->extFuncName << std::endl;
                std::set<std::string> reachable = GetReachable(callext_inst->args, pointsTo, program, func);
                
                if (execute_final) {
                    for (std::string v : reachable) {
                        joinSets(soln[callext_inst->extFuncName], sigma_prime[func->name][bb->label][v]);
                    }

                    for (auto v: callext_inst->args) {
                        if (v->IsConstInt())
                            continue;
                        std::string v_key = GetKey(program, func, v->var);
                        joinSets(soln[callext_inst->extFuncName], sigma_prime[func->name][bb->label][v_key]);
                    }
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
    
    if ((*terminal_instruction).instrType == InstructionType::JumpInstrType || 
        (*terminal_instruction).instrType == InstructionType::BranchInstrType)
    {
        // no-op
    }
    else if ((*terminal_instruction).instrType == InstructionType::RetInstrType)
    {
        RetInstruction *ret_inst = (RetInstruction *) terminal_instruction;

    }
    else if ((*terminal_instruction).instrType == InstructionType::CallDirInstrType)
    {
        CallDirInstruction *calldir_inst = (CallDirInstruction *) terminal_instruction;

        

    }
    else if ((*terminal_instruction).instrType == InstructionType::CallIdrInstrType)
    {
        CallIdrInstruction *callidir_inst = (CallIdrInstruction *) terminal_instruction;

    }
    else
    {
        std::cout << "Unknown terminal instruction type" << std::endl;
    }

    /*
    * Print sigma prime
    */
    std::cout << "sigma_prime[" << func->name << "." << bb->label << "]:" << std::endl;
    for (auto it = sigma_prime[func->name][bb->label].begin(); it != sigma_prime[func->name][bb->label].end(); it++) {
        std::cout << it->first << " -> {";
        for (auto it2 = it->second.begin(); it2 != it->second.end(); it2++) {
            std::cout << *it2 << ",";
        }
        std::cout << "}" << std::endl;
    }
    std::cout << std::endl;
    
    return;
}