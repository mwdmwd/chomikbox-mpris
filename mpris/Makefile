CFLAGS ?= -Wall -Wextra -Wno-unused-parameter

ifeq ($(shell test -d /usr/lib32/pkgconfig && echo t), t)
PKG_CONFIG_PATH ?= /usr/lib32/pkgconfig
else ifeq ($(shell test -d /usr/lib/i386-linux-gnu/pkgconfig && echo t), t)
PKG_CONFIG_PATH ?= /usr/lib/i386-linux-gnu/pkgconfig
endif

PKG_CONFIG = PKG_CONFIG_PATH=$(PKG_CONFIG_PATH) pkg-config

CFLAGS += -m32 -std=c99 -fpic
CFLAGS += $(shell $(PKG_CONFIG) --cflags glib-2.0 gio-2.0 gio-unix-2.0)

LIBS = $(shell $(PKG_CONFIG) --libs glib-2.0 gio-2.0 gio-unix-2.0)

.PHONY: all
all: server.so mpris-server.dll.so

MPRIS_GENERATED = mpris.h mpris.c
$(MPRIS_GENERATED): spec/*.xml
	gdbus-codegen --interface-prefix org.mpris --generate-c-code mpris $^

server.o: mpris.h
interface.o: interface.h

server.so: server.o interface.o mpris.o
	$(CC) -o $@ $(CFLAGS) -shared $^ $(LIBS)

mpris-server.dll.so: server.o interface.o mpris.o server.spec
	winegcc -o mpris-server.dll $(CFLAGS) -shared $^ $(LIBS)

.PHONY: clean
clean:
	rm -f *.o *.so $(MPRIS_GENERATED)