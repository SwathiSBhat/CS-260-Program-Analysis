#include <fstream>
#include <vector>
#include <unordered_set>

#include "../headers/datatypes.h"
#include "../headers/execute.hpp"

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
    Class that contains methods to perform constant analysis on function
*/
class ConstantAnalysis {
    public:
    ConstantAnalysis(Program program) : program(program) {
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
     * Get the list of names of all the int-typed local variables whose addresses were
     * taken using the $addrof command.
     * TODO: This should also include addrof of global variables but we are not doing it for assignment 1
    */
    void get_addr_of_int_types(std::unordered_set<std::string> &addr_of_int_types, const std::string &func_name) {  
        for (auto basic_block : program.funcs[func_name]->bbs) {
            for (auto instruction = basic_block.second->instructions.begin(); instruction != basic_block.second->instructions.end(); ++instruction) {
                if ((*instruction)->instrType == InstructionType::AddrofInstrType) {
                    if (dynamic_cast<AddrofInstruction*>(*instruction)->rhs->isIntType()) { 
                        if (program.funcs[func_name]->locals.count(dynamic_cast<AddrofInstruction*>(*instruction)->rhs->name) != 0) {
                            addr_of_int_types.insert(dynamic_cast<AddrofInstruction*>(*instruction)->rhs->name);
                    }
                    }
                }
            }
        }
        return; 
    }

    /*
        Initialize the abstract store for 'entry' basic block
    */
    // TODO - Make this generic to work for all basic blocks
    void InitEntryStore() {
        
        std::string bb_name = "entry";
        AbstractStore store = AbstractStore();

        // Initialize all globals and parameters in function to TOP

        // TODO: Ignoring global variables for assignment 1
        /*for (auto global_var : program.globals) {
            Variable *global_var_ptr = global_var->globalVar;
            if (global_var_ptr->isIntType()) {
                // std::cout << "Setting global: " << global_var_ptr->name << " to TOP" << std::endl;
                store.abstract_store[global_var_ptr->name] = AbstractVal::TOP;
            }  
        }*/

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
        worklist.push_back("entry");

        /*
         * We also need to initialize bb2store entries for all the basic blocks
         * in the function (I think.)
         */
        for (const auto &[bb_label, bb] : program.funcs[func_name]->bbs) {
            std::cout << "Initializing empty abstract store for " << bb_label << " basic block" << std::endl;
            bb2store[bb_label] = AbstractStore();
        }

        /*
            Worklist algorithm
            1. Pop a basic block from the worklist
            2. Perform the transfer function on the basic block
            3. For each successor of the basic block, join the abstract store of the successor with the abstract store of the current basic block
            4. If the abstract store of the successor has changed, add the successor to the worklist
        */

        while (!worklist.empty()) {
            std::string current_bb = worklist.front();
            worklist.pop_back();

            // Get the successors of the current basic block
            std::vector<std::string> successors;

            // Perform the transfer function on the current basic block
            std::cout << "Abstract store of " << current_bb << " before transfer function: " << std::endl;
            bb2store[current_bb].print();
            bb2store[current_bb] = execute(func->bbs[current_bb], bb2store[current_bb], bb2store, worklist, addr_of_int_types);
            std::cout << "Abstract store of " << current_bb << " after transfer function: " << std::endl;
            bb2store[current_bb].print();

            std::cout << "This is the worklist now:" << std::endl;
            for (const auto& i : worklist) {
                std::cout << i << " ";
            }
            std::cout << std::endl;

            // For each successor, join the abstract store and check if it has changed
            for (const auto& successor : worklist) {

                //
                // Join abstract stores and check for changes.
                //
                bool store_changed = bb2store[successor].join(bb2store[current_bb]);

                //
                // If the abstract store of the successor has changed, add it to
                // the worklist.
                
                if (store_changed) {
                    worklist.push_back(successor);
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
     * Our worklist is a vector containing BasicBlock labels.
     */
    std::vector<std::string> worklist;

    private:
    std::string funcname;
};

int main(int argc, char* argv[]) 
{
    if (argc != 4) {
        std::cerr << "Usage: constant-analysis <lir file path> <lir json filepath> <funcname>" << std::endl;
        return EXIT_FAILURE;
    }

    std::ifstream f(argv[2]);
    json lir_json = json::parse(f);

    std::string func_name = argv[3];

    Program program = Program(lir_json);
    std::cout << "********** Program created **********" << std::endl;
    ConstantAnalysis constant_analysis = ConstantAnalysis(program);
    constant_analysis.AnalyzeFunc(func_name);

    return 0;
}