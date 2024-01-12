/*
 * CS260 winter 2024 assignment 0.
 */

/*
 * diff struct for each
 */
#include <fstream>
#include <iostream>

/*
 * https://github.com/nlohmann/json
 */
#include "json.hpp"

using json = nlohmann::json;

int main(int argc, char *argv[]) {

    if (argc != 2) {
        std::cerr << "Usage: assn0 <filepath>" << std::endl;
        return EXIT_FAILURE;
    }

    /*
     * Open and parse our JSON.
     */
    std::ifstream f(argv[1]);
    json lir_json = json::parse(f);

    /*
     * Initialize accumulators for the statistics we need.
     */
    uint64_t num_struct_fields = 0;
    uint64_t num_return_funcs = 0;
    uint64_t num_func_params = 0;
    uint64_t num_local_vars = 0;
    uint64_t num_basic_blocks = 0;
    uint64_t num_instructions = 0;
    uint64_t num_terminals = 0;
    uint64_t num_int_locals_globals = 0;
    uint64_t num_struct_locals_globals = 0;
    uint64_t num_ptr_int_locals_globals = 0;
    uint64_t num_ptr_struct_locals_globals = 0;
    uint64_t num_ptr_func_locals_globals = 0;
    uint64_t num_ptr_ptr_locals_globals = 0;

    /*
     * Scan through our top-level LIR JSON object.
     */
    for (auto &[lir_key, lir_value] : lir_json.items()) {

        /*
         * Let's look at our structs.
         */
        if (lir_key == "structs") {

            /*
             * Loop through each struct.
             */
            for (auto &[structs_key, structs_val] : lir_value.items()) {

                /*
                 * Loop through all fields within that struct and count up the
                 * number of fields.
                 */
                for (auto &[struct_key, struct_val] : structs_val.items()) {
                    num_struct_fields++;
                }
            }
        }

        /*
         * Let's look at our functions.
         */
        if (lir_key == "functions") {

            /*
             * Loop through all functions.
             */
            for (auto &[func_key, func_value] : lir_value.items()) {

                /*
                 * Only increment our accumulator if the function actually
                 * returns something.
                 */
                if (func_value["ret_ty"] != nullptr) {
                    num_return_funcs++;
                }

                /*
                 * Count up the function parameters.
                 */
                num_func_params += func_value["params"].size();

                /*
                 * Count up the local variables.
                 */
                num_local_vars += func_value["locals"].size();

                /*
                 * Count up basic blocks.
                 */
                num_basic_blocks += func_value["body"].size();

                /*
                 * Count up the number of instructions by going through each
                 * block. TODO this doesn't work properly?
                 */
                for (auto &[func_body_key, func_body_val] : func_value["body"].items()) {
                    num_instructions += func_body_val.size();
                }
            }
        }
    }

    /*
     * Now let's print out our statistics.
     */
    std::cout << "Number of fields across all struct types: " << num_struct_fields << std::endl;
    std::cout << "Number of functions that return a value: " << num_return_funcs << std::endl;
    std::cout << "Number of function parameters: " << num_func_params << std::endl;
    std::cout << "Number of local variables: " << num_local_vars << std::endl;
    std::cout << "Number of basic blocks: " << num_basic_blocks << std::endl;
    std::cout << "Number of instructions: " << num_instructions << std::endl;
    std::cout << "Number of terminals: " << num_terminals << std::endl;
    std::cout << "Number of int locals/globals: " << num_int_locals_globals << std::endl;
    std::cout << "Number of struct locals/globals: " << num_struct_locals_globals << std::endl;
    std::cout << "Number of int pointer locals/globals: " << num_ptr_int_locals_globals << std::endl;
    std::cout << "Number of struct pointer locals/globals: " << num_ptr_struct_locals_globals << std::endl;
    std::cout << "Number of function pointer locals/globals: " << num_ptr_func_locals_globals << std::endl;
    std::cout << "Number of pointer pointer locals/globals: " << num_ptr_ptr_locals_globals << std::endl;
}