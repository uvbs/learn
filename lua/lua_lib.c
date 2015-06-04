/*************************************************************************
    > File Name: lua_lib.c
    > Author: sudoku.huang
    > Mail: sudoku.huang@gmail.com 
    > Created Time: Thu 04 Jun 2015 04:08:52 PM CST
 ************************************************************************/
/*
 * gcc -shared -fPIC -o mytestlib.so lua_lib.c 
 */

#include <stdio.h>
#include <string.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

static int add(lua_State * L)
{
    double op1 = luaL_checknumber(L, 1);
    double op2 = luaL_checknumber(L, 2);

    lua_pushnumber(L, op1 + op2);

    return 1;
}

static int sub(lua_State * L)
{
    double op1 = luaL_checknumber(L, 1);
    double op2 = luaL_checknumber(L, 2);

    lua_pushnumber(L, op1 - op2);

    return 1;
}

static luaL_Reg mylibs[] = {
    {"add", add},
    {"sub", sub},
    {NULL, NULL},
};

int luaopen_mytestlib(lua_State * L)
{
    const char * libName = "mytestlib";
    //luaL_register(L, libName, mylibs); //lua5.1
    luaL_newlib(L, mylibs);

    return 1;
}
