#include "parser.h"

void PJPParser::MatchError(Token token) {
    printf("Match error: token %d.\n", token);
    exit(1);
}

void PJPParser::ExpandError(std::string nonTerminal, Token token) {
    std::cout << "Expand error: nonterminal " << nonTerminal << ", token: " << token << std::endl;
    exit(1);
}

// PR -> program id ; BODY
std::unique_ptr<ProgramAST> PJPParser::ParseProgram(){
    std::cout << "Program" << std::endl;
    if(curTok == tokenProgram){
        getNextToken();
        if (curTok != tokenIdentifier) {
            MatchError((Token) curTok);
            return nullptr;
        }
        std::string idProg = m_Lexer.identifierStr;
        getNextToken();
        if (curTok != ';') {
            MatchError((Token) curTok);
            return nullptr;
        }
        getNextToken();
        //std::vector<std::unique_ptr<StatmAST>> vec;
        auto newList = llvm::make_unique<StatmListAST>();
        return llvm::make_unique<ProgramAST>(idProg, std::move(ParseBody(std::move(newList))));
    }
    else {
        ExpandError("Program", (Token) curTok);
        return nullptr;
    }

}

// BODY -> PROGMAIN | DCL BODY
std::unique_ptr<StatmListAST> PJPParser::ParseBody(std::unique_ptr<StatmListAST> list){
    std::cout << "Body" << std::endl;
    if(curTok == tokenBegin || curTok == tokenProcedure ||
       curTok == tokenFunction){
        return std::move(ParseProgMain(std::move(list)));
    }
    else if (curTok == tokenVar || curTok == tokenConst) {
        list->addStatmList(std::move(ParseDeclare()));
        return std::move(ParseBody(std::move(list)));
    }
    else {
        ExpandError("Body", (Token) curTok);
        return nullptr;
    }

}

// PROGMAIN -> FUNCTPROTO PROGMAIN1 | MAIN 
std::unique_ptr<StatmListAST> PJPParser::ParseProgMain(std::unique_ptr<StatmListAST> list){
    std::cout << "ProgMain" << std::endl;
    if (curTok == tokenBegin){
        return std::move(ParseMain(std::move(list)));
    }
    else if (curTok == tokenProcedure || curTok == tokenFunction){
        ParseFunctProto();
        return std::move(ParseProgMain1(std::move(list)));

    }
    else {
        ExpandError("ProgMain", (Token) curTok);
        return nullptr;

    }
}

// PROGMAIN1 -> DCL FUNCTBODY PROGMAIN | begin REST PROGMAIN2 | FUNCTPROTO PROGMAIN1
std::unique_ptr<StatmListAST> PJPParser::ParseProgMain1(std::unique_ptr<StatmListAST> list){
    std::cout << "ProgMain1" << std::endl;
    if (curTok == tokenVar || curTok == tokenConst){

        auto newFunct = llvm::make_unique<FunctionAST>(std::move(proto));
	auto body = llvm::make_unique<StatmListAST>();
	newFunct->setStatmList(std::move(body));
        //std::vector<std::unique_ptr<StatmAST>> vec;
        auto newList = llvm::make_unique<StatmListAST>();
	(newFunct->m_Body)->addStatm(ParseDeclare());
        (newFunct->m_Body)->addStatmList(std::move(ParseFunctBody(std::move(newList))));
        list->addStatm(std::move(newFunct));
        return std::move(ParseProgMain(std::move(list)));
    }
    else if (curTok == tokenBegin){
        getNextToken();
        //std::vector<std::unique_ptr<StatmAST>> vec;
        auto newList = llvm::make_unique<StatmListAST>();
        auto statmList = std::move(ParseRest(std::move(newList)));
        return std::move(ParseProgMain2(std::move(list), std::move(statmList)));
    }
    else if (curTok == tokenProcedure || curTok == tokenFunction){
	list->addStatm(std::move(proto));
        ParseFunctProto();
        return std::move(ParseProgMain1(std::move(list)));
    }
    else {
        ExpandError("ProgMain1", (Token) curTok);
        return nullptr;

    }
}

