#include "lexer.h"

/// getTok - Return the next token from standard input.
int PJPLexer::getTok() {
    static int lastChar = ' ';
    std::string numStr;

    // Skip any whitespace.
    while (isspace(lastChar))
        lastChar = getchar();

    if (isalpha(lastChar)) { // identifier: [a-zA-Z][a-zA-Z0-9]*
        identifierStr = lastChar;
        while (isalnum((lastChar = getchar())))
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
        return tokenIdentifier;
    }

    if (isdigit(lastChar)) {   // Number: [0-9]+
        std::string numStr;
        do {
            numStr += lastChar;
            lastChar = getchar();
        } while (isdigit(lastChar));

        numVal = strtod(numStr.c_str(), 0);
        return tokenNumber;
    }

    if (isdigit(lastChar)) {   // Number: [0-9]+
        std::string numStr;
        do {
            numStr += lastChar;
            lastChar = getchar();
        } while (isdigit(lastChar));

        numVal = strtod(numStr.c_str(), 0);
        return tokenNumber;
    }



    // Check for end of file.  Don't eat the EOF.
    if (lastChar == EOF)
        return tokenEOF;

    // Otherwise, just return the character as its ascii value.
    int thisChar = lastChar;
    lastChar = getchar();

    if (thisChar == '&') {
        if (lastChar >= '0' && lastChar <= '7') {   // octal number: &[0-7]+
            numVal = 0;
            do {
                numVal = numVal * 8 + lastChar - '0';
                lastChar = getchar();
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
                lastChar = getchar();
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
                lastChar = getchar();
            } while ((lastChar >= '0' && lastChar <= '9') || (lastChar >= 'a' && lastChar <= 'f'));

            return tokenNumber;
        }
    }

    if (thisChar == ':') {
        if (lastChar == '=') {
            lastChar = getchar();
            return tokenAssign;
        }
    }
    if (thisChar == '<') {
        if (lastChar == '>') {
            lastChar = getchar();
            return tokenNeq;
        }
    }
    return thisChar;
}


