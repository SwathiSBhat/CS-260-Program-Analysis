#pragma once
#include<string>
#include<vector>
#include<iostream>
#include "json.hpp"
#include<typeinfo>

using json = nlohmann::json;

#define STRUCT "Struct"
#define FUNCTION "Function"
#define STORE "Store"
#define RETURN "Ret"

enum DataType {
    IntType = 0,
    StructType,
    FuncType
};

enum TerminalType {
    Branch = 0,
    Jump,
    Ret,
    CallDirect,
    CallIndirect
};

// TODO: Add a print method for all classes to be able to print the data easily
// TODO: Replace all string based index access with constant names

// Forward declaration of Type class to allow usage in Variable class
class Type;

/*
 * A variable is a name and a type.
*/
class Variable {
    public:
        Variable() {};
        Variable(std::string name, Type *type) : name(name), type(type) {};
        std::string name;
        Type *type;
};

/*
 * A type is either an integer, struct, function , or pointer.
 */
class Type {
    public:
        int indirection;
        void* ptr_type;
        DataType type;

        class FunctionType {
            public:
                FunctionType(json func_type_json) {
                    
                    std::cout << "Function Type" << std::endl;
                    std::cout << func_type_json << std::endl;

                    if (func_type_json["ret_ty"].dump() == "null") {
                        ret = nullptr;
                    }
                    else if (func_type_json["ret_ty"] != nullptr) {
                        ret = new Type(func_type_json["ret_ty"]);
                    }
                    if (func_type_json["param_ty"] != nullptr) {
                        for (auto param : func_type_json["param_ty"].items()) {
                            // std::cout << "Param: " << param.value() << std::endl;
                            params.push_back(new Type(param.value()));
                        }
                    }
                }
                Type *ret;
                std::vector<Type*> params;
        };

        class StructType {
            public:
                StructType(json struct_type_json) {

                    std::cout << "Struct Type" << std::endl;
                    std::cout << struct_type_json << std::endl;

                    name = struct_type_json.dump();
                }
                std::string name;
        };
        
        Type(json type_json) : indirection(0) {
            // TODO : Need to check if any other types are defined this way
            // Only int has direct string coming in type. For example "typ": "Int"
            if (type_json.dump() == "\"Int\"")
            {
                ptr_type = nullptr;
                type = DataType::IntType;
            }
            else if (type_json["Struct"] != nullptr) { // occurs for operand with store instruction
                // This has indirection 0 and ptr_type as struct
                StructType* structPtr = new StructType(type_json["Struct"]);
                ptr_type = structPtr;
                type = DataType::StructType;
            }
            else if (type_json["Pointer"] != nullptr) {
                json ptr_json = type_json["Pointer"];
                indirection++;

                // Update indirection and ptr_type based on nested pointers
                while (ptr_json != nullptr && ptr_json.dump() != "\"Int\"" &&
                    ptr_json.begin().key() == "Pointer") {
                    ptr_json = ptr_json["Pointer"];
                    indirection++;
                }

                if (ptr_json.dump() == "\"Int\"") {
                    // TODO: Change this
                    int *intPtr = 0;
                    ptr_type = intPtr;
                    type = DataType::IntType;
                }
                else if (ptr_json.begin().key() == "Struct") {
                    ptr_type = new StructType(ptr_json["Struct"]);
                    type = DataType::StructType;
                }
                else if (ptr_json.begin().key() == "Function") {
                    ptr_type = new FunctionType(ptr_json["Function"]);
                    type = DataType::FuncType;
                }
                else
                    std::cout << "Error: Pointer type not found" << std::endl;
                std::cout << "Indirection: " << indirection << std::endl;
                std::cout << "Type: " << type << std::endl;
            }
            else std::cout << "Error: Type not found" << std::endl;
        };
};

/*
 * A struct type is a name and a nonempty list of field names and types.
 */
class Struct{
    public:
    Struct(json struct_json) {

        std::cout << "Struct" << std::endl;
        std::cout << struct_json << std::endl;
        
        if (struct_json.begin() != struct_json.end()) {
            name = struct_json.begin().key();
            for (auto &[field_key, field_val] : struct_json.begin().value().items()) {
            fields.push_back(new Variable(field_val["name"], new Type(field_val["typ"])));
        }
        }
    };
    std::string name;
    std::vector<Variable*> fields;
};

