#include <algorithm>
#include <iostream>
#include <limits>
#include <map>
#include <string>
#include <queue>
#include <set>
#include <fstream>
#include <sstream>
#include <deque>
#include "../headers/datatypes.h"
#include "./execute_taint.hpp"
#include "../headers/tokenizer.hpp"

using json = nlohmann::json;

std::unordered_map<string, std::set<string>> pointsTo; // points to information

class TaintAnalysis {
    public:
    
    TaintAnalysis(Program* program)
    {
        this->program = program;
    }

    void AnalyzeFunction() 
    {
        /*
        * Prep step for taint analysis
        * 1) call_edges data structure = map from callee function to the set of call instructions that call it
        * 2) call_returned data structure = map from function to returned abstract store. This data structure stores the latest return store for each function
        */
        PrepForTaintAnalysis();
        /*
        * Worklist now stores the context i.e (func,basic block) pair
        */
        std::deque<std::pair<string,string>> worklist;
        std::map<std::string, std::map<std::string, AbsStore>> bb2store;
        std::set<std::string> bbs_to_output;
        std::map<std::string, std::set<std::string>> soln;

        worklist.push_back(std::make_pair("main", "entry"));
        bbs_to_output.insert("main.entry");

        while(!worklist.empty()) {
            std::pair<string,string> current = worklist.front();
            worklist.pop_front();
            std::string current_func = current.first;
            std::string current_bb = current.second;

            // Perform the transfer function on the current basic block
            execute(
                program, 
                program->funcs[current_func], 
                program->funcs[current_func]->bbs[current_bb], 
                bb2store, 
                worklist, 
                bbs_to_output, 
                soln, 
                pointsTo,
                call_edges,
                call_returned,
                func_ret_op);

            // Print call_edges after every bb
            // printCallEdges();

            for (const auto &i: worklist) {
                bbs_to_output.insert(i.first + "." + i.second);
            }
        }

        /*
        * Print soln which will be the sinks to sources that can taint them
        */
        // std::cout << "Sinks to sources: " << std::endl;
        for (auto it = soln.begin(); it != soln.end(); it++) {
            if (it->second.size() == 0) {
                continue;
            }
            std::cout << it->first << " -> {";
            for (auto it2 = it->second.begin(); it2 != it->second.end(); it2++) {
                if (it2 == --it->second.end())
                    std::cout << *it2;
                else
                    std::cout << *it2 << ", ";
            }
            std::cout << "}" << std::endl;
        }
        std::cout << std::endl;
    }

    private:
    
    void PrepForTaintAnalysis() 
    {
        /*
        * 1) call_edges data structure = map from callee function to the set of call instructions that call it
        */
       // TODO - This can be done ahead of time only for context insensitive analysis
        /*for (auto func: program->funcs) {
            for (auto bb: func.second->bbs) {
                Instruction* terminal = bb.second->terminal;
                if ((*terminal).instrType == InstructionType::CallDirInstrType)
                {
                    CallDirInstruction *calldir_inst = (CallDirInstruction *) terminal;
                    // A call instruction can be uniquely identified by func_name and bb_name since it's always the terminal instruction 
                    std::string curr_context = func.first + "." + bb.first;
                    call_edges[{calldir_inst->callee, calldir_inst->callee}].insert(curr_context);
                }
                else if ((*terminal).instrType == InstructionType::CallIdrInstrType)
                {
                    CallIdrInstruction *callidr_inst = (CallIdrInstruction *) terminal;
                    // Get pointsTo of fp in call_idr and add call_edges for each of them
                    // TODO - Check if pointsTo can point to any other apart from function
                    std::string pointoToKey = isGlobalVar(callidr_inst->fp, program, func.first) ? callidr_inst->fp->name : func.first + "." + callidr_inst->fp->name;
                    std::set<std::string> points_to = pointsTo[pointoToKey];
                    for (auto point_to: points_to) {
                        std::string curr_context = func.first + "." + bb.first;
                        call_edges[{point_to, point_to}].insert(curr_context);
                    }
                }
            }
        }*/

        /*
        * 2) call_returned data structure = map from function to returned abstract store. This data structure stores the latest return store for each function
        */

       /*
       * 3) func_ret_op map = map from function to the return instruction corresponding to it
       */
        for (auto func: program->funcs) {
            for (auto bb: func.second->bbs) {
                Instruction* terminal = bb.second->terminal;
                if ((*terminal).instrType == InstructionType::RetInstrType)
                {
                    RetInstruction *ret_inst = (RetInstruction *) terminal;
                    func_ret_op[func.first] = ret_inst;
                    break;
                }
            }
        }
        
    }

    /*
    * Print call edges
    */
    void printCallEdges() {
        std::cout << "Call edges: " << std::endl;
        for (auto it = call_edges.begin(); it != call_edges.end(); it++) {
            std::cout << it->first.first << " : " << it->first.second << " -> {";
            for (auto it2 = it->second.begin(); it2 != it->second.end(); it2++) {
                std::cout << *it2 << ",";
            }
            std::cout << "}" << std::endl;
        }
        std::cout << std::endl;
    }

    // call_edges is a map from (function,cid) -> set of call instructions that call it
    // For context insensitive analysis, we can ignore the cid part of the key which will be equal to the function name
    std::map<std::pair<std::string, std::string>, std::set<std::string>> call_edges;
    // call_returned is a map from (function,cid) -> returned abstract store
    // For context insensitive analysis, we can ignore the cid part of the key which will be equal to the function name
    std::map<std::pair<std::string, std::string>, AbsStore> call_returned;
    // Map from function to the return instruction corresponding to it
    std::map<std::string, RetInstruction*> func_ret_op;
    Program *program;
};

int main(int argc, char const *argv[])
{
    if (argc != 5) {
        std::cerr << "Usage: taint_analysis <lir file> <lir json filepath> <points to soln file> <sensitivity>" << std::endl;
        return EXIT_FAILURE;
    }

    std::ifstream f(argv[2]);
    json lir_json = json::parse(f);

    std::string pointsToFile = argv[3];
    std::ifstream in(pointsToFile);
    std::string input_str(std::istreambuf_iterator<char>{in}, {});

    util::Tokenizer tk(input_str, {' '}, {"{", "}", "->", ","}, {});
    std::vector<std::string> tokens = tk.Tokens();

    // parse the points-to information
    while (!tokens.empty()) {
        std::string lhs = util::Tokenizer::Consume(tokens);

        /*
         * We don't want to get tripped up by random newlines at the end of the
         * file.
         */
        if (lhs.compare("\n") == 0) {
            break;
        }

        std::set<std::string> points_to = {};
        util::Tokenizer::ConsumeToken(tokens, "->");
        util::Tokenizer::ConsumeToken(tokens, "{");

        while(tokens.back() != "}") {
            if(tokens.back() == ",") {
                util::Tokenizer::ConsumeToken(tokens, ",");
            }
            else {
                std::string ptsto_element = util::Tokenizer::Consume(tokens);
                points_to.insert(ptsto_element);
            }

        }
        util::Tokenizer::ConsumeToken(tokens, "}");
        pointsTo[lhs] = points_to;

        if (!tokens.empty()) {
            util::Tokenizer::ConsumeToken(tokens, "\n");
        }
    }

    Program program = Program(lir_json);

    TaintAnalysis taint_analysis = TaintAnalysis(&program);
    taint_analysis.AnalyzeFunction();

    return 0;
}