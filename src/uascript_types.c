// This file is a part of uascript. License is MIT (see LICENSE file)

#include "uascript.h"
#include "lualib.h"
#include "lauxlib.h"

/* try to convert the element to a lua value */
static void uatolua(lua_State *L, int index) {
    ua_data *v = luaL_testudata(L, index, "open62541-data");
    if(!v)
        return;
    switch(v->type->typeIndex) {
    case UA_TYPES_BOOLEAN:
        lua_pushboolean(L, *(UA_Boolean*)v->data);
        break;
    case UA_TYPES_SBYTE:
        lua_pushinteger(L, *(UA_SByte*)v->data);
        break;
    case UA_TYPES_BYTE:
        lua_pushinteger(L, *(UA_Byte*)v->data);
        break;
    case UA_TYPES_INT16:
        lua_pushinteger(L, *(UA_Int16*)v->data);
        break;
    case UA_TYPES_UINT16:
        lua_pushinteger(L, *(UA_UInt16*)v->data);
        break;
    case UA_TYPES_UINT32:
        lua_pushinteger(L, *(UA_UInt32*)v->data);
        break;
    case UA_TYPES_INT32:
    case UA_TYPES_STATUSCODE:
        lua_pushinteger(L, *(UA_Int32*)v->data);
        break;
    case UA_TYPES_INT64:
        lua_pushinteger(L, *(UA_Int64*)v->data);
        break;
    case UA_TYPES_UINT64:
        lua_pushinteger(L, *(UA_UInt64*)v->data);
        break;
    case UA_TYPES_FLOAT:
        lua_pushnumber(L, *(UA_Float*)v->data);
        break;
    case UA_TYPES_DOUBLE:
        lua_pushnumber(L, *(UA_Double*)v->data);
        break;
    case UA_TYPES_STRING:
    case UA_TYPES_BYTESTRING:
    case UA_TYPES_XMLELEMENT:
        lua_pushlstring(L, ((UA_String*)v->data)->data, ((UA_String*)v->data)->length);
        break;
    default:
        return;
    }
    lua_replace(L, index);
}

/*****************/
/* Type Handling */
/*****************/

/* Guid */
static UA_Guid* parse_guid(lua_State *L, int index) {
    if(!lua_isstring(L, index)) {
        luaL_error(L, "Guid needs to be of the form \"00000000-0000-0000-0000-000000000000\"");
        return NULL;
    }
    size_t arglength;
    const char *arg = lua_tolstring(L, index, &arglength);
    if(arglength != 36) {
        luaL_error(L, "Guid needs to be of the form \"00000000-0000-0000-0000-000000000000\"");
        return NULL;
    }
    UA_Guid *guid = UA_Guid_new();
    guid->data1 = strtoull(arg, NULL, 16);
    guid->data2 = strtoull(&arg[9], NULL, 16);
    guid->data3 = strtoull(&arg[14], NULL, 16);
    UA_Int16 data4_1 = strtoull(&arg[19], NULL, 16);
    guid->data4[0] = data4_1 >> 8;
    guid->data4[1] = data4_1;
    UA_Int64 data4_2 = strtoull(&arg[24], NULL, 16);
    guid->data4[2] = data4_2 >> 40;
    guid->data4[3] = data4_2 >> 32;
    guid->data4[4] = data4_2 >> 24;
    guid->data4[5] = data4_2 >> 16;
    guid->data4[6] = data4_2 >> 8;
    guid->data4[7] = data4_2;
    return guid;
}

/* String */
static UA_String* parse_string(lua_State *L, int index) {
    if(!lua_isstring(L, index)) {
        luaL_error(L, "The argument is no string");
        return NULL;
    }
    UA_String *s = UA_String_new();
    *s = UA_STRING_ALLOC(lua_tostring(L, index));
    return s;
}

/* NodeId */
static UA_NodeId* parse_nodeid(lua_State *L, int index) {
    if(!lua_isnumber(L, index)) {
        luaL_error(L, "The first argument is no integer namespace id");
        return NULL;
    } else if(!lua_isnumber(L, index+1) && !lua_isstring(L, index+1)) {
        luaL_error(L, "Currently, only numeric and string nodeids are implemented");
        return NULL;
    }
    UA_NodeId *id = UA_NodeId_new();
    id->namespaceIndex = lua_tointeger(L, index);
    if(lua_isnumber(L, index+1)) {
        id->identifierType = UA_NODEIDTYPE_NUMERIC;
        id->identifier.numeric = lua_tointeger(L, index+1);
    } else if(lua_isstring(L, index+1)) {
        id->identifierType = UA_NODEIDTYPE_STRING;
        id->identifier.string = UA_String_fromChars(lua_tostring(L, index+1));
    } 
    return id;
}

/* QualifiedName */
static UA_QualifiedName* parse_qualifiedname(lua_State *L, int index) {
    if(!lua_isnumber(L, index)) {
        luaL_error(L, "The first argument is no integer namespace index");
        return NULL;
    } else if(!lua_isstring(L, index+1)) {
        luaL_error(L, "The second argument is no string");
        return NULL;
    }
    UA_QualifiedName *qn = UA_QualifiedName_new();
    qn->namespaceIndex = lua_tointeger(L, index);
    qn->name = UA_STRING_ALLOC(lua_tostring(L, index+1));
    return qn;
}

/* LocalizedText */
static UA_LocalizedText* parse_localizedtext(lua_State *L, int index) {
    if(!lua_isstring(L, index)) {
        luaL_error(L, "The first argument is no string");
        return NULL;
    } else if(!lua_isstring(L, index+1)) {
        luaL_error(L, "The second argument is no string");
        return NULL;
    }
    UA_LocalizedText *lt = UA_LocalizedText_new();
    lt->locale = UA_STRING_ALLOC(lua_tostring(L, index));
    lt->text = UA_STRING_ALLOC(lua_tostring(L, index+1));
    return lt;
}

