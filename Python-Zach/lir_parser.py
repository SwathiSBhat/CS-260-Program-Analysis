import sys
import json
from lir import Program, Type

def main():
    if len(sys.argv) != 2:
        print("Error: Expecting a single file in lir format to parse.", file=sys.stderr)
        return 
    program_json = json.load(open(sys.argv[1], "r"))
    program = Program(program_json)

    '''Test: Stats of Program Data Structure'''
    # Count all fields across all struct types
    count = 0
    for struct in program.structs.values():
        count += len(struct)

    retvals = params = locals = basic_blocks = insts = terms = 0
    ints = structs = pointer_to_int = pointer_to_struct = pointer_to_function = pointer_to_pointer = 0
    for function in program.funcs.values():

        # Count all functions that return a value
        if function.return_type is not None:
            retvals += 1

        # Count all function parameters
        params += len(function.params)

        # Count all the local variables
        locals += len(function.locals)

        # Count the number of basic blocks
        basic_blocks += len(function.basic_blocks)

        for block in function.basic_blocks:
            # Count the numner of instructions
            insts += len(block.insts)

            # Number of terminals 
            if block.term is not None:
                terms += 1

        # Add up all of the local ints, structs, and pointers
        for local_var in function.locals:
            if local_var.type.indirection == 0 and isinstance(local_var.type.base_type, Type.Int_Type):
                ints += 1
            if local_var.type.indirection == 0 and isinstance(local_var.type.base_type, Type.Struct_Type):
                structs += 1
            if local_var.type.indirection == 1 and isinstance(local_var.type.base_type, Type.Int_Type):
                pointer_to_int += 1
            if local_var.type.indirection == 1 and isinstance(local_var.type.base_type, Type.Struct_Type):
                pointer_to_struct += 1
            if local_var.type.indirection == 1 and isinstance(local_var.type.base_type, Type.Function_Type):
                pointer_to_function += 1
            if local_var.type.indirection > 1:
                pointer_to_pointer += 1

    for global_var in program.globals:
        if global_var.type.indirection == 0 and isinstance(global_var.type.base_type, Type.Int_Type):
            ints += 1
        if global_var.type.indirection == 0 and isinstance(global_var.type.base_type, Type.Struct_Type):
            structs += 1
        if global_var.type.indirection == 1 and isinstance(global_var.type.base_type, Type.Int_Type):
            pointer_to_int += 1
        if global_var.type.indirection == 1 and isinstance(global_var.type.base_type, Type.Struct_Type):
            pointer_to_struct += 1
        if global_var.type.indirection == 1 and isinstance(global_var.type.base_type, Type.Function_Type):
            pointer_to_function += 1
        if global_var.type.indirection > 1:
            pointer_to_pointer += 1


    print(f"Number of fields across all struct types: {count}")
    print(f"Number of functions that return a value: {retvals}")
    print(f"Number of function parameters: {params}")
    print(f"Number of local variables: {locals}")
    print(f"Number of basic blocks: {basic_blocks}")
    print(f"Number of instructions: {insts}")
    print(f"Number of terminals: {terms}")
    print(f"Number of locals and globals with int type: {ints}")
    print(f"Number of locals and globals with struct type: {structs}")
    print(f"Number of locals and globals with pointer to int type: {pointer_to_int}")
    print(f"Number of locals and globals with pointer to struct type: {pointer_to_struct}")
    print(f"Number of locals and globals with pointer to function type: {pointer_to_function}")
    print(f"Number of locals and globals with pointer to pointer type: {pointer_to_pointer}")

if __name__ == '__main__':
    main()