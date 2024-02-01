#pragma once

#include <algorithm>
#include <iostream>
#include <limits>
#include <map>
#include <string>
#include <queue>
#include<set>


/*
Return a set of all basic block names in a function
*/
std::set<std::string> get_all_bbs(Function *func) {
    std::set<std::string> bbs;
    for(auto &it: func->bbs) {
        bbs.insert(it.first);
    }
    return bbs;
}

/*
join of 2 abstract stores is the intersection of two ordered sets of strings
Joining a with b will result in the set intersection being stored in a and the result whether the set has changed or not will be returned
*/
bool join(std::set<std::string> &a, std::set<std::string> &b) {
    int initial_size = a.size();
    /*std::cout << "Joining: " << std::endl;
    for(auto &it: a) {
        std::cout << it << " ";
    }
    std::cout << std::endl;
    for(auto &it: b) {
        std::cout << it << " ";
    }
    std::cout << std::endl;*/
    std::set<std::string> result;
    std::set_intersection(a.begin(), a.end(), b.begin(), b.end(), std::inserter(result, result.begin()));
    /*std::cout << "Result: " << std::endl;
    for(auto &it: result) {
        std::cout << it << " ";
    }
    std::cout << std::endl;*/
    a = result;
    return result.size() != initial_size;
}


/*
 * Get the predecessor of all basic blocks in a function
*/
std::map<std::string, std::set<std::string>> get_predecessors(Function *func)
{
    std::map<std::string, std::set<std::string>> preds;
    preds["entry"] = {};

    std::string curr_bb = "entry";
    std::queue<std::string> q;
    q.push(curr_bb);
    std::set<std::string> visited;

    while (!q.empty())
    {
        curr_bb = q.front();
        q.pop();
        visited.insert(curr_bb);

        Instruction *terminal_instruction = func->bbs[curr_bb]->terminal;
        if (terminal_instruction->instrType == InstructionType::BranchInstrType)
        {
            BranchInstruction *branch_inst = (BranchInstruction *)terminal_instruction;

            preds[branch_inst->tt].insert(curr_bb);
            preds[branch_inst->ff].insert(curr_bb);
            if (visited.find(branch_inst->tt) == visited.end())
            {
                q.push(branch_inst->tt);
            }
            if (visited.find(branch_inst->ff) == visited.end())
            {
                q.push(branch_inst->ff);
            }
        }
        else if (terminal_instruction->instrType == InstructionType::JumpInstrType)
        {
            JumpInstruction *jump_inst = (JumpInstruction *)terminal_instruction;

            preds[jump_inst->label].insert(curr_bb);
            if (visited.find(jump_inst->label) == visited.end())
            {
                q.push(jump_inst->label);
            }
        }
        else if (terminal_instruction->instrType == InstructionType::CallDirInstrType)
        {
            CallDirInstruction *call_dir_inst = (CallDirInstruction *)terminal_instruction;
            preds[call_dir_inst->next_bb].insert(curr_bb);
            if (visited.find(call_dir_inst->next_bb) == visited.end())
            {
                q.push(call_dir_inst->next_bb);
            }
        }
        else if (terminal_instruction->instrType == InstructionType::CallIdrInstrType)
        {
            CallDirInstruction *call_dir_inst = (CallDirInstruction *)terminal_instruction;
            preds[call_dir_inst->next_bb].insert(curr_bb);
            if (visited.find(call_dir_inst->next_bb) == visited.end())
            {
                q.push(call_dir_inst->next_bb);
            }
        }
    }
    return preds;
}



