#include <algorithm>
#include <iostream>
#include <limits>
#include <map>
#include <string>
#include <queue>
#include<set>

class PDGNode {
    public:
    std::string program_point;
    std::set<std::string> dd_pred, dd_succ; // data dependency edges - predecessors and successors
    std::set<std::string> cd_pred, cd_succ; // control dependency edges - predecessors and successors

    PDGNode(std::string program_point) {
        program_point = program_point;
    }
};

int main(int argc, char const *argv[])
{
    
    return 0;
}