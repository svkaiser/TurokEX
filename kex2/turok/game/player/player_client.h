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

#ifndef _PLAYER_CLIENT_H_
#define _PLAYER_CLIENT_H_

//-----------------------------------------------------------------------------
//
// kexLocalPlayer
//
//-----------------------------------------------------------------------------

BEGIN_EXTENDED_CLASS(kexLocalPlayer, kexPlayer);
public:
                            kexLocalPlayer(void);
                            ~kexLocalPlayer(void);

    virtual void            LocalTick(void);

    bool                    ProcessInput(event_t *ev);
    void                    BuildCommands(void);
    bool                    ActionDown(const kexStr &str);
    int                     ActionHeldTime(const kexStr &str);

    kexVec3                 &MoveDiff(void) { return moveDiff; }
    kexActor                *ToWorldActor(void) { return static_cast<kexActor*>(this); }

    static void             InitObject(void);

private:
    int                     latency[NETBACKUPS];
    kexVec3                 moveDiff;
    kexVec3                 oldMoves[NETBACKUPS];
    ticcmd_t                oldCmds[NETBACKUPS];
END_CLASS();

#endif
