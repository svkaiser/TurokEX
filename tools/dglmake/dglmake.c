#include <stdio.h>
#include <io.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdarg.h>

#define false 0
#define true (!false)

typedef unsigned char byte;
typedef int dboolean;
typedef struct
{
    char    token[512];
    char*   buffer;
    char*   pointer_start;
    char*   pointer_end;
    int     linepos;
    int     rowpos;
    int     buffpos;
    int     buffsize;
} scparser_t;

scparser_t sc_parser;

typedef struct
{
    int type;
    dboolean pointer;
    char name[32];
} glinfo_t;

typedef struct
{
    glinfo_t header;
    glinfo_t args[16];
    char name[32];
    char extname[64];
} gldata_t;

static gldata_t gldata[350];

static int SC_Find(dboolean forceupper);
static char SC_GetChar(void);

//
// ReadFile
//

int ReadFile(char const* name, byte** buffer)
{
    FILE *fp;

    errno = 0;
    
    if((fp = fopen(name, "rb")))
    {
        size_t length;

        fseek(fp, 0, SEEK_END);
        length = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        *buffer = (byte*)malloc(length);
      
        if(fread(*buffer, 1, length, fp) == length)
        {
            fclose(fp);
            return length;
        }
        
        fclose(fp);
   }
   
   return -1;
}

//
// SC_Open
//

static void SC_Open(char* name)
{
    int length;

    memset(&sc_parser, 0, sizeof(scparser_t));
    memset(&gldata, 0, sizeof(gldata_t) * 350);

    length = ReadFile(name, &sc_parser.buffer);

    if(length <= -1)
        return;

    sc_parser.buffsize      = length;
    sc_parser.pointer_start = sc_parser.buffer;
    sc_parser.pointer_end   = sc_parser.buffer + sc_parser.buffsize;
    sc_parser.linepos       = 1;
    sc_parser.rowpos        = 1;
    sc_parser.buffpos       = 0;
}

//
// SC_Close
//

static void SC_Close(void)
{
    free(sc_parser.buffer);

    sc_parser.buffer         = NULL;
    sc_parser.buffsize       = 0;
    sc_parser.pointer_start  = NULL;
    sc_parser.pointer_end    = NULL;
    sc_parser.linepos        = 0;
    sc_parser.rowpos         = 0;
    sc_parser.buffpos        = 0;
}

//
// SC_ReadTokens
//

static int SC_ReadTokens(void)
{
    return (sc_parser.buffpos < sc_parser.buffsize);
}

//
// SC_Find
//

static int SC_Find(dboolean forceupper)
{
    char c = 0;
    int i = 0;
    dboolean comment = false;
    dboolean havetoken = false;
    dboolean string = false;

    memset(sc_parser.token, 0, 256);

    while(SC_ReadTokens())
    {
        c = SC_GetChar();

        if(c == '/')
            comment = true;

        if(comment == false)
        {
            if(c == '"')
            {
                if(!string)
                {
                    string = true;
                    continue;
                }
                else if(havetoken)
                    return true;
            }

            if(!string)
            {
                if(c > ' ')
                {
                    havetoken = true;
                    sc_parser.token[i++] =
                        forceupper ? toupper(c) : c;
                }
                else if(havetoken)
                    return true;
            }
            else
            {
                if(c >= ' ')
                {
                    havetoken = true;
                    sc_parser.token[i++] =
                        forceupper ? toupper(c) : c;
                }
            }
        }

        if(c == '\n')
        {
            sc_parser.linepos++;
            sc_parser.rowpos = 1;
            comment = false;
            if(string)
                sc_parser.token[i++] = c;
        }
    }

    return false;
}

//
// SC_GetChar
//

static char SC_GetChar(void)
{
    sc_parser.rowpos++;
    return sc_parser.buffer[sc_parser.buffpos++];
}

//
// GetGLArg
//

void GetGLArg(glinfo_t* info)
{
    char temp[30];
    char *temp2;
    int len;

    SC_Find(false);

    strcpy(temp, sc_parser.token);
    len = strlen(sc_parser.token);

    if(temp[len-1] == ')' || temp[len-1] == ',')
        temp[len-1] = 0;
    else if(strstr(temp, ");"))
    {
        temp[len-1] = 0;
        temp[len-2] = 0;
    }

    if(temp[0] == '*')
    {
        temp2 = (temp + 1);
        info->pointer = true;
    }
    else if(temp[0] == '(')
        temp2 = (temp + 1);
    else
        temp2 = temp;

    strcpy(info->name, temp2);
}

//
// GetGLType
//