/*
 * A global variable is a name and a type.
 */
class Global{
    public:
    Global(json global_json) {

        std::cout << "Global" << std::endl;
        std::cout << global_json << std::endl;

        globalVar = new Variable(global_json["name"], new Type(global_json["typ"]));
    };
    Variable *globalVar;
};

/*
 * An external function declaration is a name and a function type. We know its
 * name/type signature and we can call it, but we don't have access to its
 * source code.
 */
class ExternalFunction {
    public:
    ExternalFunction(json ext_func_json) {

        std::cout << "External Function" << std::endl;
        std::cout << ext_func_json << std::endl;

        name = ext_func_json.begin().key();
        funcType = new Type::FunctionType(ext_func_json.begin().value());
    };
    std::string name;
    Type::FunctionType *funcType;
};

/*
 * Instruction base class
*/
class Instruction{};

class Operand {
    public:
        Operand() {};
        Operand(Variable *var) : var(var), val(0) {};
        Operand(int val) : var(nullptr), val(val) {};
        Variable *var;
        int val;
};

/*
 * These are all instructions.
 */
/*
 * x = $addrof y
 */
class AddrofInstruction : public Instruction{
    public:
        AddrofInstruction(json inst_val) {
            std::cout << "Addrof Instruction" << std::endl;
            std::cout << inst_val << std::endl;
            if (inst_val["lhs"] != nullptr) {
                lhs = new Variable(inst_val["lhs"]["name"], new Type(inst_val["lhs"]["typ"]));
            }
            if (inst_val["rhs"] != nullptr) {
                rhs = new Variable(inst_val["rhs"]["name"], new Type(inst_val["rhs"]["typ"]));
            }
        }
        Variable *lhs;
        Variable *rhs;
};

/*
 * x = $alloc 10 [_a1]
 */
class AllocInstruction : public Instruction{
    public:
        AllocInstruction(json inst_val) {
            std::cout << "Alloc Instruction" << std::endl;
            std::cout << inst_val << std::endl;
            if (inst_val["lhs"] != nullptr) {
                lhs = new Variable(inst_val["lhs"]["name"], new Type(inst_val["lhs"]["typ"]));
            }
            if (inst_val["num"] != nullptr) {
                if (inst_val["num"]["Var"] != nullptr)
                    num = new Operand(new Variable(inst_val["num"]["Var"]["name"], new Type(inst_val["num"]["Var"]["typ"])));
                else if (inst_val["op"]["CInt"] != nullptr)
                    num = new Operand(inst_val["num"]["CInt"]);
            }
            if (inst_val["id"] != nullptr) {
                id = new Variable(inst_val["id"]["name"], new Type(inst_val["id"]["typ"]));
            }
        }
        Variable *lhs;
        Operand *num;
        Variable *id;
};

/*
 * x = $arith add y 2
 */
class ArithInstruction : public Instruction{
    public:
        ArithInstruction(json inst_val) {
            std::cout << "Arith Instruction" << std::endl;
            std::cout << inst_val << std::endl;

            if (inst_val["lhs"] != nullptr) {
                lhs = new Variable(inst_val["lhs"]["name"], new Type(inst_val["lhs"]["typ"]));
            }
            if (inst_val["op1"] != nullptr) {
                if (inst_val["op1"]["Var"] != nullptr)
                    op1 = new Operand(new Variable(inst_val["op1"]["Var"]["name"], new Type(inst_val["op1"]["Var"]["typ"])));
                else if (inst_val["op1"]["CInt"] != nullptr)
                    op1 = new Operand(inst_val["op1"]["CInt"]);
            }
            if (inst_val["op2"] != nullptr) {
                if (inst_val["op2"]["Var"] != nullptr)
                    op2 = new Operand(new Variable(inst_val["op2"]["Var"]["name"], new Type(inst_val["op2"]["Var"]["typ"])));
                else if (inst_val["op2"]["CInt"] != nullptr)
                    op2 = new Operand(inst_val["op2"]["CInt"]);
            }
            if (inst_val["aop"] != nullptr) {
                arith_op = inst_val["aop"].dump();
            }
        }
        Variable *lhs;
        std::string arith_op;
        Operand *op1;
        Operand *op2;
};

/*
 * x = $cmp eq y 2
 */
