#include <fstream>

#include "../headers/json.hpp"
#include "../headers/MFPWorklistAlgorithm.hpp"

using json = nlohmann::json;

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: test_worklist <filepath>" << std::endl;
        return EXIT_FAILURE;
    }

    /*
     * Open and parse our JSON.
     */
    std::ifstream f(argv[1]);
    json lir_json = json::parse(f);
    MFPWorklistAlgorithm a = MFPWorklistAlgorithm(lir_json);
}