#include "lexer.h"

/// getTok - Return the next token from standard input.
int PJPLexer::getTok() {
    static int lastChar = ' ';
    std::string numStr;

    // Skip any whitespace.
    while (isspace(lastChar))
        lastChar = is->get();

    if (isalpha(lastChar) || lastChar == '_') { // identifier: [a-zA-Z][a-zA-Z0-9]*
        identifierStr = lastChar;
        while (isalnum((lastChar = is->get())) || lastChar == '_')
            identifierStr += lastChar;

        if (identifierStr == "program")
            return tokenProgram;
        if (identifierStr == "function")
            return tokenFunction;
        if (identifierStr == "procedure")
            return tokenProcedure;
        if (identifierStr == "begin")
            return tokenBegin;
        if (identifierStr == "end")
            return tokenEnd;
        if (identifierStr == "readln")
            return tokenReadln;
        if (identifierStr == "writeln")
            return tokenWriteln;
        if (identifierStr == "const")
            return tokenConst;
        if (identifierStr == "var")
            return tokenVar;
        if (identifierStr == "integer")
            return tokenInteger;
        if (identifierStr == "div")
            return tokenDiv;
        if (identifierStr == "mod")
            return tokenMod;
	if (identifierStr == "exit")
            return tokenExit;
	if (identifierStr == "if")
            return tokenIf;
	if (identifierStr == "then")
            return tokenThen;
	if (identifierStr == "else")
            return tokenElse;
	if (identifierStr == "while")
            return tokenWhile;
	if (identifierStr == "do")
            return tokenDo;
	if (identifierStr == "for")
            return tokenFor;
	if (identifierStr == "to")
            return tokenTo;
	if (identifierStr == "downto")
            return tokenDownto;
	if (identifierStr == "break")
            return tokenBreak;
	if (identifierStr == "array")
            return tokenArray;
	if (identifierStr == "of")
            return tokenOf;
	if (identifierStr == "downto")
            return tokenDownto;
	if (identifierStr == "and")
            return tokenAnd;
	if (identifierStr == "or")
            return tokenOr;
	if (identifierStr == "not")
            return tokenNot;
	if (identifierStr == "dec")
            return tokenDec;
	if (identifierStr == "forward")
            return tokenForward;
        return tokenIdentifier;
    }

    if (isdigit(lastChar)) {   // Number: [0-9]+
        std::string numStr;
        do {
            numStr += lastChar;
            lastChar = is->get();
        } while (isdigit(lastChar));

        numVal = strtod(numStr.c_str(), 0);
        return tokenNumber;
    }

    if (isdigit(lastChar)) {   // Number: [0-9]+
        std::string numStr;
        do {
            numStr += lastChar;
            lastChar = is->get();
        } while (isdigit(lastChar));

        numVal = strtod(numStr.c_str(), 0);
        return tokenNumber;
    }



    // Check for end of file.  Don't eat the EOF.
    if (lastChar == EOF)
        return tokenEOF;

    // Otherwise, just return the character as its ascii value.
    int thisChar = lastChar;
    lastChar = is->get();

    if (thisChar == '&') {
        if (lastChar >= '0' && lastChar <= '7') {   // octal number: &[0-7]+
            numVal = 0;
            do {
                numVal = numVal * 8 + lastChar - '0';
                lastChar = is->get();
            } while (lastChar >= '0' && lastChar <= '7');

            return tokenNumber;
        }
    }

    if (thisChar == '$') {
        if ((lastChar >= '0' && lastChar <= '9') || (lastChar >= 'A' && lastChar <= 'F')) {   // hex number: &[0-9A-F]+
            numVal = 0;
            do {
                if (lastChar >= '0' && lastChar <= '9') numVal = numVal * 16 + lastChar - '0';
                else numVal = numVal * 16 + lastChar - 'A' + 10;
                lastChar = is->get();
            } while ((lastChar >= '0' && lastChar <= '9') || (lastChar >= 'A' && lastChar <= 'F'));

            return tokenNumber;
        }
    }

    if (thisChar == '$') {
        if ((lastChar >= '0' && lastChar <= '9') || (lastChar >= 'a' && lastChar <= 'f')) {   // hex number: &[0-9a-f]+
            numVal = 0;
            do {
                if (lastChar >= '0' && lastChar <= '9') numVal = numVal * 16 + lastChar - '0';
                else numVal = numVal * 16 + lastChar - 'a' + 10;
                lastChar = is->get();
            } while ((lastChar >= '0' && lastChar <= '9') || (lastChar >= 'a' && lastChar <= 'f'));

            return tokenNumber;
        }
    }

    if (thisChar == ':') {
        if (lastChar == '=') {
            lastChar = is->get();
            return tokenAssign;
        }
    }
    if (thisChar == '<') {
        if (lastChar == '>') {
            lastChar = is->get();
            return tokenNeq;
        }
	 if (lastChar == '=') {
            lastChar = is->get();
            return tokenLE;
        }
    }
    if (thisChar == '>') {
        if (lastChar == '=') {
            lastChar = is->get();
            return tokenGE;
        }
    }
    if (thisChar == '.') {
        if (lastChar == '.') {
            lastChar = is->get(); 
            return tokenDelim;
        }
    }
    return thisChar;
}


