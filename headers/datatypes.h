#pragma once
#include<string>
#include<vector>
#include<iostream>
#include "json.hpp"
#include<typeinfo>
#include <unordered_set>

using json = nlohmann::json;

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

enum InstructionType {
    AddrofInstrType = 0,
    AllocInstrType,
    ArithInstrType,
    CmpInstrType,
    CopyInstrType,
    GepInstrType,
    GfpInstrType,
    LoadInstrType,
    StoreInstrType,
    CallExtInstrType,
    CallIdrInstrType,
    CallDirInstrType,
    BranchInstrType,
    JumpInstrType,
    RetInstrType
};

// TODO: Replace all string based index access with constant names

// Forward declaration of Type class to allow usage in Variable class
class Type;

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
                    
                    //std::cout << "Function Type" << std::endl;
                    //std::cout << func_type_json << std::endl;

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

                void pretty_print() {
                    std::cout << "******************* Function Type *******************" << std::endl;
                    if (ret != nullptr) {
                        std::cout << "Return type: " << std::endl;
                        ret->pretty_print();
                    }
                    else {
                        std::cout << "Return type: " << std::endl;
                        std::cout << "Void" << std::endl;
                    }
                    std::cout << "Parameter types: " << std::endl;
                    if (params.size() == 0)
                        std::cout << "None" << std::endl;
                    else 
                        for (auto param : params) {
                            param->pretty_print();
                        }
                    std::cout << "******************* End of Function Type *******************" << std::endl;
                }

                Type *ret;
                std::vector<Type*> params;
        };

        class StructType {
            public:
                StructType(json struct_type_json) {
                    name = struct_type_json;
                }
                std::string name;
        };

        Type() {};
        
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
            }
            else std::cout << "Error: Type not found" << std::endl;
        };

        static bool isEqualType(Type* type1, Type *type2)
        {
            if (type1->type != type2->type)
                return false;
            else if (type1->type == DataType::IntType && 
            type1->indirection == type2->indirection && 
            type1->ptr_type == type2->ptr_type)
                return true; 
            else if (type1->type == DataType::FuncType && type1->indirection == type2->indirection)
            {
                FunctionType *f1 = (FunctionType*)type1->ptr_type;
                FunctionType *f2 = (FunctionType*)type2->ptr_type;

                if (f1->ret->type == f2->ret->type && f1->ret->indirection == f2->ret->indirection)
                {
                    if (f1->ret->type == DataType::StructType)
                    {
                        if (((StructType*)f1->ret->ptr_type)->name != ((StructType*)f2->ret->ptr_type)->name)
                        {
                            return false;
                        }
                    }

                    if (f1->params.size() == f2->params.size())
                    {
                        bool areParamsEqual = true;
                        for (int i = 0; i < f1->params.size(); i++)
                        {
                            if (!isEqualType((f1->params)[i], (f2->params)[i]))
                            {
                                areParamsEqual = false;
                                break;
                            }
                        }
                        if (areParamsEqual) {
                            return true;
                        }
                    }
                }
            }
            else if (type1->type == DataType::StructType && type1->indirection == type2->indirection)
            {
                if (((StructType*)type1->ptr_type)->name != ((StructType*)type2->ptr_type)->name)
                {
                    return false;
                }
                return true;
            }
            return false;
        }

        void pretty_print()
        {
            std::cout << "******************* Type *******************" << std::endl;
            std::cout << "Indirection level:          " << indirection << std::endl;
            switch (type){
                case DataType::IntType:
                    std::cout << "Type:         Int" << std::endl;
                    break;
                case DataType::StructType:
                    std::cout << "Type:         Struct" << std::endl;
                    std::cout << "Struct name:  " << ((StructType*)ptr_type)->name << std::endl;
                    break;
                case DataType::FuncType: {
                    std::cout << "Type:         Function" << std::endl;
                    FunctionType *funcPtr = (FunctionType*)ptr_type;
                    funcPtr->pretty_print();
                    break;
                }
                default:
                    std::cout << "Type:         Unknown" << std::endl;
                    break;
            }
            std::cout << "******************* End of Type *******************" << std::endl;
        
        }
};

/*
 * A variable is a name and a type.
*/
class Variable {
    public:
        Variable() {};
        Variable(std::string name, Type *type) : name(name), type(type) {};
        std::string name;
        Type *type;

