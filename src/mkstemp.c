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

// depend
#include "lua_errno.h"
// lua
#include <lauxlib.h>
#include <lualib.h>
// system
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static inline FILE *fd2fp(int fd)
{
    FILE *fp = fdopen(fd, "r+");

    if (fp) {
        return fp;
    }
    close(fd);

    return NULL;
}

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
    size_t len     = 0;
    char *tmpl     = (char *)luaL_checklstring(L, 1, &len);
    char *buf      = NULL;
    size_t bufsize = 0;
    int fd         = 0;
    int rc         = 0;
    FILE *fp       = NULL;

    lua_settop(L, 1);
    bufsize = (size_t)lua_tointeger(L, lua_upvalueindex(1));
    buf     = (char *)lua_touserdata(L, lua_upvalueindex(2));
    if (!buf) {
        // no size limit to check
        bufsize = len;
        buf     = lua_newuserdata(L, bufsize + 1);
    } else if (len > bufsize) {
        lua_pushnil(L);
        errno = ENAMETOOLONG;
        lua_errno_new(L, errno, "mkstemp");
        return 2;
    }
    memcpy(buf, tmpl, len);
    buf[len] = 0;
    if (lua_gettop(L) > 1) {
        // replace the template argument with the created buffer
        lua_remove(L, 1);
        lua_settop(L, 1);
    }

    // create a temporary file
    fd = mkstemp(buf);
    if (fd == -1) {
        lua_pushnil(L);
        lua_errno_new(L, errno, "mkstemp");
        return 2;
    }
    // convert file descriptor to file pointer
    fp = fd2fp(fd);
    if (fp == NULL) {
        lua_pushnil(L);
        lua_errno_new(L, errno, "mkstemp");
        return 2;
    }

    // call io.tmpfile to create a temporary file and swap the file pointer with
    // the one created by mkstemp
    lua_pushvalue(L, lua_upvalueindex(3));
    lua_call(L, 0, LUA_MULTRET);
    rc = lua_gettop(L);
    if (rc != 2) {
        lua_pushnil(L);
        lua_errno_new(L, errno, "mkstemp");
        return 2;
    }
    // return the file pointer and the generated filename
    swap_fp(L, fp);
    lua_pushnil(L);
    lua_pushlstring(L, buf, len);
    return 3;
}

LUALIB_API int luaopen_mkstemp(lua_State *L)
{
    long pathmax = pathconf(".", _PC_PATH_MAX);

    lua_errno_loadlib(L);

    // set the maximum number of bytes in a pathname
    if (pathmax != -1) {
        size_t bufsiz = (size_t)pathmax;
        // upvalue 1: template buffer size
        lua_pushinteger(L, (lua_Integer)bufsiz);
        // upvalue 2: template buffer
        lua_newuserdata(L, bufsiz + 1);
    } else {
        // _PC_PATH_MAX is indeterminate: no constraint on the length
        // pathname upvalue 1: no size limit to check
        lua_pushinteger(L, 0);
        // upvalue 2: NULL signals no constraint on the length
        lua_pushlightuserdata(L, NULL);
    }
    // upvalue 3: io.tmpfile function
    lua_getglobal(L, "io");
    if (lua_istable(L, -1)) {
        lua_getfield(L, -1, "tmpfile");
        if (lua_isfunction(L, -1)) {
            lua_remove(L, -2);
            lua_pushcclosure(L, mkstemp_lua, 3);
            return 1;
        }
    }
    return luaL_error(L, "\"io.tmpfile\" function not found");
}
