CROSS = i686-w64-mingw32-
CC = $(CROSS)cc
CXX = $(CROSS)g++

CFLAGS = -Wall -Wextra
CFLAGS += -DWIN32_LEAN_AND_MEAN -DD_WIN32_WINNT=0x501

OBJD = obj
INCD = inc
LIBD = lib

DIRS := $(OBJD) $(INCD) $(LIBD)

SRCD = ../Detours/src

OBJS = \
	$(OBJD)/detours.obj	\
	$(OBJD)/modules.obj	\
	$(OBJD)/disasm.obj \
	$(OBJD)/image.obj \
	$(OBJD)/creatwth.obj \
	$(OBJD)/disolx86.obj \
	$(OBJD)/disolx64.obj \
	$(OBJD)/disolia64.obj \
	$(OBJD)/disolarm.obj \
	$(OBJD)/disolarm64.obj

##############################################################################
##
.SUFFIXES: .cpp .h .obj

$(OBJD)/%.obj: $(SRCD)/%.cpp
	$(CXX) $(CFLAGS) -c $< -o $@

##############################################################################

all: dirs \
	$(LIBD)/detours.lib \
	$(INCD)/detours.h   \
	$(INCD)/detver.h	\

##############################################################################

clean:
	rm -rf $(DIRS) 2>/dev/null

##############################################################################

dirs:
	mkdir -p "$(INCD)" "$(LIBD)" "$(OBJD)"

$(LIBD)/detours.lib: $(OBJS)
	$(CROSS)ar rcs $@ $^

$(INCD)/detours.h: $(SRCD)/detours.h
	cp $< $@

$(INCD)/detver.h: $(SRCD)/detver.h
	cp $< $@

$(OBJD)/detours.obj: $(SRCD)/detours.cpp $(SRCD)/detours.h
$(OBJD)/modules.obj: $(SRCD)/modules.cpp $(SRCD)/detours.h
$(OBJD)/disasm.obj: $(SRCD)/disasm.cpp $(SRCD)/detours.h
$(OBJD)/image.obj: $(SRCD)/image.cpp $(SRCD)/detours.h
$(OBJD)/creatwth.obj: $(SRCD)/creatwth.cpp $(SRCD)/uimports.cpp $(SRCD)/detours.h
$(OBJD)/disolx86.obj: $(SRCD)/disasm.cpp $(SRCD)/detours.h
$(OBJD)/disolx64.obj: $(SRCD)/disasm.cpp $(SRCD)/detours.h
$(OBJD)/disolia64.obj: $(SRCD)/disasm.cpp $(SRCD)/detours.h
$(OBJD)/disolarm.obj: $(SRCD)/disasm.cpp $(SRCD)/detours.h
$(OBJD)/disolarm64.obj: $(SRCD)/disasm.cpp $(SRCD)/detours.h
