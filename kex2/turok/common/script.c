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

//#define SC_DEBUG

scparser_t *sc_parsers[MAX_NESTED_PARSERS];
int sc_numparsers;
scparser_t *sc_parser;
char sc_stringbuffer[MAX_FILEPATH];

#define MAX_NESTED_FILENAMES    128
static char sc_nested_filenames[MAX_NESTED_FILENAMES][MAX_FILEPATH];
static int sc_num_nested_filenames;

identifier_t identifier_cap;
identifier_t *current_identifier;

identifer_stack_t id_stack[MAX_IDENTIFER_STACK_LEN];
int id_stack_count;

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

static byte sc_charcode[256];

static void SC_PopIdStack(identifier_t *id, identifer_stack_t *stack);

//------------------------------------------------------------------------
//
// GENERAL ROUTINES
//
//------------------------------------------------------------------------

#ifdef SC_DEBUG

//
// SC_DebugPrintf
//

static void SC_DebugPrintf(const char *str, ...)
{
    char buf[1024];
    va_list v;

    if(!verbose)
    {
        return;
    }
    
    va_start(v, str);
    vsprintf(buf, str,v);
    va_end(v);

    fprintf(debugfile, buf);
}
#endif

//
// SC_PushNestedFilename
//

static void SC_PushNestedFilename(const char *name)
{
#ifdef SC_DEBUG
    SC_DebugPrintf("push nested file %s\n", name);
#endif
    strcpy(sc_nested_filenames[sc_num_nested_filenames++], name);
}

//
// SC_PopNestedFilename
//

static void SC_PopNestedFilename(void)
{
#ifdef SC_DEBUG
    SC_DebugPrintf("nested file pop\n");
#endif
    memset(sc_nested_filenames[--sc_num_nested_filenames], 0, 256);
}

//
// SC_GetNestedFilename
//

static char *SC_GetNestedFilename(void)
{
    if(sc_num_nested_filenames <= 0)
    {
        return NULL;
    }

    return sc_nested_filenames[sc_num_nested_filenames-1];
}

//
// SC_PushParser
//

void SC_PushParser(void)
{
    if(sc_numparsers >= MAX_NESTED_PARSERS)
    {
        SC_Error("Reached max number of nested parsers (%i)", sc_numparsers);
    }

    sc_parsers[sc_numparsers] = (scparser_t*)Z_Calloc(sizeof(scparser_t), PU_STATIC, 0);
    sc_parser = sc_parsers[sc_numparsers];

    sc_numparsers++;
}

//
// SC_PopParser
//

void SC_PopParser(void)
{
    if(sc_parser->stack != NULL && sc_parser->identifier != NULL)
    {
        SC_PopIdStack(sc_parser->identifier, sc_parser->stack);
    }

    Z_Free(sc_parsers[--sc_numparsers]);
    if(sc_numparsers <= 0)
    {
        sc_parser = NULL;
    }
    else
    {
        sc_parser = sc_parsers[sc_numparsers - 1];
    }
}

//
// SC_Open
//

scparser_t *SC_Open(const char* name)
{
    Com_DPrintf("--------SC_Open: Reading %s--------\n", name);

#ifdef SC_DEBUG
    SC_DebugPrintf("opening %s\n", name);
#endif

    // push out a new parser
    SC_PushParser();

    sc_parser->buffsize = KF_OpenFileCache(name, &sc_parser->buffer, PU_STATIC);
    
    if(sc_parser->buffsize <= 0)
    {
        if(SC_GetNestedFilename())
        {
            Com_Error("%s : %s not found", SC_GetNestedFilename(), name);
        }
        else
        {
            Com_Error("%s not found", name);
        }
    }

    Com_DPrintf("%s size: %i bytes\n", name, sc_parser->buffsize);

    SC_PushNestedFilename(name);

    sc_parser->pointer_start    = sc_parser->buffer;
    sc_parser->pointer_end      = sc_parser->buffer + sc_parser->buffsize;
    sc_parser->linepos          = 1;
    sc_parser->rowpos           = 1;
    sc_parser->buffpos          = 0;
    sc_parser->identifier       = NULL;
    sc_parser->stack            = NULL;
    sc_parser->tokentype        = TK_NONE;
    sc_parser->isamacro         = false;
    sc_parser->name             = name;

    return sc_parser;
}

//
// SC_Close
//

