#include <memory>
#include <algorithm>
#include <iostream>
#include "lexer.h"
#include "ast.h"

class PJPParser {
    int curTok;
    PJPLexer m_Lexer;
    std::string currFunct;
	std::unique_ptr<ProtoAST> proto;
	std::string program;
public:
	std::string & getName() {return program;}

    PJPParser(std::fstream * is) : m_Lexer(PJPLexer(is)) { currFunct = "main"; }

    std::unique_ptr<ProgramAST> ParseProgram();

    std::unique_ptr<StatmListAST> ParseBody(std::unique_ptr<StatmListAST> list);

    std::unique_ptr<StatmListAST> ParseProgMain(std::unique_ptr<StatmListAST>);

	std::unique_ptr<StatmListAST> ParseProgMain1(std::unique_ptr<StatmListAST>); 

	std::unique_ptr<StatmListAST> ParseProgMain2(std::unique_ptr<StatmListAST>, std::unique_ptr<StatmListAST>);

	std::unique_ptr<StatmAST> ParseFunctProto();

    std::unique_ptr<StatmListAST> ParseFunctBody(std::unique_ptr<StatmListAST>);  

    std::vector<std::pair<std::string, VarType>> ParseArg();

    std::vector<std::pair<std::string, VarType>> ParseArgID(std::vector<std::pair<std::string, VarType>> &);
	 
    std::unique_ptr<StatmListAST> ParseMain(std::unique_ptr<StatmListAST> list);

    std::unique_ptr<StatmListAST> ParseRest(std::unique_ptr<StatmListAST> list);

	std::unique_ptr<StatmListAST> ParseDeclare();

    std::unique_ptr<StatmListAST> ParseArgVar(std::unique_ptr<StatmListAST> list);
	std::vector<std::string> ParseArgVar1();
	std::vector<std::string> ParseArgVar2(std::vector<std::string>&);

    std::unique_ptr<StatmListAST> ParseArgConst(std::unique_ptr<StatmListAST> list);
	
	std::unique_ptr<StatmAST> ParseCommand();
	std::unique_ptr<StatmAST> ParseCommand1(std::string &);
	std::unique_ptr<StatmListAST> ParseCommand2();
	std::unique_ptr<StatmListAST> ParseCommList();
	std::unique_ptr<StatmAST> ParseFor(std::unique_ptr<VarRefAST> var, std::unique_ptr<ExprAST> expr);
    std::unique_ptr<ExprAST> ParseLogExpr();
	std::unique_ptr<ExprAST> ParseLogExprRest(std::unique_ptr<ExprAST>);
	std::unique_ptr<ExprAST> ParseCond();
	std::unique_ptr<ExprAST> ParseCondRest(std::unique_ptr<ExprAST>);
	char ParseROp();

	std::unique_ptr<ExprAST> ParseExpression();

    std::unique_ptr<ExprAST> ParseExprRest(std::unique_ptr<ExprAST> term);

    std::unique_ptr<ExprAST> ParseTerm();

    std::unique_ptr<ExprAST> ParseTermRest(std::unique_ptr<ExprAST> factor);

    std::unique_ptr<ExprAST> ParseFactor();

    std::unique_ptr<ExprAST> ParseFactorRest(std::string &id);

    std::vector<std::unique_ptr<ExprAST>> ParseArgCall();

    std::vector<std::unique_ptr<ExprAST>> ParseArgCall1(std::vector<std::unique_ptr<ExprAST>> &);
    
	VarType ParseType();
	int ParseNum();
	void ParseForward();
	std::unique_ptr<ExprAST> ParseIndex (bool & array);
   
    int getNextToken() {
        return curTok = m_Lexer.getTok();
    }

    void MatchError(Token token);

    void ExpandError(std::string nonTerminal, Token token);


};
