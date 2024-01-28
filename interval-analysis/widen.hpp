#pragma once

#include "interval_analysis.hpp"

/*
 * Implement the widening operator Ben discussed in lecture 3.1.
 */
abstract_interval widen(abstract_interval a, abstract_interval b) {

    /*
     * If either a or b are BOTTOM, return BOTTOM.
     */
    if (
            std::get<AbstractVals>(a) == AbstractVals::BOTTOM
            || std::get<AbstractVals>(b) == AbstractVals::BOTTOM
    ) {
        return AbstractVals::BOTTOM;
    }
}