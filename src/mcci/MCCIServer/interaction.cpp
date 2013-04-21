//#include <lua5.1/lua.hpp>
#include <lua.hpp>
#include <string>
using namespace std;
 
lua_State* myLuaState;

void InitLua(lua_State* &L)
{
    //open new state
    L = luaL_newstate();
    luaL_openlibs(L);    
}

void HaltLua(lua_State* L)
{
    lua_close(L);
}

int main(void)
{
    std::string myfile = "cfg.lua";
 
    printf("initializing lua environment\n");
    InitLua(myLuaState);
 
    printf("loading/executing lua file %s\n", myfile.c_str());
    luaL_dofile(myLuaState, myfile.c_str());
 
 
    printf("destroying lua environment\n");
    HaltLua(myLuaState);

    
}
