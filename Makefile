.POSIX:
.SUFFIXES:

C_DIR 					= c 
CWARNS 					= \
	-Wall \
	-Wextra \
	-pedantic \
	-Waggregate-return \
	-Wcast-align \
	-Wcast-qual \
	-Wdisabled-optimization \
	-Wpointer-arith \
	-Wshadow \
	-Wsign-compare \
	-Wundef \
	-Wwrite-strings \
	-Wbad-function-cast \
	-Wdeclaration-after-statement \
	-Wmissing-prototypes \
	-Wnested-externs \
	-Wstrict-prototypes 
LUA_JIT_INCLUDE = /usr/local/include/luajit-2.0/
LUA_JIT_LIB    	= /usr/local/Cellar/luajit/2.0.5/lib
CFLAGS					= $(CWARNS) -O2 -I$(LUA_JIT_INCLUDE)
LDFLAGS 				= -L$(LUA_JIT_LIB)
LDLIBS 					= -lluajit
PREFIX 					= $(HOME)/.local
OS 							= macosx

.PHONY: all clean install deps
all: c/termios.so c/shutil.so c/socket.so c/lpeg.so c/misc.so

install: all deps
	@echo "🚚 Installing libraries to '$(DESTDIR)$(PREFIX)/share/lluna'"
	@mkdir -p 		$(DESTDIR)$(PREFIX)/share/lluna
	@# Optimized lua files
	@find lua -name '*.lua' | sed -E 's%^lua/(.*)\.lua%\1%' | xargs -I{} luajit -b "./lua/{}.lua" "c/{}.lua"
	@#cp -rf lua		$(DESTDIR)$(PREFIX)/share/lluna
	@cp -rf c 		$(DESTDIR)$(PREFIX)/share/lluna

LPEG= deps/lpeg-1.0.2

c/lpeg.so:
	@$(MAKE) -C $(LPEG) $(OS) "LUADIR=$(LUA_JIT_INCLUDE)" 
	cp $(LPEG)/lpeg.so c/

c/shutil.so: src/lua_shutil.o
	$(CC) $(LDFLAGS) $(LDLIBS) --shared -undefined dynamic_lookup -o $@ $^
c/termios.so: src/lua_termios.o
	$(CC) $(LDFLAGS) $(LDLIBS) --shared -undefined dynamic_lookup -o $@ $^
c/socket.so: src/lua_socket.o
	$(CC) $(LDFLAGS) $(LDLIBS) --shared -undefined dynamic_lookup -o $@ $^

c/misc.so: src/lua_misc.o 
	$(CC) $(LDFLAGS) $(LDLIBS) --shared -undefined dynamic_lookup -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

clean:
	$(RM) **/*.o **/*.so
	$(MAKE) -C deps/lpeg-1.0.2 clean
