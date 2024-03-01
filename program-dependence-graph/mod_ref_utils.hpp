#pragma once

#include <algorithm>
#include <iostream>
#include <limits>
#include <map>
#include <string>
#include <queue>
#include<set>
#include "../headers/datatypes.h"

class Node {
    public:
    std::string name;
    std::set<Node*> preds, succs;
    std::set<std::string> mods;
    std::set<std::string> refs;
    Node(std::string name) : name(name) {}
};

/*
* Steps to compute mod/ref information:
* 1. Compute the call graph:
*   a. Node for each function
*   b. Edge from function A to function B if A calls B
* 2. Compute the transitive closure of the call graph
* 3. Initialize mod/ref information: For each function compute:
*   a. The set of globals assigned in the function and pointed to objects that function F can define i.e store operations
*   b. The set of globals read in the function and pointed to objects that function F can reference i.e load operations
* 4. Propagate mod/ref information: For each function F, propagate the mod/ref information backwards in the transitive closure of the call graph
*/

class ModRefInfo {
    public:
    std::set<std::string> mod, ref;
};

class ModRef {
    private:
    Program program_;
    std::unordered_map<std::string, std::set<std::string>> pointsTo;
    std::unordered_map<std::string, Node*> nodes_;

    Node* get_node(std::string name) {
        if (nodes_.find(name) == nodes_.end()) {
            nodes_[name] = new Node(name);
        }
        return nodes_[name];
    }
    
    void ComputeCallGraph() {
        
        // TODO - Check if a node needs to be created for each function or can the call graph just be created starting from main
        /*for (auto &it: prog->funcs) {
            Node *node = get_node(nodes_, it.first);
        }*/

        // Add edges to the call graph
        std::queue<std::string> to_visit;
        to_visit.push("main");
        std::set<std::string> visited;
        visited.insert("main");

        while (!to_visit.empty()) {
            std::string func_name = to_visit.front();
            to_visit.pop();
            Function *func = program_.funcs[func_name];
            Node *node = get_node(func_name);

            for (auto &bb: func->bbs) {
                if (bb.second->terminal->instrType == InstructionType::CallDirInstrType) {
                    CallDirInstruction *call_instr = (CallDirInstruction *)bb.second->terminal;
                    Node *callee_node = get_node(call_instr->callee);
                    // Check that callee_node is not already added to the set of successors of node
                    // TODO - Check that this works
                    if (node->succs.find(callee_node) == node->succs.end()) {
                        node->succs.insert(callee_node);
                    }
                    if (callee_node->preds.find(node) == callee_node->preds.end()) {
                        callee_node->preds.insert(node);
                    }
                    if (visited.find(call_instr->callee) == visited.end()) {
                        to_visit.push(call_instr->callee);
                        visited.insert(call_instr->callee);
                    }
                }
                // TODO - Verify this instruction using pointsTo set. The key for pointTo set might require function name
                else if (bb.second->terminal->instrType == InstructionType::CallIdrInstrType) {
                    CallIdrInstruction *call_instr = (CallIdrInstruction *)bb.second->terminal;
                    std::set<std::string> callees = pointsTo[call_instr->fp->name];
                    for (auto &callee: callees) {
                        Node *callee_node = get_node(callee);
                        if (node->succs.find(callee_node) == node->succs.end()) {
                            node->succs.insert(callee_node);
                        }
                        if (callee_node->preds.find(node) == callee_node->preds.end()) {
                            callee_node->preds.insert(node);
                        }
                        if (visited.find(callee) == visited.end()) {
                            to_visit.push(callee);
                            visited.insert(callee);
                        }
                    }
                }
            }
        }
    }

    /*
    * Compute transitive closure of the call graph
    * if there is a path from A to B and B to C, then there is a path from A to C
    */
    void ComputeTransitiveClosure() {
        std::queue<std::string> worklist;
        std::set<std::string> visited;

        worklist.push("main");
        visited.insert("main");

        while (!worklist.empty())
        {
            std::string func_name = worklist.front();
            worklist.pop();
            //std::cout << "Visiting " << func_name << std::endl;
            Node *node = get_node(func_name);
            for (auto &succ: node->succs) {
                int succ_edges = succ->preds.size() + succ->succs.size();
                if (node->name != succ->name) {
                    for (auto &pred: node->preds) {
                        int pred_edges = pred->preds.size() + pred->succs.size();
                        //if (pred->name != succ->name) {
                            if (pred->succs.find(succ) == pred->succs.end()) {
                                pred->succs.insert(succ);
                            }
                            if (succ->preds.find(pred) == succ->preds.end()) {
                                succ->preds.insert(pred);
                            }
                        //}
                        if (visited.find(pred->name) == visited.end() || pred->preds.size() + pred->succs.size() > pred_edges) {
                            worklist.push(pred->name);
                            visited.insert(pred->name);
                        }
                    }
                }
                if (visited.find(succ->name) == visited.end() || succ->preds.size() + succ->succs.size() > succ_edges) {
                    worklist.push(succ->name);
                    visited.insert(succ->name);
                }
            }
        }
    }

