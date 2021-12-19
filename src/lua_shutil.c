#include <dirent.h>
#include <errno.h>
#include <lauxlib.h>
#include <luajit.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

int lua_rmdir(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);

  if (rmdir(path) == -1) {
    luaL_error(L, strerror(errno));
  }

  return 0;
}

int lua_chdir(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);

  if (chdir(path) == -1) {
    luaL_error(L, strerror(errno));
  }

  return 0;
}

int lua_mkdir(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);

  if (mkdir(path, S_IRWXU) == -1) {
    switch (errno) {
    case EACCES:
      luaL_error(L, "the parent directory does not allow write");
    case EEXIST:
      luaL_error(L, "pathname already exists");
    case ENAMETOOLONG:
      luaL_error(L, "pathname is too long");
    default:
      luaL_error(L, "mkdir");
    }
  }
  return 0;
}
int lua_cwd(lua_State *L) {
  char buf[1024];

  getcwd(buf, sizeof(buf));
  lua_pushstring(L, buf);
  return 1;
}

typedef struct dir_iterator {
  int closed;
  DIR *d;
} dir_iterator;

int iterdir(lua_State *L) {
  const char *path = luaL_checkstring(L, 1);

  DIR *dir = opendir(path);
  if (dir == NULL) {
    luaL_error(L, "'%s' is not a directory", path);
    return 0;
  }

  dir_iterator *self = lua_newuserdata(L, sizeof(dir_iterator));
  luaL_getmetatable(L, "shutil.dir_iterator");
  lua_setmetatable(L, -2);
  self->closed = 0;
  self->d = dir;

  return 1;
}

struct lua_reg {
  const char *name;
  lua_CFunction func;
};
static const struct lua_reg registry[] = {
    {"mkdir", lua_mkdir}, {"cwd", lua_cwd}, {"dir", iterdir}, {NULL, NULL}};

int dir_iterator__next(lua_State *L) {
  dir_iterator *self = lua_touserdata(L, 1);
  struct dirent *dp;
  DIR *dir = self->d;
  while ((dp = readdir(dir)) != NULL) {
    if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0)
      continue;
    lua_pushstring(L, dp->d_name);
    return 1;
  }

  closedir(self->d);
  self->closed = 1;
  return 0;
}

int dir_iterator__gc(lua_State *L) {
  dir_iterator *self = lua_touserdata(L, 1);
  if (!self->closed)
    closedir(self->d);
  self->closed = 1;
  return 0;
}

int dir_iterator_meta(lua_State *L) {

  luaL_newmetatable(L, "shutil.dir_iterator");

  // methods
  lua_newtable(L);
  lua_pushcfunction(L, dir_iterator__next);
  lua_setfield(L, -2, "next");

  lua_setfield(L, -2, "__index");
  lua_pushcfunction(L, dir_iterator__gc);
  lua_setfield(L, -2, "__gc");

  return 1;
}

int luaopen_shutil(lua_State *L) {

  dir_iterator_meta(L);
  lua_newtable(L);

  const struct lua_reg *ptr;
  for (ptr = registry; ptr->func != NULL; ptr++) {
    lua_pushcfunction(L, ptr->func);
    lua_setfield(L, -2, ptr->name);
  }

#if __

  return 1;
}