// PROGMAIN2 -> . | ; PROGMAIN
std::unique_ptr<StatmListAST> PJPParser::ParseProgMain2(std::unique_ptr<StatmListAST> list, std::unique_ptr<StatmListAST> block){
    std::cout << "ProgMain2" << std::endl;
    if (curTok == ';'){
        getNextToken();
        auto newFunct = llvm::make_unique<FunctionAST>(std::move(proto));
        newFunct->setStatmList(std::move(block));
        list->addStatm(std::move(newFunct));
        currFunct = "main";
        return std::move(ParseProgMain(std::move(list)));
    }
    else if (curTok == '.') {
        //getNextToken();
        block->currFunct("main");
        currFunct = "main";
	list->addStatm(std::move(proto));
        list->addStatmList(std::move(block));
	return std::move(list);
    }
    else {
        ExpandError("ProgMain2", (Token) curTok);
    }
    return nullptr;
}

// FUNCTPROTO -> procedure id ( ARG ) ; | function id ( ARG ) : integer ;
std::unique_ptr<StatmAST> PJPParser::ParseFunctProto(){
    std::cout << "FunctProto" << std::endl;
    if (curTok == tokenProcedure){
        getNextToken();
        if (curTok != tokenIdentifier) {
            MatchError((Token) curTok);
            return nullptr;
        }
        std::string idFunct = m_Lexer.identifierStr;
        currFunct = idFunct;
        getNextToken();
        if (curTok != '(') {
            MatchError((Token) curTok);
            return nullptr;
        }
        getNextToken();
        std::vector<std::string> args = ParseArg();
        if (curTok != ')') {
            MatchError((Token) curTok);
            return nullptr;
        }
        getNextToken();
        if (curTok != ';') {
            MatchError((Token) curTok);
            return nullptr;
        }
        getNextToken();
        auto newProc = llvm::make_unique<ProtoAST>(idFunct, false);
        for (auto i = args.begin(); i!=args.end(); i++) newProc->addArgument(*i);
	proto = std::move(newProc);
        return nullptr;

    }
    else if (curTok == tokenFunction) {
        getNextToken();
        if (curTok != tokenIdentifier) {
            MatchError((Token) curTok);
            return nullptr;
        }
        std::string idFunct = m_Lexer.identifierStr;
        currFunct = idFunct;
        getNextToken();
        if (curTok != '(') {
            MatchError((Token) curTok);
            return nullptr;
        }
        getNextToken();
        std::vector<std::string> args = ParseArg();
        if (curTok != ')') {
            MatchError((Token) curTok);
            return nullptr;
        }
        getNextToken();
        if (curTok != ':') {
            MatchError((Token) curTok);
            return nullptr;
        }
        getNextToken();
        if (curTok != tokenInteger) {
            MatchError((Token) curTok);
            return nullptr;
        }
        getNextToken();
        if (curTok != ';') {
            MatchError((Token) curTok);
            return nullptr;
        }
        getNextToken();
        auto newFunct = llvm::make_unique<ProtoAST>(std::move(idFunct), true);
        for (auto i = args.begin(); i!=args.end(); i++) newFunct->addArgument(*i);
	proto = std::move(newFunct);
        return nullptr;

    }
    else {
        ExpandError("FunctProto", (Token) curTok);
        return nullptr;
    }
}

// FUNCTBODY -> DCL FUNCTBODY | begin REST ;
std::unique_ptr<StatmListAST> PJPParser::ParseFunctBody(std::unique_ptr<StatmListAST> list){
    std::cout << "FunctBody" << std::endl;
    switch (curTok) {
        case tokenVar:
        case tokenConst:
            list->addStatmList(std::move(ParseDeclare()));
            return std::move(ParseFunctBody(std::move(list)));
        case tokenBegin:
	{
            getNextToken();
            //std::vector<std::unique_ptr<StatmAST>> vec;
            auto newList = llvm::make_unique<StatmListAST>();
            auto statmList = std::move(ParseRest(std::move(newList)));
	    list->addStatmList(std::move(statmList));
            if (curTok != ';') {
                MatchError((Token) curTok);
                return nullptr;
            }
            getNextToken();
            currFunct = "main";
            return std::move(list);
	}
        default:
            ExpandError("FunctBody", (Token) curTok);
            return nullptr;
    }
}

