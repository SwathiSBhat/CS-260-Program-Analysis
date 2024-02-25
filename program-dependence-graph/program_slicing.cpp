#include <algorithm>
#include <iostream>
#include <limits>
#include <map>
#include <string>
#include <queue>
#include<set>
#include <fstream>
#include "../headers/datatypes.h"
#include "control_flow_analysis.hpp"
#include "mod_ref_utils.hpp"
#include "reachingdef.hpp"

using json = nlohmann::json;


class PDGNode {
    public:
    std::string program_point;
    std::set<std::string> dd_pred, dd_succ; // data dependency edges - predecessors and successors
    std::set<std::string> cd_pred, cd_succ; // control dependency edges - predecessors and successors

    PDGNode(std::string program_point): program_point(program_point) {}
};

std::unordered_map<std::string, PDGNode*> pdg; // PDG: pp -> PDGNode

class PDG {
    public:

    static std::string GetPP(std::string function, BasicBlock *bb, int idx) {
        if (idx == bb->instructions.size()) {
            return function + ":" + bb->label + ":term";
        }
        return function + ":" + bb->label + ":" + std::to_string(idx);
    }

    static void ProcessControlDependencies(Function *func, std::map<std::string, std::set<std::string>> control_dependencies) {
        std::string fname = func->name;
        
        for(const auto& [controlled, controllers]: control_dependencies) {
            
            BasicBlock *to_bb = func->bbs[controlled];
            for(const auto& controller: controllers) {
                BasicBlock *from_bb = func->bbs[controller];

                // Get index of terminal instruction in from_bb
                int last_idx = from_bb->instructions.size();

                // Get PP corresponding to the last instruction in from_bb
                std::string from_pp = PDG::GetPP(fname, from_bb, last_idx);

                if(!pdg.count(from_pp)) {
                    pdg[from_pp] = new PDGNode(from_pp);
                }

                std::string to_pp = "";
                
                // Add edge from from_bb's terminal instruction to each instruction in to_bb
                for(int i = 0; i < to_bb->instructions.size() + 1; i++) {
                    to_pp = PDG::GetPP(fname, to_bb, i);
                    
                    if(!pdg.count(to_pp)) {
                        pdg[to_pp] = new PDGNode(to_pp);
                    }

                    pdg[from_pp]->cd_succ.insert(to_pp);
                    pdg[to_pp]->cd_pred.insert(from_pp);
                }
            }
        }
    }

    /*
    * Add an edge between uses and defs in the PDG
    */
    static void ProcessDataDependencies(Function *func, std::map<std::string, std::set<std::string>> data_dependencies) {
        for(const auto& [use, definitions]: data_dependencies) {
            
            std::string use_pp = func->name + ":" + use;

            if(!pdg.count(use_pp)) {
                pdg[use_pp] = new PDGNode(use_pp);
            }
            
            for(const auto& definition: definitions) {
                std::string definition_pp = func->name + ":" + definition;

                if(!pdg.count(definition_pp)) {
                    pdg[definition_pp] = new PDGNode(definition_pp);
                }

                pdg[definition_pp]->dd_succ.insert(use_pp);
                pdg[use_pp]->dd_pred.insert(definition_pp);
            }
        }
    }

};

int main(int argc, char const *argv[])
{
   if (argc != 4) {
        std::cerr << "Usage: program_slicing <lir file path> <lir json filepath> <funcname>" << std::endl;
        return EXIT_FAILURE;
    }

    std::ifstream f(argv[2]);
    json lir_json = json::parse(f);

    std::string func_name = argv[3];

    // TODO - Parse pointsTo information

    Program program = Program(lir_json);
    ControlFlowAnalysis constant_analysis = ControlFlowAnalysis(program);
    std::map<std::string, std::set<std::string>> control_dependencies = constant_analysis.AnalyzeFunc(func_name);
    PDG::ProcessControlDependencies(program.funcs[func_name], control_dependencies);
    
    std::unordered_map<std::string, std::set<std::string>> pointsTo;

    ModRef mod_ref = ModRef(program, pointsTo);
    mod_ref.ComputeModRefInfo();

    ReachingDef reaching_def = ReachingDef(program, pointsTo, mod_ref.mod_ref_info);
    std::map<std::string, std::set<std::string>> data_dependencies = reaching_def.AnalyzeFunc(func_name);

    PDG::ProcessDataDependencies(program.funcs[func_name], data_dependencies);

    return 0;
}