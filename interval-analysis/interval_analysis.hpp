#pragma once

#include <algorithm>
#include <iostream>
#include <limits>
#include <map>
#include <string>
#include <variant>

/*
 * Some macros to represent (+/-) infinity. We need these because I'm a real
 * idiot in how I decided to code this.
 */
#define INTERVAL_INFINITY       std::numeric_limits<int>::max()
#define INTERVAL_NEG_INFINITY   std::numeric_limits<int>::min()

/*
 * String representation of TOP for my convenience.
 */
#define TOP_STR                 "(NegInf, PosInf)"

/*
 * For interval analysis, we are defining TOP as [-INF, INF].
 *
 * I realize that for interval analysis this is kind of stupid, but rewriting
 * everything would take a lot of time which we do not have...
 */
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
 * This lets us print out our interval abstract store easily.
 */
struct IntervalVisitor {
    std::string operator()(interval val) {
        std::string lower = std::to_string(val.first);
        std::string upper = std::to_string(val.second);
        if (val.first == INTERVAL_NEG_INFINITY) {
            lower = "NegInf";
        }
        if (val.second == INTERVAL_INFINITY) {
            upper = "PosInf";
        }
        std::string return_string = "[" + lower + ", " + upper + "]";
        return return_string;
    }
    std::string operator()(AbstractVals val) {
        if (val == AbstractVals::BOTTOM) {
            return "Bottom";
        } else {
            return TOP_STR;
        }
    }
};

/*
 * Pretty-print an interval abstract store.
 */
void print(const interval_abstract_store &store) {
    for (const auto &[var_name, var_value] : store) {
        std::cout << var_name << " -> " << std::visit(IntervalVisitor{}, var_value) << std::endl;
    }
}

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
        if (std::holds_alternative<AbstractVals>(a_val) && std::visit(IntervalVisitor{}, a_val) == "Bottom") {
            a[b_key] = b_val;
            a_changed = true;
            continue;
        }

        abstract_interval new_a;

        //std::cout << "DEBUG " << __FILE_NAME__ << ":" << __LINE__ << std::endl;

        if (std::holds_alternative<AbstractVals>(b_val)) {
            std::cout << "DEBUG " << __FILE_NAME__ << ":" << __LINE__ << std::endl;
        }

        /*
         * If either variable is TOP, we know to assign TOP, no questions asked.
         */
        if (
                ((std::holds_alternative<AbstractVals>(b_val)) && (std::visit(IntervalVisitor{}, b_val) == TOP_STR)) ||
                ((std::holds_alternative<AbstractVals>(a_val)) && (std::visit(IntervalVisitor{}, a_val) == TOP_STR))
        ) {

            std::cout << "DEBUG " << __FILE_NAME__ << ":" << __LINE__ << std::endl;

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
            int new_upper_bound = std::max(std::get<interval>(a_val).second,
                                           std::get<interval>(b_val).second);

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
 *
 * TODO
 */
bool widen(interval_abstract_store &a, const interval_abstract_store &b) {
    bool a_changed = false;

    /*
     * Loop through each entry in b.
     */
    for (const auto &[b_key, b_val] : b) {
        abstract_interval a_val = get_val_from_store(a, b_key);

        /*
         * If the variable isn't in a, add it and mark the store as changed.
         */
        if (std::holds_alternative<AbstractVals>(a_val) && std::visit(IntervalVisitor{}, a_val) == "Bottom") {
            a[b_key] = b_val;
            a_changed = true;
            continue;
        }

        abstract_interval new_a;

        std::cout << "DEBUG " << __FILE_NAME__ << ":" << __LINE__ << std::endl;
        std::cout << std::visit(IntervalVisitor{}, b_val) << std::endl;
        if (std::holds_alternative<AbstractVals>(b_val)) {
            std::cout << "AbstractVals" << std::endl;
        } else {
            std::cout << "interval" << std::endl;
        }
        std::cout << "DEBUG " << __FILE_NAME__ << ":" << __LINE__ << std::endl;

        /*
         * If either variable is TOP, we know to assign TOP, no questions asked.
         */
        if (
                ((std::holds_alternative<AbstractVals>(b_val)) && (std::visit(IntervalVisitor{}, b_val) == TOP_STR)) ||
                ((std::holds_alternative<AbstractVals>(a_val)) && (std::visit(IntervalVisitor{}, a_val) == TOP_STR))
                ) {

            std::cout << "DEBUG " << __FILE_NAME__ << ":" << __LINE__ << std::endl;

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
             * If a's lower bound is less than or equal to b's lower bound, the
             * new lower bound is a's lower bound. Otherwise, the new lower
             * bound is negative infinity.
             *
             * If a's upper bound is greater than or equal to b's upper bound,
             * the new upper bound is a's upper bound. Otherwise, it's infinity.
             */

            int new_lower_bound = INTERVAL_NEG_INFINITY;
            int new_upper_bound = INTERVAL_INFINITY;

            if (std::get<interval>(a_val).first <= std::get<interval>(b_val).first) {
                new_lower_bound = std::get<interval>(a_val).first;
            }

            if (std::get<interval>(a_val).second >= std::get<interval>(b_val).second) {
                new_upper_bound = std::get<interval>(a_val).second;
            }

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