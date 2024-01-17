#pragma once

/*
 * A program is a set of structs, global variables, function definitions, and
 * external function declarations.
 */
class Program;

/*
 * A type is either an integer, struct, function , or pointer.
 */
class Type;

/*
 * A struct type is a name and a nonempty list of field names and types.
 */
class Struct;

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
 * A function definition is:
 * - A name
 * - An ordered list of parameters (with names and types)
 * - An optional return type
 * - A set of local variables (with names and types)
 * - A set of basic blocks
 */
class Function;

/*
 * A basic block is a label and an ordered list of instructions, ending in a
 * terminal.
 */
class BasicBlock;

/*
 * These are all instructions.
 */

/*
 * x = $addrof y
 */
class AddrofInstruction;

/*
 * x = $alloc 10 [_a1]
 */
class AllocInstruction;

/*
 * x = $arith add y 2
 */
class ArithInstruction;

/*
 * x = $cmp eq y 2
 */
class CmpInstruction;

/*
 * x = $copy y
 */
class CopyInstruction;

/*
 * x = $gep y 10
 */
class GepInstruction;

/*
 * x = $gfp y foo
 */
class GfpInstruction;

/*
 * x = $load y
 */
class LoadInstruction;

/*
 * store x y
 */
class StoreInstruction;

/*
 * x = $call_ext foo(10)
 */
class CallExtInstruction;

/*
 * These instructions are all terminals.
 */

/*
 * $branch x bb1 bb2
 */
class BranchInstruction;

/*
 * $jump bb1
 */
class JumpInstruction;

/*
 * $ret x
 */
class RetInstruction;

/*
 * x = $call_dir foo(10) then bb1
 */
class CallDirInstruction;

/*
 * x = $call_idr fp(10) then bb1
 */
class CallIdrInstruction;