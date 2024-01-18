#pragma once
#include<string>
#include<vector>
#include<iostream>
#include "json.hpp"
#include<typeinfo>

using json = nlohmann::json;

#define STRUCT "Struct"
#define FUNCTION "Function"

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

/*
 * A program is a set of structs, global variables, function definitions, and
 * external function declarations.
 */
class Program;

class Type;

/*
 * A variable is a name and a type.
*/
class Variable {
    public:
        Variable() {};
        Variable(std::string name, Type *type) : name(name), type(type) {
            //std::cout << "Variable" << std::endl;std::cout << name << std::endl;
        };
        std::string name;
        Type *type;
};

/*
 * A struct type is a name and a nonempty list of field names and types.
 */
class Struct{
    public:
    Struct(json struct_json) {

    };
};

/*
 * A function definition is:
 * - A name
 * - An ordered list of parameters (with names and types)
 * - An optional return type
 * - A set of local variables (with names and types)
 * - A set of basic blocks
 */
class Function {
    public:
    Function(json func_json) {
    }
};

/*
 * A type is either an integer, struct, function , or pointer.
 */
class Type {
    public:
        int indirection;
        void* ptr_type;
        DataType type;
        
        Type(json type_json) : indirection(0) {
            std::cout << "Type" << std::endl;
            std::cout << type_json << std::endl;
            // TODO : Need to check if any other types are defined this way
            // Only int has direct string coming in type. For example "typ": "Int"
            if (type_json.dump() == "\"Int\"")
            {
                ptr_type = nullptr;
                type = DataType::IntType;
            }
            /*else if (type_json["Int"] != nullptr) {
                int* intPtr = new int(type_json["Int"]);
                ptr_type = intPtr;
                type = DataType::IntType;
            }*/ else if (type_json["Struct"] != nullptr) { // occurs for operand with store instruction
                Struct* structPtr = new Struct(type_json["Struct"]);
                ptr_type = structPtr;
                type = DataType::StructType;
            } /*else if (type_json["Func"] != nullptr) {
                Function* funcPtr = new Function(type_json["Function"]);
                ptr_type = funcPtr;
                type = DataType::FuncType;
            } */

            else if (type_json["Pointer"] != nullptr) {
                json ptr_json = type_json["Pointer"];
                indirection++;

                // Update indirection and ptr_type based on nested pointers
                while (ptr_json != nullptr && ptr_json.dump() != "\"Int\"" &&
                    ptr_json.begin().key() == "Pointer") {
                    ptr_json = ptr_json["Pointer"];
                    indirection++;
                }

                int* typePtr = 0;
                ptr_type = typePtr;

                if (ptr_json.dump() == "\"Int\"")
                    type = DataType::IntType;
                else if (ptr_json.begin().key() == "Struct")
                    type = DataType::StructType;
                else if (ptr_json.begin().key() == "Function")
                    type = DataType::FuncType;
                else
                    std::cout << "Error: Pointer type not found" << std::endl;
                std::cout << "Indirection: " << indirection << std::endl;
                std::cout << "Type: " << type << std::endl;
            }
            else std::cout << "Error: Type not found" << std::endl;
        };
};

/*
 * A global variable is a name and a type.
 */
class Global{

};

/*
 * An external function declaration is a name and a function type. We know its
 * name/type signature and we can call it, but we don't have access to its
 * source code.
 */
class ExternalFunction;

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
class AddrofInstruction : public Instruction{};

/*
 * x = $alloc 10 [_a1]
 */
class AllocInstruction : public Instruction{};

/*
 * x = $arith add y 2
 */
class ArithInstruction : public Instruction{};

/*
 * x = $cmp eq y 2
 */
class CmpInstruction : public Instruction{};

/*
 * x = $copy y
 */
class CopyInstruction : public Instruction{};

/*
 * x = $gep y 10
 */
class GepInstruction : public Instruction{};

/*
 * x = $gfp y foo
 */
class GfpInstruction : public Instruction{};

/*
 * x = $load y
 */
class LoadInstruction : public Instruction{};

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
class CallExtInstruction : public Instruction{};

/*
 * These instructions are all terminals.
 */

/*
 * $branch x bb1 bb2
 */
class BranchInstruction : public Instruction{};

/*
 * $jump bb1
 */
class JumpInstruction : public Instruction{};

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
class CallDirInstruction : public Instruction{};

/*
 * x = $call_idr fp(10) then bb1
 */
class CallIdrInstruction : public Instruction{};

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
                //std::cout << i.key() << " " << i.value() << std::endl;
                if (i.key() == "Store") {
                    StoreInstruction *store_inst = new StoreInstruction(i.value());
                    instructions.push_back(store_inst);
                }
            }            
        }
        // Parse terminal instruction
        if (bb_json["term"] != nullptr)
        {
            std::string term_type = bb_json["term"].begin().key();
            std::cout << "Terminal type: " << term_type << std::endl;
            if(term_type == "Branch"){
                BranchInstruction *branch_inst = new BranchInstruction();
                terminal = branch_inst;
            }
            else if(term_type == "Jump"){
                JumpInstruction *jump_inst = new JumpInstruction();
                terminal = jump_inst;
            }
            else if(term_type == "Ret"){
                RetInstruction *ret_inst = new RetInstruction(bb_json["term"]["Ret"]);
                terminal = ret_inst;
            }
            else if(term_type == "CallDirect"){
                CallDirInstruction *call_dir_inst = new CallDirInstruction();
                terminal = call_dir_inst;
            }
            else if(term_type == "CallIndirect"){
                CallIdrInstruction *call_idr_inst = new CallIdrInstruction();
                terminal = call_idr_inst;
            }
        }
    }
};