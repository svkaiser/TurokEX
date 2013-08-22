// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 2012 Samuel Villarreal
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
// 02111-1307, USA.
//
//-----------------------------------------------------------------------------
//
// DESCRIPTION: Script token/parser system
//
//-----------------------------------------------------------------------------

#include <string.h>
#include "common.h"
#include "kernel.h"
#include "zone.h"
#include "script.h"
#include "filesystem.h"

//#define SC_DEBUG

#define COMMENT_NONE        0
#define COMMENT_SINGLELINE  1
#define COMMENT_MULTILINE   2

typedef enum
{
    CHAR_NUMBER,
    CHAR_LETTER,
    CHAR_SYMBOL,
    CHAR_QUOTE,
    CHAR_SPECIAL,
    CHAR_EOF
} chartype_t;

#ifdef SC_DEBUG

//
// SC_DebugPrintf
//

static void SC_DebugPrintf(const char *str, ...) {
    char buf[1024];
    va_list v;

    if(!verbose) {
        return;
    }
    
    va_start(v, str);
    vsprintf(buf, str,v);
    va_end(v);

    fprintf(debugfile, buf);
}
#endif

kexParser parser;

//
// kexLexer::kexLexer
//

kexLexer::kexLexer(const char *filename) {
    if(cvarDeveloper.GetBool()) {
        buffsize = fileSystem.ReadExternalTextFile(filename, (byte**)(&buffer));

        if(buffsize <= 0)
            buffsize = fileSystem.OpenFile(filename, (byte**)(&buffer), PU_STATIC);
    }
    else
        buffsize = fileSystem.OpenFile(filename, (byte**)(&buffer), PU_STATIC);

    if(buffsize <= 0)
        return;

    pointer_start   = buffer;
    pointer_end     = buffer + buffsize;
    linepos         = 1;
    rowpos          = 1;
    buffpos         = 0;
    tokentype       = TK_NONE;
    name            = filename;
}

//
// kexLexer::~kexLexer
//

kexLexer::~kexLexer(void) {
    Z_Free(buffer);

    buffer          = NULL;
    buffsize        = 0;
    pointer_start   = NULL;
    pointer_end     = NULL;
    linepos         = 0;
    rowpos          = 0;
    buffpos         = 0;
}

//
// kexLexer::CheckState
//

bool kexLexer::CheckState(void) {
#ifdef SC_DEBUG
    SC_DebugPrintf("(%s): checking script state: %i : %i\n",
        name, buffpos, buffsize);
#endif

    if(buffpos < buffsize) {
        return true;
    }

    return false;
}

//
// kexLexer::CheckKeywords
//

void kexLexer::CheckKeywords(void) {
    if(!strcasecmp(token, "define")) {
        tokentype = TK_DEFINE;
    }
    else if(!strcasecmp(token, "include")) {
        tokentype = TK_INCLUDE;
    }
    else if(!strcasecmp(token, "setdir")) {
        tokentype = TK_SETDIR;
    }
    else if(!strcasecmp(token, "undef")) {
        tokentype = TK_UNDEF;
    }
}

//
// kexLexer::ClearToken
//

void kexLexer::ClearToken(void) {
    tokentype = TK_NONE;
    memset(token, 0, SC_TOKEN_LEN);
}

//
// kexLexer::GetNumber
//

int kexLexer::GetNumber(void) {
#ifdef SC_DEBUG
    SC_DebugPrintf("get number (%s)\n", token);
#endif

    Find();

    if(tokentype != TK_NUMBER) {
        parser.Error("%s is not a number", token);
    }

    return atoi(token);
}

//
// kexLexer::GetFloat
//

double kexLexer::GetFloat(void) {
#ifdef SC_DEBUG
    SC_DebugPrintf("get float (%s)\n", token);
#endif

    Find();

    if(tokentype != TK_NUMBER) {
        parser.Error("%s is not a float", token);
    }

    return atof(token);
}

//
// kexLexer::GetString
//

void kexLexer::GetString(void) {
    ExpectNextToken(TK_STRING);
    strcpy(stringToken, token);
}

//
// kexLexer::MustMatchToken
//

void kexLexer::MustMatchToken(int type) {
#ifdef SC_DEBUG
    SC_DebugPrintf("must match %i\n", type);
    SC_DebugPrintf("tokentype %i\n", tokentype);
#endif

    if(tokentype != type) {
        char *string;

        switch(type) {
        case TK_NUMBER:
            string = "a number";
            break;
        case TK_STRING:
            string = "a string";
            break;
        case TK_POUND:
            string = "a pound sign";
            break;
        case TK_COLON:
            string = "a colon";
            break;
        case TK_SEMICOLON:
            string = "a semicolon";
            break;
        case TK_LBRACK:
            string = "{";
            break;
        case TK_RBRACK:
            string = "}";
            break;
        case TK_LSQBRACK:
            string = "[";
            break;
        case TK_RSQBRACK:
            string = "]";
            break;
        case TK_LPAREN:
            string = "(";
            break;
        case TK_RPAREN:
            string = ")";
            break;
        case TK_COMMA:
            string = "a comma";
            break;
        default:
            parser.Error("Invalid token: %s", token);
            break;
        }

        parser.Error("Expected %s, but found: %s (%i : %i)",
            string, token, tokentype, type);
    }
}

