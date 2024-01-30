#pragma once

#include "interval_analysis.hpp"

/*
 * Translate a concrete value into an abstract value. In the case of interval
 * analysis, we just make an interval out of whatever integer we get.
 */
interval alpha(int val) {
    return std::make_pair(val, val);
}