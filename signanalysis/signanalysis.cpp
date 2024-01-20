#include "../headers/datatypes.h"
#include "../headers/json.hpp"
#include<iostream>
#include<vector>
#include<map>
#include<string>
#include<fstream>
#include<queue>
#include<unordered_set>
#include "../headers/abstract_store.hpp"
#include<variant>

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
        Initialize the abstract store for 'entry' basic block
    */
    void InitEntryStore() {
        
        std::string bb_name = "entry";
        AbstractStore store = AbstractStore();

        // Initialize all globals and parameters in function to TOP
        for (auto global_var : program.globals) {
            Variable *global_var_ptr = global_var->globalVar;
            if (global_var_ptr->isIntType()) {
                // std::cout << "Setting global: " << global_var_ptr->name << " to TOP" << std::endl;
                store.abstract_store[global_var_ptr->name] = AbstractVal::TOP;
            }  
        }
        for (auto param : program.funcs[funcname]->params) {
            if (param->isIntType()) {
                // std::cout << "Setting parameter: " << param->name << " to TOP" << std::endl;
                store.abstract_store[param->name] = AbstractVal::TOP;
            }  
        }
        bb2store[bb_name] = store;
        return;
    }

    /*
        Clear contents of the abstract store for a basic block
    */
    void ClearStore(std::string &bb_name) {
        bb2store.erase(bb_name);
        return;
    }

    /*
        Uber level method to run the analysis on a function
    */
    void AnalyzeFunc(const std::string &func_name) {

        Function *func = program.funcs[func_name];
        std::cout << "Analyzing function " << func_name << std::endl;
        funcname = func_name;
        
        // data structures required for prep stage
        std::unordered_set<std::string> int_type_globals; // contains names of all global variables of type int
        std::unordered_set<std::string> addr_of_int_types; // contains names of all variables that are addresses of int types
        
        // Prep steps:
        // 1. Compute set of int-typed global variables
        get_int_type_globals(int_type_globals);
        // 2. Compute set of variables that are addresses of int-typed variables
        get_addr_of_int_types(addr_of_int_types, func_name);

        /*
            Setup steps
            1. Initialize the abstract store for 'entry' basic block
            2. Add 'entry' basic block to worklist
        */
        InitEntryStore();
        worklist.push("entry");

        /*
            Worklist algorithm
            1. Pop a basic block from the worklist
            2. Perform the transfer function on the basic block
            3. For each successor of the basic block, join the abstract store of the successor with the abstract store of the current basic block
            4. If the abstract store of the successor has changed, add the successor to the worklist
        */

        while (!worklist.empty()) {
            std::string current_bb = worklist.front();
            worklist.pop();

            // Perform the transfer function on the current basic block
            //TransferFunction(current_bb);

            // Get the successors of the current basic block
            std::vector<std::string> successors; //= GetSuccessors(current_bb);

            // For each successor, join the abstract store and check if it has changed
            for (const auto& successor : successors) {
                bool store_changed = true;//JoinAbstractStores(current_bb, successor);

                // If the abstract store of the successor has changed, add it to the worklist
                if (store_changed) {
                    worklist.push(successor);
                }
            }
}
    }

    Program program;
    /*
     * Our bb2store is a map from a basic block label to an AbstractStore.
     */
    std::map<std::string, AbstractStore> bb2store;
    /*
     * Our worklist is a queue containing BasicBlock labels.
     */
    std::queue<std::string> worklist;

    private:
    std::string funcname;
};

int main(int argc, char* argv[]) 
{
    std::ifstream f(argv[1]);
    json lir_json = json::parse(f);

    Program program = Program(lir_json);
    std::cout << "********** Program created **********" << std::endl;
    SignAnalysis signAnalysis = SignAnalysis(program);
    signAnalysis.AnalyzeFunc("func2");
}