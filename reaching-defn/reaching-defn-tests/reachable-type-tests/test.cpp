
#include <unordered_set>
#include <fstream>
#include "../../../headers/datatypes.h"
#include "../../rtype.hpp"

int main(int argc, char* argv[]) 
{
    if (argc != 4) {
        std::cerr << "Usage: test <lir json filepath> <funcname> <variable name>" << std::endl;
        return EXIT_FAILURE;
    }

    std::ifstream f(argv[1]);
    json lir_json = json::parse(f);

    std::string func_name = argv[2];

    Program program = Program(lir_json);
    Function *func = program.funcs[func_name];

    std::string var_name = argv[3];
    ReachableType *var_type = new ReachableType(func->locals[var_name]->type);

    std::unordered_set<ReachableType*> rset;
    ReachableType::GetReachableType(&program, var_type, rset);

    std::cout << "Reachable types for " << var_name << " in " << func_name << std::endl;
    for (auto it = rset.begin(); it != rset.end(); it++)
    {
        (*it)->pretty_print();
    }

    return 0;
}