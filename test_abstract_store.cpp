#include "abstract_store.hpp"

int main() {
    AbstractStore as1;
    AbstractStore as2;
    as1.abstract_store = {
            {"v1", TOP},
            {"v2", 42},
            {"v3", 8}
    };
    as2.abstract_store = {
            {"v1", 6},
            {"v2", 32},
            {"v4", TOP}
    };
    std::cout << "as1:" << std::endl;
    as1.print();
    std::cout << "as2:" << std::endl;
    as2.print();
    as1.join(as2);
    std::cout << "After join:" << std::endl;
    as1.print();
    return EXIT_SUCCESS;
}