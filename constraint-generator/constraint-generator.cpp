#include <fstream>
#include <iostream>
#include <set>
#include <string>
#include <variant>

#include "../headers/datatypes.h"

/*
 * For debugging.
 */
//#define DEBUG(x) std::cout << "(" << __FILE_NAME__ << ":" << __LINE__ << ") " << x << std::endl

/*
 * For globals and $alloc identifiers, func_name will be the empty string.
 */
typedef struct SetVariable {
    std::string var_name;
    std::string func_name;
    bool is_local = true;
} SetVariable;

/*
 * We have to forward-declare a Constructor because a constructor argument can
 * be either a SetVariable or another Constructor.
 */
typedef struct Constructor Constructor;

typedef std::variant<SetVariable, Constructor> Term;

/*
 * A constructor argument can be either a SetVariable or another Constructor.
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

/*
 * Get a pointer to the variable that a particular function returns (if any).
 * Return a nullptr otherwise.
 */
Variable* get_ret_val(Function *f) {

    /*
     * Loop through each basic block and find the one that actually returns
     * a value.
     */
    for (const auto &bb : f->bbs) {
        if (bb.second->terminal->instrType == RetInstrType) {
            RetInstruction *ret = (RetInstruction *) bb.second->terminal;
            if (ret->op && !ret->op->IsConstInt()) {
                return ret->op->var;
            }
        }
    }
    return nullptr;
}

/*
 * Build a type string for a given function signature. We will use this when
 * generating the constraints for $call_idr instructions.
 */
std::string build_func_type_str(Type::FunctionType func_type) {

    /*
     * This is the string we'll eventually return.
     */
    std::string type_str = "(";

    /*
     * Loop through each parameter.
     */
    for (int i = 0; i < func_type.params.size(); i++) {

        /*
         * Add ampersands if our current parameter is a pointer.
         */
        if (func_type.params[i]->indirection > 0) {
            for (int j = 0; j < func_type.params[i]->indirection; j++) {
                type_str += "&";
            }
        }

        if (func_type.params[i]->type == IntType) {
            type_str += "int";
        } else if (func_type.params[i]->type == StructType) {
            type_str += ((Type::StructType *) func_type.params[i]->ptr_type)->name;
        }

        /*
         * Handle those pesky commas between parameters.
         */
        if (i != func_type.params.size() - 1) {
            type_str += ",";
        }
    }

    /*
     * We're done with parameters, so let's look at the return type now.
     */
    type_str += ")->";
    if (func_type.ret) {
        if (func_type.ret->indirection > 0) {
            for (int i = 0; i < func_type.ret->indirection; i++) {
                type_str += "&";
            }
            if (func_type.ret->type == IntType) {
                type_str += "int";
            } else if (func_type.ret->type == StructType) {
                type_str += ((Type::StructType *) func_type.ret->ptr_type)->name;
            }
        }
    } else {
        type_str += "_";
    }

    /*
     * Return our complete type string.
     */
    return type_str;
}

/*
 * Helper function that determines whether a variable is a local or a global.
 */
bool is_global(const std::string &func_name,
               const Variable &var,
               Program &prog) {

    /*
     * If we find a local with the same name, we know it isn't a global.
     */
    if (prog.funcs[func_name]->locals.find(var.name) != prog.funcs[func_name]->locals.end()) {
        return false;
    }

    /*
     * If we find a global with the same name, we know it's a global.
     */
    for (const auto &glob : prog.globals) {
        if (glob->globalVar->name == var.name) {
            return true;
        }
    }

    /*
     * Catch-all case.
     */
    return false;
}

Statement get_copy_constraint(CopyInstruction copy,
                              std::string func_name,
                              Program &p) {
    SetVariable x;
    x.var_name = copy.lhs->name;
    if (is_global(func_name, *(copy.lhs), p)) {
        x.is_local = false;
    } else {
        x.func_name = func_name;
    }
    SetVariable y;
    y.var_name = copy.op->var->name;
    if (is_global(func_name, *(copy.op->var), p)) {
        y.is_local = false;
    } else {
        y.func_name = func_name;
    }
    Statement s;
    s.e1 = y;
    s.e2 = x;
    return s;
}

