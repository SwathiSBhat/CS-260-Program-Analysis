#include "../headers/abstract_store.hpp"

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

    /*
     * Now let's try joining a populated abstract store with an empty abstract
     * store and see what happens.
     */
    AbstractStore as3 = AbstractStore();
    bool as3_changed = as3.join(as1);
    if (as3_changed) {
        std::cout << "Yep! as3 changed" << std::endl;
    } else {
        std::cout << "What! as3 didn't change" << std::endl;
    }
    as3.print();

    return EXIT_SUCCESS;
}