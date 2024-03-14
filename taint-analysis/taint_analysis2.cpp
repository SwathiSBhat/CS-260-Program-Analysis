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
#include "./execute_taint2.hpp"
#include "../headers/tokenizer.hpp"

using json = nlohmann::json;

std::unordered_map<string, std::set<string>> pointsTo; // points to information

class TaintAnalysis {
    public:
    
    TaintAnalysis(Program* program, int sensitivity)
    {
        this->program = program;
        this->sensitivity = sensitivity;
    }

    void AnalyzeFunction() 
    {
        /*
        * Worklist now stores the context i.e (func+cid, basic block) pair
        */
        std::deque<std::pair<string,string>> worklist;
        std::map<std::string, std::map<std::string, AbsStore>> bb2store;
        std::set<std::string> bbs_to_output;
        std::map<std::string, std::set<std::string>> soln;

        if (sensitivity == 0)
        {
            worklist.push_back(std::make_pair("main", "entry"));
            bbs_to_output.insert("main|entry");
        }
        else if (sensitivity == 1 || sensitivity == 2)
        {
            worklist.push_back(std::make_pair("main|", "entry"));
            //std::cout << "Pushed main|, entry to worklist" << std::endl;
            bbs_to_output.insert("main||entry");
        }

        while(!worklist.empty()) {
            std::pair<string,string> current = worklist.front();
            worklist.pop_front();
            std::string current_func = current.first.substr(0, current.first.find("|"));
            std::string cid = current.first.substr(current.first.find("|") + 1);
            std::string current_bb = current.second;

            //std::cout << "Current func: " << current_func << " context: " << cid << " bb: " << current_bb << std::endl;

            // Perform the transfer function on the current basic block
            execute(
                program, 
                program->funcs[current_func], 
                cid,
                program->funcs[current_func]->bbs[current_bb], 
                bb2store, 
                worklist, 
                bbs_to_output, 
                soln, 
                pointsTo,
                call_edges,
                call_returned,
                sensitivity);

            // Print call_edges after every bb
            // printCallEdges();
            //std::cout << std::endl;

            for (const auto &i: worklist) {
                if (sensitivity == 0)
                    bbs_to_output.insert(i.first + "|" + i.second);
                /*else if (sensitivity == 1 || sensitivity == 2)
                {
                    std::string current_func = current.first.substr(0, current.first.find("|"));
                    std::string cid = current.first.substr(current.first.find("|") + 1);
                    std::string current_bb = current.second;
                    bbs_to_output.insert(current_func + "|" + cid + "|" + i.second);
                }*/
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
    
    /*
    * Print call edges
    */
    void printCallEdges() {
        std::cout << "Call edges: " << std::endl;
        for (auto it = call_edges.begin(); it != call_edges.end(); it++) {
            std::cout << it->first.first << " : " << it->first.second << " -> {";
            for (auto it2 = it->second.begin(); it2 != it->second.end(); it2++) {
                std::cout << it2->first << " : " << it2->second << ",";
            }
            std::cout << "}" << std::endl;
        }
        std::cout << std::endl;
    }

    // call_edges is a map from (function,cid) -> set of call instructions that call it
    // For context insensitive analysis, we can ignore the cid part of the key which will be equal to the function name
    // Maps <func, cid> -> { < set of callsite, cid pairs > }
    std::map<std::pair<std::string, std::string>, std::set<std::pair<std::string,std::string>>> call_edges;
    // call_returned is a map from (function,cid) -> returned abstract store
    // For context insensitive analysis, we can ignore the cid part of the key which will be equal to the function name
    std::map<std::pair<std::string, std::string>, AbsStore> call_returned;
    // Map from function to the return instruction corresponding to it
    std::map<std::string, RetInstruction*> func_ret_op;
    int sensitivity;
    Program *program;
};

int GetSensitivity(std::string sensitivity) {
    if (sensitivity.compare("ci") == 0) {
        return 0;
    }
    else if (sensitivity.find("callstring") != std::string::npos) {
        // If callstring is of the form callstring-k, then k is the sensitivity level
        std::string k = sensitivity.substr(sensitivity.find("-") + 1);
        return std::stoi(k);
    }
    else if (sensitivity.compare("functional") == 0) {
        return 3;
    }
    else {
        std::cerr << "Invalid sensitivity level" << std::endl;
        exit(EXIT_FAILURE);
    }
}

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

    std::string sensitivity = argv[4];
    int sens = GetSensitivity(sensitivity);
    //std::cout << "Sensitivity: " << sens << std::endl;

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

    TaintAnalysis taint_analysis = TaintAnalysis(&program, sens);
    taint_analysis.AnalyzeFunction();

    return 0;
}