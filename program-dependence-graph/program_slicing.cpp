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

using json = nlohmann::json;


class PDGNode {
    public:
    std::string program_point;
    std::set<std::string> dd_pred, dd_succ; // data dependency edges - predecessors and successors
    std::set<std::string> cd_pred, cd_succ; // control dependency edges - predecessors and successors

    PDGNode(std::string program_point): program_point(program_point) {}
};

int main(int argc, char const *argv[])
{
   if (argc != 4) {
        std::cerr << "Usage: program-slicing <lir file path> <lir json filepath> <funcname>" << std::endl;
        return EXIT_FAILURE;
    }

    std::ifstream f(argv[2]);
    json lir_json = json::parse(f);

    std::string func_name = argv[3];

    Program program = Program(lir_json);
    ControlFlowAnalysis constant_analysis = ControlFlowAnalysis(program);
    constant_analysis.AnalyzeFunc(func_name);

    return 0;
}