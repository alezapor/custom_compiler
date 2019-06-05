#include "ast.h"


llvm::Value *NumbExprAST::codegen(PJPCodegen &codegen) {
    std::cout << "Creating integer: " << m_Val << std::endl;
    return llvm::ConstantInt::get(llvm::Type::getInt64Ty(codegen.theContext), m_Val, false);
}


llvm::Value *VarAST::codegen(PJPCodegen &codegen) {
    std::cout << "Creating identifier reference: " << m_Name << std::endl;
    // Look this variable up in the function.
    llvm::Value *V;
    if (m_Funct == "main") {
        if (codegen.globalConst.find(m_Name) != codegen.globalConst.end()) {
            return codegen.globalConst[m_Name];
        }
        if (codegen.globalVars.find(m_Name) != codegen.globalVars.end()) {
            return codegen.builder.CreateLoad(codegen.globalVars[m_Name], m_Name);
        }
        if (!V)
            return codegen.logError("Unknown variable name");
        return V;
    } else {
        if (codegen.localConst[m_Funct].find(m_Name) != codegen.localConst[m_Funct].end()) {
            return codegen.localConst[m_Funct][m_Name];
        }
        if (codegen.localVars[m_Funct].find(m_Name) != codegen.localVars[m_Funct].end()) {
            return codegen.builder.CreateLoad(codegen.localVars[m_Funct][m_Name], m_Name);
        }
        if (codegen.globalConst.find(m_Name) != codegen.globalConst.end()) {
            return codegen.globalConst[m_Name];
        }
        if (codegen.globalVars.find(m_Name) != codegen.globalVars.end()) {
            return codegen.builder.CreateLoad(codegen.globalVars[m_Name], m_Name);
        }
        if (!V)
            return codegen.logError("Unknown variable name");
        return V;

    }
}

llvm::Value *VarRefAST::codegen(PJPCodegen &codegen) {
    std::cout << "Creating identifier pointer: " << m_Name << std::endl;
    // Look this variable up in the function.
    llvm::Value *V;
    if (m_Funct == "main") V = codegen.globalVars[m_Name];
    else {
        V = codegen.localVars[m_Funct][m_Name];
        if (!V) V = codegen.globalVars[m_Name];
    }
    if (!V)
        return codegen.logError("Unknown variable name");
    return V;
}


llvm::Value *WritelnAST::codegen(PJPCodegen &codegen) {
    std::cout << "Generating writeln" << std::endl;
    llvm::Function *function = codegen.theModule->getFunction("printf");
    if (function == NULL) {
        std::cerr << "no such function" << std::endl;
    }
    llvm::Value *val = m_Expr->codegen(codegen);

    std::cout << "Creating method call: printf" << std::endl;
    return codegen.builder.CreateCall(function, {codegen.printFormat, val});
}


llvm::Value *ReadlnAST::codegen(PJPCodegen &codegen) {
    std::cout << "Generating readln" << std::endl;
    llvm::Function *function = codegen.theModule->getFunction("scanf");
    if (function == NULL) {
        std::cerr << "no such function" << std::endl;
    }
    llvm::Value *val = m_Var->codegen(codegen);

    std::cout << "Creating method call: scanf" << std::endl;
    return codegen.builder.CreateCall(function, {codegen.scanFormat, val});
}

llvm::Value *BinaryExprAST::codegen(PJPCodegen &codegen) {
    std::cout << "Creating binary operation " << m_Op << std::endl;
    llvm::Value *L = m_LHS->codegen(codegen);
    llvm::Value *R = m_RHS->codegen(codegen);
    if (!L || !R)
        return nullptr;

    switch (m_Op) {
        case '+':
            return codegen.builder.CreateAdd(L, R, "addtmp");
        case '-':
            return codegen.builder.CreateSub(L, R, "subtmp");
        case '*':
            return codegen.builder.CreateMul(L, R, "multmp");
        case '/':
            return codegen.builder.CreateSDiv(L, R, "divtmp");
        case '%':
            return codegen.builder.CreateSRem(L, R, "remtmp");
        default:
            return codegen.logError("invalid binary operator");
    }
}