//
// kexLexer::ExpectNextToken
//

void kexLexer::ExpectNextToken(int type) {
#ifdef SC_DEBUG
    SC_DebugPrintf("expect %i\n", type);
#endif
    Find();
    MustMatchToken(type);
}

//
// kexLexer::GetNumberToken
//

void kexLexer::GetNumberToken(char initial) {
    int c = initial;
    int i = 0;

    tokentype = TK_NUMBER;

    while(parser.CharCode()[c] == CHAR_NUMBER) {
        token[i++] = c;
        c = GetChar();
    }

#ifdef SC_DEBUG
    SC_DebugPrintf("get number (%s)\n", token);
#endif

    Rewind();
}

//
// kexLexer::GetLetterToken
//

void kexLexer::GetLetterToken(char initial) {
    int c = initial;
    int i = 0;
    bool haschar = false;

    while(parser.CharCode()[c] == CHAR_LETTER ||
        (haschar && parser.CharCode()[c] == CHAR_NUMBER)) {
        token[i++] = c;
        c = GetChar();
        haschar = true;
    }

    tokentype = TK_IDENIFIER;

#ifdef SC_DEBUG
    SC_DebugPrintf("get letter (%s)\n", token);
#endif

    Rewind();
    CheckKeywords();
}

//
// kexLexer::GetSymbolToken
//

void kexLexer::GetSymbolToken(char c) {
    switch(c) {
    case '#':
        tokentype = TK_POUND;
        token[0] = c;
        break;
    case ':':
        tokentype = TK_COLON;
        token[0] = c;
        break;
    case ';':
        tokentype = TK_SEMICOLON;
        token[0] = c;
        break;
    case '=':
        tokentype = TK_EQUAL;
        token[0] = c;
        break;
    case '.':
        tokentype = TK_PERIOD;
        token[0] = c;
        break;
    case '{':
        tokentype = TK_LBRACK;
        token[0] = c;
        break;
    case '}':
        tokentype = TK_RBRACK;
        token[0] = c;
        break;
    case '(':
        tokentype = TK_LPAREN;
        token[0] = c;
        break;
    case ')':
        tokentype = TK_RPAREN;
        token[0] = c;
        break;
    case '[':
        tokentype = TK_LSQBRACK;
        token[0] = c;
        break;
    case ']':
        tokentype = TK_RSQBRACK;
        token[0] = c;
        break;
    case ',':
        tokentype = TK_COMMA;
        token[0] = c;
        break;
    default:
        parser.Error("Unknown symbol: %c", c);
        break;
    }

#ifdef SC_DEBUG
    SC_DebugPrintf("get symbol (%s)\n", token);
#endif
}

//
// kexLexer::GetStringToken
//

void kexLexer::GetStringToken(void) {
    int i = 0;
    char c = GetChar();

    while(parser.CharCode()[c] != CHAR_QUOTE) {
        token[i++] = c;
        c = GetChar();
    }

    tokentype = TK_STRING;

#ifdef SC_DEBUG
    SC_DebugPrintf("get string (%s)\n", token);
#endif
}

//
// kexLexer::Find
//

bool kexLexer::Find(void) {
    char c = 0;
    int comment = COMMENT_NONE;

    ClearToken();

    while(CheckState()) {
        c = GetChar();

        if(comment == COMMENT_NONE) {
            if(c == '/') {
                char gc = GetChar();

                if(gc != '/' && gc != '*') {
                    Rewind();
                }
                else {
                    if(gc == '*') {
                        comment = COMMENT_MULTILINE;
                    }
                    else
                    {
                        comment = COMMENT_SINGLELINE;
                    }
                }
            }
        }
        else if(comment == COMMENT_MULTILINE) {
            if(c == '*') {
                char gc = GetChar();

                if(gc != '/') {
                    Rewind();
                }
                else {
                    comment = COMMENT_NONE;
                    continue;
                }
            }
        }

        if(comment == COMMENT_NONE) {
            byte bc = ((byte)c);

            if(parser.CharCode()[bc] != CHAR_SPECIAL) {
                switch(parser.CharCode()[bc]) {
                case CHAR_NUMBER:
                    GetNumberToken(c);
                    return true;
                case CHAR_LETTER:
                    GetLetterToken(c);
                    return true;
                case CHAR_QUOTE:
                    GetStringToken();
                    return true;
                case CHAR_SYMBOL:
                    GetSymbolToken(c);
                    return true;
                case CHAR_EOF:
                    tokentype = TK_EOF;
#ifdef SC_DEBUG
                    SC_DebugPrintf("EOF token\n");
#endif
                    return true;
                default:
                    break;
                }
            }
        }

        if(c == '\n') {
            linepos++;
            rowpos = 1;

            if(comment == COMMENT_SINGLELINE) {
                comment = COMMENT_NONE;
            }
        }
    }

    return false;
}

