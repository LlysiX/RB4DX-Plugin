/*
    shadps4_assist.c - RB4DX-Plugin
    Helper hook(s) for shadPS4

    Copyright (C) 2025 Emma / InvoxiPlayGames
                  2025 RBEnhanced https://rbenhanced.rocks/

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, see <https://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <time.h>
#include <GoldHEN/Common.h>
#include <orbis/libkernel.h>
#include <orbis/Sysmodule.h>
#include "rb4/data.h"
#include "plugin_common.h"

// HACK(Emma): shadPS4 has a bug where addcont files aren't seen as raw files for some reason
//             remove me when this is resolved in upstream shadPS4
// original definition:
// HOOK_C(bool, CheckCache, (Symbol path, const char *suffix, FixedString &cached_path, bool &is_raw, struct TimeSpec &timestamp))
HOOK_INIT(CheckCache);
bool (*CheckCache)(Symbol path, const char *suffix, void *cached_path, bool *is_raw, void *timestamp);
bool CheckCache_hook(Symbol path, const char *suffix, void *cached_path, bool *is_raw, void *timestamp) {
    bool r = HOOK_CONTINUE(CheckCache, bool(*)(Symbol, const char *, void *, bool *, void *), path, suffix, cached_path, is_raw, timestamp);
    if (strncmp(path.sym, "addcont", 7) == 0)
        *is_raw = true;
    return r;
}

void InitShadHooks()
{
    uint64_t base_address = get_base_address();
    CheckCache = (void*)(base_address + 0x001ad8b0);
    HOOK(CheckCache);
}

void DestroyShadHooks()
{
    HOOK(CheckCache);
}