// ARG -> id : integer ARGID | e
std::vector<std::string> PJPParser::ParseArg(){
    std::cout << "Arg" << std::endl;
    std::vector<std::string> args;
    switch (curTok) {
        case ')':
            return args;
        case tokenIdentifier:
            args.push_back(m_Lexer.identifierStr);
            getNextToken();
            if (curTok != ':') {
                MatchError((Token) curTok);
                return args;
            }
            getNextToken();
            if (curTok != tokenInteger) {
                MatchError((Token) curTok);
                return args;
            }
            getNextToken();
            return ParseArgID(args);
        default:
            ExpandError("Arg", (Token) curTok);
            return args;
    }
}

// ARGID -> ; id : integer ARGID | e
std::vector<std::string>  PJPParser::ParseArgID(std::vector<std::string> & args){
    std::cout << "ArgID" << std::endl;
    switch (curTok) {
        case ')':
            return args;
        case ';':
            getNextToken();
            if (curTok != tokenIdentifier) {
                MatchError((Token) curTok);
                return args;
            }
            args.push_back(m_Lexer.identifierStr);
            getNextToken();
            if (curTok != ':') {
                MatchError((Token) curTok);
                return args;
            }
            getNextToken();
            if (curTok != tokenInteger) {
                MatchError((Token) curTok);
                return args;
            }
            getNextToken();
            return ParseArgID(args);
        default:
            ExpandError("ArgID", (Token) curTok);
            return args;
    }
}

// MAIN -> begin REST
std::unique_ptr<StatmListAST> PJPParser::ParseMain(std::unique_ptr<StatmListAST> list){
    std::cout << "Main" << std::endl;
    if (curTok != tokenBegin){
        MatchError((Token) curTok);
        return list;
    }

    getNextToken();
    return std::move(ParseRest(std::move(list)));
}


// REST -> COMM ; REST | end 
std::unique_ptr<StatmListAST> PJPParser::ParseRest(std::unique_ptr<StatmListAST> list){
    std::cout << "Rest" << std::endl;
    switch (curTok) {
        case tokenEnd:
            getNextToken();
            //getNextToken();
            return std::move(list);
        case tokenWriteln:
        case tokenReadln:
        case tokenIdentifier:
        {
            auto comm = std::move(ParseCommand());
            list->addStatm(std::move(comm));
            if(curTok != ';'){
                MatchError((Token) curTok);
                return nullptr;
            }
            getNextToken();
            return ParseRest(std::move(list));
        }
        default:
            ExpandError("Rest", (Token) curTok);
            return nullptr;
    }
}

// DCL -> var ARGV1 : integer ; ARGV | const ARGC
std::unique_ptr<StatmListAST> PJPParser::ParseDeclare()
{
    std::cout << "Decl" << std::endl;
    switch (curTok) {
        case tokenVar:
        {
            getNextToken();
            //std::vector<std::unique_ptr<StatmAST>> vec;
            auto newList = llvm::make_unique<StatmListAST>();
            newList->addStatmList(std::move(ParseArgVar1()));
            if (curTok != ':') {
                MatchError((Token) curTok);
                return nullptr;
            }
            getNextToken();
            if (curTok != tokenInteger) {
                MatchError((Token) curTok);
                return nullptr;
            }
            getNextToken();
            if (curTok != ';') {
                MatchError((Token) curTok);
                return nullptr;
            }
            getNextToken();
            return std::move(ParseArgVar(std::move(newList)));
        }
        case tokenConst:
        {
            //std::vector<std::unique_ptr<StatmAST>> vec;
            auto newList = llvm::make_unique<StatmListAST>();
            getNextToken();
            return std::move(ParseArgConst(std::move(newList)));
        }
        default:
            ExpandError("Decl", (Token) curTok);
    }
    return nullptr;
}