llvm::Value *AssignAST::codegen(PJPCodegen &codegen) {
    std::cout << "Creating assignment for var: " << m_Var->getName() << std::endl;
    if (m_Var->getFunct() == "main") {
        if (codegen.globalVars.find(m_Var->getName()) == codegen.globalVars.end()) {
            std::cerr << "undeclared variable " << m_Var->getName() << std::endl;
            return NULL;
        }
        return codegen.builder.CreateStore(m_Expr->codegen(codegen), codegen.globalVars[m_Var->getName()], false);
    } else {
        if (codegen.localVars[m_Var->getFunct()].find(m_Var->getName()) == codegen.localVars[m_Var->getFunct()].end()) {
            if (codegen.globalVars.find(m_Var->getName()) == codegen.globalVars.end()) {
                std::cerr << "undeclared variable " << m_Var->getName() << std::endl;
                return NULL;
            }
            return codegen.builder.CreateStore(m_Expr->codegen(codegen), codegen.globalVars[m_Var->getName()], false);
        }
        return codegen.builder.CreateStore(m_Expr->codegen(codegen),
                                           codegen.localVars[m_Var->getFunct()][m_Var->getName()], false);
    }

}

llvm::Value *StatmListAST::codegen(PJPCodegen &codegen) {
    llvm::Value *last = NULL;
    for (auto it = m_StatmList.begin(); it != m_StatmList.end(); it++) {
        last = (*it)->codegen(codegen);
    }
    std::cout << "Creating block" << std::endl;
    return last;
}

llvm::Value *ProgramAST::codegen(PJPCodegen &codegen) {
    llvm::Value *last = NULL;

    std::cout << "Generating code for program " << m_Name << std::endl;
    last = m_List->codegen(codegen);
    codegen.builder.CreateRetVoid();
    return last;
}

llvm::Value *VarDeclAST::codegen(PJPCodegen &codegen) {
    std::cout << "Creating variable declaration int " << m_Name << std::endl;

    if (m_Funct == "main") {
        if (codegen.globalVars.find(m_Name) != codegen.globalVars.end()) {
            std::cout << "Double declaration" << m_Name << std::endl;
            return nullptr;
        }
        auto alloc = codegen.builder.CreateAlloca(llvm::Type::getInt64Ty(codegen.theContext), nullptr, m_Name.c_str());
        codegen.globalVars[m_Name] = alloc;

        return alloc;
    } else {
        if (codegen.localVars[m_Funct].find(m_Name) != codegen.localVars[m_Funct].end()) {
            std::cout << "Double declaration" << m_Name << std::endl;
            return nullptr;
        }
        auto alloc = codegen.builder.CreateAlloca(llvm::Type::getInt64Ty(codegen.theContext), nullptr, m_Name.c_str());
        codegen.localVars[m_Funct][m_Name] = alloc;

        return alloc;

    }
}

llvm::Value *ConstDeclAST::codegen(PJPCodegen &codegen) {
    std::cout << "Creating const declaration int " << m_Name << std::endl;
    if (m_Funct == "main") {
        if (codegen.globalConst.find(m_Name) != codegen.globalConst.end()) {
            std::cout << "Double declaration" << m_Name << std::endl;
            return nullptr;
        }
        auto tmp = llvm::ConstantInt::get(llvm::Type::getInt64Ty(codegen.theContext), m_Val, true);
        codegen.globalConst[m_Name] = tmp;

        return tmp;
    } else {
        if (codegen.localConst[m_Funct].find(m_Name) != codegen.localConst[m_Funct].end()) {
            std::cout << "Double declaration" << m_Name << std::endl;
            return nullptr;
        }
        auto tmp = llvm::ConstantInt::get(llvm::Type::getInt64Ty(codegen.theContext), m_Val, true);
        codegen.localConst[m_Funct][m_Name] = tmp;

        return tmp;

    }
}

