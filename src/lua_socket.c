#include <lauxlib.h>

#include <arpa/inet.h>  // inet_addr
#include <netinet/in.h> // sockaddr_in
#include <sys/socket.h>

#include <errno.h>  // errno
#include <stdio.h>  // fdopen
#include <string.h> // strerror
#include <unistd.h> // close

#include "lua_socket.h"

#define SOCKET_META "socket"

typedef struct lsocket {
  struct sockaddr_in addr;
  int sock_fd;
} lsocket;

static lsocket *lsocket_new(lua_State *L) {
  lsocket *self = lua_newuserdata(L, sizeof(lsocket));
  self->sock_fd = -1;

  luaL_getmetatable(L, SOCKET_META);
  lua_setmetatable(L, -2);
  return self;
}

static lsocket *lsocket_get(lua_State *L, int idx) {
  void *ud = luaL_checkudata(L, idx, SOCKET_META);
  luaL_argcheck(L, ud != NULL, idx, "`socket` expected");

  return (lsocket *)ud;
}

static int lsocket_open(lua_State *L) {
  // Open IPv4 TCP endpoint
  // TODO allow creation of IPv6 endpoints and UDP connections
  const char *address = luaL_checkstring(L, 1);
  int port = luaL_checkint(L, 2);
  lsocket *self = lsocket_new(L);

  struct sockaddr_in *addr = &self->addr;

  addr->sin_family = AF_INET;
  addr->sin_port = htons(port);
  addr->sin_addr.s_addr = inet_addr(address);

  self->sock_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (self->sock_fd == -1) {
    luaL_error(L, "Socket Creation Error: %s", strerror(errno));
    return 0;
  }
  if (bind(self->sock_fd, (struct sockaddr *)addr, sizeof(self->addr)) == -1) {
    close(self->sock_fd);
    luaL_error(L, "Bind Error: %s", strerror(errno));
    return 0;
  }

  return 1;
}

static int lsocket_close(lua_State *L) {
  lsocket *self = lsocket_get(L, 1);

  if (self->sock_fd != -1) {
    close(self->sock_fd);
    self->sock_fd = -1;
  }
  return 0;
}

static int lsocket__gc(lua_State *L) { return lsocket_close(L); }

static int lsocket_listen(lua_State *L) {
  lsocket *self = lsocket_get(L, 1);
  int q = luaL_optint(L, 2, 1);
  if (listen(self->sock_fd, q) == -1) {
    close(self->sock_fd);
    self->sock_fd = -1;
    luaL_error(L, "Listen error");
    return 0;
  }
  return 0;
}

static int lsocket_accept(lua_State *L) {
  lsocket *self = lsocket_get(L, 1);
  unsigned int len = sizeof(self->addr);
  lsocket *client = lsocket_new(L);

  client->sock_fd =
      accept(self->sock_fd, (struct sockaddr *)&client->addr, &len);

  if (client->sock_fd == -1) {
    close(self->sock_fd);
    self->sock_fd = -1;
    luaL_error(L, "Accept Error (did you forget to listen?)");
    return 0;
  }
  return 1;
}

static int lsocket_recv(lua_State *L) {
  static char buffer[4096];
  buffer[0] = '\0';
  lsocket *self = lsocket_get(L, 1);
  int bufsize = luaL_checkint(L, 2);
  int flags = luaL_optint(L, 3, 0);
  if (bufsize >= 4096) {
    luaL_error(L, "Buffer Overflow: recv parameter must be < %d", 4096);
    return 0;
  }

  recv(self->sock_fd, buffer, bufsize, flags);
  lua_pushlstring(L, buffer, bufsize);

  return 1;
}

static int lsocket_send(lua_State *L) {
  lsocket *self = lsocket_get(L, 1);
  const char *msg = luaL_checkstring(L, 2);
  int flags = luaL_optint(L, 3, 0);

  int s = send(self->sock_fd, msg, strlen(msg), flags);

  lua_pushinteger(L, s);
  return 1;
}

int luaopen_socket(lua_State *L) {
  luaL_newmetatable(L, SOCKET_META);

  lua_newtable(L);
  lua_pushcfunction(L, lsocket_accept);
  lua_setfield(L, -2, "accept");
  lua_pushcfunction(L, lsocket_close);
  lua_setfield(L, -2, "close");
  lua_pushcfunction(L, lsocket_listen);
  lua_setfield(L, -2, "listen");
  lua_pushcfunction(L, lsocket_recv);
  lua_setfield(L, -2, "recv");
  lua_pushcfunction(L, lsocket_send);
  lua_setfield(L, -2, "send");

  lua_setfield(L, -2, "__index");

  lua_pushcfunction(L, lsocket__gc);
  lua_setfield(L, -2, "__gc");

  lua_newtable(L);

  lua_pushcfunction(L, lsocket_open);
  lua_setfield(L, -2, "open");

  return 1;
}
