#include <iostream>
#include <string>
#include <variant>

/*
 * For debugging.
 */
#define DEBUG(x) std::cout << "(" << __FILE_NAME__ << ":" << __LINE__ << ") " << x << std::endl;

/*
 * For globals and $alloc identifiers, func_name will be the empty string.
 */
typedef struct SetVariable {
    std::string var_name;
    std::string func_name;
} SetVariable;

typedef struct Constructor {
    //std::vector<Term> args;
} Constructor;
struct Projection {};
typedef std::variant<SetVariable, Constructor> Term;
typedef std::variant<Term, Projection> Expression;

/*
 * Define a statement of the form e1 <= e2.
 */
struct Statement {
    Expression e1;
    Expression e2;
};

int main(int argc, char *argv[]) {
    DEBUG("Hello world");
}