int GetGLType(glinfo_t* info)
{
    char *temp;

    SC_Find(false);

    if(!strcmp(sc_parser.token, "(void);"))
        return 0;

    if(strchr(sc_parser.token, '('))
        temp = (sc_parser.token + 1);
    else
        temp = sc_parser.token;

    if(!strcmp(temp, "const"))
    {
        return GetGLType(info);
    }

    if(!strcmp(temp, "void"))
        info->type = 0;

    else if(!strcmp(temp, "GLenum"))
        info->type = 1;

    else if(!strcmp(temp, "GLboolean"))
        info->type = 2;

    else if(!strcmp(temp, "GLbitfield"))
        info->type = 3;

    else if(!strcmp(temp, "GLbyte"))
        info->type = 4;

    else if(!strcmp(temp, "GLshort"))
        info->type = 5;

    else if(!strcmp(temp, "GLint"))
        info->type = 6;

    else if(!strcmp(temp, "GLsizei"))
        info->type = 7;

    else if(!strcmp(temp, "GLubyte"))
        info->type = 8;

    else if(!strcmp(temp, "GLushort"))
        info->type = 9;

    else if(!strcmp(temp, "GLuint"))
        info->type = 10;

    else if(!strcmp(temp, "GLfloat"))
        info->type = 11;

    else if(!strcmp(temp, "GLclampf"))
        info->type = 12;

    else if(!strcmp(temp, "GLdouble"))
        info->type = 13;

    else if(!strcmp(temp, "GLclampd"))
        info->type = 14;

    else if(!strcmp(temp, "GLvoid"))
        info->type = 15;

    else if(!strcmp(temp, "GLintptrARB"))
        info->type = 16;

    else if(!strcmp(temp, "GLsizeiptrARB"))
        info->type = 17;

    else if(!strcmp(temp, "GLintptr"))
        info->type = 18;

    else if(!strcmp(temp, "GLsizeiptr"))
        info->type = 19;

    return 1;
}

//
// ReturnGLType
//

char* ReturnGLType(glinfo_t* info)
{
    switch(info->type)
    {
    case 0:
        return "void";
        break;
    case 1:
        return "GLenum";
        break;
    case 2:
        return "GLboolean";
        break;
    case 3:
        return "GLbitfield";
        break;
    case 4:
        return "GLbyte";
        break;
    case 5:
        return "GLshort";
        break;
    case 6:
        return "GLint";
        break;
    case 7:
        return "GLsizei";
        break;
    case 8:
        return "GLubyte";
        break;
    case 9:
        return "GLushort";
        break;
    case 10:
        return "GLuint";
        break;
    case 11:
        return "GLfloat";
        break;
    case 12:
        return "GLclampf";
        break;
    case 13:
        return "GLdouble";
        break;
    case 14:
        return "GLclampd";
        break;
    case 15:
        return "GLvoid";
        break;
    case 16:
        return "GLintptrARB";
        break;
    case 17:
        return "GLsizeiptrARB";
        break;
    case 18:
        return "GLintptr";
        break;
    case 19:
        return "GLsizeiptr";
        break;
    }

    return NULL;
}

//
// SlapGLStuffToFile
//

