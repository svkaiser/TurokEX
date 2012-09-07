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
// DESCRIPTION: Command System
//
//-----------------------------------------------------------------------------

#include <string.h>
#include "common.h"
#include "zone.h"

typedef struct cmd_function_s
{
	struct cmd_function_s   *next;
	char                    *name;
	cmd_t                   function;
} cmd_function_t;

static cmd_function_t *cmd_functions = NULL;

#define CMD_MAX_ARGV 32
#define CMD_BUFFER_LEN 256

static int cmd_argc;
static char cmd_argv[CMD_MAX_ARGV][CMD_BUFFER_LEN];

//
// Cmd_RunCommand
//

kbool Cmd_RunCommand(void)
{
    cmd_function_t *cmd;

    if(!cmd_argc)
    {
        return false;
    }

    for(cmd = cmd_functions; cmd; cmd = cmd->next)
    {
        if(!strcasecmp(cmd_argv[0], cmd->name))
        {
            cmd->function();
            return true;
        }
    }

    return false;
}

//
// Cmd_GetArgc
//

int Cmd_GetArgc(void)
{
    return cmd_argc;
}

//
// Cmd_GetArgv
//

char *Cmd_GetArgv(int argv)
{
    return cmd_argv[argv];
}

//
// Cmd_ExecuteCommand
//

void Cmd_ExecuteCommand(char *buffer)
{
    int len;
    int j;
    char *b_rover;
    char *a_rover;
    kbool havetoken;

    j = 0;
    cmd_argc = 0;
    len = strlen(buffer);
    b_rover = buffer;
    havetoken = false;

    while(1)
    {
        if(b_rover - buffer > len)
        {
            // end of buffer
            return;
        }

        a_rover = cmd_argv[cmd_argc];

        // skip spaces
        if(*b_rover == ' ')
        {
            while(*b_rover == ' ')
            {
                b_rover++;
            }
        }

        havetoken = false;

        // search for a token
        while(*b_rover != ' ')
        {
            // execute commands after a newline or semicolon
            if(*b_rover == '\n' || *b_rover == ';' || *b_rover == 0 && havetoken)
            {
                if(!Cmd_RunCommand())
                {
                    cvar_t *cvar;

                    // check to see if command maches a cvar
                    if(cvar = Cvar_Get(cmd_argv[0]))
                    {
                        if(cmd_argc == 1)
                        {
                            Com_Printf("%s: %s (%s)\n", cvar->name, cvar->string, cvar->defvalue);
                        }
                        else
                        {
                            Cvar_Set(cvar->name, cmd_argv[1]);
                        }
                    }
                    else
                    {
                        // no match, assume typo
                        Com_Warning("Unknown command: %s\n", cmd_argv[0]);
                    }
                }

                cmd_argc = 0;
                j = 0;
                havetoken = false;
                memset(cmd_argv, 0, CMD_MAX_ARGV*CMD_BUFFER_LEN);
                a_rover = cmd_argv[cmd_argc];
                b_rover++;
            }
            else if(*b_rover < ' ')
            {
                b_rover++;
                continue;
            }
            else
            {
                *(a_rover++) = *(b_rover++);

                // a token as been found; increment argc
                if(!havetoken)
                {
                    if(++cmd_argc >= CMD_MAX_ARGV)
                    {
                        break;
                    }

                    havetoken = true;
                }

                if(++j >= CMD_BUFFER_LEN)
                {
                    Com_Warning("Command string is too long (%i limit)\n",
                        CMD_BUFFER_LEN);
                    return;
                }
            }

            if(b_rover - buffer > len)
            {
                // end of buffer
                return;
            }
        }
    }
}

//
// Cmd_CompleteCommand
//

kbool Cmd_CompleteCommand(char *partial)
{
    cmd_function_t *cmd;
    int len;
    kbool ok;
	
    len = strlen(partial);
	
    if(!len)
    {
        return false;
    }

    ok = false;
		
    // check for exact match
    for(cmd = cmd_functions; cmd; cmd = cmd->next)
    {
        if(!strcmp(partial, cmd->name))
        {
            if(!ok)
            {
                Com_Printf("\n");
            }

            Com_Printf("%s\n", cmd->name);
            ok = true;
        }
    }

    // check for partial match
    for(cmd = cmd_functions; cmd; cmd = cmd->next)
    {
        if(!strncmp(partial,cmd->name, len))
        {
            if(!ok)
            {
                Com_Printf("\n");
            }

            Com_Printf("%s\n", cmd->name);
            ok = true;
        }
    }
    
    return ok;
}

//
// Cmd_AddCommand
//

void Cmd_AddCommand(char *name, cmd_t function)
{
	cmd_function_t *cmd;
	
    // fail if the command is a variable name
    if(Cvar_String(name)[0])
    {
        Com_Warning("Cmd_AddCommand: %s already defined as a var\n", name);
        return;
    }
    
    // fail if the command already exists
    for(cmd = cmd_functions; cmd; cmd = cmd->next)
    {
        if(!strcmp(name, cmd->name))
        {
            Com_Warning("Cmd_AddCommand: %s already defined\n", name);
            return;
        }
    }

    cmd = Z_Malloc(sizeof(cmd_function_t), PU_STATIC, 0);
    cmd->name = name;
    cmd->function = function;
    cmd->next = cmd_functions;
    cmd_functions = cmd;
}

//
// FCmd_List
//

static void FCmd_List(void)
{
    cmd_function_t *cmd;
    int i;

    i = 0;

    for(cmd = cmd_functions; cmd; cmd = cmd->next, i++)
    {
        Com_CPrintf(COLOR_CYAN, "%s\n", cmd->name);
    }
    
    Com_CPrintf(COLOR_GREEN, "%i commands\n", i);
}

//
// FCmd_Exec
//

static void FCmd_Exec(void)
{
}

//
// Cmd_Init
//

void Cmd_Init(void)
{
    Cmd_AddCommand("listcmds", FCmd_List);
    Cmd_AddCommand("exec", FCmd_Exec);
}