static UA_Variant* parse_variant(lua_State *L, int index) {
    ua_data *data = ua_getdata(L, index);
    if(data) {
        UA_Variant *v = UA_Variant_new();
        UA_Variant_setScalarCopy(v, data->data, data->type);
        return v;
    }
    ua_array *array = ua_getarray(L, index);
    if(array) {
        UA_Variant *v = UA_Variant_new();
        UA_Variant_setArrayCopy(v, *array->data, *array->length, array->type);
        return v;
    }
    return NULL;
}

static void *
ua_type_parse(lua_State *L, const UA_DataType *type, int index) {
    if(lua_isnil(L, index)) {
        if(type == &UA_TYPES[UA_TYPES_VARIANT]) {
            luaL_error(L, "Variants cannot be empty");
            return NULL;
        }
        return UA_new(type);
    }
    if(!type->builtin) {
        luaL_error(L, "Only builtin types can be instantiated with arguments");
        return NULL;
    }
    switch(type->typeIndex) {
    case UA_TYPES_STRING:
        return parse_string(L, index);
    case UA_TYPES_GUID:
        return parse_guid(L, index);
    case UA_TYPES_NODEID:
        return parse_nodeid(L, index);
    case UA_TYPES_QUALIFIEDNAME:
        return parse_qualifiedname(L, index);
    case UA_TYPES_LOCALIZEDTEXT:
        return parse_localizedtext(L, index);
    case UA_TYPES_VARIANT:
        return parse_variant(L, index);
    case UA_TYPES_SBYTE:
    case UA_TYPES_BYTE:
    case UA_TYPES_INT16:
    case UA_TYPES_UINT16:
    case UA_TYPES_INT32:
    case UA_TYPES_UINT32:
    case UA_TYPES_INT64:
    case UA_TYPES_UINT64: {
        if(!lua_isnumber(L, index))
            luaL_error(L, "Argument is not a number");
        lua_Number n = lua_tonumber(L, index);
        if(type->memSize == 1) {
            UA_Byte *byte = UA_Byte_new();
            *byte = (UA_Byte)n;
            return byte;
        } else if(type->memSize == 2) {
            UA_UInt16 *i16 = UA_UInt16_new();
            *i16 = (UA_UInt16)n;
            return i16;
        } else if(type->memSize == 4) {
            UA_UInt32 *i32 = UA_UInt32_new();
            *i32 = (UA_UInt32)n;
            return i32;
        } else {
            UA_UInt64 *i64 = UA_UInt64_new();
            *i64 = (UA_UInt64)n;
            return i64;
        }
    }
    case UA_TYPES_FLOAT: {
        if(!lua_isnumber(L, index))
            luaL_error(L, "Argument is not a number");
        lua_Number n = lua_tonumber(L, index);
        UA_Float *v = UA_Float_new();
        *v = n;
        return v;
    }
    case UA_TYPES_DOUBLE: {
        if(!lua_isnumber(L, index))
            luaL_error(L, "Argument is not a number");
        lua_Number n = lua_tonumber(L, index);
        UA_Double *v = UA_Double_new();
        *v = n;
        return v;
    }
    }
    luaL_error(L, "Cannot parse arguments for this data type");
    return NULL;
}

/* General type constructor function. Takes the datatype from its closure. */
int ua_type_instantiate(lua_State *L) {
    lua_pushnil(L); // separator between data from the user and what we add
    lua_rawgeti(L, 1, 1);
    const UA_DataType *type = lua_touserdata(L, -1);
    ua_data *data = lua_newuserdata(L, sizeof(ua_data));
    data->type = type;
    data->data = ua_type_parse(L, type, 2);
    luaL_setmetatable(L, "open62541-data");
    return 1;
}

int ua_type_tostring(lua_State *L) {
    lua_rawgeti(L, 1, 1);
    const UA_DataType *type = lua_touserdata(L, 2);
    lua_pushstring(L, "ua.types.");
    lua_pushstring(L, type->typeName);
    lua_concat(L, 2);
    return 1;
}

int ua_type_indexerr(lua_State *L) {
    return luaL_error(L, "Type cannot be indexed");
}

void ua_type_push_typetable(lua_State *L, const UA_DataType *type) {
    lua_newtable(L);
    lua_pushlightuserdata(L, (void*)(uintptr_t)type);
    lua_rawseti(L, -2, 1);
    ua_data *data = lua_newuserdata(L, sizeof(ua_data));
    UA_NodeId *typeId = UA_NodeId_new();
    *typeId = type->typeId;
    data->data= typeId;
    data->type = &UA_TYPES[UA_TYPES_NODEID];
    luaL_setmetatable(L, "open62541-data");
    lua_setfield(L, -2, "typeId");
    luaL_setmetatable(L, "open62541-type");
}

int ua_get_type(lua_State *L) {
    const UA_DataType *type = NULL;
    ua_data *data = luaL_checkudata (L, 1, "open62541-data");
    if(data)
        type = data->type;
    else {
        ua_array *array = luaL_checkudata(L, 1, "open62541-array");
        type = array->type;
    }
    if(!type)
        return luaL_error(L, "Not an ua type");
    lua_newtable(L);
    luaL_setmetatable(L, "open62541-type");
    lua_pushlightuserdata(L, (void*)(uintptr_t)type);
    lua_rawseti(L, -2, 1);
    return 1;
}

int ua_gc(lua_State *L) {
    ua_data *data = ua_getdata(L, -1);
    if(!data || !data->data)
        return 0;
    lua_getuservalue(L, -1);
    /* derived data is not deleted, tagged with a uservalue */
    if(lua_isnil(L, -1))
        UA_delete(data->data, data->type);
    return 0;
}

