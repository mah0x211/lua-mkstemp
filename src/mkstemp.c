/**
 *  Copyright (C) 2022 Masatoshi Fukunaga
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to
 *  deal in the Software without restriction, including without limitation the
 *  rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 *  sell copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 *  IN THE SOFTWARE.
 */

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
// lua
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>

static inline FILE *fd2fp(int fd)
{
    FILE *fp = fdopen(fd, "r+");

    if (fp) {
        return fp;
    }
    close(fd);

    return NULL;
}

static int REF_IO_TMPFILE = LUA_NOREF;

static inline void swap_fp(lua_State *L, FILE *fp)
{
#if LUA_VERSION_NUM >= 502
    luaL_Stream *stream = luaL_checkudata(L, -1, LUA_FILEHANDLE);
    FILE *tmpfp         = stream->f;

    stream->f = fp;
    fclose(tmpfp);

#else
    FILE **tmpfp = (FILE **)luaL_checkudata(L, -1, LUA_FILEHANDLE);
    fclose(*tmpfp);
    *tmpfp = fp;
#endif
}

static int mkstemp_lua(lua_State *L)
{
    char *tmpl = (char *)luaL_checkstring(L, 1);
    int fd     = mkstemp(tmpl);
    FILE *fp   = NULL;
    int rc     = 0;

    if (fd == -1) {
        lua_pushnil(L);
        lua_pushstring(L, strerror(errno));
        lua_pushinteger(L, errno);
        return 3;
    }

    fp = fd2fp(fd);
    if (fp == NULL) {
        lua_pushnil(L);
        lua_pushstring(L, strerror(errno));
        lua_pushinteger(L, errno);
        return 3;
    }

    lua_settop(L, 0);
    lua_rawgeti(L, LUA_REGISTRYINDEX, REF_IO_TMPFILE);
    lua_call(L, 0, LUA_MULTRET);
    rc = lua_gettop(L);
    if (rc != 1) {
        return rc;
    }
    swap_fp(L, fp);

    return 1;
}

LUALIB_API int luaopen_mkstemp(lua_State *L)
{
    REF_IO_TMPFILE = LUA_NOREF;
    lua_getglobal(L, "io");
    if (lua_istable(L, -1)) {
        lua_pushliteral(L, "tmpfile");
        lua_rawget(L, -2);
        if (lua_isfunction(L, -1)) {
            REF_IO_TMPFILE = luaL_ref(L, LUA_REGISTRYINDEX);
        }
    }

    lua_pushcfunction(L, mkstemp_lua);
    return 1;
}
