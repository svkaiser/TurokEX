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
// DESCRIPTION: Definition system
//
//-----------------------------------------------------------------------------

#include "common.h"
#include "defs.h"
#include "cachefilelist.h"

kexDefManager defManager;

//
// kexDefinition::kexDefinition
//

kexDefinition::kexDefinition(void) {
}

//
// kexDefinition::~kexDefinition
//

kexDefinition::~kexDefinition(void) {
}

//
// kexDefinition::Parse
//

void kexDefinition::Parse(kexLexer *lexer) {
    defEntry_t *defEntry;
    kexStr key;
    kexStr val;

    while(lexer->CheckState()) {
        lexer->Find();

        switch(lexer->TokenType()) {
        case TK_EOF:
            return;
        case TK_IDENIFIER:
            defEntry = entries.Add(lexer->Token());
            defEntry->name = lexer->Token();

            lexer->ExpectNextToken(TK_LBRACK);
            while(1) {
                lexer->Find();
                if(lexer->TokenType() == TK_RBRACK || lexer->TokenType() == TK_EOF) {
                    break;
                }

                key = lexer->Token();

                lexer->Find();
                if(lexer->TokenType() == TK_RBRACK || lexer->TokenType() == TK_EOF) {
                    break;
                }

                val = lexer->Token();

                defEntry->key.Add(key.c_str(), val.c_str());
            }
            break;
        default:
            break;
        }
    }
}

//
// kexDefManager::kexDefManager
//

kexDefManager::kexDefManager(void) {
}

//
// kexDefManager::~kexDefManager
//

kexDefManager::~kexDefManager(void) {
}

//
// kexDefManager::LoadDefinition
//

kexDefinition *kexDefManager::LoadDefinition(const char *file) {
    kexDefinition *def = NULL;

    if(file == NULL || file[0] == 0) {
        return NULL;
    }

    if(!(def = defs.Find(file))) {
        kexLexer *lexer;
        kexStr fileStr(file);

        if(fileStr.Length() >= MAX_FILEPATH) {
            common.Error("kexDefManager::LoadDefinition: \"%s\" is too long", file);
        }

        fileStr.StripExtension();

        if(!(lexer = parser.Open(file))) {
            return NULL;
        }

        def = defs.Add(fileStr.c_str());
        strncpy(def->fileName, file, MAX_FILEPATH);

        def->Parse(lexer);

        // we're done with the file
        parser.Close();
    }

    return def;
}

//
// kexDefManager::FindDefEntry
//
// Retrives an entry in a definition file
// The name should be <path of def file>@<entry name>
// Example: defs/damage.def@MeleeBlunt
//

defEntry_t *kexDefManager::FindDefEntry(const char *name) {
    kexDefinition *def;
    defEntry_t *defEntry;
    char tStr[64];
    int pos;
    int len;

    pos = kexStr::IndexOf(name, "@");

    if(pos == -1) {
        return NULL;
    }

    len = strlen(name);
    strncpy(tStr, name, pos);
    tStr[pos] = 0;

    if(def = LoadDefinition(tStr)) {
        strncpy(tStr, name + pos + 1, len - pos);
        tStr[len - pos] = 0;

        if(defEntry = def->entries.Find(tStr)) {
            return defEntry;
        }
    }

    return NULL;
}
