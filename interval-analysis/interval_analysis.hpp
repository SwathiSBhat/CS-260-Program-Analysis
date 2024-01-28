#pragma once

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
        if (std::get<AbstractVals>(a_val) == AbstractVals::BOTTOM) {
            a[b_key] = b_val;
            a_changed = true;
        }

        /*
         * If a[b_key] is TOP, the join technically didn't change anything.
         */
        if (
                (std::get<AbstractVals>(b_val) != AbstractVals::TOP) &&
                (std::get<AbstractVals>(a[b_key]) != AbstractVals::TOP)
        ) {
            a[b_key] = AbstractVals::TOP;
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
}