        bool isIntType() {
            return (type->indirection == 0 && type->type == DataType::IntType);
        }

        void pretty_print() {
            std::cout << "******************* Variable *******************" << std::endl;
            std::cout << "Name: " << name << std::endl;
            std::cout << "Type: " << std::endl;
            type->pretty_print();
            std::cout << "******************* End of Variable *******************" << std::endl;
        
        }
};

/*
 * A struct type is a name and a nonempty list of field names and types.
 */
class Struct{
    public:
    Struct(json struct_json) {

        // std::cout << "Struct" << std::endl;
        // std::cout << struct_json << std::endl;
        
        /*if (struct_json.begin() != struct_json.end()) {
            name = struct_json.begin().key();
            for (auto &[field_key, field_val] : struct_json.begin().value().items()) {
            fields.push_back(new Variable(field_val["name"], new Type(field_val["typ"])));
        }
        }*/

        for(const auto &field: struct_json.items())
        {
            fields.push_back(new Variable(field.value()["name"], new Type(field.value()["typ"])));
        }
    };

    void pretty_print() {
        std::cout << "******************* Struct *******************" << std::endl;
        std::cout << "Name: " << name << std::endl;
        std::cout << "Fields: " << std::endl;
        for (auto field : fields) {
            field->pretty_print();
        }
        std::cout << "******************* End of Struct *******************" << std::endl;
    }

    std::string name;
    std::vector<Variable*> fields;
};

/*
 * A global variable is a name and a type.
 */
class Global{
    public:
    Global(json global_json) {
        globalVar = new Variable(global_json["name"], new Type(global_json["typ"]));
    };

    void pretty_print() {
        std::cout << "******************* Global *******************" << std::endl;
        std::cout << "Name: " << globalVar->name << std::endl;
        std::cout << "Type: " << std::endl;
        globalVar->type->pretty_print();
        std::cout << "******************* End of Global *******************" << std::endl;
    }

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
        name = ext_func_json.begin().key();
        funcType = new Type::FunctionType(ext_func_json.begin().value());
    };

    void pretty_print() {
        std::cout << "******************* External Function *******************" << std::endl;
        std::cout << "Name: " << name << std::endl;
        std::cout << "Type: " << std::endl;
        funcType->pretty_print();
        std::cout << "******************* End of External Function *******************" << std::endl;
    }

    std::string name;
    Type::FunctionType *funcType;
};

/*
 * Instruction base class
*/
class Instruction{
    public:
        virtual void pretty_print() {};
        virtual std::string ToString() { return ""; };
        InstructionType instrType;
};

class Operand {
    public:
        Operand() {};
        Operand(Variable *var) : var(var), val(0) {};
        Operand(int val) : var(nullptr), val(val) {};

        /*
         * Returns true if the operand is a constant integer and false otherwise.
        */
        bool IsConstInt() {
            return (var == nullptr);
        }

        void pretty_print() {
            std::cout << "******************* Operand *******************" << std::endl;
            if (var != nullptr) {
                std::cout << "Variable: " << std::endl;
                var->pretty_print();
            }
            else {
                std::cout << "Value: " << val << std::endl;
            }
            std::cout << "******************* End of Operand *******************" << std::endl;
        }

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
            