/* Read the data at the index in the lua stack, replace it with an open62541 data and return a pointer */
ua_data *
ua_getdata(lua_State *L, int index) {
    ua_data *data = luaL_testudata(L, index , "open62541-data");
    if(data)
        return data;

    /* try to convert */
    if(lua_isboolean(L, index)) {
        data = lua_newuserdata(L, sizeof(ua_data));
        data->type = &UA_TYPES[UA_TYPES_BOOLEAN];
        data->data = UA_Boolean_new();
        *(UA_Boolean*)data->data = lua_toboolean(L, index);
        luaL_setmetatable(L, "open62541-data");
        lua_replace(L, index);
    } else if(lua_isnumber(L, index)) {
        lua_Number n = lua_tonumber(L, index);
        data = lua_newuserdata(L, sizeof(ua_data));
        if(n == (int)n) {
            data->data = UA_Int32_new();
            *(UA_Int32*)data->data = n;
            data->type = &UA_TYPES[UA_TYPES_INT32];
        } else {
            data->data = UA_Float_new();
            *(UA_Float*)data->data = n;
            data->type = &UA_TYPES[UA_TYPES_FLOAT];
        }
        luaL_setmetatable(L, "open62541-data");
        lua_replace(L, index);
    } else if(lua_isstring(L, index)) {
        data = lua_newuserdata(L, sizeof(ua_data));
        data->type = &UA_TYPES[UA_TYPES_STRING];
        data->data = parse_string(L, index);
        luaL_setmetatable(L, "open62541-data");
        lua_replace(L, index);
    } else
        return NULL;
    return data;
}

ua_array *
ua_getarray(lua_State *L, int index) {
    ua_array *array = luaL_testudata(L, index , "open62541-array");
    if(array)
        return array;
    if(!lua_istable(L, index))
        return NULL;

    lua_len(L, index);
    int len = lua_tointeger(L, -1);
    lua_pop(L, 1);
    if(len <= 0)
        return NULL;

    const UA_DataType *type = NULL;
    lua_rawgeti(L, index, 1);
    ua_data *data = ua_getdata(L, -1);
    if(!data) {
        luaL_error(L, "Cannot transform array content to ua type");
        return NULL;
    }
    type = data->type;

    array = lua_newuserdata(L, sizeof(ua_array));
    array->type = type;
    array->data = &array->local_data;
    array->length = &array->local_length;
    array->local_data = UA_Array_new(len, type);
    array->local_length = len;
    luaL_setmetatable(L, "open62541-array");
    UA_copy(data->data, array->local_data, type);
    lua_remove(L, -2); // the data
    
    uintptr_t pos = (uintptr_t)array->local_data + type->memSize;
    for(int i = 2; i <= len; i++) {
        lua_rawgeti(L, index, i);
        data = ua_getdata(L, -1);
        if(!data) {
            luaL_error(L, "Cannot transform array content to ua type");
            return NULL;
        }
        if(data->type != type) {
            luaL_error(L, "Array contains different data types");
            return NULL;
        }
        UA_copy(data->data, (void*)pos, type);
        pos += type->memSize;
        lua_pop(L, 1); // the data
    }
    lua_replace(L, index); // replace the lua table on the stack with the ua table
    return array;
}

/*********/
/* Index */
/*********/

/* Get the index of the member with the given name */
static int
find_memberindex(const UA_DataType *type, const char *membername) {
    for(int memberindex = 0; memberindex < type->membersSize; memberindex++) {
        const UA_DataTypeMember *member = &type->members[memberindex];
        if(!member->memberName)
            continue;
        if(strcmp(member->memberName, membername) == 0)
            return memberindex;
    }
    return -1;
}

/* get the pointer to the member and the array length if an array */
static void *
memberptr(void *parent, const UA_DataType *type, int memberindex, size_t **arraylen) {
    uintptr_t ptr = (uintptr_t)parent;
    const UA_DataTypeMember *member;
    for(int i = 0; i < memberindex; i++) {
        member = &type->members[i];
        if(!member->isArray)
            ptr += member->padding + UA_TYPES[member->memberTypeIndex].memSize;
        else
            ptr += member->padding + sizeof(size_t) + sizeof(void*);
    }
    member = &type->members[memberindex];
    if(member->isArray) {
        ptr += member->padding;
        *arraylen = (size_t*)ptr;
        ptr += sizeof(size_t);
    } else {
        *arraylen = NULL;
        ptr += member->padding;
    }
    return (void*)ptr;
}

static int
ua_return_member(lua_State *L, int dataindex,
                 void *member, const UA_DataType *membertype) {
    ua_data *memberdata = lua_newuserdata(L, sizeof(ua_data));
    memberdata->type = membertype;
    memberdata->data = member;
    luaL_setmetatable(L, "open62541-data");
    /* add a user value that prevents gc of the orig userdata */
    lua_newtable(L);
    lua_pushvalue(L, dataindex);
    lua_rawseti(L, -2, 1);
    lua_setuservalue(L, -2);
    return 1;
}

static int
ua_return_memberarray(lua_State *L, int dataindex, void *array,
                      size_t *arraylen, const UA_DataType *arraytype) {
    ua_array *memberdata = lua_newuserdata(L, sizeof(ua_array));
    memberdata->type = arraytype;
    memberdata->data = array;
    memberdata->length = arraylen;
    luaL_setmetatable(L, "open62541-array");
    /* add a user value that prevents gc of the orig userdata */
    lua_newtable(L);
    lua_pushvalue(L, 1); // orig userdata
    lua_rawseti(L, -2, 1);
    lua_setuservalue(L, -2); // set the uservalue of the member data
    return 1;
}

