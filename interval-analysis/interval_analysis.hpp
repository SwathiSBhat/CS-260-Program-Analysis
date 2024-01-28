#pragma once

enum AbstractVals {
    BOTTOM,
    TOP
};

/*
 * Some typedefs for convenience.
 */
typedef std::pair<int, int> interval
typedef std::variant<interval, AbstractVals> abstract_interval
typedef std::map<std::string, abstract_interval> interval_abstract_store;

/*
 * If var is not present in the store, return BOTTOM. Else return the value in
 * the store.
 */
abstract_interval get_val_from_store(interval_abstract_store m,
                                     std::string var) {
    if (m.count(var) == 0) {
        return AbstractVals::BOTTOM;
    }
    return m[var];
}

/*
 * Join b to a and return whether a changed or not.
 */
bool join(interval_abstract_store &a, interval_abstract_store b) {

    bool a_changed = false;

    for (const auto &[b_key, b_val] : b) {

        /*
         * If this variable isn't present in a, add it and mark it as changed.
         */
        if (a.count(b_key) == 0) {
            a[b_key] = b_val;
            store_changed = true;
        }

        /*
         * [x1, y1] JOIN [x2, y2] = [min(x1, x2), max(y1, y2)]
         */

        /*
         * Compute min(x1, x2).
         */
        if (
                std::get<AbstractVals>(a[b_key]) == AbstractVals::TOP
                || std::get<AbstractVals>(a[b_key]) == AbstractVals::TOP) {

        }
    }
}