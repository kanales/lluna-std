#include "lua_misc.h"

#include <lauxlib.h>
#include <math.h>

/**
 * Implementation from wikipedia: https://en.wikipedia.org/wiki/Perlin_noise
 */

inline static float lerp(float a0, float a1, float t) {
  return (a1 - a0) * t + a0;
}

struct vec2 {
  float x, y;
};

static struct vec2 random_grad(int ix, int iy) {
  const unsigned w = 8 * sizeof(unsigned);
  const unsigned s = w / 2; // rotation width
  unsigned a = ix, b = iy;
  a *= 3284157443;
  b ^= a << s | a >> (w - s);
  b *= 1911520717;
  a ^= b << s | b >> (w - s);
  a *= 2048419325;
  float random = a * (3.14159265 / ~(~0u >> 1)); // in [0, 2*Pi]
  struct vec2 v;
  v.x = sin(random);
  v.y = cos(random);
  return v;
}

static float dot_grad(int ix, int iy, float x, float y) {
  // Get gradient from integer coordinates
  struct vec2 gradient = random_grad(ix, iy);

  // Compute the distance vector
  float dx = x - (float)ix;
  float dy = y - (float)iy;

  // Compute the dot-product
  return (dx * gradient.x + dy * gradient.y);
}

int perlin(lua_State *L) {
  float x = luaL_checknumber(L, 1);
  float y = luaL_checknumber(L, 2);
  // Determine grid cell coordinates
  int x0 = (int)x;
  int x1 = x0 + 1;
  int y0 = (int)y;
  int y1 = y0 + 1;

  // Determine interpolation weights
  // Could also use higher order polynomial/s-curve here
  float sx = (float)x1 - x;
  float sy = (float)y1 - y;

  // Interpolate between grid point gradients
  float n0, n1, ix0, ix1, value;

  n0 = dot_grad(x0, y0, x, y);
  n1 = dot_grad(x1, y0, x, y);
  ix0 = lerp(n0, n1, sx);

  n0 = dot_grad(x0, y1, x, y);
  n1 = dot_grad(x1, y1, x, y);
  ix1 = lerp(n0, n1, sx);

  value = lerp(ix0, ix1, sy);

  lua_pushnumber(L, value);
  return 1;
}

int luaopen_misc(lua_State *L) {
  lua_newtable(L);
  lua_pushcfunction(L, perlin);
  lua_setfield(L, -2, "noise");

  return 1;
}
