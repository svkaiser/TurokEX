// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 2007-2012 Samuel Villarreal
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

#ifndef __CMD_H__
#define __CMD_H__

//
// COMMANDS
//
typedef void (*cmd_t)(void);

typedef struct cmd_function_s {
	struct cmd_function_s   *next;
	const char              *name;
	cmd_t                   function;
    void                    *object;
} cmd_function_t;

class kexCommand {
public:
    int                 GetArgc(void);
    char                *GetArgv(int argv);
    void                Execute(const char *buffer);
    bool                AutoComplete(const char *partial);
    void                Add(const char *name, cmd_t function);
    void                Add(const char *name, gObject_t *object);
    void                Init(void);
    cmd_function_t      *GetFunctions(void);
    bool                Verify(const char *name);

private:
    const static int    CMD_MAX_ARGV    = 32;
    const static int    CMD_BUFFER_LEN  = 256;

    void                ClearArgv(void);
    bool                Run(void);

    cmd_function_t      *cmd_functions;
    int                 cmd_argc;
    char                cmd_argv[CMD_MAX_ARGV][CMD_BUFFER_LEN];
};

extern kexCommand command;

#endif
