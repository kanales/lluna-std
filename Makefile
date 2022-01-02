.POSIX:
.SUFFIXES:

C_DIR 					= c 
LUA_JIT_INCLUDE =	/usr/local/include/luajit-2.0/
LUA_JIT_LIB    	= /usr/local/Cellar/luajit/2.0.5/lib
CFLAGS					= -Wall -I$(LUA_JIT_INCLUDE)
LDFLAGS 				= -L$(LUA_JIT_LIB)
LDLIBS 					= 
PREFIX 					= $(HOME)/.local

.PHONY: all clean install
all: c/termios.so c/curl.so c/shutil.so

install: all
	mkdir -p 		$(DESTDIR)$(PREFIX)/share/lluna
	cp -rf c 		$(DESTDIR)$(PREFIX)/share/lluna
	cp -rf lua 	$(DESTDIR)$(PREFIX)/share/lluna

c/shutil.so: src/lua_shutil.o
	$(CC) $(LDFLAGS) $(LDLIBS) --shared -fPIC -o $@ $^
c/curl.so: src/lua_curl.o
	$(CC) $(LDFLAGS) $(LDLIBS) --shared -fPIC -o $@ $^
c/termios.so: src/lua_termios.o
	$(CC) $(LDFLAGS) $(LDLIBS) --shared -fPIC -o $@ $^


%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<


clean:
	$(RM) **/*.o **/*.so
