# RMT setup guide for FROM 2019

## Prerequisites


- A modern C++ compiler (e.g. a recent verion of g++, clang, MSVC);
- Z3 (clone from <https://github.com/Z3Prover/z3.git>, then follow the instructions in the README file to build).

## Step by step compilation guide for Microsoft Visual Studio 2017

1. Create a new Visual C++ Empty Project;
2. Change the target build to Release x86 (optional but recommended);
3. Add all `.cpp` files from `src` under *Source files*  and all `.h` files from `src` under *Header files*;
4. Add `_CRT_SECURE_NO_WARNINGS` to *Preprocessor Definitions* (*Project > Properties > Configuration Properties > C/C++ > Preprocessor*);
5. Add `z3\src\api` to *Include Directories* and `z3\build` to *Library Directories* (*Project > Properties > Configuration Properties > VC++ Directories*);
6. Add `z3.lib` and `libz3.lib` to *Additional Dependencies* (*Project > Properties > Configuration Properties > Linker > Input*);
7. Build the project (*Ctrl + Shift + B*).