static int
ua_nodeid_index(lua_State *L, int dataindex, UA_NodeId *id, const char *key) {
    if(strcmp(key, "namespaceIndex") == 0) {
        return ua_return_member(L, dataindex, &id->namespaceIndex, &UA_TYPES[UA_TYPES_UINT16]);
    }
    if(strcmp(key, "identifier") == 0) {
        if(id->identifierType == UA_NODEIDTYPE_NUMERIC)
            return ua_return_member(L, dataindex, &id->identifier.numeric, &UA_TYPES[UA_TYPES_UINT32]);
        else if(id->identifierType == UA_NODEIDTYPE_GUID)
            return ua_return_member(L, dataindex, &id->identifier.guid, &UA_TYPES[UA_TYPES_GUID]);
        return ua_return_member(L, dataindex, &id->identifier.string, &UA_TYPES[UA_TYPES_STRING]);
    }
    return luaL_error(L, "Cannot get this index");
}

static int
ua_expandednodeid_index(lua_State *L, int dataindex, UA_ExpandedNodeId *id, const char *key) {
    if(strcmp(key, "nodeId") == 0)
        return ua_return_member(L, dataindex, &id->nodeId, &UA_TYPES[UA_TYPES_NODEID]);
    if(strcmp(key, "namespaceUri") == 0)
        return ua_return_member(L, dataindex, &id->namespaceUri, &UA_TYPES[UA_TYPES_STRING]);
    if(strcmp(key, "serverIndex") == 0)
        return ua_return_member(L, dataindex, &id->serverIndex, &UA_TYPES[UA_TYPES_UINT32]);
    return luaL_error(L, "Cannot get this index");
}

/* access to variant always returns a copy */
static int
ua_variant_index(lua_State *L, int dataindex, UA_Variant *v, const char *key) {
    if(strcmp(key, "value") == 0) {
        if(UA_Variant_isScalar(v)) {
            ua_data *data = lua_newuserdata(L, sizeof(ua_data));
            data->type = v->type;
            data->data = UA_new(v->type);
            UA_copy(v->data, data->data, v->type);
            luaL_setmetatable(L, "open62541-data");
        } else {
            ua_array *array = lua_newuserdata(L, sizeof(ua_array));
            array->type = v->type;
            array->data = &array->local_data;
            array->length = &array->local_length;
            UA_Array_copy(v->data, v->arrayLength, &array->local_data, v->type);
            array->local_length = v->arrayLength;
            luaL_setmetatable(L, "open62541-array");
        }
        return 1;
    }
    if(strcmp(key, "arrayDimensions") == 0) {
        ua_array *array = lua_newuserdata(L, sizeof(ua_array));
        array->type = &UA_TYPES[UA_TYPES_UINT32];
        array->data = &array->local_data;
        array->length = &array->local_length;
        UA_Array_copy(v->arrayDimensions, v->arrayDimensionsSize,
                      &array->local_data, &UA_TYPES[UA_TYPES_UINT32]);
        array->local_length = v->arrayDimensionsSize;
        luaL_setmetatable(L, "open62541-array");
        return 1;
    }
    return luaL_error(L, "Cannot get this index");
}

static int
ua_datavalue_index(lua_State *L, int dataindex, UA_DataValue *v, const char *key) {
    if(strcmp(key, "value") == 0) {
        if(!v->hasValue) {
            lua_pushnil(L);
            return 1;
        }
        return ua_return_member(L, dataindex, &v->value, &UA_TYPES[UA_TYPES_VARIANT]);
    }
    if(strcmp(key, "status") == 0) {
        if(!v->hasStatus) {
            lua_pushnil(L);
            return 1;
        }
        return ua_return_member(L, dataindex, &v->status, &UA_TYPES[UA_TYPES_STATUSCODE]);
    }
    if(strcmp(key, "sourceTimestamp") == 0) {
        if(!v->hasSourceTimestamp) {
            lua_pushnil(L);
            return 1;
        }
        return ua_return_member(L, dataindex, &v->sourceTimestamp, &UA_TYPES[UA_TYPES_DATETIME]);
    }
    if(strcmp(key, "sourcePicoseconds") == 0) {
        if(!v->hasSourcePicoseconds) {
            lua_pushnil(L);
            return 1;
        }
        return ua_return_member(L, dataindex, &v->sourcePicoseconds, &UA_TYPES[UA_TYPES_UINT16]);
    }
    if(strcmp(key, "serverTimestamp") == 0) {
        if(!v->hasServerTimestamp) {
            lua_pushnil(L);
            return 1;
        }
        return ua_return_member(L, dataindex, &v->serverTimestamp, &UA_TYPES[UA_TYPES_DATETIME]);
    }
    if(strcmp(key, "serverPicoseconds") == 0) {
        if(!v->hasServerPicoseconds) {
            lua_pushnil(L);
            return 1;
        }
        return ua_return_member(L, dataindex, &v->serverPicoseconds, &UA_TYPES[UA_TYPES_UINT16]);
    }
    return luaL_error(L, "Cannot get this index");
}

static int ua_index_key(lua_State *L, int dataindex, const char *key) {
    ua_data *data = luaL_checkudata(L, dataindex, "open62541-data");

    if(data->type == &UA_TYPES[UA_TYPES_NODEID])
        return ua_nodeid_index(L, dataindex, data->data, key);
    if(data->type == &UA_TYPES[UA_TYPES_EXPANDEDNODEID])
        return ua_expandednodeid_index(L, dataindex, data->data, key);
    else if(data->type == &UA_TYPES[UA_TYPES_VARIANT])
        return ua_variant_index(L, dataindex, data->data, key);
    else if(data->type == &UA_TYPES[UA_TYPES_DATAVALUE])
        return ua_datavalue_index(L, dataindex, data->data, key);

    int memberindex = find_memberindex(data->type, key);
    if(memberindex < 0)
        return luaL_error(L, "Cannot get this index %d", memberindex);

    size_t *arraylen;
    void *member = memberptr(data->data, data->type, memberindex, &arraylen);
    const UA_DataType *type = &UA_TYPES[data->type->members[memberindex].memberTypeIndex];
    if(arraylen)
        return ua_return_memberarray(L, dataindex, member, arraylen, type);
    return ua_return_member(L, dataindex, member, type);
}

