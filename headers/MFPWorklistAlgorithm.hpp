#include "datatypes.h"

class MFPWorklistAlgorithm {
private:

    /*
     * This is our worklist. I made it a vector, but we can choose to use a
     * stack or a queue here.
     */
    std::vector<BasicBlock> worklist;

    /*
     * The underlying program object.
     */
    Program p;

    /*
     * List of names of all the int-typed variables whose addresses were taken
     * using the $addrof command.
     */
    std::vector<std::string> addrof_ints;
    std::vector<std::string> global_ints;

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
    MFPWorklistAlgorithm(json j) : p(Program(j)) {
    }

    /*
     * Implement the maximal fixpoint worklist algorithm.
     */
    void mfp_worklist() {

        /*
         * First, let's compute our list of addrof_ints.
         */
        get_addrof_ints();
    }
};