class CmpInstruction : public Instruction{
    public:
        CmpInstruction(json inst_val) {
            std::cout << "Cmp Instruction" << std::endl;
            std::cout << inst_val << std::endl;
            if (inst_val["lhs"] != nullptr) {
                lhs = new Variable(inst_val["lhs"]["name"], new Type(inst_val["lhs"]["typ"]));
            }
            if (inst_val["op1"] != nullptr) {
                if (inst_val["op1"]["Var"] != nullptr)
                    op1 = new Operand(new Variable(inst_val["op1"]["Var"]["name"], new Type(inst_val["op1"]["Var"]["typ"])));
                else if (inst_val["op1"]["CInt"] != nullptr)
                    op1 = new Operand(inst_val["op1"]["CInt"]);
            }
            if (inst_val["op2"] != nullptr) {
                if (inst_val["op2"]["Var"] != nullptr)
                    op2 = new Operand(new Variable(inst_val["op2"]["Var"]["name"], new Type(inst_val["op2"]["Var"]["typ"])));
                else if (inst_val["op2"]["CInt"] != nullptr)
                    op2 = new Operand(inst_val["op2"]["CInt"]);
            }
            if (inst_val["rop"] != nullptr) {
                cmp_op = inst_val["rop"].dump();
            }
        }
        Variable *lhs;
        std::string cmp_op;
        Operand *op1;
        Operand *op2;
};

/*
 * x = $copy y
 */
class CopyInstruction : public Instruction{
    public:
        CopyInstruction(json inst_val) {
            std::cout << "Copy Instruction" << std::endl;
            std::cout << inst_val << std::endl;
            if (inst_val["lhs"] != nullptr) {
                lhs = new Variable(inst_val["lhs"]["name"], new Type(inst_val["lhs"]["typ"]));
            }
            if (inst_val["op"] != nullptr) {
                if (inst_val["op"]["Var"] != nullptr)
                    op = new Operand(new Variable(inst_val["op"]["Var"]["name"], new Type(inst_val["op"]["Var"]["typ"])));
                else if (inst_val["op"]["CInt"] != nullptr)
                    op = new Operand(inst_val["op"]["CInt"]);
            }
        }
        Variable *lhs;
        Operand *op;
};

/*
 * x = $gep y 10
 */
class GepInstruction : public Instruction{
    public:
        GepInstruction(json inst_val) {
            std::cout << "Gep Instruction" << std::endl;
            std::cout << inst_val << std::endl;
            if (inst_val["lhs"] != nullptr) {
                lhs = new Variable(inst_val["lhs"]["name"], new Type(inst_val["lhs"]["typ"]));
            }
            if (inst_val["src"] != nullptr) {
                src = new Variable(inst_val["src"]["name"], new Type(inst_val["src"]["typ"]));
            }
            if (inst_val["idx"] != nullptr) {
                if (inst_val["idx"]["Var"] != nullptr)
                    idx = new Operand(new Variable(inst_val["idx"]["Var"]["name"], new Type(inst_val["idx"]["Var"]["typ"])));
                else if (inst_val["idx"]["CInt"] != nullptr)
                    idx = new Operand(inst_val["idx"]["CInt"]);
            }
        }
        Variable *lhs;
        Variable *src;
        Operand *idx;
};

/*
 * x = $gfp y foo
 */
class GfpInstruction : public Instruction{
    public:
        GfpInstruction(json inst_val) {
            std::cout << "Gfp Instruction" << std::endl;
            std::cout << inst_val << std::endl;
            if (inst_val["lhs"] != nullptr) {
                lhs = new Variable(inst_val["lhs"]["name"], new Type(inst_val["lhs"]["typ"]));
            }
            if (inst_val["src"] != nullptr) {
                src = new Variable(inst_val["src"]["name"], new Type(inst_val["src"]["typ"]));
            }
            if (inst_val["field"] != nullptr) {
                field = new Variable(inst_val["field"]["name"], new Type(inst_val["field"]["typ"]));
            }
        }
        Variable *lhs;
        Variable *src;
        Variable *field;
};

/*
 * x = $load y
 */
class LoadInstruction : public Instruction{
    public:
        LoadInstruction(json inst_val) {
            std::cout << "Load Instruction" << std::endl;
            std::cout << inst_val << std::endl;
            if (inst_val["lhs"] != nullptr) {
                lhs = new Variable(inst_val["lhs"]["name"], new Type(inst_val["lhs"]["typ"]));
            }
            if (inst_val["src"] != nullptr) {
                src = new Variable(inst_val["src"]["name"], new Type(inst_val["src"]["typ"]));
            }
        }
        Variable *lhs;
        Variable *src;
};

