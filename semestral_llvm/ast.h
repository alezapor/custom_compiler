#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include <llvm/IR/Constants.h>
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Verifier.h"
#include "codegen.h"
#include <utility>

struct VarType {
		bool integer;
		int min;
		int max;
	};

// NodeAST - Base class for all nodes.
class NodeAST {
public:
    virtual ~NodeAST() {}

    virtual llvm::Value *codegen(PJPCodegen &cg) = 0;

};

// ExprAST - Base class for all expression nodes.
class ExprAST : public NodeAST {
public:
	virtual void currFunct(std::string funct) = 0;
};

// StatmAST - Base class for all statements and its derivates.
class StatmAST : public NodeAST {
public:
	virtual void currFunct(std::string funct) = 0;
};


// NumbExprAST - Expression class for numeric literals like "12".
class NumbExprAST : public ExprAST {
    int m_Val;
public:
    NumbExprAST(int val) : m_Val(val) {}
	void currFunct(std::string funct){}
    virtual llvm::Value *codegen(PJPCodegen &cg);
};

// VarAST - Class for variable reference
class VarAST : public ExprAST {
public:
    std::string m_Name;
    std::string m_Funct;
    VarAST(std::string name, std::string funct) : m_Name(name), m_Funct(funct) {}

    virtual llvm::Value *codegen(PJPCodegen &cg);
	void currFunct(std::string funct) {m_Funct = funct;}
    const std::string &getName() { return m_Name; }

    const std::string &getFunct() { return m_Funct; }
};

class ArrayContentAST : public VarAST {
	std::unique_ptr<ExprAST> m_Index;
public:
    ArrayContentAST(std::string name, std::string funct, std::unique_ptr<ExprAST> index ) : VarAST(name, funct), m_Index(std::move(index)) {}

    virtual llvm::Value *codegen(PJPCodegen &cg);
};



// VarAST - Class for variable pointer
class VarRefAST : public ExprAST {
public:
    std::string m_Name;
    std::string m_Funct;
    VarRefAST(std::string name, std::string funct) : m_Name(name), m_Funct(funct) {}
	
    virtual llvm::Value *codegen(PJPCodegen &cg);
	void currFunct(std::string funct) {m_Funct = funct;}
    const std::string &getName() { return m_Name; }

    const std::string &getFunct() { return m_Funct; }
};


class ArrayContentRefAST : public VarRefAST {
    std::unique_ptr<ExprAST> m_Index;
public:
    ArrayContentRefAST(std::string name, std::string funct,  std::unique_ptr<ExprAST> ind) : VarRefAST(name, funct), m_Index(std::move(ind)) {}
	
    virtual llvm::Value *codegen(PJPCodegen &cg);
};


// BinaryExprAST - Expression class for a binary operator.
class BinaryExprAST : public ExprAST {
    char m_Op;
    std::unique_ptr<ExprAST> m_LHS, m_RHS;
public:
    BinaryExprAST(char op, std::unique_ptr<ExprAST> LHS,
                  std::unique_ptr<ExprAST> RHS)
            : m_Op(op), m_LHS(std::move(LHS)), m_RHS(std::move(RHS)) {}
	void currFunct(std::string funct) {
		m_LHS->currFunct(funct);
		m_RHS->currFunct(funct);
	}
		
    virtual llvm::Value *codegen(PJPCodegen &cg);
};


// AssignAST - Class for all assignment statements
class AssignAST : public StatmAST {
    std::unique_ptr<VarRefAST> m_Var;
    std::unique_ptr<ExprAST> m_Expr;
public:
    AssignAST(std::unique_ptr<VarRefAST> var, std::unique_ptr<ExprAST> expr) :
            m_Var(std::move(var)), m_Expr(std::move(expr)) {}
	void currFunct(std::string funct) {
		m_Var->currFunct(funct);
		m_Expr->currFunct(funct);
	}
    virtual llvm::Value *codegen(PJPCodegen &cg);
};

// WritelnAST - Class for all output commands
class WritelnAST : public StatmAST {
    std::unique_ptr<ExprAST> m_Expr;
public:
    WritelnAST(std::unique_ptr<ExprAST> expr) : m_Expr(std::move(expr)) {}

    virtual llvm::Value *codegen(PJPCodegen &cg);
	void currFunct(std::string funct) {
		m_Expr->currFunct(funct);
	}
};