// ARGV -> ARGV1 : integer ; | e
std::unique_ptr<StatmListAST> PJPParser::ParseArgVar(std::unique_ptr<StatmListAST> list){
    std::cout << "ArgV" << std::endl;
    switch (curTok) {
        case tokenIdentifier:
        {
            list->addStatmList(std::move(ParseArgVar1()));
	    if (curTok != ':') {
                MatchError((Token) curTok);
                return nullptr;
            }
            getNextToken();
            if (curTok != tokenInteger) {
                MatchError((Token) curTok);
                return nullptr;
            }
            getNextToken();
            if (curTok != ';') {
                MatchError((Token) curTok);
                return nullptr;
            }
            getNextToken();
            return std::move(list);
        }
        case tokenVar:
        case tokenConst:
        case tokenBegin:
        case tokenProcedure:
        case tokenFunction:
            return std::move(list);
        default:
            ExpandError("ArgV", (Token) curTok);
            return nullptr;
    }
}


// ARGV1 -> id ARGV2
std::unique_ptr<StatmListAST> PJPParser::ParseArgVar1(){
    std::cout << "ArgV1" << std::endl;
    //std::vector<std::unique_ptr<StatmAST>> vec;
    auto newList = llvm::make_unique<StatmListAST>();
    switch (curTok) {
        case tokenIdentifier:
        {
            std::string idVar = m_Lexer.identifierStr;
            getNextToken();
            newList->addStatm(std::move(llvm::make_unique<VarDeclAST>(idVar, currFunct)));
            return std::move(ParseArgVar2(std::move(newList)));
        }
        default:
            ExpandError("ArgV1", (Token) curTok);
            return nullptr;
    }
}

// ARGV2 -> , id ARGV2 | e
std::unique_ptr<StatmListAST> PJPParser::ParseArgVar2(std::unique_ptr<StatmListAST> list){
    std::cout << "ArgV2" << std::endl;
    switch (curTok) {
        case ':':
            return std::move(list);

        case ',':
        {
            getNextToken();
            if(curTok != tokenIdentifier){
                MatchError((Token) curTok);
                return nullptr;
            }
            std::string idVar = m_Lexer.identifierStr;
            getNextToken();
            list->addStatm(std::move(llvm::make_unique<VarDeclAST>(idVar, currFunct)));
            return std::move(ParseArgVar2(std::move(list)));
        }
        default:
            ExpandError("ArgV2", (Token) curTok);
            return nullptr;
    }
}

// ARGC -> id = num ; ARGC | e
std::unique_ptr<StatmListAST> PJPParser::ParseArgConst(std::unique_ptr<StatmListAST> list){
    std::cout << "ArgC" << std::endl;
    switch (curTok) {
        case tokenProcedure:
        case tokenFunction:
        case tokenVar:
        case tokenConst:
        case tokenBegin:
            return std::move(list);
        case tokenIdentifier:
        {
            std::string id = m_Lexer.identifierStr;
            getNextToken();
            if (curTok != '=') {
                MatchError((Token) curTok);
                return nullptr;
            }
            getNextToken();
            if (curTok != tokenNumber) {
                MatchError((Token) curTok);
                return nullptr;
            }
            getNextToken();
            if (curTok != ';') {
                MatchError((Token) curTok);
                return nullptr;
            }
            getNextToken();
            list->addStatm(std::move(llvm::make_unique<ConstDeclAST>(id, m_Lexer.numVal, currFunct)));
            return std::move(ParseArgConst(std::move(list)));
        }
        default:
            ExpandError("ArgC", (Token) curTok);
            return nullptr;
    }
}

//COMM -> readln ( id ) | writeln ( E ) | id COMM1