void SlapGLStuffToFile(FILE* f, int count, dboolean addunderscore)
{
    int i;

    for(i = 0; i < count; i++)
    {
        int a = 0;

        fprintf(f, "#define d%s(", gldata[i].name);

        while(gldata[i].args[a].name[0] != 0)
        {
            fprintf(f, "%s", gldata[i].args[a++].name);
            if(gldata[i].args[a].name[0] != 0)
                fprintf(f, ", ");
        }

        fprintf(f, ") ");

        if(addunderscore)
            fprintf(f, "_%s(", gldata[i].name);
        else
            fprintf(f, "%s(", gldata[i].name);

        a = 0;
        while(gldata[i].args[a].name[0] != 0)
        {
            fprintf(f, "%s", gldata[i].args[a++].name);
            if(gldata[i].args[a].name[0] != 0)
                fprintf(f, ", ");
        }

        fprintf(f, ")\n");
    }

    fprintf(f, "\n#else\n\n");

    for(i = 0; i < count; i++)
    {
        int a = 0;

        fprintf(f, "d_inline static %s", ReturnGLType(&gldata[i].header));
        if(gldata[i].header.pointer)
            fprintf(f, "*");

        fprintf(f, " %s_DEBUG (", gldata[i].name);

        while(gldata[i].args[a].name[0] != 0)
        {
            fprintf(f, "%s", ReturnGLType(&gldata[i].args[a]));
            if(gldata[i].args[a].pointer)
                fprintf(f, "*");

            fprintf(f, " %s", gldata[i].args[a++].name);
            fprintf(f, ", ");
        }

        fprintf(f, "const char* file, int line)\n");
        fprintf(f, "{\n");
        fprintf(f, "#ifdef LOG_GLFUNC_CALLS\n");
        fprintf(f, "    I_Printf(\"file = %%s, line = %%i, %s(", gldata[i].name);

        a = 0;
        while(gldata[i].args[a].name[0] != 0)
        {
            fprintf(f, "%s=", gldata[i].args[a].name);
            if(gldata[i].args[a].pointer)
                fprintf(f, "%%p");
            else
            {
                switch(gldata[i].args[a].type)
                {
                case 0:
                    break;
                case 1:
                    fprintf(f, "0x%%x");
                    break;
                case 2:
                    fprintf(f, "%%i");
                    break;
                case 3:
                    fprintf(f, "0x%%x");
                    break;
                case 4:
                    fprintf(f, "%%i");
                    break;
                case 5:
                    fprintf(f, "%%i");
                    break;
                case 6:
                    fprintf(f, "%%i");
                    break;
                case 7:
                    fprintf(f, "0x%%x");
                    break;
                case 8:
                    fprintf(f, "%%i");
                    break;
                case 9:
                    fprintf(f, "%%i");
                    break;
                case 10:
                    fprintf(f, "%%i");
                    break;
                case 11:
                    fprintf(f, "%%f");
                    break;
                case 12:
                    fprintf(f, "%%f");
                    break;
                case 13:
                    fprintf(f, "%%f");
                    break;
                case 14:
                    fprintf(f, "%%f");
                    break;
                case 15:
                    break;
                }
            }

            a++;

            if(gldata[i].args[a].name[0] != 0)
                fprintf(f, ", ");
        }

        fprintf(f, ")\\n\", file, line");

        if(gldata[i].args[0].name[0] != 0)
            fprintf(f, ", ");

        a = 0;
        while(gldata[i].args[a].name[0] != 0)
        {
            fprintf(f, "%s", gldata[i].args[a++].name);
            if(gldata[i].args[a].name[0] != 0)
                fprintf(f, ", ");
        }

        fprintf(f, ");\n");
        fprintf(f, "#endif\n");

        if(addunderscore)
            fprintf(f, "    _%s(", gldata[i].name);
        else
            fprintf(f, "    %s(", gldata[i].name);

        a = 0;
        while(gldata[i].args[a].name[0] != 0)
        {
            fprintf(f, "%s", gldata[i].args[a++].name);
            if(gldata[i].args[a].name[0] != 0)
                fprintf(f, ", ");
        }

        fprintf(f, ");\n");
        if(strcmp(gldata[i].name, "glBegin") && strcmp(gldata[i].name, "glEnd"))
            fprintf(f, "    dglLogError(\"%s\", file, line);\n", gldata[i].name);
        fprintf(f, "}\n\n");
    }

    fprintf(f, "\n");

    for(i = 0; i < count; i++)
    {
        int a = 0;

        fprintf(f, "#define d%s(", gldata[i].name);

        while(gldata[i].args[a].name[0] != 0)
        {
            fprintf(f, "%s", gldata[i].args[a++].name);
            if(gldata[i].args[a].name[0] != 0)
                fprintf(f, ", ");
        }

        fprintf(f, ") ");
        fprintf(f, "%s_DEBUG(", gldata[i].name);

        a = 0;
        while(gldata[i].args[a].name[0] != 0)
        {
            fprintf(f, "%s", gldata[i].args[a++].name);
            fprintf(f, ", ");
        }

        fprintf(f, "__FILE__, __LINE__)\n");
    }
}

//
// SlapGLExtensionsToFile
//