/*
 * store x y
 */
class StoreInstruction : public Instruction {
    public:
        StoreInstruction(json inst_val) {
            std::cout << "Store Instruction" << std::endl;
            std::cout << inst_val << std::endl;
            if (inst_val["dst"] != nullptr) {
                dst = new Variable(inst_val["dst"]["name"], new Type(inst_val["dst"]["typ"]));
            }
            if (inst_val["op"] != nullptr) {
                    if (inst_val["op"]["Var"] != nullptr)
                        op = new Operand(new Variable(inst_val["op"]["Var"]["name"], new Type(inst_val["op"]["Var"]["typ"])));
                    else if (inst_val["op"]["CInt"] != nullptr)
                        op = new Operand(inst_val["op"]["CInt"]);
            }
        }
        Variable *dst;
        Operand *op;
};

/*
 * x = $call_ext foo(10)
 */
class CallExtInstruction : public Instruction{
    public:
        CallExtInstruction(json inst_val) {
            std::cout << "CallExt Instruction" << std::endl;
            std::cout << inst_val << std::endl;
            // lhs can be optional
            if (inst_val["lhs"].dump() == "null" || inst_val["lhs"] == nullptr)
                lhs = nullptr;
            else if (inst_val["lhs"] != nullptr) {
                lhs = new Variable(inst_val["lhs"]["name"], new Type(inst_val["lhs"]["typ"]));
            }
            if (inst_val["ext_callee"] != nullptr) {
                extFuncName = inst_val["ext_callee"].dump();
            }
            if (inst_val["args"] != nullptr) {
                for (auto &[arg_key, arg_val] : inst_val["args"].items()) {
                    if (arg_val["Var"] != nullptr)
                        args.push_back(new Operand(new Variable(arg_val["Var"]["name"], new Type(arg_val["Var"]["typ"]))));
                    else if (arg_val["CInt"] != nullptr)
                        args.push_back(new Operand(arg_val["CInt"]));
                }
            }
        }
        Variable *lhs;
        std::string extFuncName;
        std::vector<Operand*> args;
};

/*
 * These instructions are all terminals.
 */

/*
 * $branch x bb1 bb2
 */
class BranchInstruction : public Instruction{
    public:
        BranchInstruction(json inst_val) {
            std::cout << "Branch Instruction" << std::endl;
            std::cout << inst_val << std::endl;
            if (inst_val["cond"] != nullptr) {
                if (inst_val["cond"]["Var"] != nullptr)
                    condition = new Operand(new Variable(inst_val["cond"]["Var"]["name"], new Type(inst_val["cond"]["Var"]["typ"])));
                else if (inst_val["cond"]["CInt"] != nullptr)
                    condition = new Operand(inst_val["cond"]["CInt"]);
            }
            if (inst_val["tt"] != nullptr) {
                tt = inst_val["tt"].dump();
            }
            if (inst_val["ff"] != nullptr) {
                ff = inst_val["ff"].dump();
            }
        };

        Operand *condition;
        std::string tt;
        std::string ff;
};

/*
 * $jump bb1
 */
class JumpInstruction : public Instruction{
    public:
    JumpInstruction(json inst_val) : label(inst_val.dump()) {
        std::cout << "Jump Instruction" << std::endl;
        std::cout << inst_val << std::endl;
    };
    std::string label;
};

/*
 * $ret x - x can be a constant or a variable or null
 */
class RetInstruction : public Instruction{
    public:
        RetInstruction(json inst_val) {
            std::cout << "Ret Instruction" << std::endl;
            std::cout << inst_val << std::endl;
            // When return instruction is null, it means it is a void return
            if (inst_val.dump() == "null" || inst_val == nullptr)
                op = nullptr;
            else if (inst_val["Var"] != nullptr) {
                op = new Operand(new Variable(inst_val["Var"]["name"], new Type(inst_val["Var"]["typ"])));
            }
            else if (inst_val["CInt"] != nullptr)
                op = new Operand(inst_val["CInt"]);
        }
        Operand *op;
};

/*
 * x = $call_dir foo(10) then bb1
 */
