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


Node* get_node(std::unordered_map<std::string, Node*> &nodes_, std::string name) {
    if (nodes_.find(name) == nodes_.end()) {
        nodes_[name] = new Node(name);
    }
    return nodes_[name];
}

void ComputeCallGraph(Program *prog, std::unordered_map<std::string, Node*> &nodes_, std::unordered_map<std::string, std::set<std::string>>& pointsTo) {
    
    // TODO - Check if a node needs to be created for each function or can the call graph just be created starting from main
    /*for (auto &it: prog->funcs) {
        Node *node = get_node(nodes_, it.first);
    }*/

    // Add edges to the call graph
    std::queue<std::string> to_visit;
    to_visit.push("main");
    std::set<std::string> visited;
    visited.insert("main");
    Node *main_node = get_node(nodes_, "main");

    while (!to_visit.empty()) {
        std::string func_name = to_visit.front();
        to_visit.pop();
        Function *func = prog->funcs[func_name];
        Node *node = get_node(nodes_, func_name);

        for (auto &bb: func->bbs) {
            if (bb.second->terminal->instrType == InstructionType::CallDirInstrType) {
                CallDirInstruction *call_instr = (CallDirInstruction *)bb.second->terminal;
                Node *callee_node = get_node(nodes_, call_instr->callee);
                // Check that callee_node is not already added to the set of successors of node
                // TODO - Check that this works
                if (node->succs.find(callee_node) == node->succs.end()) {
                    node->succs.insert(callee_node);
                }
                if (callee_node->preds.find(node) == callee_node->preds.end()) {
                    callee_node->preds.insert(node);
                }
            }
            else if (bb.second->terminal->instrType == InstructionType::CallIdrInstrType) {
                CallIdrInstruction *call_instr = (CallIdrInstruction *)bb.second->terminal;
                std::set<std::string> callees = pointsTo[call_instr->fp->name];
                for (auto &callee: callees) {
                    Node *callee_node = get_node(nodes_, callee);
                    if (node->succs.find(callee_node) == node->succs.end()) {
                        node->succs.insert(callee_node);
                    }
                    if (callee_node->preds.find(node) == callee_node->preds.end()) {
                        callee_node->preds.insert(node);
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
void ComputeTransitiveClosure(std::unordered_map<std::string, Node*> &nodes_) {
    std::queue<std::string> worklist;
    std::set<std::string> visited;

    worklist.push("main");
    visited.insert("main");

    while (!worklist.empty())
    {
        std::string func_name = worklist.front();
        worklist.pop();
        Node *node = nodes_[func_name];
        for (auto &succ: node->succs) {
            int succ_edges = succ->preds.size() + succ->succs.size();
            if (node->name != succ->name) {
                for (auto &pred: node->preds) {
                    int pred_edges = pred->preds.size() + pred->succs.size();
                    if (pred->name != succ->name) {
                        if (pred->succs.find(succ) == pred->succs.end()) {
                            pred->succs.insert(succ);
                        }
                        if (succ->preds.find(pred) == succ->preds.end()) {
                            succ->preds.insert(pred);
                        }
                    }
                    if (pred->preds.size() + pred->succs.size() > pred_edges || visited.find(pred->name) == visited.end()) {
                        worklist.push(pred->name);
                    }
                }
            }
            if (succ->preds.size() + succ->succs.size() > succ_edges || visited.find(succ->name) == visited.end()) {
                worklist.push(succ->name);
            }
        }
    }
}

/*
* Initialize mod/ref information: For each function compute:
*  a. The set of globals assigned in the function and pointed to objects that function F can define i.e store operations
*  b. The set of globals read in the function and pointed to objects that function F can reference i.e load operations
*/
void InitModRefInfo(Program *prog, std::unordered_map<std::string, Node*> &nodes_, std::unordered_map<std::string, std::set<std::string>>& pointsTo) {
    for (auto &it: prog->funcs) {
        Function *func = it.second;
        Node *node = nodes_[it.first];
        for (auto &bb: func->bbs) {
            for (auto &instr: bb.second->instructions) {
                if (instr->instrType == InstructionType::StoreInstrType) {
                    StoreInstruction *store_instr = (StoreInstruction *)instr;
                    if (pointsTo.count(store_instr->dst->name)) {
                        for (auto &pointed_to: pointsTo[store_instr->dst->name]) {
                            node->mods.insert(pointed_to);
                        }
                    }
                }
                else if (instr->instrType == InstructionType::LoadInstrType) {
                    LoadInstruction *load_instr = (LoadInstruction *)instr;
                    if (pointsTo.count(load_instr->src->name)) {
                        for (auto &pointed_to: pointsTo[load_instr->src->name]) {
                            node->refs.insert(pointed_to);
                        }
                    }
                }
            }
        }
        // TODO - Need to handle global variables
    }
}

std::map<std::string, ModRefInfo> ComputeModRefInfo(Program *prog, std::unordered_map<std::string, std::set<std::string>>& pointsTo) {

    // Map of all nodes. One node for each function
    std::unordered_map<std::string, Node*> nodes_;
    std::map<std::string, ModRefInfo> mod_ref_info;

    ComputeCallGraph(prog, nodes_, pointsTo);
    ComputeTransitiveClosure(nodes_);
	InitModRefInfo(prog, nodes_, pointsTo);

    // Propagate mod/ref information backwards in the transitive closure of the call graph
    for(const auto& [func_name, func_ref]: nodes_) {
        for(const auto& node: func_ref->succs) {
            func_ref->refs.insert(node->refs.begin(), node->refs.end());
            func_ref->mods.insert(node->mods.begin(), node->mods.end());
        }

        mod_ref_info[func_name] = {};
        mod_ref_info[func_name].mod = func_ref->mods;
        mod_ref_info[func_name].ref = func_ref->refs;
    }

    return mod_ref_info;
}