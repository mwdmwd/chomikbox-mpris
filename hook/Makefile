CFLAGS ?= -Wall -Wextra -Wno-unused-parameter -Wno-cast-function-type

LIBS += -lkernel32 -ldetours
CROSS = i686-w64-mingw32-
CXX = $(CROSS)g++

BIND=bin
OBJD=obj

INCDS = ../detours-unix/inc ..
LIBD = ../detours-unix/lib

.PHONY: all
all: dirs \
	$(BIND)/hook.dll

.PHONY: dirs
dirs:
	mkdir -p "$(BIND)" "$(OBJD)"

$(OBJD)/%.obj: %.cpp
	$(CXX) $(CFLAGS) -c $(addprefix -I, $(INCDS)) -o $@ $<

$(OBJD)/hook.obj: app.h

$(OBJD)/app.obj: app.h offsets.h

$(BIND)/hook.dll: $(OBJD)/hook.obj $(OBJD)/app.obj
	$(CXX) -o $@ -shared -static $(CFLAGS) -L$(LIBD) $^ $(LIBS)

.PHONY: clean
clean:
	rm -rf "$(OBJD)" "$(BIND)" 2>/dev/null