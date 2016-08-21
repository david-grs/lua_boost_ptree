#include <lua5.2/lua.hpp>

#include <iostream>

struct Script
{
    Script()
    {
        mState = luaL_newstate();
        luaL_openlibs(mState);
    }

    ~Script()
    {
        Stop();
    }

    void Stop()
    {
        lua_close(mState);
    }

    void Execute(const std::string& filename)
    {
        if (luaL_loadfile(mState, filename.c_str()) != LUA_OK)
            throw std::runtime_error("error while loading lua file: " + GetLastError());

        if (lua_pcall(mState, 0, LUA_MULTRET, 0) != LUA_OK)
            throw std::runtime_error("error while running lua script: " + GetLastError());
    }

    void Call()
    {
        lua_getglobal(mState, "foo");
        lua_pushnumber(mState, 1);

        if (lua_pcall(mState, 1, 1, 0 ) != 0)
            throw std::runtime_error("failed to call callback: " + GetLastError());
    }

private:
    std::string GetLastError()
    {
        std::string message = lua_tostring(mState, -1);
        lua_pop(mState, 1);
        return message;
    }

    lua_State* mState;
};


int main(int argc, char** argv)
{
    if (argc != 2)
        throw std::runtime_error("usage: " + std::string(argv[0]) + " <lua_script>");

    Script script;
    script.Execute(argv[1]);
    script.Call();

    return 0;
}