// ReadlnAST - Class for all input commands
class ReadlnAST : public StatmAST {
    std::unique_ptr<VarRefAST> m_Var;
public:
    ReadlnAST(std::unique_ptr<VarRefAST> var) : m_Var(std::move(var)) {}

    virtual llvm::Value *codegen(PJPCodegen &cg);
	void currFunct(std::string funct) {
		m_Var->currFunct(funct);
	}
};

// ExitAST - Class for all exit commands
class ExitAST : public StatmAST {
    std::string m_Funct;
	bool retVal;
public:
    ExitAST(std::string funct, bool type) : retVal(type) {m_Funct = funct;}

    virtual llvm::Value *codegen(PJPCodegen &cg);
	void currFunct(std::string funct) {
		m_Funct = funct;
		retVal = false;
	}
};

// BreakAST - Class for all break commands
class BreakAST : public StatmAST {
public:
    BreakAST() {}

    virtual llvm::Value *codegen(PJPCodegen &cg);
	void currFunct(std::string funct) {}
};


// StatmListAST - class for a block of statements
class StatmListAST : public StatmAST {
    std::vector<std::unique_ptr<StatmAST>> m_StatmList;
public:
    StatmListAST(std::vector<std::unique_ptr<StatmAST>> list) : m_StatmList(std::move(list)) {}
	  StatmListAST(){}
    virtual llvm::Value *codegen(PJPCodegen &cg);
	
	void currFunct(std::string funct) {
		for (auto i = m_StatmList.begin(); i != m_StatmList.end(); i++){
			(*i)->currFunct(funct);
		}
	}
    void addStatm(std::unique_ptr<StatmAST> statm) {
        m_StatmList.emplace_back(std::move(statm));
    }

    int length() { return m_StatmList.size(); }

    std::vector<std::unique_ptr<StatmAST>> &getList() { return m_StatmList; }

    void addStatmList(std::unique_ptr<StatmListAST> statmList) {
        //auto list = std::move(statmList->getList());
        for (unsigned i = 0 ; i < statmList->getList().size(); i++) m_StatmList.emplace_back(std::move(statmList->getList()[i]));
    }
};

// VarDeclAST - Class for a variable declaration
class VarDeclAST : public StatmAST {
    std::string m_Name;
    std::string m_Funct;
public:
    VarDeclAST(std::string name, std::string funct) : m_Name(name), m_Funct(funct) {}
	void currFunct(std::string funct) {
		m_Funct = funct;
	}
    const std::string &getName() { return m_Name; }

    const std::string &getFunct() { return m_Funct; }

    virtual llvm::Value *codegen(PJPCodegen &cg);
};

// ArrayDeclAST - Class for a variable declaration
class ArrayDeclAST : public StatmAST {
    std::string m_Name;
    std::string m_Funct;
	int m_Min;
	int m_Max;
public:
    ArrayDeclAST(std::string name, std::string funct, int min, int max) : m_Name(name), m_Funct(funct), m_Min(min), m_Max(max) {}
	void currFunct(std::string funct) {
		m_Funct = funct;
	}
    const std::string &getName() { return m_Name; }

    const std::string &getFunct() { return m_Funct; }

    virtual llvm::Value *codegen(PJPCodegen &cg);
};


class ConstDeclAST : public StatmAST {
    std::string m_Name;
    int m_Val;
    std::string m_Funct;
public:
    ConstDeclAST(std::string &name, int val, std::string funct) : m_Name(name), m_Val(val), m_Funct(funct) {}

	void currFunct(std::string funct) {
		m_Funct = funct;
	}

    const std::string &getName() { return m_Name; }

    const std::string &getFunct() { return m_Funct; }

    virtual llvm::Value *codegen(PJPCodegen &cg);
};


// ProgramAST - class for a program
class ProgramAST : public NodeAST {
    std::unique_ptr<StatmListAST> m_List;
    std::string m_Name;
public:
    ProgramAST(std::string name, std::unique_ptr<StatmListAST> list) :
            m_List(std::move(list)), m_Name(name) {}

    virtual llvm::Value *codegen(PJPCodegen &cg);
};

// ProtoAST - class for functions' and procedures' prototypes
class ProtoAST : public StatmAST {
public:
    std::string m_Name;
    std::vector<std::pair<std::string, VarType>> m_Arguments;
    bool retVal;

