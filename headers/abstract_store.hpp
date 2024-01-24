#pragma once

#include <iostream>
#include <map>
#include <string>
#include <variant>

/*
 * These are the only abstract values we care about because constants are just
 * represented as integers (using std::variant).
 */
enum AbstractVal {
    BOTTOM,
    TOP
};

/*
 * This lets us print out the value of a variant<int, AbstractVal> easily. Helps
 * with debugging and comparison.
 */
struct AbstractValStringifyVisitor {
    std::string operator()(int val) {
        return std::to_string(val);
    }
    std::string operator()(AbstractVal val) {
        if (val == AbstractVal::BOTTOM) {
            return "Bottom";
        } else {
            return "Top";
        }
    }
};

class AbstractStore {
public:

    /*
     * We map strings (variable names) to either an integer (the actual
     * constant) or BOTTOM or TOP. Pretty cool, eh?
     */
    std::map<std::string, std::variant<int, AbstractVal>> abstract_store;

    /*
     * Constructor that lets you specify a mapping. Note that if you want an
     * abstract store without any entries, use the other constructor.
     */
    AbstractStore(std::map<std::string, std::variant<int, AbstractVal>> map) {
        abstract_store = map;
    }

    /*
     * Constructor for when you want an empty abstract store to start.
     */
    AbstractStore() {
        abstract_store = std::map<std::string, std::variant<int, AbstractVal>>();
    }

    /*
     * Get the value of a variable from the abstract store.
     */
    std::variant<int, AbstractVal> GetValFromStore(std::string var_name) {
        if (abstract_store.count(var_name) == 0) {
            return AbstractVal::BOTTOM;
        } else {
            return abstract_store[var_name];
        }
    }

    /*
     * Join this abstract store with another and store the result into this
     * abstract store. Return true if our abstract store changed and false
     * otherwise.
     */
    bool join(AbstractStore as) {

        bool store_changed = false;

        /*
         * For each key in the incoming abstract store:
         * -> If the key is not present in this one, just add it.
         * -> If the key is present, join the values.
         */
        for (const auto& pair : as.abstract_store) {
            if (abstract_store.count(pair.first) == 0) {
                abstract_store[pair.first] = pair.second;
                store_changed = true;
            } else {

                /*
                 * TODO Double-check me on this logic.
                 *
                 * Rules for joining:
                 * -> Anything joined with TOP is TOP.
                 * -> Joining any two unequal constants gives TOP.
                 * -> Joining two equal constants gives that same constant
                 * -> We don't have to worry about BOTTOM because we won't put
                 *    BOTTOM values in our abstract store.
                 */
                if (std::visit(AbstractValStringifyVisitor{}, pair.second) == "TOP" && std::visit(AbstractValStringifyVisitor{}, abstract_store[pair.first]) != "TOP") {
                    abstract_store[pair.first] = TOP;
                    store_changed = true;
                } else if (pair.second != abstract_store[pair.first]) {
                    abstract_store[pair.first] = TOP;
                    store_changed = true;
                }
            }
        }

        return store_changed;
    }

    /*
     * Pretty-print the abstract store.
     */
    void print() {

        /*
         * If the abstract store is empty, let's print something out to show
         * that.
         */
        if (abstract_store.empty()) {
            std::cout << "<Empty abstract store>" << std::endl;
        }

        /*
         * Loop through all the entries and print them out.
         *
         * TODO Eventually Ben wants us to print these out in alphabetical
         * TODO order.
         */
        for (const auto& pair : abstract_store) {
            std::cout << pair.first << " -> " << std::visit(AbstractValStringifyVisitor{}, pair.second) << std::endl;
        }
    }
};