int ua_index(lua_State *L) {
    const char *key = lua_tostring(L, 2);
    int ret = ua_index_key(L, 1, key);
    if(ret == 0)
        return 0;
    uatolua(L, -1);
    return 1;
}

/************/
/* Newindex */
/************/

static int
ua_variant_newindex(lua_State *L, UA_Variant *v, const char *key, int index) {
    if(strcmp(key, "value") == 0) {
        ua_data *data = ua_getdata(L, index);
        if(data) {
            if(v->data > UA_EMPTY_ARRAY_SENTINEL && v->storageType == UA_VARIANT_DATA) {
                if(UA_Variant_isScalar(v))
                    UA_delete(v->data, v->type);
                else
                    UA_Array_delete(v->data, v->arrayLength, v->type);
            }
            UA_Variant_setScalarCopy(v, data->data, data->type);
            return 0;
        }
        ua_array *array = ua_getarray(L, index);
        if(array) {
            if(v->data > UA_EMPTY_ARRAY_SENTINEL && v->storageType == UA_VARIANT_DATA) {
                if(UA_Variant_isScalar(v))
                    UA_delete(v->data, v->type);
                else
                    UA_Array_delete(v->data, v->arrayLength, v->type);
            }
            UA_Variant_setArrayCopy(v, *array->data, *array->length, array->type);
            return 0;
        }
        return luaL_error(L, "Cannot set this data type to the variant");
    } else if(strcmp(key, "arrayDimensions") == 0) {
        ua_array *array = ua_getarray(L, index);
        // allow int32 as most users will give a simple {1,2,3} lua array
        if(array->type != &UA_TYPES[UA_TYPES_UINT32] &&
           array->type != &UA_TYPES[UA_TYPES_INT32]) {
            return luaL_error(L, "Array dimensions need to be an array of UInt32");
        }
        // todo: test if the dimensions match
        UA_Array_delete(v->arrayDimensions, v->arrayDimensionsSize, &UA_TYPES[UA_TYPES_UINT32]);
        UA_Array_copy(*array->data, *array->length, (void**)&v->arrayDimensions, &UA_TYPES[UA_TYPES_UINT32]);
        v->arrayDimensionsSize = *array->length;
        return 0;
    }
    return luaL_error(L, "Not a valid variant index");
}

int ua_newindex(lua_State *L) {
    ua_data *parent = luaL_checkudata (L, 1, "open62541-data");
    if(!lua_isstring(L, 2))
        return luaL_error(L, "Index must be a string");
    /* variants have a special newindex */
    const char *key = lua_tostring(L, 2);
    if(parent->type == &UA_TYPES[UA_TYPES_VARIANT])
        return ua_variant_newindex(L, parent->data, key, 3);
    int memberindex = find_memberindex(parent->type, key);
    if(memberindex < 0)
        return luaL_error(L, "Index not found");

    const UA_DataType *membertype = &UA_TYPES[parent->type->members[memberindex].memberTypeIndex];
    size_t *arraylen;
    void *member = memberptr(parent->data, parent->type, memberindex, &arraylen);

    if(arraylen) {
        ua_array *array = ua_getarray(L, 3);
        if(!array)
            return luaL_error(L, "The value is not an array");
        if(membertype != array->type)
            return luaL_error(L, "Types don't match");
        UA_Array_delete(*(void**)member, *arraylen, membertype);
        UA_Array_copy(*array->data, *array->length, member, membertype);
        *arraylen = *array->length;
        return 0;
    }
    
    ua_data *data = ua_getdata(L, 3);
    if(!data)
        return luaL_error(L, "Types don't match");
    if(data->type == &UA_TYPES[UA_TYPES_INT32]) {
        UA_Int32 n = *(UA_Int32*)data->data;
        if(membertype->builtin) {
            switch(membertype->typeIndex) {
            case UA_TYPES_BOOLEAN:
                *(UA_Boolean*)member = n;
                return 0;
            case UA_TYPES_SBYTE:
            case UA_TYPES_BYTE:
                *(UA_Byte*)member = n;
                return 0;
            case UA_TYPES_INT16:
            case UA_TYPES_UINT16:
                *(UA_Int16*)member = n;
                return 0;
            case UA_TYPES_INT32:
            case UA_TYPES_UINT32:
            case UA_TYPES_STATUSCODE:
                *(UA_Int32*)member = n;
                return 0;
            case UA_TYPES_INT64:
            case UA_TYPES_UINT64:
            case UA_TYPES_DATETIME:
                *(UA_Int64*)member = n;
                return 0;
            }
        }
    }
    if(membertype != data->type)
        return luaL_error(L, "Types don't match");
    UA_deleteMembers(member, membertype);
    UA_copy(data->data, member, membertype);
    return 0;
}

/***********/
/* Iterate */
/***********/

size_t nodeid_keysSize = 2;
const char * nodeid_keys[] = {"namespaceIndex", "identifier"};

size_t expandednodeid_keysSize = 3;
const char * expandednodeid_keys[] = {"nodeId", "namespaceUri", "serverIndex"};

size_t qualifiedname_keysSize = 2;
const char * qualifiedname_keys[] = {"namespaceIndex", "name"};

size_t localizedtext_keysSize = 2;
const char * localizedtext_keys[] = {"locale", "text"};

size_t variant_keysSize = 2;
const char * variant_keys[] = {"value", "arrayDimensions"};

size_t datavalue_keysSize = 6;
const char * datavalue_keys[] = {"value", "status", "sourceTimestamp", "sourcePicoseconds",
                                 "serverTimestamp", "serverPicoseconds"};