    /*
    * Initialize mod/ref information: For each function compute:
    *  a. The set of globals assigned in the function and pointed to objects that function F can define i.e store operations
    *  b. The set of globals read in the function and pointed to objects that function F can reference i.e load operations
    */
    void InitModRefInfo() {
        for (auto &it: program_.funcs) {
            Function *func = it.second;
            Node *node = get_node(it.first);
            for (auto &bb: func->bbs) {
                for (auto &instr: bb.second->instructions) {
                    if (instr->instrType == InstructionType::StoreInstrType) {
                        StoreInstruction *store_instr = (StoreInstruction *)instr;
                        // Add func name while comparing with pts to set
                        // TODO - Handle for globals and do not add function name in that case
                        std::string pointsToKey = it.second->name + "." + store_instr->dst->name;
                        if (pointsTo.count(pointsToKey)) {
                            for (auto pointed_to: pointsTo[pointsToKey]) {
                                // Remove the function name from the pointed_to
                                if (pointed_to.find(".") != std::string::npos) {
                                    pointed_to = pointed_to.substr(pointed_to.find(".") + 1);
                                }
                                node->mods.insert(pointed_to);
                            }
                        }
                    }
                    else if (instr->instrType == InstructionType::LoadInstrType) {
                        LoadInstruction *load_instr = (LoadInstruction *)instr;
                        // Add func name while comparing with pts to set
                        // TODO - Handle for globals and do not add function name in that case
                        std::string pointsToKey = it.second->name + "." + load_instr->src->name;
                        if (pointsTo.count(pointsToKey)) {
                            for (auto pointed_to: pointsTo[pointsToKey]) {
                                // Remove the function name from the pointed_to
                                if (pointed_to.find(".") != std::string::npos) {
                                    pointed_to = pointed_to.substr(pointed_to.find(".") + 1);
                                }
                                node->refs.insert(pointed_to);
                            }
                        }
                    }
                }
            }
            // TODO - Need to handle global variables
        }
    }

    void PrintNodes() {
        std::cout << std::endl;
        for(const auto& [func_name, func_ref]: nodes_) {
            std::cout << "Node " << func_name << std::endl;
            std::cout << "Predecessors: " << std::endl;
            for(const auto& node: func_ref->preds) {
                std::cout << node->name << std::endl;
            }
            std::cout << "Successors: " << std::endl;
            for(const auto& node: func_ref->succs) {
                std::cout << node->name << std::endl;
            }
            std::cout << std::endl;
        }
    }

    void PrintModRefInfo() {
        std::cout << std::endl;
        for (const auto& [func_name, modref]: mod_ref_info) {
            std::cout << "Function " << func_name << std::endl;
            std::cout << "Mods: " << std::endl;
            for (const auto& mod: modref.mod) {
                std::cout << mod << std::endl;
            }
            std::cout << "Refs: " << std::endl;
            for (const auto& ref: modref.ref) {
                std::cout << ref << std::endl;
            }
            std::cout << std::endl;
        }
    }

    public:
    std::map<std::string, ModRefInfo> mod_ref_info;

    ModRef(Program program, std::unordered_map<std::string, std::set<std::string>> pointsTo) : program_(program), pointsTo(pointsTo) {}

    std::map<std::string, ModRefInfo> ComputeModRefInfo() {

        ComputeCallGraph();
        // PrintNodes();
        // std::cout << "Transitive closure" << std::endl;
        ComputeTransitiveClosure();
        // PrintNodes();
        InitModRefInfo();

        // Propagate mod/ref information backwards in the transitive closure of the call graph
        // TODO - Verify the below function
        for(const auto& [func_name, func_ref]: nodes_) {
            for(const auto& node: func_ref->succs) {
                func_ref->refs.insert(node->refs.begin(), node->refs.end());
                func_ref->mods.insert(node->mods.begin(), node->mods.end());
            }

            mod_ref_info[func_name] = {};
            mod_ref_info[func_name].mod = func_ref->mods;
            mod_ref_info[func_name].ref = func_ref->refs;
        }

        // std::cout << "Mod Ref Info" << std::endl;
        // PrintModRefInfo();

        return mod_ref_info;
    }
};
