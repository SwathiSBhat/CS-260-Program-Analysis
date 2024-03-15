# Description of LIR program


## Dependencies

## Generating submission zip files

```
rm submission.zip
zip -r submission.zip assn0.cpp CMakeLists.txt build-analyses.sh run-constants-analysis.sh run-intervals-analysis.sh run-generator.sh headers/ constant-analysis/ interval-analysis/ control-flow-analysis/ constraint-generator/ reaching-defn/ tests/ run-slice.sh program-dependence-graph/
```

## Build instructions

```
cmake .
make
```

This will generate executables in the `bin` directory.
