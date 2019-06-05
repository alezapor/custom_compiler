#include "parser.h"

int main() {
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();

    PJPParser parser;
    parser.getNextToken();
     
    PJPCodegen codegen;
    (parser.ParseProgram())->codegen(codegen); /* emit bytecode for the toplevel block */
    
    std::cout << "Code is generated.\n";
    codegen.theModule->print(llvm::errs(), nullptr);
    codegen.saveToObjectFile("output.o");
    return 0;
}

