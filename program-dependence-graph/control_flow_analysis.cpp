#include <fstream>
#include <vector>
#include <unordered_set>
#include <set>
#include "../headers/datatypes.h"
#include "cfa_utils.hpp"

using json = nlohmann::json;

/*
    Class that contains methods to perform constant analysis on function
*/
class ControlFlowAnalysis {
public:

    /*
     * This data structure holds a list of all basic blocks that have ever been
     * on the worklist. At the end of our analysis, we will only print out the
     * basic blocks that are on this list.
     */
    std::set<std::string> bbs_to_output;

    ControlFlowAnalysis(Program program) : program(program) {};

    /*
        Uber level method to run the analysis on a function
    */
    void AnalyzeFunc(const std::string &func_name) {

        Function *func = program.funcs[func_name];
        if (!func) {
            std::cout << "Func not found" << std::endl;
            return;
        }

        funcname = func_name;

        std::map<std::string, std::set<std::string>> cfg_preds = reversed_cfg_get_predecessors(program.funcs[funcname]);
        std::map<std::string, std::set<std::string>> cfg_succs = get_successors(program.funcs[funcname]);

        /*
        * Entry bb in reversed CFG will have no predecessors but will have successors
        * Iterate through all bbs in succ and find which bb has no predecessors
        */
        std::string entry_bb;
        for (const auto &[bb, preds] : cfg_succs) {
            if (cfg_preds[bb].empty() || cfg_preds[bb].size() == 0) {
                entry_bb = bb;
                break;
            }
        }
        
        /*
            Setup steps
            1. Initialize the abstract store for 'entry' basic block
            2. Add 'entry' basic block to worklist
        */
        bb2store[entry_bb] = std::set<std::string>();
        worklist.push_back(entry_bb);

        std::set<std::string> bottom = get_all_bbs(func);

        /*
            Worklist algorithm
            1. Pop a basic block from the worklist
            2. Perform the transfer function on the basic block - Which is just adding the current basic block to the abstract store of the basic block
            3. For each successor of the basic block, join the abstract store of the successor with the abstract store of the current basic block
            4. If the abstract store of the successor has changed, add the successor to the worklist
        */
        
        while (!worklist.empty()) {
            std::string current_bb = worklist.front();
            worklist.pop_front();
            
            std::set<std::string> sigma_prime = bb2store[current_bb];
            sigma_prime.insert(current_bb);

            /*
             * For each terminal of the basic block, join the abstract store of the current basic block with the target basic block.
            */
            for (const auto &succ : cfg_succs[current_bb]) {
                if (bb2store.find(succ) == bb2store.end()) {
                    bb2store[succ] = bottom;
                }

                bool is_changed = join(bb2store[succ], sigma_prime);
                if (is_changed) {
                    worklist.push_back(succ);
                }
            }
        }

        /* BB2store now gives the dominators without reflexive behavior i.e we need to add the curr bb ourself to bb2store */
        // Add the current basic block to the dominator set
        for (const auto &[bb, doms] : bb2store) {
            bb2store[bb].insert(bb);
        }

        /*
        * We now need to compute the dominance frontier for each basic block
        */
        std::map<std::string, std::set<std::string>> post_dominance_frontier;
        // For each basic block b, initialize the dominance frontier to be empty
        for (const auto &[bb, doms] : bb2store) {
            post_dominance_frontier[bb] = std::set<std::string>();
        }

        /*
        * Algorithm to compute post dominance frontier is as follows: (for the reversed CFG)
        * 1. For each basic block bb and it's corresponding dominator set bb2store[bb], do the following:
        * 2. let strict_bb_doms = bb2store[bb] - {bb}
        * 3. For each predecessor pred of bb, do the following:
        * 4. For each pred_dom in bb2store[pred] - strict_bb_doms, do the following:
        * 5. Add bb to post_dominance_frontier[pred_dom]
        */

        for (const auto &[bb, bb_doms] : bb2store) {

            std::set<std::string> strict_bb_doms = bb_doms;
            strict_bb_doms.erase(bb);

            for (const auto &pred : cfg_preds[bb]) {

                std::set<std::string> pred_doms;
                std::set<std::string> doms = bb2store[pred];
                std::set_difference(doms.begin(), doms.end(), strict_bb_doms.begin(), strict_bb_doms.end(), std::inserter(pred_doms, pred_doms.begin()));

                for (const auto &pred_dom : pred_doms) {
                    post_dominance_frontier[pred_dom].insert(bb);
                }
            }
        }
        
        /*
        * The solution is the contents of post_dominance_frontier
        */
        for (const auto &[bb, doms] : post_dominance_frontier) {
            std::cout << bb << " -> {";
            for(auto it = doms.begin(); it != doms.end(); it++) {
                if (std::next(it) == doms.end()) {
                    std::cout << *it;
                } else {
                    std::cout << *it << ", ";
                }
            }
            std::cout << "}" << std::endl;
        }
        std::cout << std::endl;
    }

    Program program;
    /*
     * Our bb2store is a map from a basic block label to a set of basic blocks.
     */
    std::map<std::string, std::set<std::string>> bb2store;
    /*
     * Our worklist is a queue containing BasicBlock labels.
     */
    std::deque<std::string> worklist;

private:
    std::string funcname;
};

int main(int argc, char* argv[]) 
{
    if (argc != 4) {
        std::cerr << "Usage: control-flow-analysis <lir file path> <lir json filepath> <funcname>" << std::endl;
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