class CallDirInstruction : public Instruction{
    public:
        CallDirInstruction(json inst_val) {
            std::cout << "CallDir Instruction" << std::endl;
            std::cout << inst_val << std::endl;
            // lhs can be optional
            if (inst_val["lhs"].dump() == "null" || inst_val["lhs"] == nullptr)
                lhs = nullptr;
            else if (inst_val["lhs"] != nullptr) {
                lhs = new Variable(inst_val["lhs"]["name"], new Type(inst_val["lhs"]["typ"]));
            }
            if (inst_val["callee"] != nullptr) {
                callee = inst_val["callee"].dump();
            }
            if (inst_val["args"] != nullptr) {
                for (auto &[arg_key, arg_val] : inst_val["args"].items()) {
                    if (arg_val["Var"] != nullptr)
                        args.push_back(new Operand(new Variable(arg_val["Var"]["name"], new Type(arg_val["Var"]["typ"]))));
                    else if (arg_val["CInt"] != nullptr)
                        args.push_back(new Operand(arg_val["CInt"]));
                }
            }
            if (inst_val["next_bb"] != nullptr) {
                next_bb = inst_val["next_bb"].dump();
            }
        }
        Variable *lhs;
        std::string callee;
        std::vector<Operand*> args;
        std::string next_bb;
};

/*
 * x = $call_idr fp(10) then bb1
 */
class CallIdrInstruction : public Instruction{
    public:
        CallIdrInstruction(json inst_val) {
            std::cout << "CallIdr Instruction" << std::endl;
            std::cout << inst_val << std::endl;
            // lhs can be optional
            if (inst_val["lhs"].dump() == "null" || inst_val["lhs"] == nullptr)
                lhs = nullptr;
            else if (inst_val["lhs"] != nullptr) {
                lhs = new Variable(inst_val["lhs"]["name"], new Type(inst_val["lhs"]["typ"]));
            }
            if (inst_val["fp"] != nullptr) {
                fp = new Variable(inst_val["fp"]["name"], new Type(inst_val["fp"]["typ"]));
            }
            if (inst_val["args"] != nullptr) {
                for (auto &[arg_key, arg_val] : inst_val["args"].items()) {
                    if (arg_val["Var"] != nullptr)
                        args.push_back(new Operand(new Variable(arg_val["Var"]["name"], new Type(arg_val["Var"]["typ"]))));
                    else if (arg_val["CInt"] != nullptr)
                        args.push_back(new Operand(arg_val["CInt"]));
                }
            }
            if (inst_val["next_bb"] != nullptr) {
                next_bb = inst_val["next_bb"].dump();
            }
        }
        Variable *lhs;
        Variable *fp;
        std::vector<Operand*> args;
        std::string next_bb;
};

/*
 * A basic block is a label and an ordered list of instructions, ending in a
 * terminal.
 */