void SC_Close(void)
{
#ifdef SC_DEBUG
    SC_DebugPrintf("sc_close\n");
#endif

    Z_Free(sc_parser->buffer);

    sc_parser->buffer           = NULL;
    sc_parser->buffsize         = 0;
    sc_parser->pointer_start    = NULL;
    sc_parser->pointer_end      = NULL;
    sc_parser->linepos          = 0;
    sc_parser->rowpos           = 0;
    sc_parser->buffpos          = 0;

    SC_PopParser();
    SC_PopNestedFilename();
}

//
// SC_Error
//

void SC_Error(const char *msg, ...)
{
    char buf[1024];
    va_list v;
    
    va_start(v,msg);
    vsprintf(buf,msg,v);
    va_end(v);

    Com_Error("%s : %s\n(line = %i, pos = %i)",
        SC_GetNestedFilename(),
        buf, sc_parser->linepos, sc_parser->rowpos);
}

//------------------------------------------------------------------------
//
// TOKEN SEARCHING ROUTINES
//
//------------------------------------------------------------------------

//
// SC_CheckScriptState
//

int SC_CheckScriptState(void)
{
#ifdef SC_DEBUG
    SC_DebugPrintf("(%s): checking script state: %i : %i\n",
        sc_parser->name, sc_parser->buffpos, sc_parser->buffsize);
#endif

    if(sc_parser->buffpos < sc_parser->buffsize)
    {
        return true;
    }
    else if(sc_parser->isamacro)
    {
        SC_PopParser();
    }

    return false;
}

//
// SC_CheckKeywords
//

void SC_CheckKeywords(void)
{
    if(!strcasecmp(sc_parser->token, "define"))
    {
        sc_parser->tokentype = TK_DEFINE;
    }
    else if(!strcasecmp(sc_parser->token, "include"))
    {
        sc_parser->tokentype = TK_INCLUDE;
    }
    else if(!strcasecmp(sc_parser->token, "setdir"))
    {
        sc_parser->tokentype = TK_SETDIR;
    }
    else if(!strcasecmp(sc_parser->token, "undef"))
    {
        sc_parser->tokentype = TK_UNDEF;
    }
}

//
// SC_ClearToken
//

static void SC_ClearToken(void)
{
    sc_parser->tokentype = TK_NONE;
    memset(sc_parser->token, 0, SC_TOKEN_LEN);
}

//
// SC_GetNumber
//

int SC_GetNumber(void)
{
#ifdef SC_DEBUG
    SC_DebugPrintf("get number (%s)\n", sc_parser->token);
#endif

    SC_Find();

    if(sc_parser->tokentype != TK_NUMBER)
    {
        SC_Error("%s is not a number", sc_parser->token);
    }

    return atoi(sc_parser->token);
}

//
// SC_GetFloat
//

double SC_GetFloat(void)
{
#ifdef SC_DEBUG
    SC_DebugPrintf("get float (%s)\n", sc_parser->token);
#endif

    SC_Find();

    if(sc_parser->tokentype != TK_NUMBER)
    {
        SC_Error("%s is not a float", sc_parser->token);
    }

    return atof(sc_parser->token);
}

//
// SC_GetString
//

void SC_GetString(void)
{
    SC_ExpectNextToken(TK_STRING);
    strcpy(sc_stringbuffer, sc_parser->token);
}

//
// SC_MustMatchToken
//

void SC_MustMatchToken(int type)
{
#ifdef SC_DEBUG
    SC_DebugPrintf("must match %i\n", type);
    SC_DebugPrintf("tokentype %i\n", sc_parser->tokentype);
#endif

    if(sc_parser->tokentype != type)
    {
        char *string;

        switch(type)
        {
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
            SC_Error("Invalid token: %s", sc_parser->token);
            break;
        }

        SC_Error("Expected %s, but found: %s (%i : %i)",
            string, sc_parser->token, sc_parser->tokentype, type);
    }
}

//
// SC_ExpectNextToken
//

void SC_ExpectNextToken(int type)
{
#ifdef SC_DEBUG
    SC_DebugPrintf("expect %i\n", type);
#endif
    SC_Find();
    SC_MustMatchToken(type);
}

//
// SC_CheckIdentifierArgs
//