void SlapGLExtensionsToFile(FILE* f, char* arbstring)
{
    int i = 0;
    int j;

    SC_Open("SDL_opengl.h");

    fprintf(f, "//\n// %s\n//\n", arbstring);

    while(SC_ReadTokens())
    {
        SC_Find(false);

        if(!strcmp(sc_parser.token, arbstring))
        {
            SC_Find(false);

            if(strcmp(sc_parser.token, "1"))
                continue;

            while(SC_ReadTokens())
            {
                SC_Find(false);

                if(!strcmp(sc_parser.token, "#endif"))
                {
                    int count;

                    count = i;
                    i = 0;

                    while(i < count)
                    {
                        SC_Find(false);

                        if(!strcmp(sc_parser.token, "(APIENTRYP"))
                        {
                            int j = 0;
                            int len;

                            SC_Find(false);

                            strcpy(gldata[i].extname, sc_parser.token);

                            len = strlen(gldata[i].extname);
                            gldata[i].extname[len - 1] = 0;

                            while(!strrchr(sc_parser.token, ';'))
                            {
                                if(GetGLType(&gldata[i].args[j]))
                                    GetGLArg(&gldata[i].args[j]);
                                j++;
                            }

                            i++;
                        }
                    }

                    break;
                }
                else
                {
                    if(!strcmp(sc_parser.token, "GLAPI"))
                    {
                        GetGLType(&gldata[i].header);
                        SC_Find(false);
                        SC_Find(false);

                        if(sc_parser.token[0] == 'g' && sc_parser.token[1] == 'l')
                        {
                            strcpy(gldata[i].name, sc_parser.token);
                            i++;
                        }
                    }
                }
            }
        }
    }

    fprintf(f, "extern dboolean has_%s;\n", arbstring);

    if(i > 0)
        fprintf(f, "\n");

    for(j = 0; j < i; j++)
    {
        fprintf(f, "extern %s _%s;\n", gldata[j].extname, gldata[j].name);
    }

    fprintf(f, "\n");
    fprintf(f, "#define %s_Define() \\\n", arbstring);
    fprintf(f, "dboolean has_%s = false;", arbstring);

    if(i > 0)
        fprintf(f, " \\\n");
    else
        fprintf(f, "\n");

    for(j = 0; j < i; j++)
    {
        fprintf(f, "%s _%s = NULL", gldata[j].extname, gldata[j].name);
        if(j < (i - 1))
            fprintf(f, "; \\\n");
        else
            fprintf(f, "\n");
    }

    fprintf(f, "\n");

    fprintf(f, "#define %s_Init() \\\n", arbstring);
    fprintf(f, "has_%s = GL_CheckExtension(\"%s\");", arbstring, arbstring);

    if(i > 0)
        fprintf(f, " \\\n");
    else
        fprintf(f, "\n");

    for(j = 0; j < i; j++)
    {
        fprintf(f, "_%s = GL_RegisterProc(\"%s\")", gldata[j].name, gldata[j].name);
        if(j < (i - 1))
            fprintf(f, "; \\\n");
        else
            fprintf(f, "\n");
    }

    fprintf(f, "\n");

    if(i > 0)
    {
        fprintf(f, "#ifndef USE_DEBUG_GLFUNCS\n\n");

        SlapGLStuffToFile(f, i, true);
        fprintf(f, "\n#endif // USE_DEBUG_GLFUNCS\n\n");
    }

    SC_Close();
}

//
// main
//

int main(int argc, char** argv)
{
    FILE *f = fopen("dgl.h", "w");
    int i = 0;

    SC_Open("GL.H");

    while(SC_ReadTokens())
    {
        SC_Find(false);

        if(!stricmp(sc_parser.token, "WINGDIAPI"))
        {
            GetGLType(&gldata[i].header);
            SC_Find(false);

            if(!stricmp(sc_parser.token, "APIENTRY"))
            {
                SC_Find(false);
                if(sc_parser.token[0] == 'g' && sc_parser.token[1] == 'l')
                {
                    int j = 0;

                    strcpy(gldata[i].name, sc_parser.token);

                    while(!strrchr(sc_parser.token, ';'))
                    {
                        if(GetGLType(&gldata[i].args[j]))
                            GetGLArg(&gldata[i].args[j]);
                        j++;
                    }

                    i++;
                }
            }
        }
    }

    fprintf(f, "//\n// Generated by dglmake\n//\n");
    fprintf(f, "#ifndef USE_DEBUG_GLFUNCS\n\n");
    SlapGLStuffToFile(f, i, false);
    fprintf(f, "\n#endif // USE_DEBUG_GLFUNCS\n\n");

    SC_Close();

    SlapGLExtensionsToFile(f, "GL_ARB_multitexture");
    SlapGLExtensionsToFile(f, "GL_EXT_compiled_vertex_array");
    SlapGLExtensionsToFile(f, "GL_EXT_multi_draw_arrays");
    SlapGLExtensionsToFile(f, "GL_EXT_fog_coord");
    SlapGLExtensionsToFile(f, "GL_ARB_vertex_buffer_object");
    SlapGLExtensionsToFile(f, "GL_ARB_texture_non_power_of_two");
    SlapGLExtensionsToFile(f, "GL_ARB_texture_env_combine");
    SlapGLExtensionsToFile(f, "GL_EXT_texture_env_combine");
    SlapGLExtensionsToFile(f, "GL_EXT_texture_filter_anisotropic");

    fclose(f);

    return 1;
}


