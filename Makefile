C_DIR = c 
CFLAGS = -Wall -I/usr/local/include/luajit-2.0/
LDFLAGS = -lluajit

SO_FILES = $(patsub %.so,c/$.)

.PHONY: all clean
all: c/termios.so c/curl.so

c/curl.so: src/lua_curl.o
	$(CC) $(LDFLAGS) --shared -lcurl -fPIC -o $@ $^
c/termios.so: src/lua_termios.o
	$(CC) $(LDFLAGS) --shared -fPIC -o $@ $^

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c -o $@ $^

clean:
	$(RM) **/*.o **/*.so
