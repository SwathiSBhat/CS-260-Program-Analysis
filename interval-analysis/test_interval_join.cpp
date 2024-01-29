#include <iostream>

#include "interval_analysis.hpp"

int main() {

    /*
     * This is really annoying and ugly, but I don't know any better way to
     * initialize this easily.
     */

    std::cout << "DEBUG " << __FILE_NAME__ << ":" << __LINE__ << std::endl;

    abstract_interval i1;
    std::get<interval>(i1) = {-10, 5};

    abstract_interval i2;
    std::get<AbstractVals>(i2) = TOP;

    abstract_interval i3;
    std::get<interval>(i3) = {11, 45};

    std::cout << "DEBUG " << __FILE_NAME__ << ":" << __LINE__ << std::endl;

    interval_abstract_store a = {{"v1", i1},
                               {"v2", i2},
                               {"v3", i3}};
}