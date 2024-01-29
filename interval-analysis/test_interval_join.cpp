#include <iostream>

#include "interval_analysis.hpp"
#include<variant>

int main() {

    /*
     * This is really annoying and ugly, but I don't know any better way to
     * initialize this easily.
     */

    std::cout << "DEBUG " << __FILE_NAME__ << ":" << __LINE__ << std::endl;

    std::variant<interval, AbstractVals> i1;
    i1 = std::make_pair(1, 2);

    std::variant<interval, AbstractVals> i2;
    i2 = TOP;

    std::variant<interval, AbstractVals> i3;
    i3 = std::make_pair(11,45);

    std::cout << "DEBUG " << __FILE_NAME__ << ":" << __LINE__ << std::endl;

    interval_abstract_store a = {{"v1", i1},
                               {"v2", i2},
                               {"v3", i3}};
}