# Experimental implementations of static analysis techniques

## Overview

This repository contains code that implements static analysis techniques for programs in an intermediate language (LIR) which look similar to C. The input to each analysis is usually an LIR program (plus few other inputs based on the analysis) and the output is printed to stdout. The following static analyses techniques are implemented:
1. [Constant analysis](https://github.com/SwathiSBhat/Static-program-analysis-techniques/tree/main/constant-analysis)
2. [Reaching definitions](https://github.com/SwathiSBhat/Static-program-analysis-techniques/tree/main/reaching-defn)
3. [Control Flow Analysis](https://github.com/SwathiSBhat/Static-program-analysis-techniques/tree/main/control-flow-analysis)
4. [Andersen-style pointer analysis constraint generator](https://github.com/SwathiSBhat/Static-program-analysis-techniques/tree/main/constraint-generator)
5. [Andersen-style pointer analysis constraint solver](https://github.com/SwathiSBhat/Static-program-analysis-techniques/tree/main/pointer-analysis)
6. [Program Slicing](https://github.com/SwathiSBhat/Static-program-analysis-techniques/tree/main/program-dependence-graph)
7. [Taint Analysis](https://github.com/SwathiSBhat/Static-program-analysis-techniques/tree/main/taint-analysis) - Includes context-insensitive taint analysis, k-limited callstring taint analysis and functional context sensitive taint analysis

## Dependencies

- cmake

## Build instructions

In the root directory of the repository, run the following commands
```
cmake .
make
```

## LIR description

- a LIR program is:

    - a set of struct types
    - a set of global variables
    - a set of extern function declarations
    - a set of function definitions

- a type is one of:

    - `int` (an integer)
    - a struct type (identified by its name)
    - a function type
    - a pointer to a type

- a struct type is a name and a non-empty list of field names and types.

    - these are just like C-style structs.

- a global variable is a name and a type.

- an extern function declaration is a name and a function type.

    - an extern function is one that is defined externally to the current program, i.e., we know its name and type signature and can call it, but we don't have access to its source code.

- a function definition is:

    - a name
    - an ordered list of parameters (with names and types)
    - an optional return type
    - a set of local variables (with names and types)
    - a set of basic blocks

- a basic block is a label and an ordered list of instructions, ending in a single terminal.

- an instruction is one of:

    - address-of: `x = $addrof y` assigns the address of `y` to `x`.

    - memory allocation: `x = $alloc 10 [_a1]` allocates 10 items (what type of items depends on the type of pointer variable `x`) and assigns the newly allocated address to `x`. the bracketed identifier (`_a1` in this example) is a convenient label for the allocation that we will use during some of our analyses.

    - integer arithmetic: `x = $arith add y 2` adds 2 to the value of `y` and assigns the result to `x`; similarly for the other arithmetic operators `sub`, `mul`, `div`.
    
    - integer and pointer comparison `x = $cmp eq y 2` compares `y` to 2 and assigns 0 (false) or 1 (true) to `x`; similarly for the other relational operators `neq`, `lt`, `lte`, `gt`, `gte`.

    - copy: `x = $copy y` copies the value of `y` to `x`.

    - get-element-pointer: `x = $gep y 10` takes `y` (which is a pointer to an array of elements) and assigns to `x` the address of the 10th element of the array. this is the only way to do pointer arithmetic.

    - get-field-pointer: `x = $gfp y foo` takes `y` (which is a pointer to a struct) and assigns to `x` the address of the `foo` field of the struct. the only way to access fields of a struct is via a pointer.

    - load from memory: `x = $load y` loads the value pointed to by `y` and assigns it to `x`.

    - store to memory: `$store x y` stores the value of `y` to the location pointed to by `x`.

    - call external function: `x = $call_ext foo(10)` calls the extern function `foo` with argument 10 and assigns the returned value to `x`. if the extern function does not return a value then there is no assignment, e.g., `$call_ext foo(10)`.

- a terminal signals the end of a basic block and is one of:

    - conditional branch: `$branch x bb1 bb2` will either jump to basic block `bb1` (if the value of `x` is non-zero) or `bb2` (if the value of `x` is 0).

    - unconditional jump: `$jump bb1` will always jump to basic block `bb1`.

    - return from function: `$ret x` will return from the current function with the value of `x`. if the current function does not return a value then it would just be `$ret`.

    - direct function call: `x = $call_dir foo(10) then bb1` calls internally-defined function `foo` passing the argument 10, assigns the returned value to `x`, then jumps to basic block `bb1`. if `foo` does not return a value then there is no assignment, e.g., `$call_dir foo(10) then bb1`.

    - indirect function call: `x = $call_idr fp(10) then bb1` calls an internally-defined function (which one depends on the value of the function pointer variable `fp`) passing the argument 10, assigns the returned value to `x`, then jumps to basic block `bb1`. if the called function does not return a value then there is no assignment, e.g., `$call_idr fp(10) then bb1`.

- validity requirements: in order to be a valid LIR program, the following requirements must be met:

    - the program is well-typed.

    - the program contains a function `main : () -> int`, which is the entry point to the program.

    - every struct type has at least one field.

    - an extern function's name cannot be the same as an internally-defined function's name.

    - for each function there must be exactly one basic block labeled `entry`, which is the entry point to the function.

    - for each function there must be exactly one basic block ending in a `$ret` (return) terminal.

    - all basic blocks in a function must be reachable from `entry` and all basic blocks must reach the basic block containing the `$ret` terminal.

    - every `$alloc` instruction has a unique bracketed identifier.

    - the `main` function cannot be called.

    - if a global variable has the same name as an internally-defined function then that variable is a function pointer to the associated function and has the appropriate type (but there cannot be a global variable named `main`).

# LIR example program

```
struct bar {
    f1: int
    f2: &int
}

// global pointer to function foo.
foo: &(&int) -> int

// externally-defined function func.
extern func: (int) -> int

fn main() -> int {
    entry:
        $call_dir baz(42) then exit

    exit:
        $ret 0
}

fn baz(p:int) -> _ {
    let a:int, b:&int, c:&bar, d:&&int, fp:&(&int) -> int

    entry:
        a = $copy p
        b = $addrof a
        $store b 42
        c = $alloc 42 [_1]
        c = $gep c 12
        d = $gfp c f2
        a = $cmp lte a 3
        $branch a bb2 bb3

    bb2:
        b = $load d
        a = $call_dir foo(b) then bb4

    bb3:
        fp = $copy foo
        a = $call_idr fp(b) then bb4

    bb4:
        p = $arith add p 1
        a = $arith mul a p
        a = $call_ext bar(a)
        $jump exit

    exit:
        $ret
}

fn foo(p:&int) -> int {
    let r:int

    entry:
        r = $load p
        $ret r
}
```

## LIR Parser

The file at [headers/datatypes.h](https://github.com/SwathiSBhat/Static-program-analysis-techniques/blob/main/headers/datatypes.h) contains code for converting an LIR program in JSON format into an iterable data structure. This allows us to easily iterate over basic blocks, functions, instructions, structs, etc required during program analysis.

