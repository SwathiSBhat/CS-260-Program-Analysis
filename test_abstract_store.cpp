#include "abstract_store.hpp"

int main() {
    AbstractStore my_abstract_store;
    my_abstract_store.abstract_store = {
            {"v1", TOP},
            {"v2", 42},
            {"v3", BOTTOM}
    };
    my_abstract_store.print();
    return EXIT_SUCCESS;
}