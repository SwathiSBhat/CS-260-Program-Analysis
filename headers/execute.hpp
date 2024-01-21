#include "abstract_store.hpp"
#include "datatypes.h"

/*
 * Execute a given BasicBlock against a given AbstractStore. This is a helper
 * function to be used in the MFP worklist algorithm. I figure this is important
 * and complicated enough to be given its own file, but we can refactor later as
 * needed.
 */
AbstractStore execute(BasicBlock bb, AbstractStore sigma) {

    /*
     * Make a copy of sigma that we'll return at the end of this function.
     */
    AbstractStore sigma_prime = sigma;

    /*
     * Iterate through each instruction in bb.
     */
    for (const auto &inst : bb.instructions) {

        /*
         * This is the weirdest way of checking the instruction type ever, but
         * it works for now. I would like to figure out a more elegant solution.
         *
         * TODO These are only the instructions needed for the very first test
         * TODO suite. We will need to add more later.
         */
        if (dynamic_cast<ArithInstruction*>(inst) != nullptr) {

            /*
             * Cast it.
             */
            ArithInstruction arith_inst = dynamic_cast<ArithInstruction*>(inst);

            /*
             * Let's check what kind of arithmetic instruction it is.
             *
             * TODO Need to add more arithmetic operators. It is late and I am
             * TODO lazy.
             */
            if (arith_inst.arith_op == "add") {
                sigma_prime.abstract_store[arith_inst.lhs.name] = arith_inst.op1.val + arith_inst.op1.val;
            }

        } else if (dynamic_cast<CmpInstruction*>(inst) != nullptr) {

            /*
             * Cast it.
             */
            CmpInstruction cmp_inst = dynamic_cast<CmpInstruction*>(inst);

        } else if (dynamic_cast<CopyInstruction*>(inst) != nullptr) {

            /*
             * Cast it.
             */
            CopyInstruction copy_inst = dynamic_cast<CopyInstruction*>(inst);

        } else if (dynamic_cast<BranchInstruction*>(inst) != nullptr) {

            /*
             * Cast it.
             */
            BranchInstruction branch_inst = dynamic_cast<BranchInstruction*>(inst);

        } else if (dynamic_cast<JumpInstruction*>(inst) != nullptr) {

            /*
             * Cast it.
             */
            JumpInstruction jump_inst = dynamic_cast<JumpInstruction*>(inst);

        }
    }
}