size_t diagnosticinfo_keysSize = 7;
const char *diagnosticinfo_keys[] = {"SymbolicId", "NamespaceUri", "LocalizedText", "Locale",
                                     "AdditionalInfo", "InnerStatusCode", "InnerDiagnosticInfo" };

static int
ua_iterate_builtin(lua_State *L, int dataindex, const char *key,
                   const char * keys[], size_t keysSize) {
    size_t i = 0;
    if(key) {
        for(; i<keysSize; i++) {
            if(strcmp(key, keys[i]) == 0) {
                break;
            }
        }
        /* found nothing or the last entry */
        if(i >= keysSize-1)
            return 0;
        i++; // the next index
    }
    lua_pushstring(L, keys[i]);
    return 1 + ua_index_key(L, dataindex, keys[i]);
}

/* take (v, lastkey), return (key+1, v[key+1]) or return nothing */
static int ua_iterate_atindex(lua_State *L, int dataindex) {
    ua_data *data = luaL_checkudata (L, dataindex, "open62541-data");
    const char *key = lua_tostring(L, dataindex+1);
    
    if(data->type == &UA_TYPES[UA_TYPES_NODEID])
        return ua_iterate_builtin(L, dataindex, key, nodeid_keys, nodeid_keysSize);
    else if(data->type == &UA_TYPES[UA_TYPES_EXPANDEDNODEID])
        return ua_iterate_builtin(L, dataindex, key, expandednodeid_keys, expandednodeid_keysSize);
    else if(data->type == &UA_TYPES[UA_TYPES_LOCALIZEDTEXT])
        return ua_iterate_builtin(L, dataindex, key, localizedtext_keys, localizedtext_keysSize);
    else if(data->type == &UA_TYPES[UA_TYPES_VARIANT])
        return ua_iterate_builtin(L, dataindex, key, variant_keys, variant_keysSize);
    else if(data->type == &UA_TYPES[UA_TYPES_DATAVALUE])
        return ua_iterate_builtin(L, dataindex, key, datavalue_keys, datavalue_keysSize);
    
    int memberindex;
    if(!key) {
        memberindex = 0;
    } else {
        memberindex = find_memberindex(data->type, key);
        if(memberindex < 0)
            return luaL_error(L, "No valid index");
        memberindex++;
    }
    if(memberindex >= data->type->membersSize)
        return 0;

    lua_pushstring(L, data->type->members[memberindex].memberName);
    ua_index_key(L, dataindex, data->type->members[memberindex].memberName);
    return 2;
}

static int ua_iterate(lua_State *L) {
    int ret = ua_iterate_atindex(L, 1);
    if(ret == 0)
        return 0;
    uatolua(L, -1);
    return ret;
}
 
/* Returns (ua_iterate, v, firstkey = nil) */
int ua_pairs(lua_State *L) {
    ua_data *data = luaL_checkudata (L, 1, "open62541-data");
    lua_pushcfunction(L, ua_iterate);
    lua_pushvalue(L, 1);
    lua_pushnil(L);
    return 3;
}

/************/
/* ToString */
/************/

static int ua_tostring_level(lua_State *L, size_t level);
static int ua_array_tostring_level(lua_State *L, size_t level);

/* use iterate over members. the data is at -1 */
static int
ua_tostring_structured(lua_State *L, size_t level) {
    ua_data *d = luaL_testudata(L, -1, "open62541-data");
    lua_pushnil(L);
    int k_index = lua_gettop(L);
    lua_pushfstring(L, "ua.types.%s:", d->type->typeName);
    int pushed = 1;
    while(true) {
        int ret = ua_iterate_atindex(L, k_index-1);
        if(ret == 0)
            break;
        lua_insert(L, -2); // move the key to the top
        lua_replace(L, k_index);
        int v_index = lua_gettop(L);

        lua_pushstring(L, "\n");
        for(size_t j = 0; j < level; j++)
            lua_pushstring(L, "  ");
        lua_pushvalue(L, k_index);
        lua_pushstring(L, " = ");
        pushed += level + 3;

        lua_pushvalue(L, v_index);
        lua_remove(L, v_index);
        v_index = lua_gettop(L);
        if(luaL_testudata(L, -1, "open62541-data"))
            pushed += ua_tostring_level(L, level+1);
        else
            pushed += ua_array_tostring_level(L, level+1);
        lua_remove(L, v_index);
        lua_concat(L, pushed);
        pushed = 1;
    }
    lua_remove(L, k_index);
    return 1;
}

