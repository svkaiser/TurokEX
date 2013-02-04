// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 1993-1997 Id Software, Inc.
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
// DESCRIPTION: Console cvar functionality (from Quake)
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "zone.h"

cvar_t  *cvarcap = NULL;

//
// FCmd_ListVars
//

static void FCmd_ListVars(void)
{
    cvar_t *var;

    Com_CPrintf(COLOR_GREEN, "Available cvars:\n");

    for(var = cvarcap; var; var = var->next)
    {
        Com_CPrintf(COLOR_CYAN, "%s\n", var->name);
    }
}

//
// FCmd_Seta
//

static void FCmd_Seta(void)
{
    if(Cmd_GetArgc() < 2)
    {
        return;
    }

    Cvar_Set(Cmd_GetArgv(1), Cmd_GetArgv(2));
}

//
// Cvar_Get
//

cvar_t *Cvar_Get(const char *name)
{
    cvar_t *var;
    
    for(var = cvarcap; var; var = var->next)
    {
        if(!strcmp(name, var->name))
            return var;
    }
    
    return NULL;
}

//
// Cvar_Value
//

float Cvar_Value(const char *name)
{
    cvar_t	*var;
    
    var = Cvar_Get(name);
    if(!var)
        return 0;
    
    return (float)atof(var->string);
}

//
// Cvar_String
//

char *Cvar_String(const char *name)
{
    cvar_t *var;
    
    var = Cvar_Get(name);
    if(!var)
        return "";
    
    return var->string;
}

//
// Cvar_AutoComplete
//

void Cvar_AutoComplete(const char *partial)
{
    cvar_t*     cvar;
    int         len;
    char*       name = NULL;
    kbool       match = false;
    
    strlwr((char*)partial);
    
    len = strlen(partial);
    
    if(!len)
    {
        return;
    }
    
    // check functions
    for(cvar = cvarcap; cvar; cvar = cvar->next)
    {
        if(!strncmp(partial, cvar->name, len))
        {
            if(!match)
            {
                match = true;
                Com_Printf("\n");
            }

            name = cvar->name;

            // print all matching cvars
            Com_Printf("%s\n", name);
        }
    }
}

//
// Cvar_Set
//

void Cvar_Set(const char *var_name, const char *value)
{
    cvar_t *var;
    kbool changed;
    
    var = Cvar_Get(var_name);
    if(!var)
    {	// there is an error in C code if this happens
        Com_Warning("Cvar_Set: variable %s not found\n", var_name);
        return;
    }
    
    changed = strcmp(var->string, value);
    
    Z_Free(var->string);	// free the old value string
    
    var->string = Z_Malloc(strlen(value)+1, PU_STATIC, 0);
    strcpy(var->string, value);
    var->value = (float)atof(var->string);

    if(var->callback)
        var->callback(var);
}

//
// Cvar_SetValue
//

void Cvar_SetValue(const char *var_name, float value)
{
    char val[32];
    
    Cvar_Set(var_name, kva(val, "%f",value));
}

//
// Cvar_WriteToFile
//

void Cvar_WriteToFile(FILE *file)
{
    cvar_t *cvar;

    for(cvar = cvarcap; cvar; cvar = cvar->next)
    {
        fprintf(file, "seta %s \"%s\"\n", cvar->name, cvar->string);
    }
}

//
// Cvar_Register
//

void Cvar_Register(cvar_t *variable)
{
    char *oldstr;
    
    // first check to see if it has allready been defined
    if(Cvar_Get(variable->name))
    {
        Com_Warning("Cvar_Register: Can't register variable %s, already defined\n", variable->name);
        return;
    }
    
    // copy the value off, because future sets will Z_Free it
    oldstr = variable->string;
    variable->string = Z_Malloc(strlen(variable->string)+1, PU_STATIC, 0);	
    strcpy(variable->string, oldstr);
    variable->value = (float)atof(variable->string);
    variable->defvalue = Z_Malloc(strlen(variable->string)+1, PU_STATIC, 0);
    strcpy(variable->defvalue, variable->string);
    
    // link the variable in
    variable->next = cvarcap;
    cvarcap = variable;
}

//
// Cvar_Init
//

void Cvar_Init(void)
{
    Cmd_AddCommand("listvars", FCmd_ListVars);
    Cmd_AddCommand("seta", FCmd_Seta);
}

