#include "reaching_definitions.hpp"

/*
 * The entry point to our analysis.
 */
int main(int argc, char *argv[]) {
    if (argc != 3) {
        print_debug();
        std::cout << "Usage: ./assn2_reaching_defs <path to JSON> <function name>" << std::endl;
        exit(EXIT_FAILURE);
    }
}