static void SC_CheckIdentifierArgs(void)
{
    int i;
    int len;
    identifier_t* id;

    if(sc_parser->stack == NULL || sc_parser->identifier == NULL)
    {
        return;
    }

    id = sc_parser->identifier;

    for(i = 0; i < id->numargs; i++)
    {
        if(!strcmp(sc_parser->token, sc_parser->stack[i].argname))
        {
            SC_ClearToken();

            len = strlen(sc_parser->stack[i].value);
            strncpy(sc_parser->token, sc_parser->stack[i].value, len);
            sc_parser->tokentype = sc_parser->stack[i].token_type;

            return;
        }
    }
}

//
// SC_GetNumberToken
//

static void SC_GetNumberToken(char initial)
{
    int c = initial;
    int i = 0;

    sc_parser->tokentype = TK_NUMBER;

    while(sc_charcode[c] == CHAR_NUMBER)
    {
        sc_parser->token[i++] = c;
        c = SC_GetChar();
    }

#ifdef SC_DEBUG
    SC_DebugPrintf("get number (%s)\n", sc_parser->token);
#endif

    SC_Rewind();
    SC_CheckIdentifierArgs();
}

//
// SC_GetLetterToken
//

static void SC_GetLetterToken(char initial)
{
    int c = initial;
    int i = 0;

    while(sc_charcode[c] == CHAR_LETTER)
    {
        sc_parser->token[i++] = c;
        c = SC_GetChar();
    }

    sc_parser->tokentype = TK_IDENIFIER;

#ifdef SC_DEBUG
    SC_DebugPrintf("get letter (%s)\n", sc_parser->token);
#endif

    SC_Rewind();
    SC_CheckKeywords();
    SC_CheckIdentifierArgs();
}

//
// SC_GetSymbolToken
//

static void SC_GetSymbolToken(char c)
{
    switch(c)
    {
    case '#':
        sc_parser->tokentype = TK_POUND;
        sc_parser->token[0] = c;
        break;
    case ':':
        sc_parser->tokentype = TK_COLON;
        sc_parser->token[0] = c;
        break;
    case ';':
        sc_parser->tokentype = TK_SEMICOLON;
        sc_parser->token[0] = c;
        break;
    case '=':
        sc_parser->tokentype = TK_EQUAL;
        sc_parser->token[0] = c;
        break;
    case '.':
        sc_parser->tokentype = TK_PERIOD;
        sc_parser->token[0] = c;
        break;
    case '{':
        sc_parser->tokentype = TK_LBRACK;
        sc_parser->token[0] = c;
        break;
    case '}':
        sc_parser->tokentype = TK_RBRACK;
        sc_parser->token[0] = c;
        break;
    case '(':
        sc_parser->tokentype = TK_LPAREN;
        sc_parser->token[0] = c;
        break;
    case ')':
        sc_parser->tokentype = TK_RPAREN;
        sc_parser->token[0] = c;
        break;
    case '[':
        sc_parser->tokentype = TK_LSQBRACK;
        sc_parser->token[0] = c;
        break;
    case ']':
        sc_parser->tokentype = TK_RSQBRACK;
        sc_parser->token[0] = c;
        break;
    case ',':
        sc_parser->tokentype = TK_COMMA;
        sc_parser->token[0] = c;
        break;
    default:
        SC_Error("Unknown symbol: %c", c);
        break;
    }

#ifdef SC_DEBUG
    SC_DebugPrintf("get symbol (%s)\n", sc_parser->token);
#endif
}

//
// SC_GetStringToken
//

static void SC_GetStringToken(void)
{
    int i = 0;
    char c = SC_GetChar();

    while(sc_charcode[c] != CHAR_QUOTE)
    {
        sc_parser->token[i++] = c;
        c = SC_GetChar();
    }

    sc_parser->tokentype = TK_STRING;

#ifdef SC_DEBUG
    SC_DebugPrintf("get string (%s)\n", sc_parser->token);
#endif
}

//
// SC_Find
//

