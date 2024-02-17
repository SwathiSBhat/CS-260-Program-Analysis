#include <fstream>
#include <iostream>
#include <set>
#include <string>
#include <variant>

#include "../headers/datatypes.h"

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

/*
 * TODO a constructor argument can be either a SetVariable or another
 * TODO Constructor.
 */
typedef struct Constructor {
    std::vector<SetVariable> args;
    std::string name;
} Constructor;

typedef std::variant<SetVariable, Constructor> Term;

/*
 * Define a projection of c^(-arg)(v).
 */
typedef struct Projection {
    SetVariable v;
    Constructor c;
    int arg;
} Projection;

/*
 * An Expression can either be a SetVariable, a Constructor, or a Projection.
 */
typedef std::variant<Term, Projection> Expression;

/*
 * Define a statement of the form e1 <= e2.
 */
struct Statement {
    Expression e1;
    Expression e2;
};

Statement get_copy_constraint(CopyInstruction copy) {
    DEBUG("Getting $copy constraint");
    SetVariable x;
    x.var_name = copy.lhs->name;

    /*
     * TODO Get function name somehow.
     */
    x.func_name = "";
    SetVariable y;
    y.var_name = copy.op->var->name;
    y.func_name = "";
    Statement s;
    s.e1 = y;
    s.e2 = x;
    return s;
}

Statement get_addrof_constraint(AddrofInstruction addrof) {
    DEBUG("Getting $addrof constraint");
}

Statement get_alloc_constraint(AllocInstruction alloc) {
    DEBUG("Getting $alloc constraint");
}

Statement get_gep_constraint(GepInstruction gep) {
    DEBUG("Getting $gep constraint");
    SetVariable x;
    x.var_name = gep.lhs->name;
    x.func_name = "";
    SetVariable y;
    y.var_name = gep.src->name;
    y.func_name = "";
    Statement s;
    s.e1 = y;
    s.e2 = x;
    return s;
}

Statement get_load_constraint(LoadInstruction load) {
    DEBUG("Getting $load constraint");
}

Statement get_store_constraint(StoreInstruction store) {
    DEBUG("Getting $store constraint");
}

/*
 * TODO
 */
void print_constraint(Statement c) {
    std::cout << "Yeah this is a statement all right" << std::endl;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cout << "Usage: ./assn3-constraint-generator <json> <func>" << std::endl;
        exit(EXIT_FAILURE);
    }
    Program p = Program(json::parse(std::ifstream(argv[1])));

    /*
     * Our list of constraints is represented as a vector of Statements.
     */
    std::vector<Statement> constraints;

    /*
     * TODO We also want to loop over each function in the program as well.
     */
    for (const auto &bb : p.funcs[argv[2]]->bbs) {
        DEBUG(bb.first);
        for (const auto& instruction : bb.second->instructions) {
            switch (instruction->instrType) {
                case CopyInstrType: {
                    DEBUG("Saw a $copy")
                    CopyInstruction *copy = (CopyInstruction *) instruction;

                    /*
                     * If the LHS isn't a pointer, ignore it.
                     */
                    if (copy->lhs->type->indirection != 0) {
                        constraints.push_back(get_copy_constraint(*copy));
                    }
                    break;
                }
                case AddrofInstrType: {
                    DEBUG("Saw a $addrof");
                    //constraints.push_back(get_addrof_constraint(*((AddrofInstruction *) instruction)));
                    break;
                }
                case AllocInstrType: {
                    DEBUG("Saw a $alloc");
                    //constraints.push_back(get_alloc_constraint(*((AllocInstruction *) instruction)));
                    break;
                }
                case GepInstrType: {
                    DEBUG("Saw a $gep");
                    //constraints.push_back(get_gep_constraint(*((GepInstruction *) instruction)));
                    break;
                }
                case LoadInstrType: {
                    DEBUG("Saw a $load");
                    //constraints.push_back(get_load_constraint(*((LoadInstruction *) instruction)));
                    break;
                }
                case StoreInstrType: {
                    DEBUG("Saw a $store");
                    //constraints.push_back(get_store_constraint(*((StoreInstruction *) instruction)));
                    break;
                }
                default: {
                    DEBUG("Unrecognized instruction type")
                    break;
                }
            } // End of switch-case
        }
    }
}