std::unique_ptr<StatmAST> PJPParser::ParseCommand(){
    std::cout << "Comm" << std::endl;
    switch (curTok) {
        case tokenReadln:
        {
            getNextToken();
            if (curTok != '('){
                MatchError((Token) curTok);
                return nullptr;
            }
            getNextToken();
            auto var = llvm::make_unique<VarRefAST>(m_Lexer.identifierStr, currFunct);
            auto statm = llvm::make_unique<ReadlnAST>(std::move(var));
            getNextToken();
            if (curTok != ')') {
                MatchError((Token) curTok);
                return nullptr;
            }
            getNextToken();
            return std::move(statm);
        }
        case tokenWriteln:
        {
            getNextToken();
            if (curTok != '('){
                MatchError((Token) curTok);
                return nullptr;
            }
            getNextToken();
            auto expr = std::move(ParseExpression());
            auto statm = llvm::make_unique<WritelnAST>(std::move(expr));
            //getNextToken();
            if (curTok != ')') {
                MatchError((Token) curTok);
                return nullptr;
            }
            getNextToken();
            return std::move(statm);
        }
        case tokenIdentifier:
        {
            std::string id = m_Lexer.identifierStr;
            /*auto var = llvm::make_unique<VarAST>(m_Lexer.identifierStr, currFunct);*/
            getNextToken();
            /*
                if (curTok != tokenAssign){
                    MatchError((Token) curTok);
                    return nullptr;
                }
                getNextToken();
                auto expr = std::move(ParseExpression());
                auto statm = llvm::make_unique<AssignAST>(std::move(var), std::move(expr));*/
            return std::move(ParseCommand1(id));
        }
        default:
            ExpandError("Comm", (Token) curTok);
            return nullptr;
    }
}

// COMM1 -> := E | ( ARGCALL )
std::unique_ptr<StatmAST> PJPParser::ParseCommand1(std::string & id){
    std::cout << "Comm1" << std::endl;
    switch (curTok) {
        case tokenAssign:
        {
            getNextToken();
            auto var = llvm::make_unique<VarAST>(id, currFunct);
            auto expr = std::move(ParseExpression());
            auto statm = llvm::make_unique<AssignAST>(std::move(var), std::move(expr));
            return std::move(statm);
        }
        case '(':
        {
            getNextToken();
            auto args = std::move(ParseArgCall());
            if (curTok != ')') {
                MatchError((Token)curTok);
                return nullptr;
            }
            getNextToken();
            auto result = llvm::make_unique<FunctionCallAST>(id,
                                                             std::move(args));
            return std::move(result);
        }
        default:
            ExpandError("Comm1", (Token) curTok);
            return nullptr;
    }
}


// E -> T E'
std::unique_ptr<ExprAST> PJPParser::ParseExpression() {
    std::cout << "E" << std::endl;
    return ParseExprRest(std::move(ParseTerm()));
}

// E' -> + T E' | - T E' | e
std::unique_ptr<ExprAST> PJPParser::ParseExprRest(std::unique_ptr<ExprAST> term){
    std::cout << "E'" << std::endl;
    switch (curTok) {
        case '+':
        {
            getNextToken();
            auto result = std::move(ParseTerm());
            //getNextToken();
            return ParseExprRest(llvm::make_unique<BinaryExprAST>('+', std::move(term), std::move(result)));
        }
        case '-':
        {
            getNextToken();
            auto result = std::move(ParseTerm());
            //getNextToken();
            return ParseExprRest(llvm::make_unique<BinaryExprAST>('-', std::move(term), std::move(result)));
        }
        case ';': case ')': case ',':
            return std::move(term);
        default:
            ExpandError("ExprRest", (Token) curTok);
            return nullptr;
    }
}

// T -> F T'
std::unique_ptr<ExprAST> PJPParser::ParseTerm() {
    std::cout << "T" << std::endl;
    return ParseTermRest(std::move(ParseFactor()));
}

