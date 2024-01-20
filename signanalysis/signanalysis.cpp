#include "../datatypes.h"
#include "../json.hpp"
#include<iostream>
#include<vector>
#include<map>
#include<string>
#include<fstream>
#include<queue>
#include<unordered_set>

using json = nlohmann::json;

// Define the abstract domain
enum AbsDomain {
    absBottom = 0,
    absPos,
    absZero,
    absNeg,
    absTop
};


/*
    Class that contains methods to perform sign analysis on function
*/
class SignAnalysis {
    public:
    SignAnalysis(Program program) : program(program) {
    };

    /*
    Method to get the set of int-typed global variables
    */
    void get_int_type_globals(std::unordered_set<std::string> &int_type_globals) {
        for (auto global_var : program.globals) {
            Variable *global_var_ptr = global_var->globalVar;
            if (global_var_ptr->isIntType()) {
                int_type_globals.insert(global_var_ptr->name);
            }  
        }
        return;
    }

    /*
    Method to get the set of variables that are addresses of int-typed variables
    */
    void get_addr_of_int_types(std::unordered_set<std::string> &addr_of_int_types, const std::string &func_name) {
        for (auto global_var : program.globals) {
            Variable *global_var_ptr = global_var->globalVar;
            if (global_var_ptr->type->indirection > 0 && global_var_ptr->type->type == DataType::IntType) {
                addr_of_int_types.insert(global_var_ptr->name);
            }  
        }
        
        for (auto local_var : program.funcs[func_name]->locals) {
            Variable *local_var_ptr = local_var;
            if (local_var_ptr->type->indirection > 0 && local_var_ptr->type->type == DataType::IntType) {
                addr_of_int_types.insert(local_var_ptr->name);
            }  
        }
        return; 
    }

    /*
        Uber level method to run the analysis on a function
    */
    void AnalyzeFunc(const std::string &func_name) {
        Function *func = program.funcs[func_name];
        std::cout << "Analyzing function " << func_name << std::endl;
        // Prep steps:
        // 1. Compute set of int-typed global variables
        get_int_type_globals(int_type_globals);
        // 2. Compute set of variables that are addresses of int-typed variables
        get_addr_of_int_types(addr_of_int_types, func_name);
    }

    Program program;
    std::queue<std::string> worklist;
    // data structures required for prep stage
    std::unordered_set<std::string> int_type_globals; // contains names of all global variables of type int
    std::unordered_set<std::string> addr_of_int_types; // contains names of all variables that are addresses of int types
};

int main(int argc, char* argv[]) 
{
    std::ifstream f(argv[1]);
    json lir_json = json::parse(f);

    Program program = Program(lir_json);
    std::cout << "********** Program created **********" << std::endl;
    SignAnalysis signAnalysis = SignAnalysis(program);
    signAnalysis.AnalyzeFunc("main");
}