            if (inst_val["lhs"] != nullptr) {
                lhs = new Variable(inst_val["lhs"]["name"], new Type(inst_val["lhs"]["typ"]));
            }
            if (inst_val["rhs"] != nullptr) {
                rhs = new Variable(inst_val["rhs"]["name"], new Type(inst_val["rhs"]["typ"]));
            }
        }

        std::string ToString() {
            return lhs->name + " = $addrof " + rhs->name;
        }

        void pretty_print() {
            std::cout << "******************* Addrof Instruction *******************" << std::endl;
            std::cout << "Instruction type: " << instrType << std::endl;
            std::cout << "LHS: " << std::endl;
            lhs->pretty_print();
            std::cout << "RHS: " << std::endl;
            rhs->pretty_print();
            std::cout << "******************* End of Addrof Instruction *******************" << std::endl;
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
            // std::cout << "Alloc Instruction" << std::endl;
            // std::cout << inst_val << std::endl;
            
            if (inst_val["lhs"] != nullptr) {
                lhs = new Variable(inst_val["lhs"]["name"], new Type(inst_val["lhs"]["typ"]));
            }
            if (inst_val["num"] != nullptr) {
                if (inst_val["num"]["Var"] != nullptr)
                    num = new Operand(new Variable(inst_val["num"]["Var"]["name"], new Type(inst_val["num"]["Var"]["typ"])));
                else if (inst_val["num"]["CInt"] != nullptr)
                    num = new Operand(inst_val["num"]["CInt"]);
            }
            if (inst_val["id"] != nullptr) {
                id = new Variable(inst_val["id"]["name"], new Type(inst_val["id"]["typ"]));
            }
        }

        std::string ToString() {
            std::string num_str = (num->var != nullptr) ? num->var->name : std::to_string(num->val);
            return lhs->name + " = $alloc " + num_str + " " + "[" + id->name + "]";
        }

        void pretty_print() {
            std::cout << "******************* Alloc Instruction *******************" << std::endl;
            std::cout << "Instruction type: " << instrType << std::endl;
            std::cout << "LHS: " << std::endl;
            lhs->pretty_print();
            std::cout << "Num: " << std::endl;
            num->pretty_print();
            std::cout << "ID: " << std::endl;
            id->pretty_print();
            std::cout << "******************* End of Alloc Instruction *******************" << std::endl;
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
            // std::cout << "Arith Instruction" << std::endl;
            // std::cout << inst_val << std::endl;

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
                arith_op = inst_val["aop"];
            }
        }

        std::string ToString() {
            std::string op1_str = (op1->var != nullptr) ? op1->var->name : std::to_string(op1->val);
            std::string op2_str = (op2->var != nullptr) ? op2->var->name : std::to_string(op2->val);
            std::string aop_str = "";
            if (arith_op == "Add")
                aop_str = "add";
            else if (arith_op == "Subtract")
                aop_str = "sub";
            else if (arith_op == "Multiply")
                aop_str = "mul";
            else if (arith_op == "Divide")
                aop_str = "div";
            return lhs->name + " = $arith " + aop_str + " " + op1_str + " " + op2_str;
        }

        void pretty_print() {
            std::cout << "******************* Arith Instruction *******************" << std::endl;
            std::cout << "Instruction type: " << instrType << std::endl;
            std::cout << "LHS: " << std::endl;
            lhs->pretty_print();
            std::cout << "Operand 1: " << std::endl;
            op1->pretty_print();
            std::cout << "Operand 2: " << std::endl;
            op2->pretty_print();
            std::cout << "Arithmetic Operation: " << arith_op << std::endl;
            std::cout << "******************* End of Arith Instruction *******************" << std::endl;
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
            // std::cout << "Cmp Instruction" << std::endl;
            // std::cout << inst_val << std::endl;
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
                cmp_op = inst_val["rop"];
            }
        }

        std::string ToString() {
            std::string op1_str = (op1->var != nullptr) ? op1->var->name : std::to_string(op1->val);
            std::string op2_str = (op2->var != nullptr) ? op2->var->name : std::to_string(op2->val);
            std::string rop_str = "";
            if (cmp_op == "Eq")
                rop_str = "eq";
            else if (cmp_op == "Neq")
                rop_str = "neq";
            else if (cmp_op == "Less")
                rop_str = "lt";
            else if (cmp_op == "LessEq")
                rop_str = "lte";
            else if (cmp_op == "Greater")
                rop_str = "gt";
            else if (cmp_op == "GreaterEq")
                rop_str = "gte";
            return lhs->name + " = $cmp " + rop_str + " " + op1_str + " " + op2_str;
        }

        void pretty_print() {
            std::cout << "******************* Cmp Instruction *******************" << std::endl;
            std::cout << "Instruction type: " << instrType << std::endl;
            std::cout << "LHS: " << std::endl;
            lhs->pretty_print();
            std::cout << "Operand 1: " << std::endl;
            op1->pretty_print();
            std::cout << "Operand 2: " << std::endl;
            op2->pretty_print();
            std::cout << "Comparison Operation: " << cmp_op << std::endl;
            std::cout << "******************* End of Cmp Instruction *******************" << std::endl;
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
            // std::cout << "Copy Instruction" << std::endl;
            // std::cout << inst_val << std::endl;
            
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

        std::string ToString() {
            std::string op_str = (op->var != nullptr) ? op->var->name : std::to_string(op->val);
            return lhs->name + " = $copy " + op_str;
        }

        void pretty_print() {
            std::cout << "******************* Copy Instruction *******************" << std::endl;
            std::cout << "Instruction type: " << instrType << std::endl;
            std::cout << "LHS: " << std::endl;
            lhs->pretty_print();
            std::cout << "Operand: " << std::endl;
            op->pretty_print();
            std::cout << "******************* End of Copy Instruction *******************" << std::endl;
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
            //std::cout << "Gep Instruction" << std::endl;
            // std::cout << inst_val << std::endl;
            
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

        std::string ToString() {
            std::string idx_str = (idx->var != nullptr) ? idx->var->name : std::to_string(idx->val);
            return lhs->name + " = $gep " + src->name + " " + idx_str;
        }

        void pretty_print() {
            std::cout << "******************* Gep Instruction *******************" << std::endl;
            std::cout << "Instruction type: " << instrType << std::endl;
            std::cout << "LHS: " << std::endl;
            lhs->pretty_print();
            std::cout << "Source: " << std::endl;
            src->pretty_print();
            std::cout << "Index: " << std::endl;
            idx->pretty_print();
            std::cout << "******************* End of Gep Instruction *******************" << std::endl;
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
            // std::cout << "Gfp Instruction" << std::endl;
            // std::cout << inst_val << std::endl;
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

        std::string ToString()
        {
            return lhs->name + " = $gfp " + src->name + " " + field->name;
        }

        void pretty_print() {
            std::cout << "******************* Gfp Instruction *******************" << std::endl;
            std::cout << "Instruction type: " << instrType << std::endl;
            std::cout << "LHS: " << std::endl;
            lhs->pretty_print();
            std::cout << "Source: " << std::endl;
            src->pretty_print();
            std::cout << "Field: " << std::endl;
            field->pretty_print();
            std::cout << "******************* End of Gfp Instruction *******************" << std::endl;
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
            // std::cout << "Load Instruction" << std::endl;
            // std::cout << inst_val << std::endl;
            if (inst_val["lhs"] != nullptr) {
                lhs = new Variable(inst_val["lhs"]["name"], new Type(inst_val["lhs"]["typ"]));
            }
            if (inst_val["src"] != nullptr) {
                src = new Variable(inst_val["src"]["name"], new Type(inst_val["src"]["typ"]));
            }
        }

        std::string ToString()
        {
            return lhs->name + " = $load " + src->name;
        }

        void pretty_print() {
            std::cout << "******************* Load Instruction *******************" << std::endl;
            std::cout << "Instruction type: " << instrType << std::endl;
            std::cout << "LHS: " << std::endl;
            lhs->pretty_print();
            std::cout << "Source: " << std::endl;
            src->pretty_print();
            std::cout << "******************* End of Load Instruction *******************" << std::endl;
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
            // std::cout << "Store Instruction" << std::endl;
            // std::cout << inst_val << std::endl;
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

        std::string ToString()
        {
            std::string op_str = (op->var != nullptr) ? op->var->name : std::to_string(op->val);
            return "$store " + dst->name + " " + op_str;
        }

        void pretty_print() {
            std::cout << "******************* Store Instruction *******************" << std::endl;
            std::cout << "Instruction type: " << instrType << std::endl;
            std::cout << "Destination: " << std::endl;
            dst->pretty_print();
            std::cout << "Operand: " << std::endl;
            op->pretty_print();
            std::cout << "******************* End of Store Instruction *******************" << std::endl;
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
            // std::cout << "CallExt Instruction" << std::endl;
            // std::cout << inst_val << std::endl;
            
            // lhs can be optional
            if (inst_val["lhs"].dump() == "null" || inst_val["lhs"] == nullptr)
                lhs = nullptr;
            else if (inst_val["lhs"] != nullptr) {
                lhs = new Variable(inst_val["lhs"]["name"], new Type(inst_val["lhs"]["typ"]));
            }
            if (inst_val["ext_callee"] != nullptr) {
                extFuncName = inst_val["ext_callee"];
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

        std::string ToString() {
            std::string args_str = "";
            int i = 0;
            for (auto arg : args) {
                args_str += (arg->var != nullptr) ? arg->var->name : std::to_string(arg->val);
                if (i != args.size() - 1)
                    args_str += ", ";
                i += 1;
            }
            std::string lhs_str = (lhs != nullptr) ? lhs->name + " = " : "";
            return lhs_str + "$call_ext " + extFuncName + "(" + args_str + ")";
        }

        void pretty_print() {
            std::cout << "******************* CallExt Instruction *******************" << std::endl;
            std::cout << "Instruction type: " << instrType << std::endl;
            std::cout << "LHS: " << std::endl;
            if (lhs != nullptr)
                lhs->pretty_print();
            else
                std::cout << "None" << std::endl;
            std::cout << "External Function Name: " << extFuncName << std::endl;
            std::cout << "Arguments: " << std::endl;
            for (auto arg : args) {
                arg->pretty_print();
            }
            std::cout << "******************* End of CallExt Instruction *******************" << std::endl;
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
            // std::cout << "Branch Instruction" << std::endl;
            // std::cout << inst_val << std::endl;
            
            if (inst_val["cond"] != nullptr) {
                if (inst_val["cond"]["Var"] != nullptr)
                    condition = new Operand(new Variable(inst_val["cond"]["Var"]["name"], new Type(inst_val["cond"]["Var"]["typ"])));
                else if (inst_val["cond"]["CInt"] != nullptr)
                    condition = new Operand(inst_val["cond"]["CInt"]);
            }
            if (inst_val["tt"] != nullptr) {
                tt = inst_val["tt"];
            }
            if (inst_val["ff"] != nullptr) {
                ff = inst_val["ff"];
            }
        };

        std::string ToString()
        {
            std::string cond_str = (condition->var != nullptr) ? condition->var->name : std::to_string(condition->val);
            return "$branch " + cond_str + " " + tt + " " + ff;
        }

        void pretty_print() {
            std::cout << "******************* Branch Instruction *******************" << std::endl;
            std::cout << "Instruction type: " << instrType << std::endl;
            std::cout << "Condition: " << std::endl;
            condition->pretty_print();
            std::cout << "True branch: " << tt << std::endl;
            std::cout << "False branch: " << ff << std::endl;
            std::cout << "******************* End of Branch Instruction *******************" << std::endl;
        }

        Operand *condition;
        std::string tt;
        std::string ff;
};

/*
 * $jump bb1
 */
class JumpInstruction : public Instruction{
    public:
    JumpInstruction(json inst_val) : label(inst_val) {
        // std::cout << "Jump Instruction" << std::endl;
        // std::cout << inst_val << std::endl;
    };

    std::string ToString() {
        return "$jump " + label;
    }

    void pretty_print() {
        std::cout << "******************* Jump Instruction *******************" << std::endl;
        std::cout << "Instruction type: " << instrType << std::endl;
        std::cout << "Label: " << label << std::endl;
        std::cout << "******************* End of Jump Instruction *******************" << std::endl;
    }

    /*
     * The label of the BasicBlock we're jumping to.
     */
    std::string label;
};

/*
 * $ret x - x can be a constant or a variable or null
 */
class RetInstruction : public Instruction{
    public:
        RetInstruction(json inst_val) {
            // std::cout << "Ret Instruction" << std::endl;
            // std::cout << inst_val << std::endl;
            
            // When return instruction is null, it means it is a void return
            if (inst_val.dump() == "null" || inst_val == nullptr)
                op = nullptr;
            else if (inst_val["Var"] != nullptr) {
                op = new Operand(new Variable(inst_val["Var"]["name"], new Type(inst_val["Var"]["typ"])));
            }
            else if (inst_val["CInt"] != nullptr)
                op = new Operand(inst_val["CInt"]);
        }

        std::string ToString()
        {
            if (op == nullptr)
                return "$ret";
            else
                return "$ret " + ((op->var != nullptr) ? op->var->name : std::to_string(op->val));
        }

        void pretty_print() {
            std::cout << "******************* Ret Instruction *******************" << std::endl;
            std::cout << "Instruction type: " << instrType << std::endl;
            if (op == nullptr)
                std::cout << "Operand: " << std::endl;
            else
                op->pretty_print();
            std::cout << "******************* End of Ret Instruction *******************" << std::endl;
        }

        Operand *op;
};

/*
 * x = $call_dir foo(10) then bb1
 */
class CallDirInstruction : public Instruction{
    public:
        CallDirInstruction(json inst_val) {
            // std::cout << "CallDir Instruction" << std::endl;
            // std::cout << inst_val << std::endl;
            
            // lhs can be optional
            if (inst_val["lhs"].dump() == "null" || inst_val["lhs"] == nullptr)
                lhs = nullptr;
            else if (inst_val["lhs"] != nullptr) {
                lhs = new Variable(inst_val["lhs"]["name"], new Type(inst_val["lhs"]["typ"]));
            }
            if (inst_val["callee"] != nullptr) {
                callee = inst_val["callee"];
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
                next_bb = inst_val["next_bb"];
            }
        }

        std::string ToString()
        {
            std::string args_str = "";
            int i = 0;
            for (auto arg : args) {
                args_str += (arg->var != nullptr) ? arg->var->name : std::to_string(arg->val);
                if (i != args.size() - 1)
                    args_str += ", ";
                i += 1;
            }
            std::string lhs_str = (lhs != nullptr) ? lhs->name + " = " : "";
            return lhs_str + "$call_dir " + callee + "(" + args_str + ") then " + next_bb;
        }

        void pretty_print() {
            std::cout << "******************* CallDir Instruction *******************" << std::endl;
            std::cout << "Instruction type: " << instrType << std::endl;
            std::cout << "LHS: " << std::endl;
            if (lhs != nullptr)
                lhs->pretty_print();
            else
                std::cout << "None" << std::endl;
            std::cout << "Callee: " << callee << std::endl;
            std::cout << "Arguments: " << std::endl;
            for (auto arg : args) {
                arg->pretty_print();
            }
            std::cout << "Next Basic Block: " << next_bb << std::endl;
            std::cout << "******************* End of CallDir Instruction *******************" << std::endl;
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
            // std::cout << "CallIdr Instruction" << std::endl;
            // std::cout << inst_val << std::endl;
            
            // lhs can be optional
            if (inst_val["lhs"].dump() == "null" || inst_val["lhs"] == nullptr)
                lhs = nullptr;
            else if (inst_val["lhs"] != nullptr) {
                lhs = new Variable(inst_val["lhs"]["name"], new Type(inst_val["lhs"]["typ"]));
            }
            if (inst_val["callee"] != nullptr) {
                fp = new Variable(inst_val["callee"]["name"], new Type(inst_val["callee"]["typ"]));
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
                next_bb = inst_val["next_bb"];
            }
        }

        std::string ToString()
        {
            std::string args_str = "";
            int i = 0;
            for (auto arg : args) {
                args_str += (arg->var != nullptr) ? arg->var->name : std::to_string(arg->val);
                if (i != args.size() - 1)
                    args_str += ", ";
                i += 1;
            }
            std::string lhs_str = (lhs != nullptr) ? lhs->name + " = " : "";
            return lhs_str + "$call_idr " + fp->name + "(" + args_str + ") then " + next_bb;
        }

        void pretty_print() {
            std::cout << "******************* CallIdr Instruction *******************" << std::endl;
            std::cout << "Instruction type: " << instrType << std::endl;
            std::cout << "LHS: " << std::endl;
            if (lhs != nullptr)
                lhs->pretty_print();
            else
                std::cout << "None" << std::endl;
            std::cout << "Function Pointer: " << std::endl;
            fp->pretty_print();
            std::cout << "Arguments: " << std::endl;
            for (auto arg : args) {
                arg->pretty_print();
            }
            std::cout << "Next Basic Block: " << next_bb << std::endl;
            std::cout << "******************* End of CallIdr Instruction *******************" << std::endl;
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
                    store_inst->instrType = InstructionType::StoreInstrType;
                    instructions.push_back(store_inst);
                }
                else if (i.key() == "AddrOf") {
                    AddrofInstruction *addrof_inst = new AddrofInstruction(i.value());
                    addrof_inst->instrType = InstructionType::AddrofInstrType;
                    instructions.push_back(addrof_inst);
                }
                else if (i.key() == "Load") {
                    LoadInstruction *load_inst = new LoadInstruction(i.value());
                    load_inst->instrType = InstructionType::LoadInstrType;
                    instructions.push_back(load_inst);
                }
                else if (i.key() == "Alloc") {
                    AllocInstruction *alloc_inst = new AllocInstruction(i.value());
                    alloc_inst->instrType = InstructionType::AllocInstrType;
                    instructions.push_back(alloc_inst);
                }
                else if (i.key() == "Arith") {
                    ArithInstruction *arith_inst = new ArithInstruction(i.value());
                    arith_inst->instrType = InstructionType::ArithInstrType;
                    instructions.push_back(arith_inst);
                }
                else if (i.key() == "Cmp") {
                    CmpInstruction *cmp_inst = new CmpInstruction(i.value());
                    cmp_inst->instrType = InstructionType::CmpInstrType;
                    instructions.push_back(cmp_inst);
                }
                else if (i.key() == "Copy") {
                    CopyInstruction *copy_inst = new CopyInstruction(i.value());
                    copy_inst->instrType = InstructionType::CopyInstrType;
                    instructions.push_back(copy_inst);
                }
                else if (i.key() == "Gep") {
                    GepInstruction *gep_inst = new GepInstruction(i.value());
                    gep_inst->instrType = InstructionType::GepInstrType;
                    instructions.push_back(gep_inst);
                }
                else if (i.key() == "Gfp") {
                    GfpInstruction *gfp_inst = new GfpInstruction(i.value());
                    gfp_inst->instrType = InstructionType::GfpInstrType;
                    instructions.push_back(gfp_inst);
                }
                else if (i.key() == "CallExt") {
                    CallExtInstruction *call_ext_inst = new CallExtInstruction(i.value());
                    call_ext_inst->instrType = InstructionType::CallExtInstrType;
                    instructions.push_back(call_ext_inst);
                }
            }            
        }
        // Parse terminal instruction
        if (bb_json["term"] != nullptr)
        {
            std::string term_type = bb_json["term"].begin().key();
            //std::cout << "Terminal type: " << term_type << std::endl;
            if(term_type == "Branch"){
                BranchInstruction *branch_inst = new BranchInstruction(bb_json["term"]["Branch"]);
                branch_inst->instrType = InstructionType::BranchInstrType;
                terminal = branch_inst;
            }
            else if(term_type == "Jump"){
                JumpInstruction *jump_inst = new JumpInstruction(bb_json["term"]["Jump"]);
                jump_inst->instrType = InstructionType::JumpInstrType;
                terminal = jump_inst;
            }
            else if(term_type == "Ret"){
                RetInstruction *ret_inst = new RetInstruction(bb_json["term"]["Ret"]);
                ret_inst->instrType = InstructionType::RetInstrType;
                terminal = ret_inst;
            }
            else if(term_type == "CallDirect"){
                CallDirInstruction *call_dir_inst = new CallDirInstruction(bb_json["term"]["CallDirect"]);
                call_dir_inst->instrType = InstructionType::CallDirInstrType;
                terminal = call_dir_inst;
            }
            else if(term_type == "CallIndirect"){
                CallIdrInstruction *call_idr_inst = new CallIdrInstruction(bb_json["term"]["CallIndirect"]);
                call_idr_inst->instrType = InstructionType::CallIdrInstrType;
                terminal = call_idr_inst;
            }
        }
    }

    void pretty_print(json what_to_print) {
        std::cout << "******************* Basic Block *******************" << std::endl;
        std::cout << "Label:        " << label << std::endl;
        if (what_to_print["functions"]["bbs"]["instructions"] != nullptr && what_to_print["functions"]["bbs"]["instructions"] == "true")
        std::cout << "Instructions: " << std::endl;
        for (auto inst : instructions) {
            inst->pretty_print();
        }
        std::cout << "Terminal: " << std::endl;
        terminal->pretty_print();
        std::cout << "******************* End of Basic Block *******************" << std::endl;
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
class Function {
    public:
    Function(json func_json): params(std::vector<Variable*>()), locals(std::unordered_map<std::string,Variable*>()), bbs(std::unordered_map<std::string, BasicBlock*>()) {

        // std::cout << "Function" << std::endl;
        // std::cout << func_json << std::endl;

        if (func_json["id"] != nullptr) {
            name = func_json["id"];
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
                locals.insert({local_val["name"], new Variable(local_val["name"], new Type(local_val["typ"]))});
            }
        }
        if (func_json["body"] != nullptr) {
            for (auto &[bb_key, bb_val] : func_json["body"].items()) {
                auto bb = new BasicBlock(bb_val);
                bbs[bb->label] = bb;
            }
        }
    }

    void print_pretty(json what_to_print) {
        std::cout << "******************* Function *******************" << std::endl;
        std::cout << "Name: " << name << std::endl;
        std::cout << "Parameters: " << std::endl;
        for (auto param : params) {
            param->pretty_print();
        }
        std::cout << "Return Type: " << std::endl;
        if (ret != nullptr)
            ret->pretty_print();
        else
            std::cout << "void" << std::endl;
        std::cout << "Locals: " << std::endl;
        for (auto local : locals) {
            local.second->pretty_print();
        }
        if (what_to_print["functions"]["bbs"] != nullptr)
        std::cout << "Basic Blocks: " << std::endl;
        for (auto &[bb_key, bb_val] : bbs) {
            bb_val->pretty_print(what_to_print);
        }
        std::cout << "******************* End of Function *******************" << std::endl;
    }

    std::string name;
    std::vector<Variable*> params;
    std::unordered_map<std::string, Variable*> locals;
    std::unordered_map<std::string, BasicBlock*> bbs;
    Type *ret;
};

/*
 * A program is a set of structs, global variables, function definitions, and
 * external function declarations.
 */
class Program {
    public:
        Program(json program_json): structs(std::unordered_map<std::string, Struct*>()), globals(std::vector<Global*>()), funcs(std::unordered_map<std::string, Function*>()), ext_funcs(std::unordered_map<std::string, ExternalFunction*>()) {
            // std::cout << "Program" << std::endl;
            
            if (program_json["structs"] != nullptr) {
                for (auto &[st_key, st_val] : program_json["structs"].items()) {
                    Struct *st = new Struct(st_val);
                    structs[st_key] = st;
                    structs[st_key]->name = st_key;
                }
            }
            if (program_json["globals"] != nullptr) {
                for (auto &[global_key, global_val] : program_json["globals"].items()) {
                    globals.push_back(new Global(global_val));
                }
            }
            if (program_json["functions"] != nullptr) {
                for (auto &[func_key, func_val] : program_json["functions"].items()) {
                    Function *func = new Function(func_val);
                    funcs[func_key] = func;
                }
            }
            if (program_json["externs"] != nullptr) {
                for (auto &[ext_func_key, ext_func_val] : program_json["externs"].items()) {
                    ExternalFunction *ext_func = new ExternalFunction(ext_func_val);
                    ext_funcs[ext_func_key] = ext_func;
                    ext_func->name = ext_func_key;
                }
            }
        };

        void print_pretty(json what_to_print) {
            std::cout << "******************* Program *******************" << std::endl;
            if (what_to_print["structs"] != nullptr && what_to_print["structs"] == "true") {
                std::cout << "Structs: " << std::endl;
                for (auto st = structs.begin(); st != structs.end(); ++st) {
                    st->second->pretty_print();
                }
            }
            if (what_to_print["globals"] != nullptr && what_to_print["globals"] == "true") {
                std::cout << "Globals: " << std::endl;
                for (auto global : globals) {
                    global->pretty_print();
                }
            }
            if (what_to_print["functions"] != nullptr) {
                std::cout << "Functions: " << std::endl;
                for (auto it = funcs.begin(); it != funcs.end(); ++it) {
                    it->second->print_pretty(what_to_print);
                }
            }
            if (what_to_print["externs"] != nullptr && what_to_print["externs"] == "true") {
                std::cout << "External Functions: " << std::endl;
                for (auto it = ext_funcs.begin(); it != ext_funcs.end(); ++it) {
                    it->second->pretty_print();
                }
            }
            std::cout << "******************* End of Program *******************" << std::endl;
        }

        std::unordered_map<std::string, Struct*> structs;
        std::vector<Global*> globals;
        std::unordered_map<std::string, Function*> funcs;
        std::unordered_map<std::string, ExternalFunction*> ext_funcs;
};