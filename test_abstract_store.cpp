#include "abstract_store.hpp"

int main() {

    /*
     * This is how you define an AbstractStore, which is just a glorified
     * wrapper around a std::map.
     */
    AbstractStore as1 = AbstractStore({
        {"v1", TOP},
        {"v2", 42},
        {"v3", 8}
    });
    AbstractStore as2 = AbstractStore({
        {"v1", 6},
        {"v2", 32},
        {"v4", TOP}
    });

    /*
     * We can also pretty-print them out.
     */
    std::cout << "as1:" << std::endl;
    as1.print();
    std::cout << "as2:" << std::endl;
    as2.print();

    /*
     * Try joining as2 to as1.
     */
    as1.join(as2);
    std::cout << "After join:" << std::endl;
    as1.print();

    return EXIT_SUCCESS;
}