#include <map>
#include <iostream>
#include <memory>
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include <llvm/IR/Constants.h>
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include <llvm/Support/TargetSelect.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/ExecutionEngine/GenericValue.h>
#include <llvm/IR/IRPrintingPasses.h>
#include <llvm/Bitcode/BitstreamReader.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Bitcode/BitstreamWriter.h>
#include "llvm/Support/TargetRegistry.h"


class PJPCodegen {
public:
    llvm::IRBuilder<> builder;
    llvm::LLVMContext theContext;
    std::unique_ptr<llvm::Module> theModule;
    std::map<std::string, llvm::GlobalValue *> globalVars;
    std::map<std::string, llvm::Value *> globalConst;
    std::map<std::string, std::map<std::string, llvm::AllocaInst *>> localVars;
	std::map<std::string, llvm::Value *> globalMin;
	std::map<std::string, std::map<std::string, llvm::Value *>> localMin;
    std::map<std::string, std::map<std::string, llvm::Value *>> localConst;
    llvm::Function *mainFunction;/*
static BasicBlock * mainFunctBlock;*/
    llvm::Value *printFormat;
    llvm::Value *scanFormat;

    llvm::Value *logError(const char *Str) {
        printf("Logic error: %s\n", Str);
        return nullptr;
    }

    PJPCodegen();

    void saveToObjectFile(const std::string &filename);
};
