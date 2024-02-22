# CS260 Program Analysis

```
rm submission.zip
zip -r submission.zip assn0.cpp CMakeLists.txt build-analyses.sh run-constants-analysis.sh run-intervals-analysis.sh run-generator.sh headers/ constant-analysis/ interval-analysis/ control-flow-analysis/ constraint-generator/ tests/
```
## Generating submission zip files
### Assignment 2
```
zip -r submission.zip CMakeLists.txt build-analyses.sh run-control.sh run-rdef.sh headers/ control-flow-analysis/ tests/ 
```

## Build instructions

```
cmake .
make
```

This will generate executables in the `bin` directory.
