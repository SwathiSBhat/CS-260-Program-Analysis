#include <iostream>
#include <variant>

#include "interval_analysis.hpp"

int main() {

    /*
     * This is really annoying and ugly, but I don't know any better way to
     * initialize this easily.
     */

    std::cout << "DEBUG " << __FILE_NAME__ << ":" << __LINE__ << std::endl;

    interval_abstract_store s1 = {{"v1", std::make_pair(1, 2)},
                                 {"v2", TOP},
                                 {"v3", std::make_pair(11, 45)}};

    interval_abstract_store s2 = {{"v1", std::make_pair(1, 5)},
                                 {"v2", BOTTOM},
                                 {"v3", std::make_pair(-90, 4)}};

    std::cout << "DEBUG " << __FILE_NAME__ << ":" << __LINE__ << std::endl;

    /*
     * Join s2 to s1, modifying a in-place.
     */
    if (join(s1, s2)) {
        std::cout << "New value of a:" << std::endl;
        print(s1);
    } else {
        std::cout << "No change" << std::endl;
    }
}