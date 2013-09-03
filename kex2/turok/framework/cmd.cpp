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
#include "js.h"

static void FCmd_Stub(void);
static void FCmd_List(void);
static void FCmd_Exec(void);

kexCommand command;

//
// kexCommand::Run
//

bool kexCommand::Run(void) {
    cmd_function_t *cmd;

    if(!cmd_argc) {
        return false;
    }

    for(cmd = cmd_functions; cmd; cmd = cmd->next) {
        if(!strcasecmp(cmd_argv[0], cmd->name)) {
            if(cmd->object) {
                int i;
                char **argv = (char**)(Z_Alloca(sizeof(char*) * cmd_argc));

                for(i = 0; i < cmd_argc-1; i++)
                    argv[i] = Z_Strdupa(cmd_argv[i+1]);

                J_CallObject(cmd->object, argv, cmd_argc-1);
            }
            else if(cmd->function) {
                cmd->function();
            }

            return true;
        }
    }

    return false;
}

//
// kexCommand::ClearArgv
//

void kexCommand::ClearArgv(void) {
    memset(cmd_argv, 0, CMD_MAX_ARGV * CMD_BUFFER_LEN);
}

//
// kexCommand::GetArgc
//

int kexCommand::GetArgc(void) {
    return cmd_argc;
}

//
// kexCommand::GetArgv
//

char *kexCommand::GetArgv(int argv) {
    return cmd_argv[argv];
}

//
// kexCommand::Execute
//

void kexCommand::Execute(const char *buffer) {
    int len;
    int j;
    char *b_rover;
    char *a_rover;
    kbool havetoken;
    kbool inquotes;

    j = 0;
    cmd_argc = 0;
    ClearArgv();
    len = strlen(buffer);
    b_rover = (char*)buffer;
    havetoken = false;
    inquotes = false;

    while(1) {
        if(b_rover - buffer > len) {
            // end of buffer
            return;
        }

        a_rover = cmd_argv[cmd_argc];

        // skip spaces
        if(*b_rover == ' ') {
            while(*b_rover == ' ') {
                b_rover++;
            }
        }

        havetoken = false;

        // search for a token
        while(*b_rover != ' ' || inquotes) {
            if(*b_rover == '"') {
                if(inquotes) {
                    inquotes = false;
                }
                else {
                    inquotes = true;
                }

                b_rover++;
                continue;
            }

            // execute commands after a newline or semicolon
            if(*b_rover == '\n' || (*b_rover == ';' && !inquotes) || *b_rover == 0 && havetoken) {
                if(inquotes) {
                    common.Warning("Command contains incomplete quote\n");
                    return;
                }

                if(!Run()) {
                    kexCvar *cvar;

                    // check to see if command maches a cvar
                    if(cvar = cvarManager.Get(cmd_argv[0])) {
                        if(cmd_argc == 1) {
                            common.Printf("%s: %s (%s)\n", cvar->GetName(),
                                cvar->GetValue(), cvar->GetDefaultValue());
                        }
                        else {
                            cvarManager.Set(cvar->GetName(), cmd_argv[1]);
                        }
                    }
                    else {
                        // no match, assume typo
                        common.Warning("Unknown command: %s\n", cmd_argv[0]);
                    }
                }

                cmd_argc = 0;
                j = 0;
                havetoken = false;
                memset(cmd_argv, 0, CMD_MAX_ARGV*CMD_BUFFER_LEN);
                a_rover = cmd_argv[cmd_argc];
                b_rover++;
            }
            else if(*b_rover < ' ') {
                b_rover++;
                continue;
            }
            else {
                *(a_rover++) = *(b_rover++);

                // a token as been found; increment argc
                if(!havetoken) {
                    if(++cmd_argc >= CMD_MAX_ARGV) {
                        break;
                    }

                    havetoken = true;
                }

                if(++j >= CMD_BUFFER_LEN) {
                    common.Warning("Command string is too long (%i limit)\n",
                        CMD_BUFFER_LEN);
                    return;
                }
            }

            if(b_rover - buffer > len) {
                // end of buffer
                return;
            }
        }
    }
}

//
// kexCommand::AutoComplete
//

bool kexCommand::AutoComplete(const char *partial) {
    int len = strlen(partial);
	
    if(!len) {
        return false;
    }

    bool ok = false;
		
    // check for exact match
    for(cmd_function_t *cmd = cmd_functions; cmd; cmd = cmd->next) {
        if(!strcmp(partial, cmd->name)) {
            if(!ok) {
                common.Printf("\n");
            }

            common.Printf("%s\n", cmd->name);
            ok = true;
        }
    }

    // check for partial match
    for(cmd_function_t *cmd = cmd_functions; cmd; cmd = cmd->next) {
        if(!strncmp(partial,cmd->name, len)) {
            if(!ok) {
                common.Printf("\n");
            }

            common.Printf("%s\n", cmd->name);
            ok = true;
        }
    }
    
    return ok;
}

//
// kexCommand::Verify
//

bool kexCommand::Verify(const char *name) {
    cmd_function_t *cmd;

    // fail if the command is a variable name
    if(cvarManager.Get(name)) {
        common.Warning("Cmd_AddCommand: %s already defined as a var\n", name);
        return false;
    }
    
    // fail if the command already exists
    for(cmd = cmd_functions; cmd; cmd = cmd->next) {
        if(!strcmp(name, cmd->name)) {
            common.Warning("Cmd_AddCommand: %s already defined\n", name);
            return false;
        }
    }

    return true;
}

//
// kexCommand::Add
//

void kexCommand::Add(const char *name, cmd_t function) {
	cmd_function_t *cmd;
	
    if(!Verify(name))
        return;

    cmd             = (cmd_function_t*)(Z_Malloc(sizeof(cmd_function_t), PU_STATIC, 0));
    cmd->name       = name;
    cmd->function   = function;
    cmd->next       = cmd_functions;
    cmd->object     = NULL;
    cmd_functions   = cmd;
}

//
// kexCommand::AddObject
//

void kexCommand::Add(const char *name, gObject_t *object) {
	cmd_function_t *cmd;
	
    if(!Verify(name))
        return;

    cmd             = (cmd_function_t*)(Z_Malloc(sizeof(cmd_function_t), PU_STATIC, 0));
    cmd->name       = Z_Strdup(name, PU_STATIC, 0);
    cmd->function   = FCmd_Stub;
    cmd->next       = cmd_functions;
    cmd->object     = object;
    cmd_functions   = cmd;
}

//
// kexCommand::GetFunctions
//

cmd_function_t *kexCommand::GetFunctions(void) {
    return cmd_functions;
}

//
// kexCommand::Init
//

void kexCommand::Init(void) {
    cmd_functions = NULL;

    Add("listcmds", FCmd_List);
    Add("exec", FCmd_Exec);

    common.Printf("Command System Initialized\n");
}

//
// FCmd_List
//

static void FCmd_List(void) {
    cmd_function_t *cmd;
    int i = 0;

    for(cmd = command.GetFunctions(); cmd; cmd = cmd->next, i++) {
        common.CPrintf(COLOR_CYAN, "%s\n", cmd->name);
    }
    
    common.CPrintf(COLOR_GREEN, "%i commands\n", i);
}

//
// FCmd_Exec
//

static void FCmd_Exec(void) {
}

//
// FCmd_Stub
//

static void FCmd_Stub(void) {
}
