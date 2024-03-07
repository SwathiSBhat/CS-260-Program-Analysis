#include <algorithm>
#include <iostream>
#include <limits>
#include <map>
#include <string>
#include <queue>
#include<set>
#include <fstream>
#include <sstream>
#include <deque>
#include "../headers/datatypes.h"
#include "./execute_taint.hpp"
#include "../headers/tokenizer.hpp"

using json = nlohmann::json;

std::unordered_map<string, std::set<string>> pointsTo; // points to info

class TaintAnalysis {
    public:
    
    TaintAnalysis(Program* program)
    {
        this->program = program;
    }
    
    void AnalyzeFunction() {

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
            execute(program, program->funcs[current_func], program->funcs[current_func]->bbs[current_bb], bb2store, worklist, bbs_to_output, soln, pointsTo);

            for (const auto &i: worklist) {
                bbs_to_output.insert(i.first + "." + i.second);
            }
        }

        /*
         * Once we've completed the worklist algorithm, let's execute our
         * transfer function once more on each basic block to get their exit
         * abstract stores.
         */
        for (auto func: program->funcs) {
            for (auto bb: func.second->bbs) {
                execute(program, func.second, bb.second, bb2store, worklist, bbs_to_output, soln, pointsTo, true);
            }
        }

        /*
        * Print soln which will be the sinks to sources that can taint them
        */
        std::cout << "Sinks to sources: " << std::endl;
        for (auto it = soln.begin(); it != soln.end(); it++) {
            std::cout << it->first << "-> {";
            for (auto it2 = it->second.begin(); it2 != it->second.end(); it2++) {
                std::cout << *it2 << ",";
            }
            std::cout << "}" << std::endl;
        }
        
    }

    private:
    Program *program;
};

int main(int argc, char const *argv[])
{
    if (argc != 4) {
        std::cerr << "Usage: taint_analysis <lir file> <lir json filepath> <points to soln file>" << std::endl;
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