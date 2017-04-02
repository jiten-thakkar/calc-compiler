Skeleton for "calc" language compiler.
The language is defined [here](https://utah.instructure.com/courses/377698/assignments/3420338) 
and [here](https://utah.instructure.com/courses/377698/assignments/3426197) as part of [advanced compiler](https://utah.instructure.com/courses/377698/assignments/syllabus) class.

build the compiler like this:

mkdir build
cd build
cmake .. -DLLVM_DIR=$LLVM -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_C_COMPILER=clang -DCMAKE_BUILD_TYPE=Debug
make

run the compiler like this:

./calcc < input_file 2> out.ll

run the generated code like this:

clang ../driver.c out.ll -Wall -o calc

//cmake .. -DLLVM_DIR=/usr/local/Cellar/llvm@3.8/3.8.1/lib/llvm-3.8/share/llvm/cmake/ -DCMAKE_CXX_COMPILER=clang++-3.8 -DCMAKE_C_COMPILER=clang-3.8 -DCMAKE_BUILD_TYPE=Debug
