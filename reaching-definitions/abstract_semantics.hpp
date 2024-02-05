#pragma once

#include <algorithm>
#include "reaching_definitions.hpp"

/*
 * These semantics all come from Ben's week 3.2 lecture notes.
 */

namespace reaching_definitions {

    /*
     * Abstract semantics for the $addrof instruction. This one is pretty
     * simple. All we do here is update sigma[lhs] to be the current program
     * point.
     */
    void addrof_semantics(AddrofInstruction *addrof,
                          abstract_store &sigma,
                          program_point pp) {
        sigma[addrof->lhs->name] = {pp};
    }

    /*
     * Abstract semantics for the $alloc instruction.
     */
    void alloc_semantics(AllocInstruction *alloc,
                         std::map<program_point, std::set<program_point>> &solution,
                         abstract_store &sigma,
                         program_point pp) {

        /*
         * Get the DEF and USE sets for this instruction.
         */
        std::set<std::string> def;
        std::set<std::string> use;
        get_def_use_sets(alloc, def, use);

        /*
         * For each variable v in the USE set, update solution[pp] with the set
         * union of itself and sigma[v].
         */
        for (const auto &v : use) {
            std::set_union(solution[pp].begin(),
                           solution[pp].end(),
                           sigma[v].begin(),
                           sigma[v].end(),
                           std::inserter(solution[pp], solution[pp].begin()));
        }

        /*
         * Set sigma[lhs] to be {pp}, throwing away any previous value it had.
         */
        sigma[alloc->lhs->name] = {pp};
    }

    /*
     * Abstract semantics for the $arith instruction.
     */
    void arith_semantics(ArithInstruction *arith,
                         std::map<program_point, std::set<program_point>> &solution,
                         abstract_store &sigma,
                         program_point pp) {

        /*
         * Get the DEF and USE sets.
         */
        std::set<std::string> def;
        std::set<std::string> use;
        get_def_use_sets(arith, def, use);

        /*
         * For each variable v in the USE set, update solution[pp] with the set
         * union of itself and sigma[v].
         */
        for (const auto &v : use) {
            std::set_union(solution[pp].begin(),
                           solution[pp].end(),
                           sigma[v].begin(),
                           sigma[v].end(),
                           std::inserter(solution[pp], solution[pp].begin()));
        }

        /*
         * Set sigma[lhs] to {pp}, throwing away any previous value it had.
         */
        sigma[arith->lhs->name] = {pp};
    }

    /*
     * Abstract semantics for the $branch instruction. For every variable in the
     * USE set, update this program point's solution by doing a set union with
     * itself and that variable's sigma.
     */
    void branch_semantics(BranchInstruction *branch,
                          std::map<program_point, std::set<program_point>> &solution,
                          abstract_store &sigma,
                          program_point pp) {

        /*
         * First, get the DEF and USE sets.
         */
        std::set<std::string> def;
        std::set<std::string> use;
        get_def_use_sets(branch, def, use);

        /*
         * Update this program point's solution.
         */
        for (const auto &v : use) {
            std::set_union(solution[pp].begin(),
                           solution[pp].end(),
                           sigma[v].begin(),
                           sigma[v].end(),
                           std::inserter(solution[pp], solution[pp].begin()));
        }
    }

    /*
     * Abstract semantics for the $cmp instruction.
     */
    void cmp_semantics(CmpInstruction *cmp,
                       std::map<program_point, std::set<program_point>> &solution,
                       abstract_store &sigma,
                       program_point pp) {

        /*
         * Get the DEF and USE sets.
         */
        std::set<std::string> def;
        std::set<std::string> use;
        get_def_use_sets(cmp, def, use);

        /*
         * For each variable v in the USE set, update solution[pp] with the set
         * union of itself and sigma[v].
         */
        for (const auto &v : use) {
            std::set_union(solution[pp].begin(),
                           solution[pp].end(),
                           sigma[v].begin(),
                           sigma[v].end(),
                           std::inserter(solution[pp], solution[pp].begin()));
        }

        /*
         * Set sigma[lhs] to {pp}, throwing away any previous values.
         */
        sigma[cmp->lhs->name] = {pp};
    }

    /*
     * Abstract semantics for the $copy instruction.
     */
    void copy_semantics(CopyInstruction *copy,
                        std::map<program_point, std::set<program_point>> &solution,
                        abstract_store &sigma,
                        program_point pp) {

        /*
         * Get the DEF and USE sets.
         */
        std::set<std::string> def;
        std::set<std::string> use;
        get_def_use_sets(copy, def, use);

        /*
         * For each variable v in the USE set, update solution[pp] with the set
         * union of itself and sigma[v].
         */
        for (const auto &v : use) {
            std::set_union(solution[pp].begin(),
                           solution[pp].end(),
                           sigma[v].begin(),
                           sigma[v].end(),
                           std::inserter(solution[pp], solution[pp].begin()));
        }

        /*
         * Strongly update sigma[lhs].
         */
        sigma[copy->lhs->name] = {pp};
    }

    /*
     * Abstract semantics for the $gep instruction.
     */
    void gep_semantics(GepInstruction *gep,
                       std::map<program_point, std::set<program_point>> &solution,
                       abstract_store &sigma,
                       program_point pp) {

        /*
         * Get the DEF and USE sets.
         */
        std::set<std::string> def;
        std::set<std::string> use;
        get_def_use_sets(gep, def, use);

        /*
         * For each variable v in the USE set, update solution[pp] with the set
         * union of itself and sigma[v].
         */
        for (const auto &v : use) {
            std::set_union(solution[pp].begin(),
                           solution[pp].end(),
                           sigma[v].begin(),
                           sigma[v].end(),
                           std::inserter(solution[pp], solution[pp].begin()));
        }

        /*
         * Strongly update sigma[lhs].
         */
        sigma[gep->lhs->name] = {pp};
    }

    /*
     * Abstract semantics for the $gfp instruction.
     */
    void gfp_semantics(GfpInstruction *gfp,
                       std::map<program_point, std::set<program_point>> &solution,
                       abstract_store &sigma,
                       program_point pp) {

        /*
         * Get the DEF and USE sets.
         */
        std::set<std::string> def;
        std::set<std::string> use;
        get_def_use_sets(gfp, def, use);

        /*
         * For each variable v in the USE set, update solution[pp] with the set
         * union of itself and sigma[v].
         */
        for (const auto &v : use) {
            std::set_union(solution[pp].begin(),
                           solution[pp].end(),
                           sigma[v].begin(),
                           sigma[v].end(),
                           std::inserter(solution[pp], solution[pp].begin()));
        }

        /*
         * Strongly update sigma[lhs].
         */
        sigma[gfp->lhs->name] = {pp};
    }
}