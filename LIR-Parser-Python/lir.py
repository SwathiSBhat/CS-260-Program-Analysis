from enum import Enum
from typing import Union, List

class Type:
    class Int_Type:
        def __repr__(self):
            return f"Int"

    class Struct_Type:
        def __init__(self, struct_json):
            self.struct_name = str(struct_json)

        def __repr__(self):
            return f"Struct({self.struct_name})"

    class Function_Type:
        def __init__(self, func_type_json):
            self.return_type = None
            if func_type_json["ret_ty"] is not None:
                self.return_type = Type(func_type_json["ret_ty"])
            self.param_types = [Type(param_type_json) for param_type_json in func_type_json["param_ty"]]
        
        def __repr__(self):
            return f"Function(return_type={self.return_type}, param_types={self.param_types})"

    def __init__(self, type_json):  
        self.indirection = 0
        # Count the levels of indirection
        while not isinstance(type_json, str) and "Pointer" in type_json.keys():
            type_json = type_json["Pointer"]
            self.indirection += 1
        if not isinstance(type_json, str):
            if "Function" in type_json.keys():
                self.base_type = Type.Function_Type(type_json["Function"])
            elif "Struct" in type_json.keys():
                self.base_type = Type.Struct_Type(type_json["Struct"])
            else:
                raise ValueError(f"Unrecognized type: {type_json}")
        elif type_json == "Int":
            self.base_type = Type.Int_Type() 
        else:
            raise ValueError(f"Unrecognized type: {type_json}")
    
    def __repr__(self):
        return f"Type(indirection={self.indirection}, base_type={self.base_type})"
    
class Variable:
    def __init__(self, var_json):
        self.name = var_json["name"]
        self.type = Type(var_json["typ"])

    def __repr__(self):
        return f"Variable(name={self.name}, type={self.type})"

class Operand:
    def __init__(self, operand_json):
        if "Var" in operand_json.keys():
            self.op = Variable(operand_json["Var"])
        elif "CInt" in operand_json.keys():
            self.op = int(operand_json["CInt"])
        else:
            raise ValueError("unrecognized operand type")
        
    def isVariable(self):
        return isinstance(self.op, Variable)
    
    def isInt(self):
        return isinstance(self.op, int)
    
    def __repr__(self):
        return f"{self.op}"
    
'''Instruction/Terminator Classes'''
class ArithInstr:
    def __init__(self, arith_inst_json):
        self.lhs = Variable(arith_inst_json["lhs"])
        self.aop = arith_inst_json["aop"]
        self.op1 = Operand(arith_inst_json["op1"])
        self.op2 = Operand(arith_inst_json["op2"])

    def __repr__(self):
        return f"ArithInstr(lhs={self.lhs}, aop={self.aop}, op1={self.op1}, op2={self.op2})"

class CmpInstr:
    def __init__(self, cmp_inst_json):
        self.lhs = Variable(cmp_inst_json["lhs"])
        self.rop = cmp_inst_json["rop"]
        self.op1 = Operand(cmp_inst_json["op1"])
        self.op2 = Operand(cmp_inst_json["op2"])

    def __repr__(self):
        return f"CmpInstr(lhs={self.lhs}, rop={self.rop}, op1={self.op1}, op2={self.op2})"

class CopyInstr:
    def __init__(self, copy_inst_json):
        self.lhs = Variable(copy_inst_json["lhs"])
        self.rhs = Operand(copy_inst_json["op"])

    def __repr__(self):
        return f"CopyInstr(lhs={self.lhs}, rhs={self.rhs})"

class AllocInstr:
    def __init__(self, alloc_inst_json):
        self.lhs = Variable(alloc_inst_json["lhs"])
        self.num = Operand(alloc_inst_json["num"])
        self.id = Variable(alloc_inst_json["id"])

    def __repr__(self):
        return f"AllocInstr(lhs={self.lhs}, num={self.num}, id={self.id})"

class AddrOfInstr:
    def __init__(self, addrof_inst_json):
        self.lhs = Variable(addrof_inst_json["lhs"])
        self.rhs = Variable(addrof_inst_json["rhs"])

    def __repr__(self):
        return f"AddrOfInstr(lhs={self.lhs}, rhs={self.rhs})"

class LoadInstr:
    def __init__(self, load_inst_json):
        self.lhs = Variable(load_inst_json["lhs"])
        self.src = Variable(load_inst_json["src"])

    def __repr__(self):
        return f"LoadInstr(lhs={self.lhs}, src={self.src})"

class StoreInstr:
    def __init__(self, store_inst_json):
        self.dst = Variable(store_inst_json["dst"])
        self.op = Operand(store_inst_json["op"])
    
    def __repr__(self):
        return f"StoreInstr(dst={self.dst}, op={self.op})"

class GepInstr:
    def __init__(self, gep_inst_json):
        self.lhs = Variable(gep_inst_json["lhs"])
        self.src = Variable(gep_inst_json["src"])
        self.index = Operand(gep_inst_json["idx"])

    def __repr__(self):
        return f"GepInstr(lhs={self.lhs}, src={self.src}, index={self.index})"

class GfpInstr:
    def __init__(self, gfp_inst_json):
        self.lhs = Variable(gfp_inst_json["lhs"])
        self.src = Variable(gfp_inst_json["src"])
        self.field = Variable(gfp_inst_json["field"])

    def __repr__(self):
        return f"GfpInstr(lhs={self.lhs}, src={self.src}, field={self.field})"

