#include <fstream>
#include <stack>

#include "interval_analysis.hpp"
#include "interval_execute.hpp"
#include "../headers/datatypes.h"

/*
 * Class to perform integer interval analysis on a function.
 */
class IntervalAnalysis {
public:

    /*
     * This data structure holds a list of all basic blocks that have ever been
     * on the worklist. At the end of our analysis, we will only print out the
     * basic blocks that are on this list.
     */
    std::set<std::string> bbs_to_output;

    /*
     * The underlying program data structure.
     */
    Program program;

    /*
     * Our bb2store is a map from a basic block label to an
     * interval_abstract_store.
     */
    std::map<std::string, interval_abstract_store> bb2store;

    /*
     * Our worklist is a deque of basic block labels.
     */
    std::deque<std::string> worklist;

    /*
     * We also need to maintain the list of loop headers.
     */
    std::unordered_set<std::string> loop_headers;

    /*
     * The name of the function to analyze.
     */
    std::string func_name;

    /*
     * Constructor.
     */
    IntervalAnalysis(Program p) : program(p) {};

    /*
     * TODO
     */
    void get_int_type_globals() {}

    /*
     * TODO
     */
    void get_addrof_ints() {}

    /*
     * Get the list of loop headers through post-order traversal of all basic
     * blocks in the function.
     */
    void get_loop_headers(std::unordered_set<std::string> &loop_headers,
                          const std::string &func_name) {
        std::unordered_set<std::string> visited;
        std::vector<std::string> post_order;
        std::vector<std::string> loop_headers_vec;
        std::string entry_bb = "entry";
        std::string exit_bb = "exit";
        std::string current_bb = entry_bb;
        std::stack<std::string> stack;
        stack.push(current_bb);
        while (!stack.empty()) {
            current_bb = stack.top();
            stack.pop();
            if (visited.count(current_bb) == 0) {
                visited.insert(current_bb);
                post_order.push_back(current_bb);
                Instruction *instr = program.funcs[func_name]->bbs[current_bb]->terminal;
                switch (instr->instrType) {
                    case InstructionType::BranchInstrType: {
                        BranchInstruction *branch_instr = dynamic_cast<BranchInstruction *>(instr);
                        stack.push(branch_instr->tt);
                        stack.push(branch_instr->ff);
                        break;
                    }
                    case InstructionType::JumpInstrType: {
                        JumpInstruction *jump_instr = dynamic_cast<JumpInstruction *>(instr);
                        stack.push(jump_instr->label);
                        break;
                    }
                    case InstructionType::CallDirInstrType: {
                        CallDirInstruction *call_dir_instr = dynamic_cast<CallDirInstruction *>(instr);
                        stack.push(call_dir_instr->next_bb);
                        break;
                    }
                    case InstructionType::CallIdrInstrType: {
                        CallIdrInstruction *call_idr_instr = dynamic_cast<CallIdrInstruction *>(instr);
                        stack.push(call_idr_instr->next_bb);
                        break;
                    }
                    case InstructionType::RetInstrType: {
                        std::cout << "$ret instruction " << __FILE_NAME__ << ":" << __LINE__ << std::endl;
                    }
                    default: {
                        std::cout << "Unrecognized instruction " << __FILE_NAME__ << ":" << __LINE__ << std::endl;
                    }
                } // End of switch-case
            }
        } // End of while-loop
        std::reverse(post_order.begin(), post_order.end());
        std::cout << "Printing post order " << __FILE_NAME__ << ":" << __LINE__ << std::endl;
        for (auto bb : post_order) {
            std::cout << bb << " ";
        }
        std::cout << std::endl;
        return;
    }

    /*
     * Initialize the abstract store for the entry basic block.
     */
    void InitEntryStore() {
        for (auto param : program.funcs[func_name]->params) {
            if (param->isIntType()) {
                bb2store["entry"][param->name] = AbstractVals::TOP;
            }
        }
    }

    /*
     * Run interval analysis on a given function.
     */
    void AnalyzeFunc() {
        Function *func = program.funcs[func_name];

        /*
         * Data structures required for the prep stage.
         */
        std::unordered_set<std::string> addrof_ints;

        /*
         * Populate the set of loop headers for which widening will be
         * performed.
         */
        get_loop_headers(loop_headers, func_name);

        /*
         * First initialize the abstract store for the entry basic block. Then
         * add the entry basic block to the worklist.
         */
        InitEntryStore();
        worklist.push_back("entry");

        /*
         * Worklist algorithm.
         */
        while (!worklist.empty()) {
            std::string current_bb = worklist.front();
            worklist.pop_front();

            /*
             * Perform our transfer function on the current basic block.
             *
             * TODO
             */
            execute(func->bbs[current_bb],
                    bb2store[current_bb],
                    bb2store,
                    worklist,
                    addrof_ints,
                    bbs_to_output,
                    false);

            /*
             * Keep track of all the basic blocks we add to the worklist.
             */
            for (const auto &i : worklist) {
                bbs_to_output.insert(i);
            }
        }
    }
};

/*
 * This is the entry point for our interval analysis.
 */
int main(int argc, char *argv[]) {
    if (argc != 4) {
        std::cout << __FILE_NAME__ << ":" << __LINE__ << std::endl;
        return EXIT_FAILURE;
    }
    //std::ifstream f(argv[2]);
    std::ifstream f("/Users/vinayakgajjewar/PhD/CS260/CS-260-Program-Analysis/constant-analysis/constant-analysis-tests/constants-noptr-nocall.lir.json");
    Program p = Program(json::parse(f));
    IntervalAnalysis i = IntervalAnalysis(p);
    //i.func_name = argv[3];
    i.func_name = "test";
    i.AnalyzeFunc();
}