llvm::Value *ProtoAST::codegen(PJPCodegen &codegen) {
    std::cout << "Function decl: " << m_Name << std::endl;
    std::vector < llvm::Type * > params;
    for (unsigned i = 0; i < m_Arguments.size(); i++) {
        std::cout << i << std::endl;
        params.push_back(llvm::Type::getInt64Ty(codegen.theContext));
    }
    llvm::Type *type;
    if (retVal) type = llvm::Type::getInt64Ty(codegen.theContext);
    else type = llvm::Type::getVoidTy(codegen.theContext);
    llvm::FunctionType *FT = llvm::FunctionType::get(type, params, false);
    llvm::Function *F = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, m_Name, codegen.theModule.get());
    int i = 0;
    for (auto &par : F->args()) {
        par.setName(m_Arguments[i++]);
    }
    return F;
}

llvm::Value *FunctionAST::codegen(PJPCodegen &codegen) {
    std::cout << "Function def: " << m_Proto->m_Name << std::endl;
    llvm::Function *F = codegen.theModule->getFunction(m_Proto->m_Name);
    if (!F) {
        m_Proto->codegen(codegen);
        F = codegen.theModule->getFunction(m_Proto->m_Name);
    }
    if (!F) return nullptr;
    if (!m_Body) return nullptr;

    llvm::BasicBlock *BB = llvm::BasicBlock::Create(codegen.theContext, m_Proto->m_Name+"block", F);
    auto mainBlock = codegen.builder.GetInsertBlock();
    codegen.builder.SetInsertPoint(BB);
    codegen.localVars[m_Proto->m_Name][m_Proto->m_Name] =  codegen.builder.CreateAlloca(llvm::Type::getInt64Ty(codegen.theContext), nullptr, m_Proto->m_Name);
    for (auto &arg : F->args()) {
        // Create an alloca for this variable.
        auto alloc = codegen.builder.CreateAlloca(arg.getType(), nullptr, arg.getName());
        // Store the initial value into the alloca.
        codegen.builder.CreateStore(&arg, alloc);
        // Add arguments to variable symbol table.
        codegen.localVars[m_Proto->m_Name][arg.getName()] = alloc;
    }
    (this->m_Body)->codegen(codegen);
    if (m_Proto->retVal) codegen.builder.CreateRet(codegen.builder.CreateLoad(codegen.localVars[m_Proto->m_Name][m_Proto->m_Name])); 
    else  codegen.builder.CreateRetVoid();
    codegen.builder.SetInsertPoint(mainBlock);
    return nullptr;
}

llvm::Value *FunctionCallAST::codegen(PJPCodegen &codegen) {
    std::cout << "Generating function call: " << m_Name << std::endl;
    llvm::Function *function = codegen.theModule->getFunction(m_Name);
    if (function == NULL) {
        std::cerr << "no such function" << std::endl;
    }
    std::vector < llvm::Value * > args;
    for (auto i = m_Arguments.begin(); i != m_Arguments.end(); i++) {
        auto res = (*i)->codegen(codegen);
        args.push_back(res);
    }

    std::cout << "Creating method call:" << m_Name << std::endl;
    return codegen.builder.CreateCall(function, args);
}

llvm::Value *FunctionCallExprAST::codegen(PJPCodegen &codegen) {
    std::cout << "Generating function call: " << m_Name << std::endl;
    llvm::Function *function = codegen.theModule->getFunction(m_Name);
    if (function == NULL) {
        std::cerr << "no such function" << std::endl;
    }
    std::vector < llvm::Value * > args;
    for (auto i = m_Arguments.begin(); i != m_Arguments.end(); i++) {
        auto res = (*i)->codegen(codegen);
        args.push_back(res);
    }

    std::cout << "Creating method call:" << m_Name << std::endl;
    return codegen.builder.CreateCall(function, args);
}