    ProtoAST(std::string name, bool retVal) : m_Name(name), retVal(retVal) {}
	void currFunct(std::string funct) {}
    void addArgument(std::string &arg, VarType type) { m_Arguments.push_back(make_pair(arg, type)); }

    virtual llvm::Value *codegen(PJPCodegen &cg);
};

// FunctionAST - class for functions and procedures
class FunctionAST : public StatmAST {
    std::unique_ptr<ProtoAST> m_Proto;
public:
	std::unique_ptr<StatmListAST> m_Body;
    FunctionAST(std::unique_ptr<ProtoAST> proto) : m_Proto(std::move(proto)) {}

    void addStatm(std::unique_ptr<StatmAST> statm) { m_Body->addStatm(std::move(statm)); }
	void currFunct(std::string funct) {}
	void setStatmList(std::unique_ptr<StatmListAST> statmList) {
		m_Body = std::move(statmList);
	}
    void addStatmList(std::unique_ptr<StatmListAST> statmList) {
        if (statmList && statmList->length() != 0) m_Body->addStatmList(std::move(statmList));
    }

    virtual llvm::Value *codegen(PJPCodegen &cg);
};

class FunctionCallExprAST : public ExprAST {
    std::string m_Name;
    std::vector<std::unique_ptr<ExprAST>> m_Arguments;
public:
    FunctionCallExprAST(std::string name, std::vector<std::unique_ptr<ExprAST>> args) : m_Name(name),
                                                                                    m_Arguments(std::move(args)) {}
	void currFunct(std::string funct) {
		for (auto i = m_Arguments.begin(); i != m_Arguments.end(); i++){
			(*i)->currFunct(funct);
		}	
	}
    virtual llvm::Value *codegen(PJPCodegen &cg);
};

class IfAST : public StatmAST {
	std::unique_ptr<ExprAST> Cond;
	std::unique_ptr<StatmListAST> Then, Else;
public:
	IfAST(std::unique_ptr<ExprAST> Cond, std::unique_ptr<StatmListAST> Then, std::unique_ptr<StatmListAST> Else) : Cond(std::move(Cond)), Then(std::move(Then)), Else(std::move(Else)) {}
	void currFunct(std::string funct) {
		Then->currFunct(funct);
		Else->currFunct(funct);
	}
	virtual llvm::Value *codegen(PJPCodegen &cg);
};

class WhileAST : public StatmAST {
	std::unique_ptr<ExprAST> Cond;
	std::unique_ptr<StatmListAST> List;
public:
	WhileAST(std::unique_ptr<ExprAST> Cond, std::unique_ptr<StatmListAST> List) : Cond(std::move(Cond)), List(std::move(List)) {}
	void currFunct(std::string funct) {
		List->currFunct(funct);
	}
	virtual llvm::Value *codegen(PJPCodegen &cg);
};

class ForAST : public StatmAST {
	std::unique_ptr<VarRefAST> m_Var;
	std::unique_ptr<ExprAST> m_Start, m_End, m_Step;
	std::unique_ptr<StatmListAST> m_Body;
public:
	ForAST(std::unique_ptr<VarRefAST> v, std::unique_ptr<ExprAST> st, std::unique_ptr<ExprAST> by, std::unique_ptr<ExprAST> end, std::unique_ptr<StatmListAST> bd) : m_Var(std::move(v)), m_Start(std::move(st)), m_End(std::move(end)), m_Step(std::move(by)), m_Body(std::move(bd)) {}
    void currFunct(std::string funct) {
		m_Body->currFunct(funct);
	}
	virtual llvm::Value *codegen(PJPCodegen &cg);
};


class FunctionCallAST : public StatmAST {
    std::string m_Name;
    std::vector<std::unique_ptr<ExprAST>> m_Arguments;
public:
    FunctionCallAST(std::string name, std::vector<std::unique_ptr<ExprAST>> args) : m_Name(name),
                                                                                    m_Arguments(std::move(args)) {}
	void currFunct(std::string funct) {
		for (auto i = m_Arguments.begin(); i != m_Arguments.end(); i++){
			(*i)->currFunct(funct);
		}	
	}
    virtual llvm::Value *codegen(PJPCodegen &cg);
};


