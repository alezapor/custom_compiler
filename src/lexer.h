#include <cstdio>
#include <string>
#include <fstream>

enum Token {
    tokenEOF = -1,

    // key words
    tokenProgram = -2,
    tokenBegin = -3,
    tokenEnd = -4,
    tokenReadln = -5,
    tokenWriteln = -6,
    tokenAssign = -7,
    tokenConst = -8,
    tokenVar = -9,
    tokenInteger = -10,
    tokenDiv = -11,
    tokenMod = -12,
    tokenProcedure = -13,
    tokenFunction = -14,
	tokenExit = -15,
	tokenIf = -16,
	tokenThen = -17,
	tokenElse = -18,
    tokenWhile = -19,
	tokenDo = -20,
  	tokenFor = -21,
	tokenTo = -22,
	tokenDownto = -23,
	tokenNeq = -24,
	tokenBreak = -25,
	tokenArray = - 26,
	tokenOf = -27,
	tokenGE = -28,
	tokenLE = -29,
	tokenDelim = -30, // ..
	tokenAnd = -31,
	tokenOr = -32,
	tokenNot = -33,
	tokenDec = -34,
	tokenForward = -35,


    tokenIdentifier = -40,
    tokenNumber = -41
};

class PJPLexer {
public:
    PJPLexer(std::fstream * is) {this->is = is;}

    std::string identifierStr;    // Filled in if tokIdentifier
    int numVal;                     // Filled in if tokNumber
    int getTok();
	std::fstream * is;
};
