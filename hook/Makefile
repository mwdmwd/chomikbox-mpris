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

$(BIND)/hook.dll: $(OBJD)/hook.obj
	$(CXX) -o $@ -shared -static $(CFLAGS) -L$(LIBD) $< $(LIBS)

.PHONY: clean
clean:
	rm -rf "$(OBJD)" "$(BIND)" 2>/dev/null