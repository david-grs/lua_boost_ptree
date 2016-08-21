#include <lua5.2/lua.hpp>

#include <iostream>
#include <sstream>

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

        lua_pushlightuserdata(mState, this);
        lua_pushcclosure(mState, &Script::ReceiveHelper, 1);
        lua_setglobal(mState, "send");

        if (lua_pcall(mState, 0, LUA_MULTRET, 0) != LUA_OK)
            throw std::runtime_error("error while running lua script: " + GetLastError());
    }

    void Call()
    {
        lua_getglobal(mState, "recv");
        lua_pushnumber(mState, 1);

        if (lua_pcall(mState, 1, 1, 0 ) != 0)
            throw std::runtime_error("failed to call callback: " + GetLastError());
    }

private:
    int Receive(lua_State* lua_state)
    {
        std::stringstream str_buf;
        while(lua_gettop(lua_state))
        {
            str_buf.str(std::string());
            str_buf << " ";
            switch(lua_type(lua_state, lua_gettop(lua_state)))
            {
            case LUA_TNUMBER:
                str_buf << "script returned the number: "
                    << lua_tonumber(lua_state, lua_gettop(lua_state));
                break;
            case LUA_TTABLE:
                str_buf << "script returned a table";
                break;
            case LUA_TSTRING:
                str_buf << "script returned the string: "
                    << lua_tostring(lua_state, lua_gettop(lua_state));
                break;
            case LUA_TBOOLEAN:
                str_buf << "script returned the boolean: "
                    << lua_toboolean(lua_state, lua_gettop(lua_state));
                break;
            default:
                str_buf << "script returned an unknown-type value";
            }
            lua_pop(lua_state, 1);
            std::cout << str_buf.str() << std::endl;
        }

        return 0;
    }

    static int ReceiveHelper(lua_State* L)
    {
        Script* script = (Script*)lua_touserdata(L, lua_upvalueindex(1));
        return script->Receive(L);
    }

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
