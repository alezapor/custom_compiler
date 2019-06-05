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
public:
    PJPParser() : m_Lexer(PJPLexer()) { currFunct = "main"; }

    std::unique_ptr<ProgramAST> ParseProgram();

    std::unique_ptr<StatmListAST> ParseBody(std::unique_ptr<StatmListAST> list);

    std::unique_ptr<StatmListAST> ParseProgMain(std::unique_ptr<StatmListAST>);

	std::unique_ptr<StatmListAST> ParseProgMain1(std::unique_ptr<StatmListAST>); 

	std::unique_ptr<StatmListAST> ParseProgMain2(std::unique_ptr<StatmListAST>, std::unique_ptr<StatmListAST>);

	std::unique_ptr<StatmAST> ParseFunctProto();

    std::unique_ptr<StatmListAST> ParseFunctBody(std::unique_ptr<StatmListAST>);  

    std::vector<std::string> ParseArg();

    std::vector<std::string> ParseArgID(std::vector<std::string> &);
	 
    std::unique_ptr<StatmListAST> ParseMain(std::unique_ptr<StatmListAST> list);

    std::unique_ptr<StatmListAST> ParseRest(std::unique_ptr<StatmListAST> list);

	std::unique_ptr<StatmListAST> ParseDeclare();

    std::unique_ptr<StatmListAST> ParseArgVar(std::unique_ptr<StatmListAST> list);
	std::unique_ptr<StatmListAST> ParseArgVar1();
	std::unique_ptr<StatmListAST> ParseArgVar2(std::unique_ptr<StatmListAST> list);

    std::unique_ptr<StatmListAST> ParseArgConst(std::unique_ptr<StatmListAST> list);
	
	std::unique_ptr<StatmAST> ParseCommand();
	std::unique_ptr<StatmAST> ParseCommand1(std::string &);
	std::unique_ptr<StatmListAST> ParseCommand2();
	std::unique_ptr<StatmListAST> ParseCommList();
	std::unique_ptr<StatmAST> ParseFor(std::string id, std::unique_ptr<ExprAST> expr);
	std::unique_ptr<ExprAST> ParseCond();
	char ParseROp();

	std::unique_ptr<ExprAST> ParseExpression();

    std::unique_ptr<ExprAST> ParseExprRest(std::unique_ptr<ExprAST> term);

    std::unique_ptr<ExprAST> ParseTerm();

    std::unique_ptr<ExprAST> ParseTermRest(std::unique_ptr<ExprAST> factor);

    std::unique_ptr<ExprAST> ParseFactor();

    std::unique_ptr<ExprAST> ParseFactorRest(std::string &id);

    std::vector<std::unique_ptr<ExprAST>> ParseArgCall();

    std::vector<std::unique_ptr<ExprAST>> ParseArgCall1(std::vector<std::unique_ptr<ExprAST>> &);

    
    int getNextToken() {
        return curTok = m_Lexer.getTok();
    }

    void MatchError(Token token);

    void ExpandError(std::string nonTerminal, Token token);


};