//
// kexLexer::GetChar
//

char kexLexer::GetChar(void) {
    int c;

#ifdef SC_DEBUG
    SC_DebugPrintf("(%s): get char\n", name);
#endif

    rowpos++;
    c = buffer[buffpos++];

    if(parser.CharCode()[c] == CHAR_EOF)
        c = 0;

#ifdef SC_DEBUG
    SC_DebugPrintf("get char: %i\n", c);
#endif

    return c;
}

//
// kexLexer::Rewind
//

void kexLexer::Rewind(void) {
#ifdef SC_DEBUG
    SC_DebugPrintf("(%s): rewind\n", name);
#endif

    rowpos--;
    buffpos--;
}

//
// kexLexer::GetIDForTokenList
//

int kexLexer::GetIDForTokenList(const sctokens_t *tokenlist, const char *token) {
    int i;
    for(i = 0; tokenlist[i].id != -1; i++) {
        if(tokenlist[i].token == NULL) {
            continue;
        }

        if(!strcmp(token, tokenlist[i].token)) {
            return tokenlist[i].id;
        }
    }

    return i;
}

//
// kexLexer::ExpectTokenListID
//

void kexLexer::ExpectTokenListID(const sctokens_t *tokenlist, int id)
{
    Find();
    if(GetIDForTokenList(tokenlist, token) != id) {
        parser.Error("Expected \"%s\" but found %s",
            tokenlist[id].token, token);
    }
}

//
// kexLexer::AssignFromTokenList
//

void kexLexer::AssignFromTokenList(const sctokens_t *tokenlist, char *str, int id, bool expect) {
    if(expect) {
        ExpectTokenListID(tokenlist, id);
    }
    ExpectNextToken(TK_EQUAL);
    GetString();
    strcpy(str, stringToken);
}

//
// kexLexer::AssignFromTokenList
//

void kexLexer::AssignFromTokenList(const sctokens_t *tokenlist, unsigned int *var, int id, bool expect) {
    if(expect) {
        ExpectTokenListID(tokenlist, id);
    }
    ExpectNextToken(TK_EQUAL);
    *var = GetNumber();
}

//
// kexLexer::AssignFromTokenList
//

void kexLexer::AssignFromTokenList(const sctokens_t *tokenlist, unsigned short *var, int id, bool expect) {
    if(expect) {
        ExpectTokenListID(tokenlist, id);
    }
    ExpectNextToken(TK_EQUAL);
    *var = GetNumber();
}

//
// kexLexer::AssignFromTokenList
//

void kexLexer::AssignFromTokenList(const sctokens_t *tokenlist, float *var, int id, bool expect) {
    if(expect) {
        ExpectTokenListID(tokenlist, id);
    }
    ExpectNextToken(TK_EQUAL);
    *var = (float)GetFloat();
}

//
// kexLexer::AssignVectorFromTokenList
//

void kexLexer::AssignVectorFromTokenList(const sctokens_t *tokenlist, vec3_t var, int id, bool expect) {
    if(expect) {
        ExpectTokenListID(tokenlist, id);
    }
    ExpectNextToken(TK_EQUAL);
    ExpectNextToken(TK_LBRACK);
    var[0] = (float)GetFloat();
    var[1] = (float)GetFloat();
    var[2] = (float)GetFloat();
    ExpectNextToken(TK_RBRACK);
}

//
// kexLexer::AssignFromTokenList
//

