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
  int af = luaL_checkint(L, 1);
  int st = luaL_checkint(L, 2);
  int flags = luaL_optint(L, 3, IPPROTO_IP);

  lsocket *self = lsocket_new(L);
  self->sock_fd = socket(af, st, flags);
  if (self->sock_fd == -1) {
    luaL_error(L, "Socket Creation Error: %s", strerror(errno));
    return 0;
  }

  return 1;
}

static int lsocket_connect(lua_State *L) {
  lsocket *self = lsocket_get(L, 1);
  const char *address = luaL_checkstring(L, 2);
  int port = luaL_checkint(L, 3);

  struct sockaddr_in *addr = &self->addr;

  addr->sin_family = AF_INET;
  addr->sin_port = htons(port);
  addr->sin_addr.s_addr = inet_addr(address);

  if (connect(self->sock_fd, (struct sockaddr *)addr, sizeof(*addr)) == -1) {
    luaL_error(L, "Connection Error: %s", strerror(errno));
    return 0;
  }

  lua_settop(L, 1);
  return 1;
}

static int lsocket_bind(lua_State *L) {
  lsocket *self = lsocket_get(L, 1);
  const char *address = luaL_checkstring(L, 2);
  int port = luaL_checkint(L, 3);

  struct sockaddr_in *addr = &self->addr;

  addr->sin_family = AF_INET;
  addr->sin_port = htons(port);
  addr->sin_addr.s_addr = inet_addr(address);

  if (bind(self->sock_fd, (struct sockaddr *)addr, sizeof(self->addr)) == -1) {
    luaL_error(L, "Bind Error: %s", strerror(errno));
    return 0;
  }

  lua_settop(L, 1);

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

#define SOCKET_FIELDS                                                          \
  X("accept", lsocket_accept)                                                  \
  X("close", lsocket_close)                                                    \
  X("listen", lsocket_listen)                                                  \
  X("recv", lsocket_recv)                                                      \
  X("send", lsocket_send)                                                      \
  X("connect", lsocket_connect)                                                \
  X("bind", lsocket_bind)

#define lua_pushdefine(L, idx, K)                                              \
  lua_pushinteger(L, K);                                                       \
  lua_setfield(L, (idx) < 0 ? (idx)-1 : (idx) + 1, #K)

int luaopen_socket(lua_State *L) {
  luaL_newmetatable(L, SOCKET_META);

  lua_newtable(L);

#define X(field, fun)                                                          \
  lua_pushcfunction(L, fun);                                                   \
  lua_setfield(L, -2, field);
  SOCKET_FIELDS
#undef X

  lua_setfield(L, -2, "__index");

  lua_pushcfunction(L, lsocket__gc);
  lua_setfield(L, -2, "__gc");

  lua_newtable(L);

  lua_pushcfunction(L, lsocket_open);
  lua_setfield(L, -2, "open");

  // Address families
  lua_pushdefine(L, -1, AF_UNIX);
  lua_pushdefine(L, -1, AF_INET);
  lua_pushdefine(L, -1, AF_INET6);
  lua_pushdefine(L, -1, SOCK_STREAM);
  lua_pushdefine(L, -1, SOCK_DGRAM);
  lua_pushdefine(L, -1, SOCK_RDM);
  lua_pushdefine(L, -1, SOCK_SEQPACKET);
  lua_pushdefine(L, -1, IPPROTO_IP);
  lua_pushdefine(L, -1, IPPROTO_TCP);
  lua_pushdefine(L, -1, IPPROTO_RAW);

  return 1;
}
