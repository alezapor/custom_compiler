#include <cstdio>
#include <string>

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


    tokenIdentifier = -20,
    tokenNumber = -21
};

class PJPLexer {
public:
    PJPLexer() {}

    std::string identifierStr;    // Filled in if tokIdentifier
    int numVal;                     // Filled in if tokNumber
    int getTok();
};