static int
ua_tostring_level(lua_State *L, size_t level) {
    ua_data *d = luaL_testudata(L, -1, "open62541-data");
    void *data = d->data;
    const UA_DataType *type = d->type;

    char out[37];
    switch(type->typeIndex) {
    case UA_TYPES_BOOLEAN:
        if(*(UA_Boolean*)data)
            lua_pushstring(L, "true");
        else
            lua_pushstring(L, "false");
        return 1;
    case UA_TYPES_SBYTE:
        snprintf(out, 40, "%hhi", *(UA_SByte*)data);
        lua_pushstring(L, out);
        return 1;
    case UA_TYPES_BYTE:
        snprintf(out, 40, "%hhu", *(UA_Byte*)data);
        lua_pushstring(L, out);
        return 1;
    case UA_TYPES_INT16:
        snprintf(out, 40, "%hi", *(UA_Int16*)data);
        lua_pushstring(L, out);
        return 1;
    case UA_TYPES_UINT16:
        snprintf(out, 40, "%hu", *(UA_UInt16*)data);
        lua_pushstring(L, out);
        return 1;
    case UA_TYPES_INT32:
        snprintf(out, 40, "%i", *(UA_Int32*)data);
        lua_pushstring(L, out);
        return 1;
    case UA_TYPES_UINT32:
        snprintf(out, 40, "%u", *(UA_UInt32*)data);
        lua_pushstring(L, out);
        return 1;
    case UA_TYPES_INT64:
        snprintf(out, 40, "%li", *(UA_Int64*)data);
        lua_pushstring(L, out);
        return 1;
    case UA_TYPES_UINT64:
        snprintf(out, 40, "%lu", *(UA_UInt64*)data);
        lua_pushstring(L, out);
        return 1;
    case UA_TYPES_STATUSCODE:
        snprintf(out, 40, "0x%08x", *(UA_StatusCode*)data);
        lua_pushstring(L, out);
        return 1;
    case UA_TYPES_FLOAT:
        snprintf(out, 40, "%f", *(UA_Float*)data);
        lua_pushstring(L, out);
        return 1;
    case UA_TYPES_DOUBLE:
        snprintf(out, 40, "%lf", *(UA_Double*)data);
        lua_pushstring(L, out);
        return 1;
    case UA_TYPES_GUID: {
        UA_Guid *id = (UA_Guid*)data;
        snprintf(out, 37, "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
                 id->data1, id->data2, id->data3, id->data4[0], id->data4[1],
                 id->data4[2], id->data4[3], id->data4[4], id->data4[5], id->data4[6], id->data4[7]);
        lua_pushstring(L, out);
        return 1;
    }
    case UA_TYPES_DATETIME: {
        UA_String str = UA_DateTime_toString(*(UA_DateTime*)data);
        lua_pushlstring(L, str.data, str.length);
        UA_String_deleteMembers(&str);
        return 1;
    }
    case UA_TYPES_STRING:
    case UA_TYPES_BYTESTRING:
    case UA_TYPES_XMLELEMENT: {
        UA_String *s = (UA_String*)data;
        if(s->data == NULL) {
            if(level == 0)
                lua_pushfstring(L, "");
            else
                lua_pushfstring(L, "ua.types.%s()", type->typeName);
            return 1;
        } else {
            if(level == 0) {
                lua_pushlstring(L, s->data, s->length);
                return 1;
            } else {
                lua_pushstring(L, "\"");
                lua_pushlstring(L, s->data, s->length);
                lua_pushstring(L, "\"");
                lua_concat(L, 3);
                return 1;
            }
        }
    }
    case UA_TYPES_EXTENSIONOBJECT:
        lua_pushfstring(L, "ua.types.%s", type->typeName);
        return 1;
    case UA_TYPES_EXPANDEDNODEID:
    case UA_TYPES_LOCALIZEDTEXT:
    case UA_TYPES_DIAGNOSTICINFO:
    case UA_TYPES_QUALIFIEDNAME:
    case UA_TYPES_NODEID:
    case UA_TYPES_VARIANT:
    case UA_TYPES_DATAVALUE:
    default:
        return ua_tostring_structured(L, level);
    }
    return luaL_error(L, "Cannot convert the value to a string");
}

int ua_tostring(lua_State *L) {
    return ua_tostring_level(L, 0);
}

/******************/
/* Array Handling */
/******************/

static int
ua_array_tostring_level(lua_State *L, size_t level) {
    ua_array *array = luaL_checkudata (L, -1, "open62541-array");
    size_t len = *array->length;
    const UA_DataType *type = array->type;
    
    int pushed = 1;
    if(len > 0)
        lua_pushfstring(L, "ua.Array(ua.types.%s,%d):", type->typeName, (int)len);
    else {
        if(*array->data > NULL)
            lua_pushfstring(L, "ua.Array(ua.types.%s,0):", type->typeName);
        else
            lua_pushfstring(L, "ua.Array(ua.types.%s,-1):", type->typeName);
    }
    uintptr_t p = (uintptr_t)*array->data;
    for(int i = 0; i < len; i++) {
        lua_pushstring(L, "\n");
        for(size_t j = 0; j < level; j++)
            lua_pushstring(L, "  ");
        lua_pushfstring(L, "%d = ", i+1);
        pushed += level + 2;

        /* fake data element */
        ua_data *data = lua_newuserdata(L, sizeof(ua_data));
        data->type = type;
        data->data = (void*)p;
        luaL_setmetatable(L, "open62541-data");
        int vi = lua_gettop(L);
        pushed += ua_tostring_level(L, level+1);
        data->data = NULL;
        lua_remove(L, vi);

        p += type->memSize;
        lua_concat(L, pushed);
        pushed = 1;
    }
    return 1;
}

int ua_array_tostring(lua_State *L) {
    return ua_array_tostring_level(L, 0);
}

int ua_array_new(lua_State *L) {
    lua_getfield(L, LUA_REGISTRYINDEX, "open62541-type");
    if(!lua_getmetatable(L, 1) || !lua_rawequal(L, -1, -2))
        return luaL_error(L, "The first argument is not a type");
    lua_rawgeti(L, 1, 1);
    const UA_DataType *type = lua_touserdata(L, -1);

    if(!lua_isnumber(L, 2))
        return luaL_error(L, "The second argument is not a valid array size");

    int size = lua_tointeger(L, 2);
    ua_array *array = lua_newuserdata(L, sizeof(ua_array));
    array->type = type;
    array->data = &array->local_data;
    array->length = &array->local_length;
    if(size < 0) {
        array->local_length = 0;
        array->local_data = NULL;
    } else {
        array->local_length = size;
        array->local_data = UA_Array_new(size, type);
    }
    luaL_setmetatable(L, "open62541-array");
    return 1;
}

