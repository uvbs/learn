/*************************************************************************
    > File Name: lua_inter_cxx.cpp
    > Author: sudoku.huang
    > Mail: sudoku.huang@gmail.com 
    > Created Time: Thu 04 Jun 2015 03:07:44 PM CST
 ************************************************************************/

#include<iostream>
using namespace std;

#ifdef __cplusplus
extern "C" {
#endif

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#ifdef __cplusplus
}
#endif

int add(lua_State * L)
{
    int a = lua_tointeger(L, 1);
    int b = lua_tointeger(L, 2);

    lua_pushinteger(L, a + b);

    return 1;
}

int main(int argc, char * argv[])
{
    char buff[256];
    int error;

    lua_State * L = luaL_newstate();
    luaopen_base(L);
    luaopen_io(L);
    luaopen_string(L);
    luaopen_math(L);

    lua_register(L, "add", add);

    luaL_dofile(L, "test_add.lua");
    lua_getglobal(L, "lua_add");
    lua_pushinteger(L, 6);
    lua_pushinteger(L, 5);
    lua_pcall(L, 2, 1, 0);

    int result = lua_tointeger(L, -1);
    lua_pop(L, 1);

    printf("result = %d\n", result);

    lua_close(L);

    return 0;
}

/*
 * g++ lua_inter_cxx.cpp -llua -ldl
 */
