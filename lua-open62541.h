#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "lua5.2/lua.h"
#include "lua5.2/lauxlib.h"
#include "lua5.2/lualib.h"

#include "open62541.h"

/* UA userdata is always of the below type. The original userdata "owns" the
   memory and needs to garbage-collect it. All derived versions (i.e. created
   during a member access) holds the original userdata in their user value
   field. That way, the original data is garbage collected only when all derived
   members are garbage collected before. */
typedef struct {
    const UA_DataType *type;
    void *data;
} ua_data;

int ua_gc(lua_State *L);
int ua_index(lua_State *L);
int ua_newindex(lua_State *L);
int ua_tostring(lua_State *L);
int ua_pairs(lua_State *L);
void ua_populate_types(lua_State *L);

/* Arrays are of a special type and have a different metatable. Arrays cannot be
   created standalone, but are always part of an enclosing type. Indexing arrays
   returns a copy of the element. */
typedef struct {
    const UA_DataType *type;
    UA_Int32 *length;
    void **data;
} ua_array;

int ua_array_index(lua_State *L);
int ua_array_len(lua_State *L);
int ua_array_newindex(lua_State *L);
int ua_array_append(lua_State *L);
int ua_array_remove(lua_State *L);
int ua_array_concat(lua_State *L);
int ua_array_pairs(lua_State *L);

/* Server */
int ua_server_new(lua_State *L);
int ua_server_gc(lua_State *L);
int ua_server_start(lua_State *L);
int ua_server_stop(lua_State *L);
int ua_server_add_variablenode(lua_State *L);
int ua_server_add_objectnode(lua_State *L);
int ua_server_add_objecttypenode(lua_State *L);
int ua_server_add_referencetypenode(lua_State *L);
int ua_server_add_reference(lua_State *L);

/* Populate the Module */
int luaopen_open62541(lua_State *L);