class BasicBlock {
    public:
    std::string label;
    std::vector<Instruction*> instructions;
    Instruction* terminal;
    BasicBlock(json bb_json) : label(bb_json["id"]){
        for (auto &[inst_key, inst_val] : bb_json["insts"].items()) {
            // Store each instruction inside basic block structure based on instruction type
            for (auto i = inst_val.items().begin(); i != inst_val.items().end(); ++i) {
                if (i.key() == "Store") {
                    StoreInstruction *store_inst = new StoreInstruction(i.value());
                    instructions.push_back(store_inst);
                }
                else if (i.key() == "AddrOf") {
                    AddrofInstruction *addrof_inst = new AddrofInstruction(i.value());
                    instructions.push_back(addrof_inst);
                }
                else if (i.key() == "Load") {
                    LoadInstruction *load_inst = new LoadInstruction(i.value());
                    instructions.push_back(load_inst);
                }
                else if (i.key() == "Alloc") {
                    AllocInstruction *alloc_inst = new AllocInstruction(i.value());
                    instructions.push_back(alloc_inst);
                }
                else if (i.key() == "Arith") {
                    ArithInstruction *arith_inst = new ArithInstruction(i.value());
                    instructions.push_back(arith_inst);
                }
                else if (i.key() == "Cmp") {
                    CmpInstruction *cmp_inst = new CmpInstruction(i.value());
                    instructions.push_back(cmp_inst);
                }
                else if (i.key() == "Copy") {
                    CopyInstruction *copy_inst = new CopyInstruction(i.value());
                    instructions.push_back(copy_inst);
                }
                else if (i.key() == "Gep") {
                    GepInstruction *gep_inst = new GepInstruction(i.value());
                    instructions.push_back(gep_inst);
                }
                else if (i.key() == "Gfp") {
                    GfpInstruction *gfp_inst = new GfpInstruction(i.value());
                    instructions.push_back(gfp_inst);
                }
                else if (i.key() == "CallExt") {
                    CallExtInstruction *call_ext_inst = new CallExtInstruction(i.value());
                    instructions.push_back(call_ext_inst);
                }
            }            
        }
        // Parse terminal instruction
        if (bb_json["term"] != nullptr)
        {
            std::string term_type = bb_json["term"].begin().key();
            std::cout << "Terminal type: " << term_type << std::endl;
            if(term_type == "Branch"){
                BranchInstruction *branch_inst = new BranchInstruction(bb_json["term"]["Branch"]);
                terminal = branch_inst;
            }
            else if(term_type == "Jump"){
                JumpInstruction *jump_inst = new JumpInstruction(bb_json["term"]["Jump"]);
                terminal = jump_inst;
            }
            else if(term_type == "Ret"){
                RetInstruction *ret_inst = new RetInstruction(bb_json["term"]["Ret"]);
                terminal = ret_inst;
            }
            else if(term_type == "CallDirect"){
                CallDirInstruction *call_dir_inst = new CallDirInstruction(bb_json["term"]["CallDirect"]);
                terminal = call_dir_inst;
            }
            else if(term_type == "CallIndirect"){
                CallIdrInstruction *call_idr_inst = new CallIdrInstruction(bb_json["term"]["CallIndirect"]);
                terminal = call_idr_inst;
            }
        }
    }
};

/*
 * A function definition is:
 * - A name
 * - An ordered list of parameters (with names and types)
 * - An optional return type
 * - A set of local variables (with names and types)
 * - A set of basic blocks
 */
// TODO: It is better to store the basic blocks in a map with key as label of basic block and value as basic block contents
class Function {
    public:
    Function(json func_json) {

        std::cout << "Function" << std::endl;
        std::cout << func_json << std::endl;

        if (func_json["id"] != nullptr) {
            name = func_json["id"].dump();
        }
        
        if (func_json["params"] != nullptr) {
            for (auto &[param_key, param_val] : func_json["params"].items()) {
                params.push_back(new Variable(param_val["name"], new Type(param_val["typ"])));
            }
        }
        
        if (func_json["ret_ty"].dump() == "null" || func_json["ret_ty"] == nullptr)
            ret = nullptr;
        else if (func_json["ret_ty"] != nullptr) {
            ret = new Type(func_json["ret_ty"]);
        }

        if (func_json["locals"] != nullptr) {
            for (auto &[local_key, local_val] : func_json["locals"].items()) {
                locals.push_back(new Variable(local_val["name"], new Type(local_val["typ"])));
            }
        }
        if (func_json["body"] != nullptr) {
            for (auto &[bb_key, bb_val] : func_json["body"].items()) {
                bbs.push_back(new BasicBlock(bb_val));
            }
        }
    }
    std::string name;
    std::vector<Variable*> params;
    std::vector<Variable*> locals;
    std::vector<BasicBlock*> bbs;
    Type *ret;
};

/*
 * A program is a set of structs, global variables, function definitions, and
 * external function declarations.
 */
class Program {
    public:
        Program(json program_json) {
            std::cout << "Program" << std::endl;
            
            if (program_json["structs"] != nullptr) {
                for (auto st : program_json["structs"].items()) {
                    structs.push_back(new Struct(st));
                }
            }
            if (program_json["globals"] != nullptr) {
                for (auto &[global_key, global_val] : program_json["globals"].items()) {
                    globals.push_back(new Global(global_val));
                }
            }
            if (program_json["functions"] != nullptr) {
                for (auto &[func_key, func_val] : program_json["functions"].items()) {
                    funcs.push_back(new Function(func_val));
                }
            }
            if (program_json["externs"] != nullptr) {
                for (auto &[ext_func_key, ext_func_val] : program_json["externs"].items()) {
                    ext_funcs.push_back(new ExternalFunction(ext_func_val));
                }
            }
        };
        std::vector<Struct*> structs;
        std::vector<Global*> globals;
        std::vector<Function*> funcs;
        std::vector<ExternalFunction*> ext_funcs;
};