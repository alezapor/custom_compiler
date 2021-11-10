The goal of this project is to extend LLVM frontend and create a custom compiler for simple Pascal-like language.

### What features this compiler supports

* Main function, printing numbers (print, println), reading numbers (readln)
* Global variables support, expressions, assignment, number constants in decadic base
* Number constants in hexadecimal and octal base ($ and & prefix)
* If, While (along with exit statement)
* For (to and downto; along with exit statement)
* Nested blocks
* Statically allocated arrays (indexed in any interval)
* Procedures, Functions, Local variables, parameters of functions and procedures
* Recursion, indirect recursion

### Fast setup and some instructions 

The **samples** directory contains examples of Pascal programs our compiler can compile. Supported syntax is describer in **grammar.txt**

Compilation (creating an executable for our compiler): `make compile`.<br>
Producing executable of pascal source file using custom compiler: 
* `make run <test.p>`
* `clang obj/<programName>.o -o bin/<executable>`
* `./bin/<executable>` <br>

To run all tests: `make tests`.<br>
To clean all object files and executables: `make clean`.