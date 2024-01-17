#pragma once
#include<string>
#include<vector>
#include<iostream>
#include "json.hpp"
#include<typeinfo>

using json = nlohmann::json;

enum DataType {
    IntType = 0,
    StructType,
    FuncType
};
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
        // TODO : Need to check if any other types are defined this way
        // Only int has direct string coming in type. For example "typ": "Int"
        Type(std::string intType) : indirection(0),ptr_type(nullptr), type(DataType::IntType){};
        /*
         *  TODO: Need to add logic for nested pointers here
        */
        Type(json type_json) : indirection(0) {
            std::cout << "Type" << std::endl;
            std::cout << type_json << std::endl;
            if (type_json.dump() == "\"Int\"")
            {
                ptr_type = nullptr;
                type = DataType::IntType;
            }
            else if (type_json["Int"] != nullptr) {
                int* intPtr = new int(type_json["Int"]);
                ptr_type = intPtr;
                type = DataType::IntType;
            } else if (type_json["Struct"] != nullptr) {
                Struct* structPtr = new Struct(type_json["Struct"]);
                ptr_type = structPtr;
                type = DataType::StructType;
            } else if (type_json["Func"] != nullptr) {
                Function* funcPtr = new Function(type_json["Func"]);
                ptr_type = funcPtr;
                type = DataType::FuncType;
            } 
            // TODO: Need to populate correct type. For now, defaulting to int
            else if (type_json["Pointer"] != nullptr) {
                indirection++;
                int* typePtr = 0;
                ptr_type = typePtr;
                type = DataType::IntType;
            }
            else std::cout << "Error: Type not found" << std::endl;
        };
};

/*
 * A global variable is a name and a type.
 */
class Global;

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

/*
 * A basic block is a label and an ordered list of instructions, ending in a
 * terminal.
 */
class BasicBlock {
    std::string label;
    std::vector<Instruction> instructions;
};

class Operand {
    public:
        Operand() {};
        Operand(Variable *var) : var(var), val(0) {
            //std::cout << "Operand" << std::endl;std::cout << var->name << std::endl;
        };
        Operand(int val) : var(nullptr), val(val) {
            //std::cout << "Operand CInt" << std::endl;std::cout << val << std::endl;
        };
    Variable *var;
    int val;
};

/*
 * These are all instructions.
 */
/*
 * x = $addrof y
 */
class AddrofInstruction : Instruction{};

/*
 * x = $alloc 10 [_a1]
 */
class AllocInstruction : Instruction{};

/*
 * x = $arith add y 2
 */
class ArithInstruction : Instruction{};

/*
 * x = $cmp eq y 2
 */
class CmpInstruction : Instruction{};

/*
 * x = $copy y
 */
class CopyInstruction : Instruction{};

/*
 * x = $gep y 10
 */
class GepInstruction : Instruction{};

/*
 * x = $gfp y foo
 */
class GfpInstruction : Instruction{};

/*
 * x = $load y
 */
class LoadInstruction : Instruction{};

/*
 * store x y
 */
class StoreInstruction : Instruction {
    public:
        StoreInstruction(json inst_val) {
            std::cout << "Store Instruction" << std::endl;
            std::cout << inst_val << std::endl;
            if (inst_val["dst"] != nullptr) {
                dst = new Variable(inst_val["dst"]["name"], new Type(inst_val["dst"]["typ"]));
            }
            if (inst_val["op"] != nullptr) {
                    //std::cout << "op" << std::endl;
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
class CallExtInstruction : Instruction{};

/*
 * These instructions are all terminals.
 */

/*
 * $branch x bb1 bb2
 */
class BranchInstruction : Instruction{};

/*
 * $jump bb1
 */
class JumpInstruction : Instruction{};

/*
 * $ret x
 */
class RetInstruction : Instruction{};

/*
 * x = $call_dir foo(10) then bb1
 */
class CallDirInstruction : Instruction{};

/*
 * x = $call_idr fp(10) then bb1
 */
class CallIdrInstruction : Instruction{};