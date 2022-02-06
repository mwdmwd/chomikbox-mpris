LIBS += -lkernel32 -ldetours
CROSS = i686-w64-mingw32-
CXX = $(CROSS)g++

BIND = bin
OBJD = obj

INCD = ../detours-unix/inc
LIBD = ../detours-unix/lib
SRCD = ../Detours/samples/withdll

##############################################################################

all: dirs \
	$(BIND)/withdll.exe

clean:
	rm -rf "$(OBJD)" "$(BIND)" 2>/dev/null

##############################################################################

dirs:
	mkdir -p "$(BIND)" "$(OBJD)"

$(OBJD)/%.obj: $(SRCD)/%.cpp
	$(CXX) -c -I$(INCD) -o $@ $<

$(BIND)/withdll.exe: $(OBJD)/withdll.obj
	$(CXX) -static-libstdc++ -static-libgcc -static $(CFLAGS) -o $@ $^ -L$(LIBD) $(LIBS)