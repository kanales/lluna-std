#include <curl/curl.h>
#include <lauxlib.h>
#include <luajit.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct response {
  char *data;
  int code;
  size_t size;
};
typedef struct response response_t;
response_t *response_new(lua_State *L) {
  response_t *self = (response_t *)lua_newuserdata(L, sizeof(struct response));
  luaL_getmetatable(L, "response");
  lua_setmetatable(L, -2);
  self->data = NULL;
  return self;
}
response_t *response_get(lua_State *L) {
  void *ud = luaL_checkudata(L, 1, "response");
  luaL_argcheck(L, ud != NULL, 1, "`response` expected");

  return (response_t *)ud;
}

int response__gc(lua_State *L) {
  response_t *self = response_get(L);
  free(self->data);
  return 0;
}

int response__index(lua_State *L) {
  response_t *self = response_get(L);
  const char *s = lua_tostring(L, -1);

  if (strcmp(s, "data") == 0) {
    lua_pushstring(L, self->data);
    return 1;
  }

  if (strcmp(s, "code") == 0) {
    lua_pushinteger(L, self->code);
    return 1;
  }

  return 0;
}

int response_register(lua_State *L) {
  luaL_newmetatable(L, "response");
  lua_pushcfunction(L, response__index);
  lua_setfield(L, -2, "__index");

  lua_pushcfunction(L, response__gc);
  lua_setfield(L, -2, "__gc");

  return 0;
}

///
size_t write_callback(char *ptr, size_t size, size_t nmemb, void *data) {
  size_t realsize = size * nmemb;
  struct response *res = (struct response *)data;

  char *p;

  if (res->data == NULL) {
    p = malloc(res->size + realsize + 1);
  } else {
    p = realloc(res->data, res->size + realsize + 1);
  }
  if (p == NULL)
    return 0; // out of memory

  res->data = p;
  memcpy(&res->data[res->size], ptr, realsize);
  res->size += realsize;
  res->data[res->size] = '\0';
  return realsize;
}

int _perform(lua_State *L, CURL *curl) {

  response_t *res = response_new(L);

  // TODO set headers

  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)res);

  int err;
  if ((err = curl_easy_perform(curl)) != CURLE_OK) {
    luaL_error(L, "curl_easy_perform() failed: %s\n", curl_easy_strerror(err));
    return 1;
  }

  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &res->code);
  return 0;
}

int get(lua_State *L) {

  CURL *curl;
  curl = curl_easy_init();
  if (!curl)
    return 0; // error

  struct curl_slist *chunk = NULL;

  const char *url;
  if (lua_isnil(L, 1))
    url = lua_tostring(L, 1);
  else {
    url = luaL_checkstring(L, 1);
  }

  curl_easy_setopt(curl, CURLOPT_URL, url);

  if (lua_istable(L, 2)) {

    lua_pushnil(L);
    while (lua_next(L, 2)) {
      const char *v = lua_tostring(L, -1);
      chunk = curl_slist_append(chunk, v);
      lua_pop(L, 1);
    }
  }

  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
  curl_easy_setopt(curl, CURLOPT_HTTPGET, 1);
  if (_perform(L, curl))
    return 0;
  curl_easy_cleanup(curl);
  curl_slist_free_all(chunk);

  return 1;
}
int post(lua_State *L) {

  CURL *curl;
  curl = curl_easy_init();
  if (!curl)
    return 0; // error

  struct curl_slist *chunk = NULL;

  const char *url;
  url = luaL_checkstring(L, 1);

  curl_easy_setopt(curl, CURLOPT_URL, url);

  if (lua_gettop(L) > 1) {
    const char *data = luaL_checkstring(L, 2);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
  }

  if (lua_gettop(L) > 2 && lua_istable(L, 2)) {

    lua_pushnil(L);
    while (lua_next(L, 2)) {
      const char *v = lua_tostring(L, -1);
      chunk = curl_slist_append(chunk, v);
      lua_pop(L, 1);
    }
  }

  curl_easy_setopt(curl, CURLOPT_HTTPPOST, 1);
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);

  if (_perform(L, curl))
    return 0;
  curl_easy_cleanup(curl);
  curl_slist_free_all(chunk);

  return 1;
}

int luaopen_curl(lua_State *L) {
  /** TODO
   * - metatable for response
   * - use to json
   */
  response_register(L);

  lua_newtable(L);

  lua_pushcfunction(L, get);
  lua_setfield(L, -2, "get");

  lua_pushcfunction(L, post);
  lua_setfield(L, -2, "post");
  return 1;
}