int SC_Find(void)
{
    char c = 0;
    int comment = COMMENT_NONE;

    SC_ClearToken();

    while(SC_CheckScriptState())
    {
        c = SC_GetChar();

        if(comment == COMMENT_NONE)
        {
            if(c == '/')
            {
                char gc = SC_GetChar();

                if(gc != '/' && gc != '*')
                {
                    SC_Rewind();
                }
                else
                {
                    if(gc == '*')
                    {
                        comment = COMMENT_MULTILINE;
                    }
                    else
                    {
                        comment = COMMENT_SINGLELINE;
                    }
                }
            }
        }
        else if(comment == COMMENT_MULTILINE)
        {
            if(c == '*')
            {
                char gc = SC_GetChar();

                if(gc != '/')
                {
                    SC_Rewind();
                }
                else
                {
                    comment = COMMENT_NONE;
                    continue;
                }
            }
        }

        if(comment == COMMENT_NONE)
        {
            byte bc = ((byte)c);

            if(sc_charcode[bc] != CHAR_SPECIAL)
            {
                switch(sc_charcode[bc])
                {
                case CHAR_NUMBER:
                    SC_GetNumberToken(c);
                    return true;
                case CHAR_LETTER:
                    SC_GetLetterToken(c);
                    return true;
                case CHAR_QUOTE:
                    SC_GetStringToken();
                    return true;
                case CHAR_SYMBOL:
                    SC_GetSymbolToken(c);
                    return true;
                case CHAR_EOF:
                    sc_parser->tokentype = TK_EOF;
#ifdef SC_DEBUG
                    SC_DebugPrintf("EOF token\n");
#endif
                    return true;
                default:
                    break;
                }
            }
        }

        if(c == '\n')
        {
            sc_parser->linepos++;
            sc_parser->rowpos = 1;

            if(comment == COMMENT_SINGLELINE)
            {
                comment = COMMENT_NONE;
            }
        }
    }

    return false;
}

//
// SC_GetChar
//

char SC_GetChar(void)
{
    int c;

#ifdef SC_DEBUG
    SC_DebugPrintf("(%s): get char\n", sc_parser->name);
#endif

    sc_parser->rowpos++;
    c = sc_parser->buffer[sc_parser->buffpos++];

    if(c == 127)
    {
        c = 0;
    }

#ifdef SC_DEBUG
    SC_DebugPrintf("get char: %i\n", c);
#endif

    return c;
}

//
// SC_Rewind
//

void SC_Rewind(void)
{
#ifdef SC_DEBUG
    SC_DebugPrintf("(%s): rewind\n", sc_parser->name);
#endif

    sc_parser->rowpos--;
    sc_parser->buffpos--;
}

//------------------------------------------------------------------------
//
// IDENTIFIER ROUTINES
//
//------------------------------------------------------------------------

//
// SC_AddIdentifier
//

void SC_AddIdentifier(void)
{
    identifier_t *id;
    int len;
    char *buffer;
    int row;
    int pos;

    id = (identifier_t*)Z_Calloc(sizeof(*id), PU_STATIC, 0);
    strcpy(id->name, sc_parser->token);
    id->numargs = 0;

    row = sc_parser->rowpos;
    pos = sc_parser->buffpos;

    SC_Find();
    if(sc_parser->tokentype == TK_LPAREN)
    {
        id->has_args = true;
        while(1)
        {
            SC_Find();
            if(strlen(sc_parser->token) >= MAX_IDENTIFIER_LEN)
            {
                SC_Error("Arg name (%s) for %s is longer than %i characters",
                    sc_parser->token, id->name, MAX_IDENTIFIER_LEN);
            }

            strcpy(id->argnames[id->numargs], sc_parser->token);
            id->numargs++;

            SC_Find();
            if(sc_parser->tokentype == TK_RPAREN)
            {
                break;
            }
            else
            {
                SC_MustMatchToken(TK_COMMA);

                if(id->numargs >= MAX_IDENTIFIER_ARGS)
                    SC_Error("Too many args for identifier %s", id->name);

                continue;
            }
        }
    }
    else
    {
        sc_parser->rowpos = row;
        sc_parser->buffpos = pos;

        id->has_args = false;
    }

    len = 0;
    buffer = sc_parser->buffer + sc_parser->buffpos;

    do
    {
        if(buffer[len] == '\\')
        {
            buffer[len] = '\n';
            while(buffer[len++] < ' ');
        }

    } while(buffer[len++] >= ' ');

    id->bufsize = len;
    id->buffer = (char*)Z_Calloc(len+1, PU_STATIC, 0);
    strncpy(id->buffer, buffer, len);

    id->buffer[len] = 127;

    identifier_cap.prev->next = id;
    id->next = &identifier_cap;
    id->prev = identifier_cap.prev;
    identifier_cap.prev = id;

    sc_parser->rowpos += len;
    sc_parser->buffpos += len;

#ifdef SC_DEBUG
    SC_DebugPrintf("add identifier\n");
#endif
}

