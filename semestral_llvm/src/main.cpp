#include "parser.h"

int main(int argc, char * argv[]) {
    llvm::InitializeNativeTarget();
    llvm::InitializeNativeTargetAsmPrinter();
    llvm::InitializeNativeTargetAsmParser();
    
    std::fstream is;
    is.open(argv[1], std::fstream::in);
    if(!is){
        printf("Cannot open file.\n");
	exit(1);
    }
  
    PJPParser parser(&is);
    parser.getNextToken();
     
    PJPCodegen codegen;
    (parser.ParseProgram())->codegen(codegen); /* emit bytecode for the toplevel block */
    
    std::cout << "Code is generated.\n";
    codegen.theModule->print(llvm::errs(), nullptr);
    codegen.saveToObjectFile("obj/"+parser.getName()+".o");
    is.close();
    return 0;
}

