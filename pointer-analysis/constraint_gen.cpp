#include <fstream>
#include <string>
#include <map>
#include <queue>
#include<variant>
#include <set>
#include <iostream>
#include "../headers/datatypes.h"

using json = nlohmann::json;

class ConstraintGenerator {
    public:
    Program program;
    std::set<std::string> result;

    ConstraintGenerator(Program program) : program(program) {};

    void Analyze()
    {
        /*
        * Since this is an interprocedural analysis, we analyze each function in the program
        */
       for (auto const& func : program.funcs) {
            std::string func_name = func.first;
           for (auto const& bb : func.second->bbs) {
               std::cout << "Analyzing basic block: " << bb.first << std::endl;
               for (auto inst = bb.second->instructions.begin(); inst != bb.second->instructions.end(); inst++) {
                    std::cout << "Analyzing instruction: " << (*inst)->instrType << std::endl;
                    // x = $copy y
                    // [y] <= [x]
                    if ((*inst)->instrType == InstructionType::CopyInstrType)
                    {
                        // Ignore if x is not a pointer
                        CopyInstruction *copy_inst = (CopyInstruction *) (*inst);
                        if (copy_inst->lhs->type->indirection == 0)
                            continue;
                        std::string lhs = func_name + "." + copy_inst->op->var->name;
                        std::string rhs = func_name + "." + copy_inst->lhs->name;
                        result.insert(lhs + " <= " + rhs);
                    }
                    // x = $gep y op
                    // [y] <= [x]
                    else if ((*inst)->instrType == InstructionType::GepInstrType)
                    {
                        GepInstruction *gep_inst = (GepInstruction *) (*inst);
                        std::string lhs = func_name + "." + gep_inst->src->name;
                        std::string rhs = func_name + "." + gep_inst->lhs->name;
                        result.insert(lhs + " <= " + rhs);
                    }
                    // x = $addrof y
                    // ref(const(y), [y]) <= [x]
                    else if ((*inst)->instrType == InstructionType::AddrofInstrType)
                    {
                        AddrofInstruction *addrof_inst = (AddrofInstruction *) (*inst);
                        std::string lhs = "ref(" + func_name + "." + addrof_inst->rhs->name + "," + func_name + "." + addrof_inst->rhs->name + ")";
                        std::string rhs = func_name + "." + addrof_inst->lhs->name; 
                        result.insert(lhs + " <= " + rhs);
                    }
                    // x = $alloc op [id]
                    // ref(const(op), [id]) <= [x]
                    else if ((*inst)->instrType == InstructionType::AllocInstrType)
                    {
                        AllocInstruction *alloc_inst = (AllocInstruction *) (*inst);
                        std::string lhs = "ref(" + alloc_inst->id->name + "," + alloc_inst->id->name + ")";
                        std::string rhs = func_name + "." + alloc_inst->lhs->name;
                        result.insert(lhs + " <= " + rhs);
                    }
                    // x = $load y
                    // proj(ref,1,[y]) <= [x]
                    else if ((*inst)->instrType == InstructionType::LoadInstrType)
                    {
                        LoadInstruction *load_inst = (LoadInstruction *) (*inst);
                        // Ignore if x is not a pointer
                        if (load_inst->lhs->type->indirection == 0)
                            continue;
                        std::string lhs = "proj(ref,1," + func_name + "." + load_inst->src->name + ")";
                        std::string rhs = func_name + "." + load_inst->lhs->name;
                        result.insert(lhs + " <= " + rhs);
                    }
                    // $store x y
                    // Ignore if y is not a pointer
                    // [y] <= proj(ref,1,[x])
                    else if ((*inst)->instrType == InstructionType::StoreInstrType)
                    {
                        StoreInstruction *store_inst = (StoreInstruction *) (*inst);
                        if (store_inst->op->var->type->indirection == 0)
                            continue;
                        std::string lhs = func_name + "." + store_inst->op->var->name;
                        std::string rhs = "proj(ref,1," + func_name + "." + store_inst->dst->name + ")";
                        result.insert(lhs + " <= " + rhs);
                    }
               }
           }
       }

       // print result
         for (auto const& res : result) {
              std::cout << res << std::endl;
         }
    }
};

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: constraint-solver <file path>" << std::endl;
        return EXIT_FAILURE;
    }

    std::ifstream f(argv[1]);
    json lir_json = json::parse(f);

    Program program = Program(lir_json);
    ConstraintGenerator constraint_gen = ConstraintGenerator(program);
    constraint_gen.Analyze();
    return 0;
}