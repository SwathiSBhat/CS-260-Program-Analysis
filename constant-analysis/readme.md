## Description

Implements the intraprocedural integer constant analysis using MFP (maximal / minimal fixed point) worklist algorithm

## Analysis Output

The result of each analysis will be, for the analyzed function, a map from each basic block to the abstract values at the end of that basic block for all variables that are not ⊥ . The output will be printed to stdout with the following format:
```
<basic block label>:
<variable name 1> -> <abstract value>
```

## Steps to run analysis

At the root directory, of the repository, after running the instruction for build:
```
./assn1_constant_analysis ./constant-analysis/constant-analysis-tests/test.1.lir.json
```
