#pragma once

#include <algorithm>
#include <map>
#include <string>

enum AbstractVals {
    BOTTOM,
    TOP
};

/*
 * Some typedefs for convenience.
 */
typedef std::pair<int, int> interval;
typedef std::variant<interval, AbstractVals> abstract_interval;
typedef std::map<std::string, abstract_interval> interval_abstract_store;

/*
 * If var is not present in the store, return BOTTOM. Else return the value in
 * the store.
 */
abstract_interval get_val_from_store(interval_abstract_store m,
                                     const std::string &var) {
    if (m.count(var) == 0) {
        return AbstractVals::BOTTOM;
    }
    return m[var];
}

/*
 * Join b to a and return whether a changed or not.
 */
bool join(interval_abstract_store &a, const interval_abstract_store &b) {
    bool a_changed = false;

    /*
     * Loop through each entry in b.
     */
    for (const auto &[b_key, b_val] : b) {
        abstract_interval a_val = get_val_from_store(a, b_key);

        /*
         * If the variable isn't in a, add it and mark the store as changed.
         */
        if (std::holds_alternative<AbstractVals>(a_val) && std::get<AbstractVals>(a_val) == AbstractVals::BOTTOM) {
            a[b_key] = b_val;
            a_changed = true;
        }

        abstract_interval new_a;

        /*
         * If either variable is TOP, we know to assign TOP, no questions asked.
         */
        if (
                (std::get<AbstractVals>(b_val) == AbstractVals::TOP) ||
                (std::get<AbstractVals>(a_val) == AbstractVals::TOP)
        ) {
            new_a = AbstractVals::TOP;

            /*
             * Check whether this actually changes things. If so, assign the new
             * value.
             */
            if (a_val != new_a) {
                a_changed = true;
                a[b_key] = new_a;
            }
        } else {

            /*
             * The new lower bound is the minimum of the two lower bounds. The
             * new upper bound is the maximum of the two upper bounds.
             */
            int new_lower_bound = std::min(std::get<interval>(a_val).first,
                                           std::get<interval>(b_val).first);
            int new_upper_bound = std::max(std::get<interval>(a_val).first,
                                           std::get<interval>(b_val).first);

            std::get<interval>(new_a) = {new_lower_bound, new_upper_bound};

            /*
             * Check whether this actually changes things. If so, assign the new
             * value.
             */
            if (std::get<interval>(new_a) != std::get<interval>(a_val)) {
                a_changed = true;
                a[b_key] = new_a;
            }
        }
    }
    return a_changed;
}

/*
 * Implement the widening operator Ben discussed in lecture 3.1.
 */
bool widen(abstract_interval &a, abstract_interval b) {

    /*
     * If either a or b are BOTTOM, return BOTTOM.
     */
    if (std::get<AbstractVals>(a) == AbstractVals::BOTTOM || std::get<AbstractVals>(b) == AbstractVals::BOTTOM) {
        return AbstractVals::BOTTOM;
    }

    /*
     * TODO
     */
    return true;
}