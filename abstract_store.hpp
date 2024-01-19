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
 * with debugging.
 */
struct AbstractValStringifyVisitor {
    std::string operator()(int val) {
        return std::to_string(val);
    }
    std::string operator()(AbstractVal val) {
        if (val == BOTTOM) {
            return "BOTTOM";
        } else {
            return "TOP";
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
     * Join this abstract store with another and store the result into this
     * abstract store.
     */
    void join(AbstractStore as) {

        /*
         * For each key in the incoming abstract store:
         * -> If the key is not present in this one, just add it.
         * -> If the key is present, join the values.
         */
        for (const auto& pair : as.abstract_store) {
            if (abstract_store.count(pair.first) == 0) {
                abstract_store[pair.first] = pair.second;
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
                if (pair.second == TOP) {
                    abstract_store[pair.first] = TOP;
                } else if (pair.second != abstract_store[pair.first]) {
                    abstract_store[pair.first] = TOP;
                }
            }
        }
    }

    /*
     * Pretty-print the abstract store.
     */
    void print() {
        for (const auto& pair : abstract_store) {
            std::cout << pair.first << " -> " << pair.second.get << std::endl;
        }
    }
};