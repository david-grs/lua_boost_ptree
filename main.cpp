#include <lua5.2/lua.hpp>

#include "picojson.h"

#include <iostream>
#include <sstream>

namespace detail
{

void json_to_table_impl(lua_State* L, const picojson::value& v)
{
    if (v.is<picojson::array>())
    {
        auto& array = v.get<picojson::array>();

        lua_newtable(L);
        int i = 1;

        for (auto& v : array)
        {
            json_to_table_impl(L, v);
            lua_rawseti(L, -2, i++);
        }
    }
    else if (v.is<picojson::object>())
    {
        auto& object = v.get<picojson::object>();

        lua_newtable(L);
        int top = lua_gettop(L);

        for (auto& p : object)
        {
            lua_pushstring(L, p.first.c_str());
            json_to_table_impl(L, p.second);
            lua_settable(L, top);
        }
    }
    else if (v.is<double>())
    {
        lua_pushnumber(L, v.get<double>());
    }
    else if (v.is<std::string>())
    {
        lua_pushstring(L, v.get<std::string>().c_str());
    }
    else if (v.is<bool>())
    {
        lua_pushboolean(L, v.get<bool>());
    }
}

}

void json_to_table(lua_State* L, const std::string& json)
{
    picojson::value v;
    std::string err = picojson::parse(v, json);

    if (!err.empty())
        throw std::runtime_error("json parsing failed: " + err);

    detail::json_to_table_impl(L, v);
}

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

    void Call(const std::string& json)
    {
        lua_getglobal(mState, "recv");
        json_to_table(mState, json);

        if (lua_pcall(mState, 1, 1, 0 ) != 0)
            throw std::runtime_error("failed to call callback: " + GetLastError());
    }

private:
    int Receive(lua_State* L)
    {
        std::ostringstream oss;

        lua_settop(L, 1);
        luaL_checktype(L, 1, LUA_TTABLE);

        lua_pushvalue(L, -1);
        lua_pushnil(L);

        while (lua_next(L, -2))
        {
            lua_pushvalue(L, -2);
            oss << "\"" << lua_tostring(L, -1) << "\": ";

            switch(lua_type(L, -2))
            {
            //case LUA_TTABLE: oss << lua_tonumber(L, -2); break;
            case LUA_TNUMBER: oss << lua_tonumber(L, -2); break;
            case LUA_TBOOLEAN: oss << lua_toboolean(L, -2); break;
            case LUA_TSTRING: oss << "\"" << lua_tostring(L, -2) << "\""; break;
            default: break;
            }

            lua_pop(L, 2);
        }

        std::cout << oss.str()<<std::endl;

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

    /*
    script.Call("[1,2,3]");
    script.Call("[\"FX\", \"FX1\", \"FX2\"]");
    script.Call("[\"FX\", \"FX1\", \"FX2\", 3, 4.5]");
    script.Call("{\"FX\": {\"1\": 123, \"4\": 4566, \"1000\": [\"bar\", \"foo\"]}, \"FX2\": [1,[[1,2,[5,[5,[5,6]]]], [[533,122],[99,100]], \"miaou\"],3]}");
    script.Call("[[1,2,[5,[5,[5,6]]]], [[533,122],[99,100]], \"miaou\"]");
    */

    return 0;
}
