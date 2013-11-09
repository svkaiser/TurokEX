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
// DESCRIPTION: Debug statistics
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "client.h"
#include "server.h"
#include "render.h"

static kbool bStatPerf = false;
static kbool bStatMemory = false;

static unsigned statFrame = 0;

#define MAX_STAT_NAME 32

typedef struct
{
    union
    {
        int     *i;
        float   *f;
    } value;

    float avgValue;
    float sumValue;
    kbool bFloat;
    char name[MAX_STAT_NAME];
} statData_t;

typedef struct
{
    statData_t *data;
    int count;
} debugStats_t;

static float debugStats_RowOffset = 0;
static debugStats_t perfStats;

//
// Debug_RegisterPerfStatVar
//

void Debug_RegisterPerfStatVar(float *value, const char *name, kbool bFloat)
{
    perfStats.data = (statData_t*)Mem_Realloc(perfStats.data,
        sizeof(statData_t) * (++perfStats.count), hb_static);

    if(bFloat)
        perfStats.data[perfStats.count-1].value.f = value;
    else
        perfStats.data[perfStats.count-1].value.i = (int*)value;

    perfStats.data[perfStats.count-1].avgValue = *value;
    perfStats.data[perfStats.count-1].bFloat = bFloat;
    strncpy(perfStats.data[perfStats.count-1].name, name, MAX_STAT_NAME);
}

//
// Debug_UpdateStatFrame
//

void Debug_UpdateStatFrame(void)
{
    int i;

    if(bStatPerf)
    {
        for(i = 0; i < perfStats.count; i++)
        {
            statData_t *stat;

            stat = &perfStats.data[i];

            if(stat->bFloat)
                stat->sumValue += *stat->value.f;
            else
                stat->sumValue += (float)*stat->value.i;

            if(!(statFrame & 3))
            {
                float avg = (stat->sumValue / 4.0f);

                if(avg != 0)
                {
                    stat->avgValue = avg;
                    stat->sumValue = 0;
                }
            }
        }
    }

    statFrame++;
}

//
// Debug_DrawStats
//

void Debug_DrawStats(void)
{
    int i;

    debugStats_RowOffset = 0;

    if(bStatPerf)
    {
        for(i = 0; i < perfStats.count; i++)
        {
            statData_t *stat;

            stat = &perfStats.data[i];

            //Draw_Text(32, 64 + debugStats_RowOffset, COLOR_GREEN, 1, "%s: %f",
                //stat->name, stat->avgValue);

            debugStats_RowOffset += 16;
        }
    }
}

//
// FCmd_StatPerf
//

static void FCmd_StatPerf(void)
{
    char buf[256];

    if(command.GetArgc() < 2)
        return;

    strncpy(buf, command.GetArgv(1), 256);

    if(!strcmp(buf, "perf"))
        bStatPerf ^= 1;
    else if(!strcmp(buf, "mem"))
        bStatMemory ^= 1;
}

//
// Debug_Init
//

void Debug_Init(void)
{
    command.Add("stat", FCmd_StatPerf);
}