//
// SC_RemoveIdentifier
//

void SC_RemoveIdentifier(void)
{
    for(current_identifier = identifier_cap.next;
        current_identifier != &identifier_cap;
        current_identifier = current_identifier->next)
    {
        identifier_t *id = current_identifier;

        if(!strcmp(id->name, sc_parser->token))
        {
            identifier_t* next = current_identifier->next;
            (next->prev = current_identifier = id->prev)->next = next;

            Z_Free(id->buffer);
            Z_Free(id);

#ifdef SC_DEBUG
            SC_DebugPrintf("remove identifier\n");
#endif

            return;
        }
    }

    SC_Error("Unknown identifier '%s'", sc_parser->token);
}

//
// SC_PushIdStack
//

void SC_PushIdStack(char *name)
{
    int len;

    if(id_stack_count >= MAX_IDENTIFER_STACK_LEN)
    {
        SC_Error("Stack overflowed\n");
    }

    id_stack[id_stack_count].argname = name;
    id_stack[id_stack_count].token_type = sc_parser->tokentype;
    len = strlen(sc_parser->token);
    memset(id_stack[id_stack_count].value, 0, MAX_IDENTIFER_VALUE_LEN);
    strncpy(id_stack[id_stack_count].value, sc_parser->token, len);

    id_stack_count++;

#ifdef SC_DEBUG
    SC_DebugPrintf("push id stack\n");
#endif
}

//
// SC_PopIdStack
//

static void SC_PopIdStack(identifier_t *id,
                          identifer_stack_t *stack)
{
    int i;

    for(i = 0; i < id->numargs; i++)
    {
        if(id_stack_count <= 0)
        {
            SC_Error("Tried to pop an empty stack for identifier '%s'",
                id->name);
        }

        memset(id_stack[id_stack_count].value, 0, MAX_IDENTIFER_VALUE_LEN);
        id_stack_count--;
    }

#ifdef SC_DEBUG
    SC_DebugPrintf("pop id stack\n");
#endif
}

//------------------------------------------------------------------------
//
// TOKEN IDENTIFIERS
//
//------------------------------------------------------------------------

//
// SC_GetIDForToken
//

int SC_GetIDForToken(const sctokens_t *tokenlist, const char *token)
{
    int i;

    for(i = 0; tokenlist[i].id != -1; i++)
    {
        if(tokenlist[i].token == NULL)
        {
            continue;
        }

        if(!strcmp(token, tokenlist[i].token))
        {
            return tokenlist[i].id;
        }
    }

    return i;
}

//
// SC_ExpectTokenID
//

void SC_ExpectTokenID(const sctokens_t *tokenlist, int id, scparser_t *parser)
{
    SC_Find();
    if(SC_GetIDForToken(tokenlist, parser->token) != id)
    {
        SC_Error("Expected \"%s\" but found %s",
            tokenlist[id].token, parser->token);
    }
}

//------------------------------------------------------------------------
//
// ASSIGNMENT ROUTINES
//
//------------------------------------------------------------------------

//
// SC_AssignString
//

void SC_AssignString(const sctokens_t *tokenlist, char *str, int id,
                     scparser_t *parser, kbool expect)
{
    if(expect)
    {
        SC_ExpectTokenID(tokenlist, id, parser);
    }

    SC_ExpectNextToken(TK_EQUAL);
    SC_GetString();
    strcpy(str, sc_stringbuffer);
}

//
// SC_AssignInteger
//

void SC_AssignInteger(const sctokens_t *tokenlist, unsigned int *var, int id,
                             scparser_t *parser, kbool expect)
{
    if(expect)
    {
        SC_ExpectTokenID(tokenlist, id, parser);
    }

    SC_ExpectNextToken(TK_EQUAL);
    *var = SC_GetNumber();
}

//
// SC_AssignWord
//

void SC_AssignWord(const sctokens_t *tokenlist, unsigned short *var, int id,
                             scparser_t *parser, kbool expect)
{
    if(expect)
    {
        SC_ExpectTokenID(tokenlist, id, parser);
    }

    SC_ExpectNextToken(TK_EQUAL);
    *var = SC_GetNumber();
}

//
// SC_AssignFloat
//

