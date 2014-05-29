// Emacs style mode select   -*- C++ -*- 
//-----------------------------------------------------------------------------
//
// Copyright(C) 2014 Samuel Villarreal
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
// DESCRIPTION: Base System API
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "systemBase.h"

//
// kexSystemBase::kexSystemBase
//

kexSystemBase::kexSystemBase(void) {
    this->bShuttingDown = false;
}

//
// kexSystemBase::Sleep
//

void kexSystemBase::Sleep(unsigned long usecs) {
}

//
// kexSystemBase::GetMS
//

int kexSystemBase::GetMS(void) {
    return 0;
}

//
// kexSystemBase::GetTicks
//

int kexSystemBase::GetTicks(void) {
    return 0;
}

//
// kexSystemBase::SpawnInternalConsole
//

void kexSystemBase::SpawnInternalConsole(void) {
}

//
// kexSystemBase::ShowInternalConsole
//

void kexSystemBase::ShowInternalConsole(bool show) {
}

//
// kexSystemBase::DestroyInternalConsole
//

void kexSystemBase::DestroyInternalConsole(void) {
}

//
// kexSystemBase::GetWindowFlags
//

int kexSystemBase::GetWindowFlags(void) {
    return 0;
}

//
// kexSystemBase::Log
//

void kexSystemBase::Log(const char *fmt, ...) {
}

//
// kexSystemBase::GetWindowTitle
//

const char *kexSystemBase::GetWindowTitle(void) {
    return NULL;
}

//
// kexSystemBase::SetWindowTitle
//

void kexSystemBase::SetWindowTitle(const char *string) {
}

//
// kexSystemBase::SetWindowGrab
//

void kexSystemBase::SetWindowGrab(const bool bEnable) {
}

//
// kexSystemBase::WarpMouseToCenter
//

void kexSystemBase::WarpMouseToCenter(void) {
}

//
// kexSystemBase::SwapLE16
//

short kexSystemBase::SwapLE16(const short val) {
    return val;
}

//
// kexSystemBase::SwapBE16
//

short kexSystemBase::SwapBE16(const short val) {
    return val;
}

//
// kexSystemBase::SwapLE32
//

int kexSystemBase::SwapLE32(const int val) {
    return val;
}

//
// kexSystemBase::SwapBE32
//

int kexSystemBase::SwapBE32(const int val) {
    return val;
}

//
// kexSystemBase::GetProcAddress
//

void *kexSystemBase::GetProcAddress(const char *proc) {
    return NULL;
}

//
// kexSystemBase::CheckParam
//

int kexSystemBase::CheckParam(const char *check) {
    return -1;
}

//
// kexSystemBase::Printf
//

void kexSystemBase::Printf(const char *string, ...) {
}

//
// kexSystemBase::CPrintf
//

void kexSystemBase::CPrintf(rcolor color, const char *string, ...) {
}

//
// kexSystemBase::Warning
//

void kexSystemBase::Warning(const char *string, ...) {
}

//
// kexSystemBase::DPrintf
//

void kexSystemBase::DPrintf(const char *string, ...) {
}

//
// kexSystemBase::Error
//

void kexSystemBase::Error(const char* string, ...) {
}

//
// kexSystemBase::GetBaseDirectory
//

const char *kexSystemBase::GetBaseDirectory(void) {
    return NULL;
}
