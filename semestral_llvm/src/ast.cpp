#include "ast.h"

static bool exitInBlock = false;
static bool breakInBlock = false;
static llvm::BasicBlock * whereToBreak;
static llvm::BasicBlock * returnBlock;

static llvm::AllocaInst *CreateEntryBlockAlloca(llvm::Function * theFunction,
			const std::string &VarName, llvm::Type * type) {
    llvm::IRBuilder<> TmpB(&theFunction->getEntryBlock(), 
			theFunction->getEntryBlock().begin());
    return TmpB.CreateAlloca(type,  nullptr, VarName.c_str());
}

llvm::Value *NumbExprAST::codegen(PJPCodegen &codegen) {
    std::cout << "Creating integer: " << m_Val << std::endl;
    return llvm::ConstantInt::get(llvm::Type::getInt64Ty(codegen.theContext), m_Val, true);
}


llvm::Value *VarAST::codegen(PJPCodegen &codegen) {
    std::cout << "Creating identifier reference: " << m_Name << std::endl;
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

llvm::Value *ArrayContentAST::codegen(PJPCodegen &codegen) {
    std::cout << "Creating array item reference: " << m_Name << std::endl;
    llvm::Value *V;
    if (m_Funct == "main") {
        if (codegen.globalVars.find(m_Name) != codegen.globalVars.end()) {
	    std::vector<llvm::Value*> indices;
	    indices.push_back(llvm::ConstantInt::get(llvm::Type::getInt64Ty(codegen.theContext), 0, true));
	    indices.push_back(codegen.builder.CreateSub(m_Index->codegen(codegen), codegen.globalMin[m_Name]));
            return codegen.builder.CreateLoad(codegen.builder.CreateGEP(codegen.globalVars[m_Name], indices));
        }
        if (!V)
            return codegen.logError("Unknown array content");
        return V;
    } else {
        if (codegen.localVars[m_Funct].find(m_Name) != codegen.localVars[m_Funct].end()) {
             std::vector<llvm::Value*> indices;
	    indices.push_back(llvm::ConstantInt::get(llvm::Type::getInt64Ty(codegen.theContext), 0, true));
	    indices.push_back(codegen.builder.CreateSub(m_Index->codegen(codegen), codegen.localMin[m_Funct][m_Name]));
            return codegen.builder.CreateLoad(codegen.builder.CreateGEP(codegen.localVars[m_Funct][m_Name], indices));
        }
        if (codegen.globalVars.find(m_Name) != codegen.globalVars.end()) {
            std::vector<llvm::Value*> indices;
	    indices.push_back(llvm::ConstantInt::get(llvm::Type::getInt64Ty(codegen.theContext), 0, true));
	    indices.push_back(codegen.builder.CreateSub(m_Index->codegen(codegen), codegen.globalMin[m_Name]));
             return codegen.builder.CreateLoad(codegen.builder.CreateGEP(codegen.globalVars[m_Name], indices));
        }
        if (!V)
            return codegen.logError("Unknown array content");
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

llvm::Value *ArrayContentRefAST::codegen(PJPCodegen &codegen){
    std::cout << "Creating array item pointer: " << m_Name << std::endl;
    llvm::Value *V;
    if (m_Funct == "main") {
        if (codegen.globalVars.find(m_Name) != codegen.globalVars.end()) {
	    std::vector<llvm::Value*> indices;
	    indices.push_back(llvm::ConstantInt::get(llvm::Type::getInt64Ty(codegen.theContext), 0, true));
	    indices.push_back(codegen.builder.CreateSub(m_Index->codegen(codegen), codegen.globalMin[m_Name]));
            return codegen.builder.CreateGEP(codegen.globalVars[m_Name], indices);
        }
        if (!V)
            return codegen.logError("Unknown array content pointer");
        return V;
    } else {
        if (codegen.localVars[m_Funct].find(m_Name) != codegen.localVars[m_Funct].end()) {
            std::vector<llvm::Value*> indices;
	    indices.push_back(llvm::ConstantInt::get(llvm::Type::getInt64Ty(codegen.theContext), 0, true));
	    indices.push_back(codegen.builder.CreateSub(m_Index->codegen(codegen), codegen.localMin[m_Funct][m_Name]));
            return codegen.builder.CreateGEP(codegen.localVars[m_Funct][m_Name], indices);
        }
        if (codegen.globalVars.find(m_Name) != codegen.globalVars.end()) {
            std::vector<llvm::Value*> indices;
	    indices.push_back(llvm::ConstantInt::get(llvm::Type::getInt64Ty(codegen.theContext), 0, true));
	    indices.push_back(codegen.builder.CreateSub(m_Index->codegen(codegen), codegen.globalMin[m_Name]));
             return codegen.builder.CreateGEP(codegen.globalVars[m_Name], indices);
        }
        if (!V)
            return codegen.logError("Unknown array content pointer");
        return V;

    }
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

llvm::Value *ExitAST::codegen(PJPCodegen &codegen) {
    std::cout << "Generating exit" << std::endl;
     exitInBlock = true;
     breakInBlock = true;
     return codegen.builder.CreateBr(returnBlock);
   
}

llvm::Value *BreakAST::codegen(PJPCodegen &codegen) {
	breakInBlock = true;
	exitInBlock = false;
	if (whereToBreak) {
  		auto res = codegen.builder.CreateBr(whereToBreak);
		whereToBreak = nullptr;
		return res;
	}
	else{
		std::cerr << "Break outside the loop" << std::endl;
		return nullptr;
	}
	
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
	case '!':
 	    return codegen.builder.CreateICmpNE(L, R, "neqtmp");
	case '=':
 	    return codegen.builder.CreateICmpEQ(L, R, "eqtmp");
	case '>':
 	    return codegen.builder.CreateICmpSGT(L, R, "gttmp");
	case '<':
 	    return codegen.builder.CreateICmpSLT(L, R, "lttmp");
	case '}':
	    return codegen.builder.CreateICmpSGE(L, R, "getmp");
	case '{':
	    return codegen.builder.CreateICmpSLE(L, R, "letmp");
	case '&':
	    return codegen.builder.CreateAnd(L, R, "andtmp");
	case '|':
	    return codegen.builder.CreateOr(L, R, "ortmp");
	case '~':
	    return codegen.builder.CreateNeg(L, "negtmp");
        default:
	    std::cout << m_Op <<std::endl;
            return codegen.logError("invalid binary operator");
    }
}

llvm::Value *AssignAST::codegen(PJPCodegen &codegen) {
    std::cout << "Creating assignment for var: " << m_Var->getName() << std::endl;
        return codegen.builder.CreateStore(m_Expr->codegen(codegen),
                                           m_Var->codegen(codegen), false);
}

llvm::Value *StatmListAST::codegen(PJPCodegen &codegen) {
    llvm::Value *last = NULL;
    for (auto it = m_StatmList.begin(); it != m_StatmList.end(); it++) {
        //if(exitInBlock) break;
	last = (*it)->codegen(codegen);
    }
    std::cout << "Creating block" << std::endl;
    return last;
}

llvm::Value *ProgramAST::codegen(PJPCodegen &codegen) {
    llvm::Value *last = NULL;
    llvm::BasicBlock* returnBB = llvm::BasicBlock::Create(codegen.theContext, "returnBB");
    llvm::Function *F = codegen.builder.GetInsertBlock()->getParent();
    returnBlock = returnBB;
    std::cout << "Generating code for program " << m_Name << std::endl;
    
    last = m_List->codegen(codegen);
    codegen.builder.CreateBr(returnBB);
    F->getBasicBlockList().push_back(returnBB);
    codegen.builder.SetInsertPoint(returnBB);
	
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
	(codegen.theModule)->getOrInsertGlobal(m_Name, llvm::Type::getInt64Ty(codegen.theContext));
	llvm::GlobalVariable* gVar = (codegen.theModule)->getNamedGlobal(m_Name);
	gVar->setAlignment(8);
	gVar->setInitializer(llvm::ConstantInt::get(llvm::Type::getInt64Ty(codegen.theContext), 0, true));
        /*auto alloc = codegen.builder.CreateAlloca(llvm::Type::getInt64Ty(codegen.theContext), nullptr, m_Name.c_str());*/
        codegen.globalVars[m_Name] = gVar;

        return gVar;
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

llvm::Value *ArrayDeclAST::codegen(PJPCodegen &codegen) {
    std::cout << "Creating array declaration " << m_Name << std::endl;
    if (m_Funct == "main") {
        if (codegen.globalVars.find(m_Name) != codegen.globalVars.end()) {
            std::cout << "Double declaration" << m_Name << std::endl;
            return nullptr;
        }
        codegen.globalMin[m_Name] = llvm::ConstantInt::get(llvm::Type::getInt64Ty(codegen.theContext), m_Min, true);
	(codegen.theModule)->getOrInsertGlobal(m_Name, llvm::ArrayType::get(llvm::Type::getInt64Ty(codegen.theContext), m_Max-m_Min+1));
	llvm::GlobalVariable* gVar = (codegen.theModule)->getNamedGlobal(m_Name);
	gVar->setAlignment(16);
	gVar->setInitializer(llvm::ConstantArray::get(llvm::ArrayType::get(llvm::Type::getInt64Ty(codegen.theContext), m_Max-m_Min+1), llvm::ConstantInt::get(llvm::Type::getInt64Ty(codegen.theContext), 0)));
        codegen.globalVars[m_Name] = gVar;
        return gVar;
    } else {
        if (codegen.localVars[m_Funct].find(m_Name) != codegen.localVars[m_Funct].end()) {
            std::cout << "Double declaration" << m_Name << std::endl;
            return nullptr;
        }
	codegen.localMin[m_Funct][m_Name] = llvm::ConstantInt::get(llvm::Type::getInt64Ty(codegen.theContext), m_Min, true);
        auto alloc = codegen.builder.CreateAlloca(llvm::ArrayType::get(llvm::Type::getInt64Ty(codegen.theContext), m_Max-m_Min+1), nullptr, m_Name.c_str());
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
        if (m_Arguments[i].second.integer) params.push_back(llvm::Type::getInt64Ty(codegen.theContext));
	else params.push_back(llvm::ArrayType::get(llvm::Type::getInt64Ty(codegen.theContext), m_Arguments[i].second.max-m_Arguments[i].second.min+1));
    }
    llvm::Type *type;
    if (retVal) type = llvm::Type::getInt64Ty(codegen.theContext);
    else type = llvm::Type::getVoidTy(codegen.theContext);
    llvm::FunctionType *FT = llvm::FunctionType::get(type, params, false);
    llvm::Function *F = llvm::Function::Create(FT, llvm::Function::ExternalLinkage, m_Name, codegen.theModule.get());
    unsigned i = 0;
    for (auto &par : F->args()) {
        par.setName(m_Arguments[i++].first);
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
    llvm::BasicBlock* returnBB = llvm::BasicBlock::Create(codegen.theContext, "returnBB");
    auto oldReturnBlock = returnBlock;
    returnBlock = returnBB;
    
    auto mainBlock = codegen.builder.GetInsertBlock();
    codegen.builder.SetInsertPoint(BB);
    if (m_Proto->retVal) codegen.localVars[m_Proto->m_Name][m_Proto->m_Name] =  CreateEntryBlockAlloca(F, m_Proto->m_Name, llvm::Type::getInt64Ty(codegen.theContext));
    for (auto &arg : F->args()) {
        // Create an alloca for this variable.
        auto alloc = CreateEntryBlockAlloca(F, arg.getName(), arg.getType());
        // Store the initial value into the alloca.
        codegen.builder.CreateStore(&arg, alloc);
        // Add arguments to variable symbol table.
        codegen.localVars[m_Proto->m_Name][arg.getName()] = alloc;
    }
    exitInBlock = false;
    (this->m_Body)->codegen(codegen);
    codegen.builder.CreateBr(returnBB);
    F->getBasicBlockList().push_back(returnBB);
    codegen.builder.SetInsertPoint(returnBB);
    if (m_Proto->retVal) codegen.builder.CreateRet(codegen.builder.CreateLoad(codegen.localVars[m_Proto->m_Name][m_Proto->m_Name])); 
    else  codegen.builder.CreateRetVoid();
    codegen.builder.SetInsertPoint(mainBlock);
    exitInBlock = false;
    breakInBlock = false;
    returnBlock = oldReturnBlock;
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
    exitInBlock = false;

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
    exitInBlock = false;
    std::cout << "Creating method call:" << m_Name << std::endl;
    return codegen.builder.CreateCall(function, args);
}

llvm::Value * IfAST::codegen(PJPCodegen &codegen){
    llvm::Value* condV = Cond->codegen(codegen);
    if (!condV) return nullptr;
    
    llvm::Function *F = codegen.builder.GetInsertBlock()->getParent();
    llvm::BasicBlock *thenBB = llvm::BasicBlock::Create(codegen.theContext, "then", F);
    llvm::BasicBlock *elseBB = llvm::BasicBlock::Create(codegen.theContext, "else");
    llvm::BasicBlock *mergeBB = llvm::BasicBlock::Create(codegen.theContext, "ifcont");
    codegen.builder.CreateCondBr(condV, thenBB, elseBB);
    
    codegen.builder.SetInsertPoint(thenBB);
    breakInBlock = false;
    llvm::Value* thenV = Then->codegen(codegen);
    if (!thenV) return nullptr;
    if (!breakInBlock) codegen.builder.CreateBr(mergeBB);
    thenBB = codegen.builder.GetInsertBlock();
    F->getBasicBlockList().push_back(elseBB);
   
    codegen.builder.SetInsertPoint(elseBB);
    breakInBlock = false;
    Else->codegen(codegen);
    //if (!elseV) 
    if (!breakInBlock) codegen.builder.CreateBr(mergeBB);
    elseBB = codegen.builder.GetInsertBlock();
    F->getBasicBlockList().push_back(mergeBB);
    codegen.builder.SetInsertPoint(mergeBB);
    /*llvm::PHINode *PN = codegen.builder.CreatePHI(llvm::Type::getInt64Ty(codegen.theContext), 2, "iftmp");
    PN->addIncoming(thenV, thenBB);
    PN->addIncoming(elseV, elseBB);*/
    breakInBlock = false;
    exitInBlock = false;
    return nullptr;
}

llvm::Value * WhileAST::codegen(PJPCodegen &codegen){
	llvm::Function* f = codegen.builder.GetInsertBlock()->getParent();
	llvm::BasicBlock* loopBB = llvm::BasicBlock::Create(codegen.theContext, "whileLoop");
	llvm::BasicBlock* loopCondBB = llvm::BasicBlock::Create(codegen.theContext, "whileCond", f);
	llvm::BasicBlock* afterLoopBB = llvm::BasicBlock::Create(codegen.theContext, "afterWhileLoop");
	codegen.builder.CreateBr(loopCondBB);
	codegen.builder.SetInsertPoint(loopCondBB);
	llvm::Value* cond = Cond->codegen(codegen);
	codegen.builder.CreateCondBr(cond, loopBB, afterLoopBB);
	f->getBasicBlockList().push_back(loopBB);
	codegen.builder.SetInsertPoint(loopBB);
	llvm::BasicBlock *oldBreakPoint = whereToBreak;
	whereToBreak = afterLoopBB;
	breakInBlock = false;
	List->codegen(codegen);
	whereToBreak = oldBreakPoint;
	if (!breakInBlock) codegen.builder.CreateBr(loopCondBB);
	f->getBasicBlockList().push_back(afterLoopBB);
	codegen.builder.SetInsertPoint(afterLoopBB);
	breakInBlock = false;
	exitInBlock = false;
	return nullptr;
}

llvm::Value *ForAST::codegen(PJPCodegen &codegen){
	llvm::Function *f = codegen.builder.GetInsertBlock()->getParent();
	std::string name = f->getName();
	llvm::AllocaInst * Alloca = CreateEntryBlockAlloca(f, m_Var->m_Name, llvm::Type::getInt64Ty(codegen.theContext));
	// Emit the start code first, without 'variable' in scope.
	llvm::Value *StartVal = m_Start->codegen(codegen);
	if (!StartVal) {
		return nullptr;
	}
	/*
	auto init = llvm::make_unique<AssignAST>(std::move(m_Var), std::move(m_Start));
	init->codegen(codegen);*/
	codegen.builder.CreateStore(StartVal, Alloca);
	// Make the new basic block for the loop header, inserting after current
	// block.
	llvm::BasicBlock *LoopBB = llvm::BasicBlock::Create(codegen.theContext, "forLoop", f);
	llvm::BasicBlock *AfterLoopBB = llvm::BasicBlock::Create(codegen.theContext, "afterForLoop");
	// Insert an explicit fall through from the current block to the LoopBB.
	codegen.builder.CreateBr(LoopBB);

	// Start insertion in LoopBB.
	codegen.builder.SetInsertPoint(LoopBB);
        llvm::Value *OldVal;
	if (name == "main") {
		OldVal = codegen.globalVars[m_Var->m_Name];
		codegen.globalVars[m_Var->m_Name] = (llvm::GlobalVariable*) Alloca;
	}
	else {
   	        OldVal = codegen.localVars[name][m_Var->m_Name];
		codegen.localVars[name][m_Var->m_Name] = Alloca;
	}
	
	llvm::BasicBlock * oldBreakPoint = whereToBreak;
	whereToBreak = AfterLoopBB;
	m_Body->codegen(codegen);
	whereToBreak = oldBreakPoint;
	// Emit the step value.
	llvm::Value *StepVal = nullptr;
	if (m_Step) {
		StepVal = m_Step->codegen(codegen);
		if (!StepVal)
			return nullptr;
	} else {
		// If not specified, use.
		StepVal = llvm::ConstantInt::get(codegen.theContext, llvm::APInt(64, 1, true));
	}

	// Compute the end condition.
	llvm::Value *EndCond = m_End->codegen(codegen);
	if (!EndCond) return nullptr;
        llvm::Value *CurVar = codegen.builder.CreateLoad(Alloca, (m_Var->m_Name).c_str());
	llvm::Value *NextVar = codegen.builder.CreateAdd(CurVar, StepVal, "nextvar");
        codegen.builder.CreateStore(NextVar, Alloca);
	auto condEnd = codegen.builder.CreateICmpNE(CurVar, EndCond, "eqtmp");
	// Insert the conditional branch into the end of LoopEndBB.
	codegen.builder.CreateCondBr(condEnd, LoopBB, AfterLoopBB);
	f->getBasicBlockList().push_back(AfterLoopBB);

	codegen.builder.SetInsertPoint(AfterLoopBB);
	breakInBlock = false;
	exitInBlock = false;
	// Restore the unshadowed variable.
	if (OldVal){
            if (name == "main") codegen.globalVars[m_Var->m_Name] = (llvm::GlobalVariable*) OldVal;
	    else codegen.localVars[name][m_Var->m_Name] = (llvm::AllocaInst*) OldVal;
	}
	else{
	    if (name == "main") codegen.globalVars.erase(m_Var->m_Name);
	    else codegen.localVars[name].erase(m_Var->m_Name);
	}
	return nullptr;
}


