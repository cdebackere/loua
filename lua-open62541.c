#include "lua-open62541.h"

static const struct luaL_Reg open62541 [] = {
    {"server",   ua_server_new},
    {NULL, NULL} /* sentinel */
};

static void addNodeId(lua_State *L, int identifier, const char *name) {
    ua_data *data = lua_newuserdata(L, sizeof(ua_data));
    data->type = &UA_TYPES[UA_TYPES_NODEID];
    data->data = UA_NodeId_new();
    luaL_setmetatable(L, "open62541-data");
    *((UA_NodeId*)data->data) = UA_NODEID_NUMERIC(0, identifier);
    lua_setfield(L, -2, name);
}

static void addNodeIds(lua_State *L) {
    lua_newtable(L);

    addNodeId(L, 0, "null");

    /* References */
    addNodeId(L, 31, "references");
    addNodeId(L, 32, "nonhierarchicalreferences");
    addNodeId(L, 33, "hierarchicalreferences");
    addNodeId(L, 35, "organizes");
    addNodeId(L, 40, "hastypedefinition");
    addNodeId(L, 45, "hassubtype");
    addNodeId(L, 46, "hasproperty");
    addNodeId(L, 47, "hascomponent");
        
    /* Object Types */
    addNodeId(L, 58, "baseobjecttype");
      addNodeId(L, 61, "foldertype");

    /* Objects */
    addNodeId(L, 84, "root");
    addNodeId(L, 85, "objects");
    addNodeId(L, 86, "types");
      addNodeId(L, 90, "datatypes");
      addNodeId(L, 88, "objecttypes");
      addNodeId(L, 58, "baseobjecttype");
      addNodeId(L, 91, "referencetypes");
      addNodeId(L, 89, "variabletypes");
    addNodeId(L, 87, "views");

    lua_setfield(L, -2, "nodeids");
}

int luaopen_open62541(lua_State *L) {
    /* metatable for builtin types */
    luaL_newmetatable(L, "open62541-builtin");
    lua_pushcfunction(L, ua_tostring);
    lua_setfield(L, -2, "__tostring");
    lua_pushcfunction(L, ua_gc);
    lua_setfield(L, -2, "__gc");
    lua_pop(L, 1);

    /* metatable for structured types */
    luaL_newmetatable(L, "open62541-data");
    lua_pushcfunction(L, ua_gc);
    lua_setfield(L, -2, "__gc");
    lua_pushcfunction(L, ua_index);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, ua_newindex);
    lua_setfield(L, -2, "__newindex");
    lua_pushcfunction(L, ua_pairs);
    lua_setfield(L, -2, "__pairs");
    lua_pop(L, 1);

    /* metatable for arrays */
    luaL_newmetatable(L, "open62541-array");
    /* branched out in the index function */
    // lua_pushcfunction(L, ua_array_append);
    // lua_setfield(L, -2, "append");
    lua_pushcfunction(L, ua_array_index);
    lua_setfield(L, -2, "__index");
    lua_pushcfunction(L, ua_array_len);
    lua_setfield(L, -2, "__len");
    lua_pushcfunction(L, ua_array_pairs);
    lua_setfield(L, -2, "__pairs");
    lua_pop(L, 1);

    luaL_newmetatable(L, "open62541-server");
    lua_pushcfunction(L, ua_server_gc);
    lua_setfield(L, -2, "__gc");
    lua_newtable(L);
    lua_pushcfunction(L, ua_server_start);
    lua_setfield(L, -2, "start");
    lua_pushcfunction(L, ua_server_stop);
    lua_setfield(L, -2, "stop");
    lua_pushcfunction(L, ua_server_add_variablenode);
    lua_setfield(L, -2, "addVariableNode");
    lua_pushcfunction(L, ua_server_add_objectnode);
    lua_setfield(L, -2, "addObjectNode");
    lua_pushcfunction(L, ua_server_add_objecttypenode);
    lua_setfield(L, -2, "addObjectTypeNode");
    lua_pushcfunction(L, ua_server_add_referencetypenode);
    lua_setfield(L, -2, "addReferenceTypeNode");
    lua_pushcfunction(L, ua_server_add_reference);
    lua_setfield(L, -2, "addReference");
    lua_setfield(L, -2, "__index");
    lua_pop(L, 1);

    /* create the module */
    luaL_newlib(L, open62541);

    /* if no member in the module is found, try to create a type-new function */
    lua_newtable(L);
    ua_populate_types(L);
    lua_setfield(L, -2, "types");

    addNodeIds(L);

    return 1;
}