// T' -> * F T' | div F T' | mod F T' | e
std::unique_ptr<ExprAST> PJPParser::ParseTermRest(std::unique_ptr<ExprAST> factor){
    std::cout << "T'" << std::endl;
    switch (curTok) {
        case '*':
        {
            getNextToken();
            auto result = std::move(ParseFactor());
            return ParseExprRest(llvm::make_unique<BinaryExprAST>('*', std::move(factor), std::move(result)));
        }
        case tokenDiv:
        {
            getNextToken();
            auto result = std::move(ParseFactor());
            return ParseExprRest(llvm::make_unique<BinaryExprAST>('/', std::move(factor), std::move(result)));
        }
        case tokenMod:
        {
            getNextToken();
            auto result = std::move(ParseFactor());
            return ParseExprRest(llvm::make_unique<BinaryExprAST>('%', std::move(factor), std::move(result)));
        }
        case ';': case '+': case '-': case ')': case ',':
            return std::move(factor);
        default:
            ExpandError("TermRest", (Token) curTok);
            return nullptr;
    }
}

/// F -> num | id F' | ( E )
std::unique_ptr<ExprAST> PJPParser::ParseFactor() {
    std::cout << "F" << std::endl;
    switch (curTok) {
        case tokenIdentifier:
        {
            std::string id = m_Lexer.identifierStr;
            getNextToken(); // consume the number
            return std::move(ParseFactorRest(id));
        }
        case tokenNumber:
        {
            auto result = llvm::make_unique<NumbExprAST>(m_Lexer.numVal);
            getNextToken(); // consume the number
            return llvm::make_unique<NumbExprAST>(m_Lexer.numVal);
        }
        case '(':
        {
            getNextToken();
            auto result = std::move(ParseExpression());
            if (curTok != ')') {
                MatchError((Token)curTok);
                return nullptr;
            }
            getNextToken();
            return std::move(result);
        }
        default:
            ExpandError("Factor", (Token) curTok);
            return nullptr;
    }
}

/// F' -> ( ARGCALL ) | e
std::unique_ptr<ExprAST> PJPParser::ParseFactorRest(std::string & id) {
    std::cout << "FactorRest" << std::endl;
    switch (curTok) {
        case tokenDiv:
        case tokenMod:
        case '-':
        case '+':
        case '*':
        case ')':
        case ';':
        {
            auto result = llvm::make_unique<VarAST>(id, currFunct);
            return std::move(result);
        }
        case '(':
        {
            getNextToken();
            auto args = std::move(ParseArgCall());
            if (curTok != ')') {
                MatchError((Token)curTok);
                return nullptr;
            }
            getNextToken();
            auto result = llvm::make_unique<FunctionCallExprAST>(id,
                                                             std::move(args));
            return std::move(result);
        }
        default:
            ExpandError("FactorRest", (Token) curTok);
            return nullptr;
    }
}

//ARGCALL -> E , ARGCALL1 | e
std::vector<std::unique_ptr<ExprAST>>  PJPParser::ParseArgCall(){
    std::vector<std::unique_ptr<ExprAST>> args;
    std::cout << "ArgCall" << std::endl;
    switch (curTok) {
        case tokenIdentifier:
        case tokenNumber:
        case '(':
        {
            auto expr = std::move(ParseExpression());
            args.push_back(std::move(expr));
            return ParseArgCall1(args);
        }
        case ')':
        {
            return args;
        }
        default:
            ExpandError("ArgCall", (Token) curTok);
            return args;
    }
}

//ARGCALL1 -> , E ARGCALL1 | e
std::vector<std::unique_ptr<ExprAST>>  PJPParser::ParseArgCall1(std::vector<std::unique_ptr<ExprAST>> & args){
    std::cout << "ArgCall1" << std::endl;
    switch (curTok) {
        case ',':
        {
            getNextToken();
            args.push_back(std::move(ParseExpression()));
            getNextToken();
            return ParseArgCall1(args);
        }
        case ')':
        {
            return std::move(args);
        }
        default:
            ExpandError("ArgCall1", (Token) curTok);
            return std::move(args);
    }
}