Statement get_addrof_constraint(AddrofInstruction addrof, std::string func_name) {
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
    SetVariable x;
    x.var_name = alloc.lhs->name;
    x.func_name = func_name;
    Constructor y;
    y.name = "ref";
    SetVariable y_arg;
    y_arg.var_name = alloc.id->name;
    y_arg.func_name = "";
    y_arg.is_local = false;
    y.args.push_back(y_arg);
    y.args.push_back(y_arg);
    Statement s;
    s.e1 = y;
    s.e2 = x;
    return s;
}

Statement get_gep_constraint(GepInstruction gep, std::string func_name) {
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

Statement get_gfp_constraint(GfpInstruction gfp, std::string func_name) {
    SetVariable x;
    x.var_name = gfp.lhs->name;
    x.func_name = func_name;
    SetVariable y;
    y.var_name = gfp.src->name;
    y.func_name = func_name;
    Statement s;
    s.e1 = y;
    s.e2 = x;
    return s;
}

Statement get_load_constraint(LoadInstruction load, std::string func_name) {
    SetVariable x;
    x.var_name = load.lhs->name;
    x.func_name = func_name;
    Projection y;
    y.arg = 1;
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

std::vector<Statement> get_call_dir_constraint(CallDirInstruction call_dir,
                                  Function *func,
                                  Function *callee) {

    /*
     * This is the list of constraints we'll eventually return.
     */
    std::vector<Statement> statements;

    /*
     * [retval(<func>)] <= [x]
     */
    if (call_dir.lhs && call_dir.lhs->type->indirection != 0) {
        Variable *ret_var = get_ret_val(callee);
        if (!ret_var) {
            exit(EXIT_FAILURE);

        }
        SetVariable x;
        x.func_name = func->name;
        x.var_name = call_dir.lhs->name;
        SetVariable ret_val;
        ret_val.func_name = callee->name;
        ret_val.var_name = ret_var->name;
        Statement s;
        s.e1 = ret_val;
        s.e2 = x;
        statements.push_back(s);
    }

    /*
     * For each argument, if it's a pointer, [arg] <= [param] such that param is
     * the corresponding function parameter.
     */
    for (int i = 0; i < callee->params.size(); i++) {
        if (callee->params[i]->type->indirection != 0) {
            SetVariable arg;
            arg.func_name = func->name;
            arg.var_name = call_dir.args[i]->var->name;
            SetVariable param;
            param.func_name = callee->name;
            param.var_name = callee->params[i]->name;
            Statement s;
            s.e1 = arg;
            s.e2 = param;
            statements.push_back(s);
        }
    }

    /*
     * Return our list of constraints generated by this instruction.
     */
    return statements;
}

Statement get_call_idr_constraint(CallIdrInstruction *call_idr, std::string func_name) {
    Type::FunctionType *func_type = (Type::FunctionType *) call_idr->fp->type->ptr_type;
    std::vector<Term> lam_args;

    /*
     * Create a dummy set variable and push it back.
     */
    SetVariable dummy;
    dummy.var_name = "_DUMMY";
    dummy.is_local = false;
    lam_args.push_back(dummy);

    /*
     * If the return type is a pointer, create a set variable for it. Else, use
     * a dummy.
     */
    if (func_type->ret && func_type->ret->indirection != 0) {
        if (call_idr->lhs && call_idr->lhs->type->indirection != 0) {
            SetVariable x;
            x.func_name = func_name;
            x.var_name = call_idr->lhs->name;
            lam_args.push_back(x);
        } else {

            /*
             * We already created a dummy set variable above, so we can just
             * reuse that.
             */
            lam_args.push_back(dummy);
        }
    }

    /*
     * Loop through each argument, filtering out the non-pointers.
     */
    for (const auto& arg : call_idr->args) {
        if (arg->var && arg->var->type->indirection != 0) {
            SetVariable s;
            s.func_name = func_name;
            s.var_name = arg->var->name;
            lam_args.push_back(s);
        }
    }

    std::string type_str = build_func_type_str(*func_type);

    SetVariable e1;
    e1.func_name = func_name;
    e1.var_name = call_idr->fp->name;
    Constructor e2;
    e2.name = "lam_[" + type_str + "]";
    e2.args = lam_args;
    Statement s;
    s.e1 = e1;
    s.e2 = e2;
    return s;
}

/*
 * Create and return all the constraints for global function pointers.
 */
std::vector<Statement> get_global_func_ptr_constraints(Program prog) {

    /*
     * This is the list of constraints we'll eventually return.
     */
    std::vector<Statement> constraints;

    /*
     * We'll use this data structure to keep track of function type signatures
     * that we've encountered before.
     */
    std::set<std::string> func_types;

    /*
     * Loop through all the program globals and filter out the ones that aren't
     * function pointers.
     */
    for (const auto &global : prog.globals) {
        if (global->globalVar->type->indirection != 0 && global->globalVar->type->type == DataType::FuncType) {
            std::string type_str = build_func_type_str(*((Type::FunctionType *) global->globalVar->type->ptr_type));

            /*
             * If we've encountered this function type before, skip it.
             */
            if (func_types.find(type_str) != func_types.end()) {
                continue;
            }

            /*
             * This is a unique function type, so add it.
             */
            func_types.insert(type_str);

            std::string func_name = global->globalVar->name;
            std::vector<Term> lam_args;
            SetVariable func_set_var;
            func_set_var.var_name = func_name;
            func_set_var.is_local = false;
            lam_args.push_back(func_set_var);
            if (prog.funcs[func_name]->ret && prog.funcs[func_name]->ret->indirection != 0) {
                SetVariable s;
                s.func_name = func_name;
                s.var_name = get_ret_val(prog.funcs[func_name])->name;
                lam_args.push_back(s);
            }

            /*
             * Loop through each parameter and filter out the ones that aren't
             * pointer-typed.
             */
            for (const auto &arg : prog.funcs[func_name]->params) {
                if (arg->type->indirection != 0) {
                    SetVariable s;
                    s.func_name = func_name;
                    //s.var_name = get_ret_val(prog.funcs[func_name])->name;
                    s.var_name = arg->name;
                    lam_args.push_back(s);
                }
            }

            /*
             * It's time to build the actual constraint.
             */
            Constructor e1;
            e1.name = "lam_[" + type_str + "]";
            e1.args = lam_args;
            SetVariable e2;
            e2.var_name = func_name;
            e2.is_local = false;
            Statement s;
            s.e1 = e1;
            s.e2 = e2;
            constraints.push_back(s);
        }
    }

    /*
     * Return our list of constraints.
     */
    return constraints;
}

/*
 * Return a nice string representation of a given SetVariable.
 */
std::string build_set_var_str(SetVariable s) {
    std::string ret_str = "";
    if (s.is_local) {
        ret_str += s.func_name + "." + s.var_name;
    } else {
        ret_str += s.var_name;
    }
    return ret_str;
}

/*
 * Return a string representation of an Expression.
 */
std::string build_expr_str(Expression e) {
    std::string e_str = "";
    if (std::holds_alternative<Term>(e)) {
        Term e_term = std::get<Term>(e);
        if (std::holds_alternative<SetVariable>(e_term)) {
            SetVariable e_set_var = std::get<SetVariable>(e_term);
            e_str = build_set_var_str(e_set_var);
        } else if (std::holds_alternative<Constructor>(e_term)) {
            Constructor e_constructor = std::get<Constructor>(e_term);
            e_str = e_constructor.name + "(";
            for (int i = 0; i < e_constructor.args.size(); i++) {
                SetVariable e_set_var = std::get<SetVariable>(e_constructor.args[i]);
                e_str += build_set_var_str(e_set_var);
                if (i != e_constructor.args.size() - 1) {
                    e_str += ",";
                }
            }
            e_str += ")";
        }
    } else if (std::holds_alternative<Projection>(e)) {
        Projection e_proj = std::get<Projection>(e);
        e_str = "proj(";
        e_str += e_proj.c.name;
        e_str += ",";
        e_str += std::to_string(e_proj.arg);
        e_str += ",";
        e_str += build_set_var_str(e_proj.v);
        e_str += ")";
    }
    return e_str;
}

/*
 * Return a string representation of a given Statement.
 */
std::string build_constraint(Statement c) {

    /*
     * This is the constraint string we're eventually going to be returning.
     */
    std::string constraint = "";
    constraint += build_expr_str(c.e1);
    constraint += " <= ";
    constraint += build_expr_str(c.e2);
    constraint += "\n";
    return constraint;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cout << "Usage: ./assn3-constraint-generator <json>" << std::endl;
        exit(EXIT_FAILURE);
    }
    Program p = Program(json::parse(std::ifstream(argv[1])));

    /*
     * Our list of constraints is represented as a vector of Statements.
     */
    std::vector<Statement> constraints;

    /*
     * First things first, let's add the constraints generated by global
     * function pointers.
     */
    std::vector<Statement> global_func_ptr_constraints = get_global_func_ptr_constraints(p);
    constraints.insert(constraints.end(),
                       global_func_ptr_constraints.begin(),
                       global_func_ptr_constraints.end());

    /*
     * We also want to loop over each function in the program.
     */
    for (const auto &func : p.funcs) {
        std::string func_name = func.first;
        for (const auto &bb : func.second->bbs) {
            for (const auto& instruction : bb.second->instructions) {
                switch (instruction->instrType) {
                    case CopyInstrType: {
                        CopyInstruction *copy = (CopyInstruction *) instruction;

                        /*
                         * If the LHS isn't a pointer, ignore it.
                         */
                        if (copy->lhs->type->indirection != 0) {
                            constraints.push_back(get_copy_constraint(*copy, func_name, p));
                        }
                        break;
                    }
                    case AddrofInstrType: {
                        constraints.push_back(get_addrof_constraint(*((AddrofInstruction *) instruction), func_name));
                        break;
                    }
                    case AllocInstrType: {
                        constraints.push_back(get_alloc_constraint(*((AllocInstruction *) instruction), func_name));
                        break;
                    }
                    case GepInstrType: {
                        constraints.push_back(get_gep_constraint(*((GepInstruction *) instruction), func_name));
                        break;
                    }
                    case GfpInstrType: {
                        constraints.push_back(get_gfp_constraint(*((GfpInstruction *) instruction), func_name));
                        break;
                    }
                    case LoadInstrType: {
                        LoadInstruction *load = (LoadInstruction *) instruction;

                        /*
                         * Check that the lhs is a pointer.
                         */
                        if (load->lhs->type->indirection != 0) {
                            constraints.push_back(get_load_constraint(*load, func_name));
                        }
                        break;
                    }
                    case StoreInstrType: {
                        StoreInstruction *store = (StoreInstruction *) instruction;

                        /*
                         * Check that the value being stored is also a pointer.
                         */
                        if (!store->op->IsConstInt()) {
                            if (store->op->var->type->indirection != 0) {
                                constraints.push_back(get_store_constraint(*store, func_name));
                            }
                        }
                        break;
                    }
                    default: {
                        break;
                    }
                } // End of switch-case
            }

            /*
             * Let's not forget about the terminal instruction.
             */
            Instruction *terminal = bb.second->terminal;
            switch (terminal->instrType) {
                case CallDirInstrType: {
                    CallDirInstruction *call_dir = (CallDirInstruction *) terminal;
                    std::vector<Statement> statements = get_call_dir_constraint(*call_dir, func.second, p.funcs[call_dir->callee]);
                    for (const Statement &s : statements) {
                        constraints.push_back(s);
                    }
                    break;
                }
                case CallIdrInstrType: {
                    CallIdrInstruction *call_idr = (CallIdrInstruction *) terminal;
                    constraints.push_back(get_call_idr_constraint(call_idr, func_name));
                    break;
                }
                default: {
                    break;
                }
            } // End of switch-case
        }
    }

    /*
     * Now print out all our constraints.
     */
    std::set<std::string> str_constraints;
    for (const auto &constraint : constraints) {
        str_constraints.insert(build_constraint(constraint));
    }
    for (const auto& constraint_str : str_constraints) {
        std::cout << constraint_str;
    }
}