void SC_AssignFloat(const sctokens_t *tokenlist, float *var, int id,
                             scparser_t *parser, kbool expect)
{
    if(expect)
    {
        SC_ExpectTokenID(tokenlist, id, parser);
    }

    SC_ExpectNextToken(TK_EQUAL);
    *var = (float)SC_GetFloat();
}

//
// SC_AssignVector
//

void SC_AssignVector(const sctokens_t *tokenlist, vec3_t var, int id,
                             scparser_t *parser, kbool expect)
{
    if(expect)
    {
        SC_ExpectTokenID(tokenlist, id, parser);
    }

    SC_ExpectNextToken(TK_EQUAL);
    SC_ExpectNextToken(TK_LBRACK);

    var[0] = (float)SC_GetFloat();
    var[1] = (float)SC_GetFloat();
    var[2] = (float)SC_GetFloat();

    SC_ExpectNextToken(TK_RBRACK);
}

//
// SC_AssignArray
//

void SC_AssignArray(const sctokens_t *tokenlist, arraytype_t type,
                    void **data, int count, int id,
                    scparser_t *parser, kbool expect, int tag)
{
    void *buf;

    if(expect)
    {
        SC_ExpectTokenID(tokenlist, id, parser);
    }
    else if(count <= 0)
    {
        SC_Error("Parsing \"%s\" array with count = 0",
            tokenlist[id].token);
    }

    SC_ExpectNextToken(TK_EQUAL);
    SC_ExpectNextToken(TK_LBRACK);

    buf = NULL;

    if(count <= 0)
    {
        // skip null arrays. note that parser will assume a -1 followed by a
        // closing bracket

        SC_Find();  // skip the -1 number
        SC_ExpectNextToken(TK_RBRACK);
    }
    else
    {
        int i;
        size_t len;

        switch(type)
        {
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

        switch(type)
        {
        case AT_SHORT:
            {
                word *wbuf = (word*)buf;

                for(i = 0; i < count; i++)
                {
                    wbuf[i] = SC_GetNumber();
                }
            }
            break;
        case AT_INTEGER:
            {
                int *ibuf = (int*)buf;

                for(i = 0; i < count; i++)
                {
                    ibuf[i] = SC_GetNumber();
                }
            }
            break;
        case AT_FLOAT:
            {
                float *fbuf = (float*)buf;

                for(i = 0; i < count; i++)
                {
                    fbuf[i] = (float)SC_GetFloat();
                }
            }
            break;
        case AT_DOUBLE:
            {
                double *dbuf = (double*)buf;

                for(i = 0; i < count; i++)
                {
                    dbuf[i] = SC_GetFloat();
                }
            }
            break;
        case AT_VECTOR:
            {
                vec3_t *vbuf = (vec3_t*)buf;

                for(i = 0; i < count; i++)
                {
                    vbuf[i][0] = (float)SC_GetFloat();
                    vbuf[i][1] = (float)SC_GetFloat();
                    vbuf[i][2] = (float)SC_GetFloat();
                }
            }
            break;
        default:
            break;
        }

        SC_ExpectNextToken(TK_RBRACK);
    }

    *data = buf;
}

//
// SC_Init
//

void SC_Init(void)
{
    int i;

    sc_num_nested_filenames = 0;
    sc_numparsers = 0;
    identifier_cap.prev = identifier_cap.next  = &identifier_cap;
    id_stack_count = 0;
    memset(id_stack, 0, sizeof(*id_stack));

    for(i = 0; i < 256; i++)
    {
        sc_charcode[i] = CHAR_SPECIAL;
    }

    for(i = '!'; i <= '~'; i++)
    {
        sc_charcode[i] = CHAR_SYMBOL;
    }

    for(i = '0'; i <= '9'; i++)
    {
        sc_charcode[i] = CHAR_NUMBER;
    }

    for(i = 'A'; i <= 'Z'; i++)
    {
        sc_charcode[i] = CHAR_LETTER;
    }

    for(i = 'a'; i <= 'z'; i++)
    {
        sc_charcode[i] = CHAR_LETTER;
    }

    sc_charcode['"'] = CHAR_QUOTE;
    sc_charcode['_'] = CHAR_LETTER;
    sc_charcode['-'] = CHAR_NUMBER;
    sc_charcode['.'] = CHAR_NUMBER;
    sc_charcode[127] = CHAR_EOF;
}
