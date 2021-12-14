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
    // printf("%s\n", self->data);
    lua_pushstring(L, self->data);
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

int get(lua_State *L) {

  CURL *curl;
  curl = curl_easy_init();
  const char *url = lua_tostring(L, -1);

  response_t *res = response_new(L);

  // TODO set headers

  curl_easy_setopt(curl, CURLOPT_URL, url);

  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)res);
  curl_easy_setopt(curl, CURLOPT_HTTPGET, 1);

  curl_easy_perform(curl);

  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &res->code);

  curl_easy_cleanup(curl);

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
  return 1;
}