void kexLexer::AssignFromTokenList(const sctokens_t *tokenlist, arraytype_t type,
                    void **data, int count, int id, bool expect, int tag) {
    void *buf;

    if(expect) {
        ExpectTokenListID(tokenlist, id);
    }
    else if(count <= 0) {
        parser.Error("Parsing \"%s\" array with count = 0",
            tokenlist[id].token);
    }

    ExpectNextToken(TK_EQUAL);
    ExpectNextToken(TK_LBRACK);

    buf = NULL;

    if(count <= 0) {
        // skip null arrays. note that parser will assume a -1 followed by a
        // closing bracket

        Find();  // skip the -1 number
        ExpectNextToken(TK_RBRACK);
    }
    else {
        int i;
        size_t len;

        switch(type) {
        case AT_SHORT:
            len = sizeof(short);
            break;
        case AT_INTEGER:
            len = sizeof(int);
            break;
        case AT_FLOAT:
            len = sizeof(float);
            break;
        case AT_DOUBLE:
            len = sizeof(double);
            break;
        case AT_VECTOR:
            len = sizeof(vec3_t);
            break;
        default:
            break;
        }

        buf = (void*)Z_Calloc(len * count, tag, 0);

        switch(type) {
        case AT_SHORT: {
                word *wbuf = (word*)buf;

                for(i = 0; i < count; i++) {
                    wbuf[i] = GetNumber();
                }
            }
            break;
        case AT_INTEGER: {
                int *ibuf = (int*)buf;

                for(i = 0; i < count; i++) {
                    ibuf[i] = GetNumber();
                }
            }
            break;
        case AT_FLOAT: {
                float *fbuf = (float*)buf;

                for(i = 0; i < count; i++) {
                    fbuf[i] = (float)GetFloat();
                }
            }
            break;
        case AT_DOUBLE: {
                double *dbuf = (double*)buf;

                for(i = 0; i < count; i++) {
                    dbuf[i] = GetFloat();
                }
            }
            break;
        case AT_VECTOR: {
                vec3_t *vbuf = (vec3_t*)buf;

                for(i = 0; i < count; i++) {
                    vbuf[i][0] = (float)GetFloat();
                    vbuf[i][1] = (float)GetFloat();
                    vbuf[i][2] = (float)GetFloat();
                }
            }
            break;
        default:
            break;
        }

        ExpectNextToken(TK_RBRACK);
    }

    *data = buf;
}

//
// kexParser::kexParser
//

kexParser::kexParser(void) {
    int i;

    numNestedFilenames = 0;
    numLexers = 0;

    for(i = 0; i < 256; i++) {
        charcode[i] = CHAR_SPECIAL;
    }
    for(i = '!'; i <= '~'; i++) {
        charcode[i] = CHAR_SYMBOL;
    }
    for(i = '0'; i <= '9'; i++) {
        charcode[i] = CHAR_NUMBER;
    }
    for(i = 'A'; i <= 'Z'; i++) {
        charcode[i] = CHAR_LETTER;
    }
    for(i = 'a'; i <= 'z'; i++) {
        charcode[i] = CHAR_LETTER;
    }

    charcode['"'] = CHAR_QUOTE;
    charcode['_'] = CHAR_LETTER;
    charcode['-'] = CHAR_NUMBER;
    charcode['.'] = CHAR_NUMBER;
    charcode[127] = CHAR_EOF;
}

//
// kexParser::~kexParser
//

kexParser::~kexParser(void) {
}

//
// kexParser::GetNestedFileName
//

const char *kexParser::GetNestedFileName(void) const {
    if(numNestedFilenames <= 0) {
        return NULL;
    }

    return nestedFilenames[numNestedFilenames-1];
}

//
// kexParser::PushFileName
//

void kexParser::PushFileName(const char *name) {
#ifdef SC_DEBUG
    SC_DebugPrintf("push nested file %s\n", name);
#endif
    strcpy(nestedFilenames[numNestedFilenames++], name);
}

//
// kexParser::PopFileName
//

void kexParser::PopFileName(void) {
#ifdef SC_DEBUG
    SC_DebugPrintf("nested file pop\n");
#endif
    memset(nestedFilenames[--numNestedFilenames], 0, 256);
}

//
// kexParser::PushLexer
//

void kexParser::PushLexer(const char *filename) {
    if(numLexers >= MAX_NESTED_PARSERS) {
        Error("Reached max number of nested lexers (%i)", numLexers);
    }

    lexers[numLexers] = new kexLexer(filename);
    currentLexer = lexers[numLexers];

    numLexers++;
}

//
// kexParser::PopLexer
//

void kexParser::PopLexer(void) {
    delete lexers[--numLexers];
    if(numLexers <= 0) {
        currentLexer = NULL;
    }
    else {
        currentLexer = lexers[numLexers - 1];
    }
}

//
// kexParser::Error
//

void kexParser::Error(const char *msg, ...) {
    char buf[1024];
    va_list v;
    
    va_start(v,msg);
    vsprintf(buf,msg,v);
    va_end(v);

    common.Error("%s : %s\n(line = %i, pos = %i)",
        GetNestedFileName(),
        buf, currentLexer->LinePos(), currentLexer->RowPos());
}

//
// kexParser::Open
//

kexLexer *kexParser::Open(const char* filename) {
#ifdef SC_DEBUG
    SC_DebugPrintf("opening %s\n", filename);
#endif

    // push out a new lexer
    PushLexer(filename);
    
    if(currentLexer->BufferSize() <= 0) {
        PopLexer();
        return NULL;
    }

    PushFileName(filename);

    return currentLexer;
}

//
// kexParser::Close
//

void kexParser::Close(void) {
    PopLexer();
    PopFileName();
}
