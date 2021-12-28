# :full_moon_with_face: Lluna STD

This repo contains the code for the Lluna project standard library.

It contains all the modules accessible from the lluna runtime (and [lua](#usage-from-lua) ). The modules contained in this repository porvide a lightweight interface to the system and extends the base lua library to improve user experience (see [lua](https://github.com/kanales/lluna/blob/master/README.md#what-is-lluna) for more information.

## Modules 

:construction: this section is a work in progress.

- `hashlib`: hash algorithm implementations
- `input`: user friendly terminal inputs
- `json`: json implementation (originally by rxi)
- `getopt`: simple getopt like argument parser 
- `path`: simple and user friendly interface to the filesystem
  - :construction:

### Extensions 

With lluna some extensions to the base library are provided. In the lua runtime the function defined in the extension `modulex` are inserted to the `module` table.
- `tablex`:
  - `extend` 
- `stringx`:
  - `split`: reverse of `table.concat`
- `mathx`: math library extensions (loaded to math when using lluna)
  - `gcd`
  - `clamp`
  - `lerp`
  - `wrap`

## Usage from lua

Although its supposed to be used with the [lluna](https://github.com/kanales/lluna) runtime, it can be simply loaded from luaJIT/lua-5.1 by adding the `c` and `lua` folders to `LUA_CPATH` and `LUA_PATH` respectively. Furthermore, if any lua script is preceded by `require("lluna")()` the behaviour will be exactly the same as using the lluna runtime. 
