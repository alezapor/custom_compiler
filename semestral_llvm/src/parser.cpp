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
	program = idProg;
        getNextToken();
        if (curTok != ';') {
            MatchError((Token) curTok);
            return nullptr;
        }
        getNextToken();
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
        auto newList = llvm::make_unique<StatmListAST>();
	(newFunct->m_Body)->addStatm(ParseDeclare());
        (newFunct->m_Body)->addStatmList(std::move(ParseFunctBody(std::move(newList))));
        list->addStatm(std::move(newFunct));
        return std::move(ParseProgMain(std::move(list)));
    }
    else if (curTok == tokenBegin){
        getNextToken();
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

// PROGMAIN2 -> . | ; BODY
std::unique_ptr<StatmListAST> PJPParser::ParseProgMain2(std::unique_ptr<StatmListAST> list, std::unique_ptr<StatmListAST> block){
    std::cout << "ProgMain2" << std::endl;
    if (curTok == ';'){
        getNextToken();
        auto newFunct = llvm::make_unique<FunctionAST>(std::move(proto));
        newFunct->setStatmList(std::move(block));
        list->addStatm(std::move(newFunct));
        currFunct = "main";
        return std::move(ParseBody(std::move(list)));
    }
    else if (curTok == '.') {
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

// FUNCTPROTO -> procedure id ( ARG ) ; FORWARD | function id ( ARG ) : integer ; FORWARD
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
        std::vector<std::pair<std::string, VarType>> args = ParseArg();
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
        for (auto i = args.begin(); i!=args.end(); i++) newProc->addArgument(i->first, i->second);
	proto = std::move(newProc);
	ParseForward();
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
        std::vector<std::pair<std::string, VarType>> args = ParseArg();
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
        for (auto i = args.begin(); i!=args.end(); i++) newFunct->addArgument(i->first, i->second);
	proto = std::move(newFunct);
	ParseForward();
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

// ARG -> id : TYPE ARGID | e
std::vector<std::pair<std::string, VarType>> PJPParser::ParseArg(){
    std::cout << "Arg" << std::endl;
    std::vector<std::pair<std::string, VarType>> args;
    switch (curTok) {
        case ')':
            return args;
        case tokenIdentifier:
	{
            std::string id = m_Lexer.identifierStr;
            getNextToken();
            if (curTok != ':') {
                MatchError((Token) curTok);
                return args;
            }
            getNextToken();
	    args.push_back(make_pair(id, ParseType()));
            return ParseArgID(args);
	}
        default:
            ExpandError("Arg", (Token) curTok);
            return args;
    }
}

// ARGID -> ; id : TYPE ARGID | e
std::vector<std::pair<std::string, VarType>>  PJPParser::ParseArgID(std::vector<std::pair<std::string, VarType>> & args){
    std::cout << "ArgID" << std::endl;
    switch (curTok) {
        case ')':
            return args;
        case ';':
	{
            getNextToken();
            if (curTok != tokenIdentifier) {
                MatchError((Token) curTok);
                return args;
            }
            std::string id = m_Lexer.identifierStr;
            getNextToken();
            if (curTok != ':') {
                MatchError((Token) curTok);
                return args;
            }
            getNextToken();
            args.push_back(make_pair(id, ParseType()));
            return ParseArgID(args);
	}
        default:
            ExpandError("ArgID", (Token) curTok);
            return args;
    }
}

// MAIN -> begin REST .
std::unique_ptr<StatmListAST> PJPParser::ParseMain(std::unique_ptr<StatmListAST> list){
    std::cout << "Main" << std::endl;
    if (curTok != tokenBegin){
        MatchError((Token) curTok);
        return list;
    }
    getNextToken();
    auto rest = std::move(ParseRest(std::move(list)));
    if (curTok != '.'){
        MatchError((Token) curTok);
        return list;
    }
    return std::move(rest);
}


// REST -> COMM ; REST | end 
std::unique_ptr<StatmListAST> PJPParser::ParseRest(std::unique_ptr<StatmListAST> list){
    std::cout << "Rest" << std::endl;
    switch (curTok) {
        case tokenEnd:
            getNextToken();
            return std::move(list);
        case tokenWriteln:
        case tokenReadln:
        case tokenIdentifier:
	case tokenExit:
	case tokenBreak:
	case tokenIf:
	case tokenWhile:
	case tokenFor:
	case tokenDec:
        {
            auto comm = std::move(ParseCommand());
	    if (curTok != ';') {
                MatchError((Token) curTok);
                return nullptr;
            }
            getNextToken();
            list->addStatm(std::move(comm));
            return ParseRest(std::move(list));
        }
        default:
            ExpandError("Rest", (Token) curTok);
            return nullptr;
    }
}

// DCL -> var ARGV1 : TYPE ; ARGV | const ARGC
std::unique_ptr<StatmListAST> PJPParser::ParseDeclare()
{
    std::cout << "Decl" << std::endl;
    switch (curTok) {
        case tokenVar:
        {
            getNextToken();
            //std::vector<std::unique_ptr<StatmAST>> vec;
            auto newList = llvm::make_unique<StatmListAST>();
            auto vec = ParseArgVar1();
            if (curTok != ':') {
                MatchError((Token) curTok);
                return nullptr;
            }
            getNextToken();
            VarType type = ParseType();
	    for (unsigned i = 0; i < vec.size(); i++){
		if (type.integer) newList->addStatm(std::move(llvm::make_unique<VarDeclAST>(vec[i], currFunct)));
		else newList->addStatm(std::move(llvm::make_unique<ArrayDeclAST>(vec[i], currFunct, type.min, type.max)));
	    }
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

// ARGV -> ARGV1 : TYPE ; | e
std::unique_ptr<StatmListAST> PJPParser::ParseArgVar(std::unique_ptr<StatmListAST> list){
    std::cout << "ArgV" << std::endl;
    switch (curTok) {
        case tokenIdentifier:
        {
            std::vector<std::string> vec = ParseArgVar1();
	    if (curTok != ':') {
                MatchError((Token) curTok);
                return nullptr;
            }
            getNextToken();
            VarType type = ParseType();
            //getNextToken();
            if (curTok != ';') {
                MatchError((Token) curTok);
                return nullptr;
            }
	    for (unsigned i = 0; i < vec.size(); i++){
		if (type.integer) list->addStatm(std::move(llvm::make_unique<VarDeclAST>(vec[i], currFunct)));
		else list->addStatm(std::move(llvm::make_unique<ArrayDeclAST>(vec[i], currFunct, type.min, type.max)));
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
std::vector<std::string> PJPParser::ParseArgVar1(){
    std::cout << "ArgV1" << std::endl;
    std::vector<std::string> vec;
    switch (curTok) {
        case tokenIdentifier:
        {
            std::string idVar = m_Lexer.identifierStr;
            getNextToken();
            vec.push_back(idVar);
            return ParseArgVar2(vec);
        }
        default:
            ExpandError("ArgV1", (Token) curTok);
            return vec;
    }
}

// ARGV2 -> , id ARGV2 | e
std::vector<std::string> PJPParser::ParseArgVar2(std::vector<std::string> & vec){
    std::cout << "ArgV2" << std::endl;
    switch (curTok) {
        case ':':
            return vec;

        case ',':
        {
            getNextToken();
            if(curTok != tokenIdentifier){
                MatchError((Token) curTok);
                return vec;
            }
            std::string idVar = m_Lexer.identifierStr;
            getNextToken();
            vec.push_back(idVar);
            return ParseArgVar2(vec);
        }
        default:
            ExpandError("ArgV2", (Token) curTok);
            return vec;
    }
}

// ARGC -> id = NUM ; ARGC | e
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
            int num = ParseNum();
            if (curTok != ';') {
                MatchError((Token) curTok);
                return nullptr;
            }
            getNextToken();
            list->addStatm(std::move(llvm::make_unique<ConstDeclAST>(id, num, currFunct)));
            return std::move(ParseArgConst(std::move(list)));
        }
        default:
            ExpandError("ArgC", (Token) curTok);
            return nullptr;
    }
}

//NUM -> - num | num
int PJPParser::ParseNum(){
    std::cout << "Num" << std::endl;
    switch (curTok) {
        case '-':
	{
	    getNextToken();
            if (curTok != tokenNumber) {
                MatchError((Token) curTok);
                return 0;
            }
            int num = 0 - m_Lexer.numVal;
            getNextToken();
	    return num;
	}
        case tokenNumber:
        {
	    int num = m_Lexer.numVal;
            getNextToken();
            return num;
        }
        default:
            ExpandError("Num", (Token) curTok);
            return 0;
    }

}

//COMM -> readln ( id INDEX ) | writeln ( E ) | id COMM1 | exit | if LOGEXPR then COMM_LIST COMM2 | while LOGEXPR do COMM_LIST | for id := E FOR | break | dec ( id )

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
	     if (curTok != tokenIdentifier){
                MatchError((Token) curTok);
                return nullptr;
            }
	    std::string id = m_Lexer.identifierStr;
	    getNextToken();
	    bool isArrayItem = false; 
	    auto expr = std::move(ParseIndex(isArrayItem));
	    
            std::unique_ptr<VarRefAST> var;
	    if (!isArrayItem) var = llvm::make_unique<VarRefAST>(id, currFunct);
	    else var = llvm::make_unique<ArrayContentRefAST>(id, currFunct, std::move(expr));
            auto statm = llvm::make_unique<ReadlnAST>(std::move(var));
            if (curTok != ')') {
                MatchError((Token) curTok);
                return nullptr;
            }
            getNextToken();
            return std::move(statm);
        }
	case tokenDec:
	{
	    getNextToken();
	    if (curTok != '('){
                MatchError((Token) curTok);
                return nullptr;
            }
            getNextToken();
	    if (curTok != tokenIdentifier){
                MatchError((Token) curTok);
                return nullptr;
            }
	    std::string id = m_Lexer.identifierStr;
	    getNextToken();
	    if (curTok != ')') {
                MatchError((Token) curTok);
                return nullptr;
            }
            getNextToken();
	    auto var = llvm::make_unique<VarAST>(id, currFunct);
	    auto one = llvm::make_unique<NumbExprAST>(1);
	    auto ref = llvm::make_unique<VarRefAST>(id, currFunct);
	    return llvm::make_unique<AssignAST>(std::move(ref), llvm::make_unique<BinaryExprAST>('-', std::move(var), std::move(one)));
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
            getNextToken();
            auto comm1 = std::move(ParseCommand1(id));
	    return std::move(comm1);
        }
	case tokenExit:
        {
	    getNextToken();
	    auto statm = llvm::make_unique<ExitAST>(currFunct, (proto)? proto->retVal : false);
            return std::move(statm);
        }
	case tokenBreak:
        {
	    getNextToken();
	    auto statm = llvm::make_unique<BreakAST>();
            return std::move(statm);
        }
	case tokenIf:
        {
	    getNextToken();
	    auto cond = std::move(ParseLogExpr());
            if (curTok != tokenThen) {
                MatchError((Token) curTok);
                return nullptr;
            }
            getNextToken();
	    auto then = std::move(ParseCommList());
	    auto els = std::move(ParseCommand2());
            return llvm::make_unique<IfAST>(std::move(cond), std::move(then), std::move(els));
        }
	case tokenWhile:
        {
	    getNextToken();
	    auto cond = std::move(ParseLogExpr());
            if (curTok != tokenDo) {
                MatchError((Token) curTok);
                return nullptr;
            }
            getNextToken();
	    auto toDo = std::move(ParseCommList());
            return llvm::make_unique<WhileAST>(std::move(cond), std::move(toDo));
        }
	case tokenFor:
        {
	    getNextToken();
	    if (curTok != tokenIdentifier) {
                MatchError((Token) curTok);
                return nullptr;
            }
	    std::string id = m_Lexer.identifierStr;
            getNextToken();
	    bool isArrayItem = false; 
	    auto index = std::move(ParseIndex(isArrayItem));
            if (curTok != tokenAssign) {
                MatchError((Token) curTok);
                return nullptr;
            }
            getNextToken();
	    auto expr = std::move(ParseExpression());
	    std::unique_ptr<VarRefAST> var;
	    if (!isArrayItem) var = llvm::make_unique<VarRefAST>(id, currFunct);
	    else var = llvm::make_unique<ArrayContentRefAST>(id, currFunct, std::move(index));
            return std::move(ParseFor(std::move(var), std::move(expr)));
        }
        default:
            ExpandError("Comm", (Token) curTok);
            return nullptr;
    }
}

// COMM1 -> := E | [ E ] := E | ( ARGCALL )
std::unique_ptr<StatmAST> PJPParser::ParseCommand1(std::string & id){
    std::cout << "Comm1" << std::endl;
    switch (curTok) {
        case tokenAssign:
        {
            getNextToken();
            auto var = llvm::make_unique<VarRefAST>(id, currFunct);
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
	case '[':
        {
            getNextToken();
	    auto index = std::move(ParseExpression());
	    if (curTok != ']') {
                MatchError((Token)curTok);
                return nullptr;
            }
            getNextToken();
            if (curTok != tokenAssign) {
                MatchError((Token)curTok);
                return nullptr;
            }
            getNextToken();
            auto var = llvm::make_unique<ArrayContentRefAST>(id, currFunct, std::move(index));
            auto expr = std::move(ParseExpression());
            auto statm = llvm::make_unique<AssignAST>(std::move(var), std::move(expr));
            return std::move(statm);
        }
        default:
            ExpandError("Comm1", (Token) curTok);
            return nullptr;
    }
}

//COMM2 -> | else COMM_LIST
std::unique_ptr<StatmListAST> PJPParser::ParseCommand2(){
    std::cout << "Comm2" << std::endl;
    switch (curTok) {
        case ';':
        {
            //getNextToken();
	    auto newList = llvm::make_unique<StatmListAST>();
            return std::move(newList);
        }
        case tokenElse:
        {
            getNextToken();
            auto statm = std::move(ParseCommList());
	    return std::move(statm);
        }
        default:
            ExpandError("Comm2", (Token) curTok);
            return nullptr;
    }

}

//COMM_LIST -> COMM | begin REST
std::unique_ptr<StatmListAST> PJPParser::ParseCommList(){
    std::cout << "CommList" << std::endl;
    switch (curTok) {
        case tokenWriteln:
        case tokenReadln:
        case tokenIdentifier:
	case tokenExit:
	case tokenBreak:
	case tokenIf:
	case tokenWhile:
	case tokenFor:
	case tokenDec:
        {
	    auto newList = llvm::make_unique<StatmListAST>();
            newList->addStatm(std::move(ParseCommand()));
	    return std::move(newList);
        }
        case tokenBegin:
        {
	    getNextToken();
            auto newList = llvm::make_unique<StatmListAST>();
            auto res = std::move(ParseRest(std::move(newList)));
	    return std::move(res);
        }
        default:
            ExpandError("CommList", (Token) curTok);
            return nullptr;
    }

}

// FOR -> to E do COMM_LIST | downto E do COMM_LIST 
std::unique_ptr<StatmAST> PJPParser::ParseFor(std::unique_ptr<VarRefAST> var, std::unique_ptr<ExprAST> st){
    std::cout << "For" << std::endl;
    switch (curTok) {
        case tokenTo:
        {
	    getNextToken();
	    auto end = std::move(ParseExpression());
	    if (curTok != tokenDo) {
                MatchError((Token)curTok);
                return nullptr;
            }
            getNextToken();
	    auto body = std::move(ParseCommList());
	    return llvm::make_unique<ForAST>(std::move(var), std::move(st), llvm::make_unique<NumbExprAST>(1), std::move(end), std::move(body));
        }
        case tokenDownto:
        {
	    getNextToken();
	    auto end = std::move(ParseExpression());
	    if (curTok != tokenDo) {
                MatchError((Token)curTok);
                return nullptr;
            }
            getNextToken();
	    auto body = std::move(ParseCommList());
	    return llvm::make_unique<ForAST>(std::move(var), std::move(st), llvm::make_unique<NumbExprAST>(-1), std::move(end), std::move(body));
        }
        default:
            ExpandError("For", (Token) curTok);
            return nullptr;
    }

}

//LOGEXPR -> COND LOGEXPR' | not COND
std::unique_ptr<ExprAST> PJPParser::ParseLogExpr() {
    std::cout << "LogExpr" << std::endl;
    switch (curTok) {
        case tokenIdentifier:
	case tokenNumber:
	case '(':
	case '-':
        {
    	    auto l = std::move(ParseCond());
	    return std::move(ParseLogExprRest(std::move(l)));
	}
	case tokenNot:
	{	 
            getNextToken();
	    auto l = std::move(ParseCond());
	    return llvm::make_unique<BinaryExprAST>('~', std::move(l), std::move(l));
	}
 	default:
            ExpandError("LogExpr", (Token) curTok);
            return nullptr;
    }
}

//LOGEXPR' -> e | and COND | or COND
std::unique_ptr<ExprAST> PJPParser::ParseLogExprRest(std::unique_ptr<ExprAST> left){
    std::cout << "LogExprRest" << std::endl;
    switch (curTok) {
        case tokenAnd:
        {
	    getNextToken();
	    auto r = std::move(ParseCond());
	    return llvm::make_unique<BinaryExprAST>('&', std::move(left), std::move(r));
	}
	case tokenOr:
        {
	    getNextToken();
	    auto r = std::move(ParseCond());
	    return llvm::make_unique<BinaryExprAST>('|', std::move(left), std::move(r));
	}
	case tokenThen:
	case tokenDo:
	case ')':
            return std::move(left);
 	default:
            ExpandError("LogExprRest", (Token) curTok);
            return nullptr;
    }


}


// COND -> E COND'
std::unique_ptr<ExprAST> PJPParser::ParseCond() {
    std::cout << "Cond" << std::endl;
    switch (curTok) {
        case tokenIdentifier:
	case tokenNumber:
	case '(':
	case '-':
        {
    	    auto l = std::move(ParseExpression());
	    return std::move(ParseCondRest(std::move(l)));
	}
 	default:
            ExpandError("Cond", (Token) curTok);
            return nullptr;
    }
}

// COND' -> ROp E | e
std::unique_ptr<ExprAST> PJPParser::ParseCondRest(std::unique_ptr<ExprAST> left){
    std::cout << "CondRest" << std::endl;
    switch (curTok) {
        case '>':
	case '<':
	case tokenLE:
	case tokenGE:
	case tokenNeq:
	case '=':
        {
	    char op = ParseROp();
	    auto r = std::move(ParseExpression());
	    return llvm::make_unique<BinaryExprAST>(op, std::move(left), std::move(r));
	}
	case tokenThen:
	case tokenDo:
	case ')':
	case tokenAnd:
	case tokenOr:
            return std::move(left);
 	default:
            ExpandError("CondRest", (Token) curTok);
            return nullptr;
    }


}

// ROp -> <> | > | = | < | >= | <=
char PJPParser::ParseROp(){
    std::cout << "ROp" << (int) curTok<< std::endl;
    switch (curTok) {
        case tokenNeq:
	   getNextToken();
	   return '!';	
	case '=':
	case '>':
	case '<':
        {
            char tok = curTok;
	    getNextToken();
    	    return tok;
	}
	case tokenLE:
	   getNextToken();
	   return '{';
	case tokenGE:
	   getNextToken();
	   return '}';
 	default:
            ExpandError("ROp", (Token) curTok);
            return '\0';
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
            return ParseExprRest(llvm::make_unique<BinaryExprAST>('+', std::move(term), std::move(result)));
        }
        case '-':
        {
            getNextToken();
            auto result = std::move(ParseTerm());
            return ParseExprRest(llvm::make_unique<BinaryExprAST>('-', std::move(term), std::move(result)));
        }
        case ';': case ')': case ',': case '=': case tokenThen:
	case tokenDo: case '<': case tokenNeq: case '>': case tokenTo: case tokenDownto: case tokenLE: case tokenGE: case ']': case tokenElse:
	case tokenAnd: case tokenOr:
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
            return ParseTermRest(llvm::make_unique<BinaryExprAST>('*', std::move(factor), std::move(result)));
        }
        case tokenDiv:
        {
            getNextToken();
            auto result = std::move(ParseFactor());
            return ParseTermRest(llvm::make_unique<BinaryExprAST>('/', std::move(factor), std::move(result)));
        }
        case tokenMod:
        {
            getNextToken();
            auto result = std::move(ParseFactor());
            return ParseTermRest(llvm::make_unique<BinaryExprAST>('%', std::move(factor), std::move(result)));
        }
        case '+': case '-': case ';': case ')': case ',': case '=': case tokenThen:
	case tokenDo: case '<': case tokenNeq: case '>': case tokenTo: case tokenDownto: case tokenLE: case tokenGE: case ']': case tokenElse:
	case tokenAnd: case tokenOr:
            return std::move(factor);
        default:
            ExpandError("TermRest", (Token) curTok);
            return nullptr;
    }
}

/// F -> num | - F | id F' | ( COND )
std::unique_ptr<ExprAST> PJPParser::ParseFactor() {
    std::cout << "F" << std::endl;
    switch (curTok) {
        case tokenIdentifier:
        {
            std::string id = m_Lexer.identifierStr;
            getNextToken(); // consume the id
            return std::move(ParseFactorRest(id));
        }
        case tokenNumber:
        {
            auto result = llvm::make_unique<NumbExprAST>(m_Lexer.numVal);
            getNextToken(); // consume the number
            return llvm::make_unique<NumbExprAST>(m_Lexer.numVal);
        }
	case '-':
        {
	    getNextToken();
            return llvm::make_unique<BinaryExprAST>('-', llvm::make_unique<NumbExprAST>(0), std::move(ParseFactor()));
        }
        case '(':
        {
            getNextToken();
            auto result = std::move(ParseLogExpr());
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

/// F' -> ( ARGCALL ) | e | [ E ]
std::unique_ptr<ExprAST> PJPParser::ParseFactorRest(std::string & id) {
    std::cout << "FactorRest" << std::endl;
    switch (curTok) {
        case tokenDiv: case tokenMod: case '*': case '+': case '-': case ';': case ')': case ',': case '=': case tokenThen: case tokenAnd:
	case tokenDo: case '<': case tokenNeq: case '>': case tokenTo: case tokenDownto: case tokenLE: case tokenGE: case ']': case tokenElse:
	case tokenOr:
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
	case '[':
	{
	    getNextToken();
	    auto index = std::move(ParseExpression());
	    if (curTok != ']') {
                MatchError((Token)curTok);
                return nullptr;
            }
	    getNextToken();
	    auto result = llvm::make_unique<ArrayContentAST>(id, currFunct, std::move(index));
	   return std::move(result);
	}
        default:
            ExpandError("FactorRest", (Token) curTok);
            return nullptr;
    }
}

//ARGCALL -> E ARGCALL1 | e
std::vector<std::unique_ptr<ExprAST>>  PJPParser::ParseArgCall(){
    std::vector<std::unique_ptr<ExprAST>> args;
    std::cout << "ArgCall" << std::endl;
    switch (curTok) {
        case tokenIdentifier:
        case tokenNumber:
        case '(':
	case '-':
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

//TYPE -> integer | array [ NUM .. NUM ] of integer
VarType PJPParser::ParseType(){
std::cout << "Type" << std::endl;
    VarType a;
    switch (curTok) {
        case tokenInteger:
        {
            getNextToken();
	    a.integer = true;
            return a;
        }
        case tokenArray:
        {
	    getNextToken();
	    a.integer = false;
	    if (curTok != '[') {
                MatchError((Token)curTok);
                return a;
            }
            getNextToken();
	    a.min = ParseNum();
	    if (curTok != tokenDelim) {
                MatchError((Token)curTok);
                return a;
            }
	    getNextToken();
	    a.max = ParseNum();
	    if (curTok != ']') {
                MatchError((Token)curTok);
                return a;
            }
            getNextToken();
	    if (curTok != tokenOf) {
                MatchError((Token)curTok);
                return a;
            }
            getNextToken();
	    if (curTok != tokenInteger) {
                MatchError((Token)curTok);
                return a;
            }
            getNextToken();
            return a;
        }
        default:
            ExpandError("ArgCall1", (Token) curTok);
            return a;
    }
}

//INDEX -> [ E ] | e
std::unique_ptr<ExprAST> PJPParser::ParseIndex(bool & index){
    std::cout << "Index" << std::endl;
    switch (curTok) {
        case '[':
        {
	    getNextToken();
	    auto expr = std::move(ParseExpression());
	    if (curTok != ']') {
                MatchError((Token)curTok);
                return nullptr;
            }
            getNextToken();
	    index = true;
            return std::move(expr);
        }
	case tokenAssign:
	case ')':
	{
	    index = false;
	    return nullptr;
	}
        default:
            ExpandError("Index", (Token) curTok);
            return nullptr;
    }
}

//FORWARD -> forward ; | e
void PJPParser::ParseForward(){
    std::cout << "Forward" << std::endl;
     switch (curTok) {
        case tokenForward:
        {
	    getNextToken();
	    if (curTok != ';') {
                MatchError((Token)curTok);
                return;
            }
            getNextToken();
        }
	case tokenBegin:
	case tokenVar:
	case tokenProcedure:
	case tokenFunction:
	case tokenConst:
	{
	   return;
	}
        default:
            ExpandError("Forward", (Token) curTok);
            return ;
    }	
}

