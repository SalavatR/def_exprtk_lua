#define EXTENSION_NAME ExprtkLua
#define LIB_NAME "ExprtkLua"
#define MODULE_NAME "exprtk_lua"
#define DLIB_LOG_DOMAIN LIB_NAME

#include <dmsdk/sdk.h>

#include <format>

#include "exprtk.hpp"

template < typename T >
static int CompileExpression(lua_State * L) {
    if (!lua_isstring(L, 1) || !lua_istable(L, 2)) {
        lua_pushstring(L, "Expected string and table as arguments");
        lua_error(L);
        return 0;
    }

    typedef exprtk::expression < T > expression_t;
    typedef exprtk::parser < T > parser_t;
    typedef exprtk::parser_error::type error_t;
    typedef exprtk::symbol_table < T > symbol_table_t;
    expression_t expression;
    parser_t parser;

    symbol_table_t symbol_table;
    symbol_table.add_constants();

    const char * expression_string = luaL_checkstring(L, 1);

    lua_pushnil(L); 
    while (lua_next(L, 2) != 0) {
        const char * key = luaL_checkstring(L, -2);
        if (lua_isboolean(L, -1)) {
            bool val1 = lua_toboolean(L, -1);
            symbol_table.add_constant(key, val1);
        } else if (lua_isnumber(L, -1)) {
            T val1 = T(luaL_checknumber(L, -1));
            symbol_table.add_constant(key, val1);
        } else if (lua_isstring(L, -1)) {
            std::string val1 = luaL_checkstring(L, -1);
            symbol_table.add_stringvar(key, val1);
        } else {
            lua_createtable(L, 0, 3);

            lua_pushstring(L, "success");
            lua_pushboolean(L, false);
            lua_settable(L, -3);

            lua_pushstring(L, "expression");
            lua_pushstring(L, expression_string);
            lua_settable(L, -3);

            lua_pushstring(L, "error");
            lua_pushstring(L, strcat("Invalid value type for ", key));
            lua_settable(L, -3);

            return 1;
        }
        lua_pop(L, 1);
    }

    expression.register_symbol_table(symbol_table);

    if (!parser.compile(expression_string, expression)) {

        lua_createtable(L, 0, 3);

        lua_pushstring(L, "success");
        lua_pushboolean(L, false);
        lua_settable(L, -3);

        lua_pushstring(L, "expression");
        lua_pushstring(L, expression_string);
        lua_settable(L, -3);

        lua_pushstring(L, "error");
        lua_pushstring(L, parser.error().c_str());
        lua_settable(L, -3);

        return 1;
    }
    const T result = expression.value();
    lua_createtable(L, 0, 2);

    lua_pushstring(L, "success");
    lua_pushboolean(L, true);
    lua_settable(L, -3);

    lua_pushstring(L, "result");
    lua_pushnumber(L, result);
    lua_settable(L, -3);

    return 1;

}

static int GetVariables(lua_State * L) {
    const char * expression_string = luaL_checkstring(L, 1);
    std::vector < std::string > variable_list;

    if (exprtk::collect_variables(expression_string, variable_list)) {
        lua_createtable(L, 0, variable_list.size());
        int newTable = lua_gettop(L);
        int index = 1;
        for (const auto &
                var: variable_list) {

            lua_pushstring(L,
                var.c_str());
            lua_rawseti(L, newTable, index);

            ++index;
        }
        return 1;
    } else
        dmLogError("Can't get variables from expression %s", expression_string);
    lua_pushnil(L);
    return 1;

}

static const luaL_reg Module_methods[] = {
    {
        "compile_expression",
        CompileExpression < double >
    },
    {
        "get_variables",
        GetVariables
    },
    {
        0,
        0
    }
};

static void LuaInit(lua_State * L) {
    int top = lua_gettop(L);
    luaL_register(L, MODULE_NAME, Module_methods);
    lua_pop(L, 1);
    assert(top == lua_gettop(L));
}

dmExtension::Result InitializeExprtkLua(dmExtension::Params * params) {
    LuaInit(params -> m_L);
    return dmExtension::RESULT_OK;
}

dmExtension::Result AppInitializeExprtkLua(dmExtension::AppParams * params) {
    dmLogInfo("Registered extension exprtk");
    return dmExtension::RESULT_OK;
}

dmExtension::Result AppFinalizeExprtkLua(dmExtension::AppParams * params) {
    return dmExtension::RESULT_OK;
}

DM_DECLARE_EXTENSION(EXTENSION_NAME, LIB_NAME, AppInitializeExprtkLua, AppFinalizeExprtkLua, InitializeExprtkLua, 0, 0, 0)