class CallExtInstr:
    def __init__(self, call_ext_inst_json):
        self.lhs = None
        if call_ext_inst_json["lhs"] is not None:
            self.lhs = Variable(call_ext_inst_json["lhs"])
        self.ext_callee = str(call_ext_inst_json["ext_callee"])
        self.args = [Operand(arg_json) for arg_json in call_ext_inst_json["args"]]

    def __repr__(self):
        return f"CallExtInstr(lhs={self.lhs}, ext_callee={self.ext_callee}, args={self.args})"

class CallDirect:
    def __init__(self, call_direct_json):
        self.lhs = None 
        if call_direct_json["lhs"] is not None:
            self.lhs = Variable(call_direct_json["lhs"])
        self.callee = str(call_direct_json["callee"])
        self.args = [Operand(arg_json) for arg_json in call_direct_json["args"]]
        self.next_bb = str(call_direct_json["next_bb"])

    def __repr__(self):
        return f"CallDirect(lhs={self.lhs}, callee={self.callee}, args={self.args}, next_bb={self.next_bb})"

class CallIndirect:
    def __init__(self, call_indirect_json):
        self.lhs = None 
        if call_indirect_json["lhs"] is not None:
            self.lhs = Variable(call_indirect_json["lhs"])
        self.callee = Variable(call_indirect_json["callee"])
        self.args = [Operand(arg_json) for arg_json in call_indirect_json["args"]]
        self.next_bb = str(call_indirect_json["next_bb"])

    def __repr__(self):
        return f"CallIndirect(lhs={self.lhs}, callee={self.callee}, args={self.args}, next_bb={self.next_bb})"

class Ret:
    def __init__(self, ret_json):
        self.retval = None
        if ret_json is not None:
            self.retval = Operand(ret_json) 

    def __repr__(self):
        return f"Ret(retval={self.retval})"

class Jump:
    def __init__(self, jump_json):
        self.next_bb = str(jump_json) 

    def __repr__(self):
        return f"Jump(next_bb={self.next_bb})"

class Branch:
    def __init__(self, branch_json):
        self.condition = Operand(branch_json["cond"])
        self.tt = str(branch_json["tt"])
        self.ff = str(branch_json["ff"])

    def __repr__(self):
        return f"Branch(cond={self.condition}, tt={self.tt}, ff={self.ff})"

'''End Instruction/Terminator Classes'''

def parse_inst_json(inst_json):
    inst_types = inst_json.keys()
    if "Arith" in inst_types:
        return ArithInstr(inst_json["Arith"])
    elif "Cmp" in inst_types:
        return CmpInstr(inst_json["Cmp"])
    elif "Copy" in inst_types:
        return CopyInstr(inst_json["Copy"])
    elif "Alloc" in inst_types:
        return AllocInstr(inst_json["Alloc"])
    elif "AddrOf" in inst_types:
        return AddrOfInstr(inst_json["AddrOf"])
    elif "Load" in inst_types:
        return LoadInstr(inst_json["Load"])
    elif "Store" in inst_types:
        return StoreInstr(inst_json["Store"])
    elif "Gep" in inst_types:
        return GepInstr(inst_json["Gep"])
    elif "Gfp" in inst_types:
        return GfpInstr(inst_json["Gfp"])
    elif "CallExt" in inst_types:
        return CallExtInstr(inst_json["CallExt"])
    else:
        raise ValueError("Unrecognized instruction in LIR json")
    
def parse_term_json(term_json):
    term_types = term_json.keys()
    if "CallDirect" in term_types:
        return CallDirect(term_json["CallDirect"])
    elif "CallIndirect" in term_types:
        return CallIndirect(term_json["CallIndirect"])
    elif "Ret" in term_types:
        return Ret(term_json["Ret"])
    elif "Jump" in term_types:
        return Jump(term_json["Jump"])
    elif "Branch" in term_types:
        return Branch(term_json["Branch"])
    else:
        raise ValueError("Unrecognized terminator in LIR json")

'''CFG Classes'''
class BasicBlock:
    def __init__(self, basic_block_json):
        self.name = basic_block_json["id"]
        self.insts = [parse_inst_json(inst_json) for inst_json in basic_block_json["insts"]]
        self.term = parse_term_json(basic_block_json["term"])

    def __repr__(self):
        return f"BasicBlock(name={self.name}, insts={self.insts}, term={self.term})"

class Function:
    def __init__(self, func_json):
        self.name = func_json["id"]
        self.return_type = None 
        if func_json["ret_ty"] is not None:
            self.return_type = Type(func_json["ret_ty"])
        self.params = [Variable(param_json) for param_json in func_json["params"]]
        self.locals = [Variable(local_var_json) for local_var_json in func_json["locals"]]
        self.basic_blocks = [BasicBlock(basic_block_json) for basic_block_json in func_json["body"].values()]

class Program:
    def __init__(self, program_json):
        # maps struct_name -> dict(field_name, field_type) 
        self.structs = {struct_name : {Variable(field) for field in struct_desc} for struct_name, struct_desc in program_json["structs"].items()}
        # maps var_name -> var_type 
        self.globals = [Variable(var) for var in program_json["globals"]]
        # maps func_name -> func_def 
        self.funcs = {func_name : Function(func_def) for func_name, func_def in program_json["functions"].items()}
        # maps ext_func_name -> ext_func_type 
        self.ext_funcs = {ext_func_name : Type.Function_Type(ext_func_type["Function"]) for ext_func_name, ext_func_type in program_json["externs"].items()}