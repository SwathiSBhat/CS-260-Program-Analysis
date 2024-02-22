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

/*
 * TODO I'm not sure this is correct.
 */
Statement get_addrof_constraint(AddrofInstruction addrof, std::string func_name) {
    DEBUG("Getting $addrof constraint");
    SetVariable x;
    x.var_name = addrof.lhs->name;
    x.func_name = func_name;
    Constructor y;
    y.name = "ref";
    SetVariable y_arg;
    y_arg.var_name = addrof.rhs->name;
    y_arg.func_name = func_name;
    y.args.push_back(y_arg);
    y.args.push_back(y_arg);
    Statement s;
    s.e1 = y;
    s.e2 = x;
    return s;
}

Statement get_alloc_constraint(AllocInstruction alloc, std::string func_name) {
    DEBUG("Getting $alloc constraint");
    SetVariable x;
    x.var_name = alloc.lhs->name;
    x.func_name = func_name;
    Constructor y;
    y.name = "ref";
    SetVariable y_arg;
    y_arg.var_name = alloc.id->name;

    /*
     * TODO We don't want to print out a dot here.
     */
    y_arg.func_name = "";
    y.args.push_back(y_arg);
    y.args.push_back(y_arg);
    Statement s;
    s.e1 = y;
    s.e2 = x;
    return s;
}

Statement get_gep_constraint(GepInstruction gep, std::string func_name) {
    DEBUG("Getting $gep constraint");
    SetVariable x;
    x.var_name = gep.lhs->name;
    x.func_name = func_name;
    SetVariable y;
    y.var_name = gep.src->name;
    y.func_name = func_name;
    Statement s;
    s.e1 = y;
    s.e2 = x;
    return s;
}

Statement get_load_constraint(LoadInstruction load, std::string func_name) {
    DEBUG("Getting $load constraint");
    SetVariable x;
    x.var_name = load.lhs->name;
    x.func_name = func_name;
    Projection y;
    y.arg = 1;

    /*
     * TODO What should I put in the args field?
     */
    Constructor y_constructor;
    y_constructor.name = "ref";
    y.c = y_constructor;
    SetVariable y_set_var;
    y_set_var.var_name = load.src->name;
    y_set_var.func_name = func_name;
    y.v = y_set_var;
    Statement s;
    s.e1 = y;
    s.e2 = x;
    return s;
}

Statement get_store_constraint(StoreInstruction store, std::string func_name) {
    DEBUG("Getting $store constraint");
    SetVariable y;
    y.var_name = store.op->var->name;
    y.func_name = func_name;
    Projection x;
    x.arg = 1;
    Constructor x_constructor;
    x_constructor.name = "ref";
    x.c = x_constructor;
    SetVariable x_set_var;
    x_set_var.var_name = store.dst->name;
    x_set_var.func_name = func_name;
    x.v = x_set_var;
    Statement s;
    s.e1 = y;
    s.e2 = x;
    return s;
}

/*
 * TODO I should refactor this. And add comments.
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
        } else if (std::holds_alternative<Constructor>(e1_term)) {
            Constructor e1_constructor = std::get<Constructor>(e1_term);
            e1_str = e1_constructor.name + "(";
            for (const Term &term : e1_constructor.args) {

                /*
                 * TODO I could factor this out.
                 */
                e1_str += std::get<SetVariable>(term).func_name + "." + std::get<SetVariable>(term).var_name + ",";
            }
            e1_str += ")";
        }
    } else if (std::holds_alternative<Projection>(c.e1)) {
        Projection e1_proj = std::get<Projection>(c.e1);
        e1_str = "proj(";
        e1_str += e1_proj.c.name;
        e1_str += ",";
        e1_str += std::to_string(e1_proj.arg);
        e1_str += ",";
        e1_str += e1_proj.v.func_name;
        e1_str += ".";
        e1_str += e1_proj.v.var_name;
        e1_str += ")";
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
        } else if (std::holds_alternative<Constructor>(e2_term)) {
            Constructor e2_constructor = std::get<Constructor>(e2_term);
            e2_str = e2_constructor.name + "(";
            for (const Term &term: e2_constructor.args) {
                e2_str += std::get<SetVariable>(term).func_name + "." + std::get<SetVariable>(term).var_name + ",";
            }
            e2_str += ")";
        }
    } else if (std::holds_alternative<Projection>(c.e2)) {
        Projection e2_proj = std::get<Projection>(c.e2);
        e2_str = "proj(";
        e2_str += e2_proj.c.name;
        e2_str += ",";
        e2_str += std::to_string(e2_proj.arg);
        e2_str += ",";
        e2_str += e2_proj.v.func_name;
        e2_str += ".";
        e2_str += e2_proj.v.var_name;
        e2_str += ")";
    }

    /*
     * Now let's actually print out our constraint.
     */
    std::cout << e1_str << " <= " << e2_str << std::endl;
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
                    constraints.push_back(get_addrof_constraint(*((AddrofInstruction *) instruction), p.funcs[argv[2]]->name));
                    break;
                }
                case AllocInstrType: {
                    DEBUG("Saw a $alloc");
                    constraints.push_back(get_alloc_constraint(*((AllocInstruction *) instruction), p.funcs[argv[2]]->name));
                    break;
                }
                case GepInstrType: {
                    DEBUG("Saw a $gep");
                    constraints.push_back(get_gep_constraint(*((GepInstruction *) instruction), p.funcs[argv[2]]->name));
                    break;
                }
                case LoadInstrType: {
                    DEBUG("Saw a $load");
                    LoadInstruction *load = (LoadInstruction *) instruction;

                    /*
                     * Check that the lhs is a pointer.
                     */
                    if (load->lhs->type->indirection != 0) {
                        constraints.push_back(get_load_constraint(*load, p.funcs[argv[2]]->name));
                    }
                    break;
                }
                case StoreInstrType: {
                    DEBUG("Saw a $store");
                    StoreInstruction *store = (StoreInstruction *) instruction;

                    /*
                     * Check that the value being stored is also a pointer.
                     */
                    if (!store->op->IsConstInt()) {
                        if (store->op->var->type->indirection != 0) {
                            constraints.push_back(get_store_constraint(*store, p.funcs[argv[2]]->name));
                        }
                    }
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