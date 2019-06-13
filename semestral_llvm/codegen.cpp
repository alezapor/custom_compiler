#include "codegen.h"

PJPCodegen::PJPCodegen() : builder(llvm::IRBuilder<>(theContext)) {
    theModule = llvm::make_unique<llvm::Module>("main", theContext);
    std::vector < llvm::Type * > noargs;
    llvm::FunctionType *FT = llvm::FunctionType::get(llvm::Type::getVoidTy(theContext), noargs, false);

    mainFunction = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, "main", theModule.get());
    // Create a new basic block to start insertion into.
    llvm::BasicBlock *mainFunctBlock = llvm::BasicBlock::Create(theContext, "entry", mainFunction, 0);
    builder.SetInsertPoint(mainFunctBlock);
    printFormat = builder.CreateGlobalString("%d\n", "format", 0);
    scanFormat = builder.CreateGlobalString("%d", "format", 0);
    llvm::Function *mprintf = llvm::Function::Create(
            llvm::FunctionType::get(llvm::Type::getInt32Ty(theContext), {llvm::Type::getInt8PtrTy(theContext)}, true),
            llvm::Function::ExternalLinkage, llvm::Twine("printf"), theModule.get());
    mprintf->setCallingConv(llvm::CallingConv::C);

    llvm::Function *mscanf = llvm::Function::Create(
            llvm::FunctionType::get(llvm::Type::getInt32Ty(theContext), {llvm::Type::getInt8PtrTy(theContext)}, true),
            llvm::Function::ExternalLinkage, llvm::Twine("scanf"), theModule.get());
    mscanf->setCallingConv(llvm::CallingConv::C);
    
}

void PJPCodegen::saveToObjectFile(const std::string &filename) {
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();

    auto TargetTriple = llvm::sys::getDefaultTargetTriple();
    theModule->setTargetTriple(TargetTriple);

    std::string Error;
    auto Target = llvm::TargetRegistry::lookupTarget(TargetTriple, Error);

    // Print an error and exit if we couldn't find the requested target.
    // This generally occurs if we've forgotten to initialise the
    // TargetRegistry or we have a bogus target triple.
    if (!Target) {
        std::cerr << Error << std::endl;
        return;
    }

    auto CPU = "generic";
    auto Features = "";

    llvm::TargetOptions opt;
    auto RM = llvm::Reloc::Model();
    auto TheTargetMachine =
            Target->createTargetMachine(TargetTriple, CPU, Features, opt, RM);

    theModule->setDataLayout(TheTargetMachine->createDataLayout());

    std::error_code EC;
    llvm::raw_fd_ostream dest(filename, EC, llvm::sys::fs::F_None);

    if (EC) {
        llvm::errs() << "Could not open file: " << EC.message();
        return;
    }

    llvm::legacy::PassManager pass;
    auto FileType = llvm::TargetMachine::CGFT_ObjectFile;

    if (TheTargetMachine->addPassesToEmitFile(pass, dest, FileType)) {
        llvm::errs() << "TheTargetMachine can't emit a file of this type";
        return;
    }

    pass.run(*theModule);
    dest.flush();


}
