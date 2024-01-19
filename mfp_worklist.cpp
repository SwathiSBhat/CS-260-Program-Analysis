/*
 * MFP worklist algorithm for CS260 assignment 1.
 */

#include <vector>

#include "abstract_store.hpp"
#include "datatypes.h"

/*
 * I just want to implement the MFP worklist algorithm in main() for now, and
 * then later we can move it into a class or something.
 */
int main() {

    /*
     * Our bb2store is a map from a BasicBlock to an AbstractStore.
     */
    std::map<BasicBlock, AbstractStore> bb2store;

    /*
     * Our worklist is a vector of BasicBlocks.
     */
    std::vector<BasicBlock> worklist;

    /*
     * Before we start, we must set parameters and global variables to TOP.
     */
}