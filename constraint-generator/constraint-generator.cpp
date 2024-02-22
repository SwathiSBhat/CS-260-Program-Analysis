#include <fstream>
#include <iostream>
#include <set>
#include <string>
#include <variant>

#include "../headers/datatypes.h"

/*
 * For debugging.
 */
#define DEBUG(x) std::cout << "(" << __FILE_NAME__ << ":" << __LINE__ << ") " << x << std::endl

/*
 * For globals and $alloc identifiers, func_name will be the empty string.
 */
typedef struct SetVariable {
    std::string var_name;
    std::string func_name;
} SetVariable;

/*
 * We have to forward-declare a Constructor because a constructor argument can
 * be either a SetVariable or another Constructor.
 */
typedef struct Constructor Constructor;

typedef std::variant<SetVariable, Constructor> Term;

/*
 * TODO A constructor argument can be either a SetVariable or another
 * TODO Constructor.
 */
typedef struct Constructor {
    std::vector<Term> args;
    std::string name;
} Constructor;

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

Statement get_copy_constraint(CopyInstruction copy, std::string func_name) {
    DEBUG("Getting $copy constraint");
    SetVariable x;
    x.var_name = copy.lhs->name;

    /*
     * TODO Get function name somehow.
     */
    x.func_name = func_name;
    SetVariable y;
    y.var_name = copy.op->var->name;
    y.func_name = func_name;
    Statement s;
    s.e1 = y;
    s.e2 = x;
    return s;
}

/*
 * TODO Add function name argument to all of these.
 */

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
 * TODO I should refactor this.
 */
void print_constraint(Statement c) {

    /*
     * First, let's look at e1.
     */
    std::string e1_str = "";
    if (std::holds_alternative<Term>(c.e1)) {
        Term e1_term = std::get<Term>(c.e1);
        if (std::holds_alternative<SetVariable>(e1_term)) {
            SetVariable e1_set_var = std::get<SetVariable>(e1_term);
            e1_str = e1_set_var.func_name + "." + e1_set_var.var_name;
        }
    }

    /*
     * Now let's look at e2.
     */
    std::string e2_str = "";
    if (std::holds_alternative<Term>(c.e2)) {
        Term e2_term = std::get<Term>(c.e2);
        if (std::holds_alternative<SetVariable>(e2_term)) {
            SetVariable e2_set_var = std::get<SetVariable>(e2_term);
            e2_str = e2_set_var.func_name + "." + e2_set_var.var_name;
        }
    }

    /*
     * Now let's actually print out our constraint.
     */
    std::cout << e1_str << " <= " << e2_str << std::endl;
}

/*
 * Print a set variable without a trailing newline.
 */
void print_set_var(SetVariable s) {
    std::cout << s.func_name << "." << s.var_name;
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
                    DEBUG("Saw a $copy");
                    CopyInstruction *copy = (CopyInstruction *) instruction;

                    /*
                     * If the LHS isn't a pointer, ignore it.
                     */
                    if (copy->lhs->type->indirection != 0) {
                        constraints.push_back(get_copy_constraint(*copy, p.funcs[argv[2]]->name));
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
                    DEBUG("Unrecognized instruction type");
                    break;
                }
            } // End of switch-case
        }
    }

    /*
     * Now print out all our constraints.
     */
    for (const auto &constraint : constraints) {
        print_constraint(constraint);
    }
}