int ua_array_index(lua_State *L) {
    ua_array *array = luaL_checkudata (L, 1, "open62541-array");
    if(!lua_isnumber(L, 2))
        return luaL_error(L, "Arrays can only be indexed numerically");
    int index = lua_tointeger(L, 2);
    index--; // lua is 1-indexed
    if(index < 0 || index >= *array->length)
        return 0;
	uintptr_t ptr = (uintptr_t)*array->data;
    ptr += index * array->type->memSize;

    /* convert to a native type */
    switch(array->type->typeIndex) {
    case UA_TYPES_BOOLEAN:
        lua_pushboolean(L, *(UA_Boolean*)ptr);
        return 1;
    case UA_TYPES_SBYTE:
        lua_pushinteger(L, *(UA_SByte*)ptr);
        return 1;
    case UA_TYPES_BYTE:
        lua_pushinteger(L, *(UA_Byte*)ptr);
        return 1;
    case UA_TYPES_INT16:
        lua_pushinteger(L, *(UA_Int16*)ptr);
        return 1;
    case UA_TYPES_UINT16:
        lua_pushinteger(L, *(UA_UInt16*)ptr);
        return 1;
    case UA_TYPES_UINT32:
        lua_pushinteger(L, *(UA_UInt32*)ptr);
        return 1;
    case UA_TYPES_INT32:
    case UA_TYPES_STATUSCODE:
        lua_pushinteger(L, *(UA_Int32*)ptr);
        return 1;
    case UA_TYPES_INT64:
        lua_pushinteger(L, *(UA_Int64*)ptr);
        return 1;
    case UA_TYPES_UINT64:
        lua_pushinteger(L, *(UA_UInt64*)ptr);
        return 1;
    case UA_TYPES_FLOAT:
        lua_pushnumber(L, *(UA_Float*)ptr);
        return 1;
    case UA_TYPES_DOUBLE:
        lua_pushnumber(L, *(UA_Double*)ptr);
        return 1;
    case UA_TYPES_STRING:
    case UA_TYPES_BYTESTRING:
    case UA_TYPES_XMLELEMENT:
        lua_pushlstring(L, ((UA_String*)ptr)->data, ((UA_String*)ptr)->length);
        return 1;
    default:
        break;
    }

    /* return a copy */
    ua_data *data = lua_newuserdata(L, sizeof(ua_data));
    data->type = array->type;
    data->data = UA_new(array->type);
    UA_copy((void*)ptr, data->data, array->type);
    luaL_setmetatable(L, "open62541-data");
    return 1;
}

int ua_array_newindex(lua_State *L) {
    ua_array *array = luaL_checkudata (L, 1, "open62541-array");
    if(!array)
        return luaL_error(L, "Not an array");
    if(!lua_isnumber(L, 2))
        return luaL_error(L, "Not an array index");
    int index = lua_tointeger(L, 2) - 1;

    /* removing an entry == set to nil */
    if(lua_isnil(L, 3)) {
        if(index < 0 || index >= *array->length)
            return luaL_error(L, "Cannot remove an entry out of the range");
        if(*array->length == 1) {
            UA_Array_delete(*array->data, 1, array->type);
            *array->data = UA_EMPTY_ARRAY_SENTINEL;
            *array->length = 0;
            return 0;
        }
        uintptr_t arr = (uintptr_t)*array->data;
        UA_deleteMembers((void*)(arr + (array->type->memSize*index)), array->type);
        for(size_t i = index+1; i < *array->length; i++)
            memcpy((void*)(arr + (array->type->memSize*(i-1))),
                   (void*)(arr + (array->type->memSize*i)), array->type->memSize);
        *array->length -= 1;
        return 0;
    }

    ua_data *data = ua_getdata(L, 3);
    if(!data || data->type != array->type)
        return luaL_error(L, "The value is of the wrong type");

    /* insert an entry */
    if(index == *array->length) {
        size_t newsize = *array->length + 1;
        if(*array->data == UA_EMPTY_ARRAY_SENTINEL) 
            *array->data = malloc(array->type->memSize);
        else
            *array->data = realloc(*array->data, newsize * array->type->memSize);
        *array->length = newsize;
		uintptr_t ptr = (uintptr_t)*array->data;
        ptr += (newsize-1) * array->type->memSize;
        UA_copy(data->data, (void*)ptr, array->type);
        return 0;
    }

    /* overwrite an entry */
    if(index < 0 || index >= *array->length)
        return luaL_error(L, "The index is out of range");
	uintptr_t ptr = (uintptr_t)*array->data + (index * array->type->memSize);
    UA_deleteMembers((void*)ptr, array->type);
    UA_copy(data->data, (void*)ptr, array->type);
    return 0;
}

int ua_array_len(lua_State *L) {
    ua_array *array = luaL_checkudata (L, 1, "open62541-array");
    if(*array->length <= 0)
        lua_pushinteger(L, 0);
    else
        lua_pushinteger(L, *array->length);
    return 1;
}

static int
ua_array_iterate(lua_State *L) {
    ua_array *array = luaL_checkudata (L, 1, "open62541-array");
    if(!array)
        return luaL_error(L, "Not a open62541 datatype");
    if(!lua_isnumber(L, 2))
        return luaL_error(L, "Can only iterate over numeric indices");
    int index = lua_tonumber(L, 2);
    if(index < 0 || index >= *array->length)
        return 0;
    lua_pushnumber(L, index+1);
    ua_data *data = lua_newuserdata(L, sizeof(ua_data));
    data->type = array->type;
    data->data = UA_new(array->type);
	uintptr_t ptr = (uintptr_t)*array->data;
    ptr += index * array->type->memSize;
    UA_copy((void*)ptr, data->data, array->type);
    luaL_setmetatable(L, "open62541-data");
    return 2;
}
        
int ua_array_pairs(lua_State *L) {
    ua_array *array = luaL_checkudata (L, 1, "open62541-array");
    if(!array)
        return luaL_error(L, "Not a open62541 datatype");
    lua_pushcfunction(L, ua_array_iterate);
    lua_pushvalue(L, 1);
    lua_pushinteger(L, 0);
    return 3;
}
