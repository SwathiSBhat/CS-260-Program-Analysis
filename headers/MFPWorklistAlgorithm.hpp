#include "datatypes.h"
#include "abstract_store.hpp"

class MFPWorklistAlgorithm {
private:

    /*
     * This is our worklist. I made it a vector, but we can choose to use a
     * stack or a queue here. Each string in the worklist must be a BasicBlock
     * label.
     */
    std::vector<std::string> worklist;

    /*
     * The underlying program object.
     */
    Program p;

    /*
     * Our bb2store is a map from a basic block name to an entry abstract store.
     */
    std::map<std::string, AbstractStore> bb2store;

    /*
     * List of names of all the int-typed variables whose addresses were taken
     * using the $addrof command.
     */
    std::vector<std::string> addrof_ints;
    std::vector<std::string> global_ints;

    /*
     * Set all global variables and function parameters to TOP.
     */
    void init_entry_stores() {

        //for (const auto &func : p.funcs) {

            /*
             * For each basic block in the program, add it to bb2store.
             */
            //for (const auto &basic_block : func.second.bbs) {

                /*
                 * Give each basic block an empty abstract store for now.
                 */
            //    bb2store[basic_block.second.label] = AbstractStore({});
            //}

            /*
             * Set each function parameter to TOP.
             *
             * TODO Is this only for "entry" or for all basic blocks?
             */
        //}
    }

    /*
     * Get the list of names of all the int-typed variables whose addresses were
     * taken using the $addrof command. I made this a private function because
     * it should really only be called by mfp_worklist().
     */
    void get_addrof_ints() {

        /*
         * First, loop through each function in the program.
         */
        for (const auto &func : p.funcs) {

            /*
             * Loop through each basic block in that function.
             */
            for (const auto &basic_block : func.second->bbs) {

                /*
                 * Loop through each instruction in that basic block.
                 */
                for (const auto &instruction : basic_block.second->instructions) {

                    /*
                     * If it's a $addrof instruction, add it to our list.
                     */
                    if (dynamic_cast<AddrofInstruction*>(instruction) != nullptr) {
                        addrof_ints.push_back(dynamic_cast<AddrofInstruction*>(instruction)->rhs->name);
                    }
                }
            }
        }
    }

    /*
     * At least for assignment 1, we don't need to worry about global variables.
     */
    void get_global_ints();
public:

    /*
     * Constructor that takes in a JSON object.
     *
     * TODO Add more constructors for ease of use.
     */
    //MFPWorklistAlgorithm(json j) : p(Program(j)),  {
    //}

    /*
     * Implement the maximal fixpoint worklist algorithm.
     */
    void mfp_worklist() {

        /*
         * First, let's compute our list of addrof_ints.
         */
        get_addrof_ints();

        /*
         * Initialize bb2store.
         */
        init_entry_stores();

        /*
         * The worklist starts off with only one basic block: "entry".
         */
        //worklist.push_back("entry");

        /*
         * The core of the MFP worklist algorithm.
         */
        //while (!worklist.empty()) {
        //    std::string sigma = worklist.pop_back();
        //}
    }
};