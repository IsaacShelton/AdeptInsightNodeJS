
CC=emcc
AR=emar
SRCDIR=../src
OBJDIR=../obj
C_SOURCES=$(wildcard $(SRCDIR)/AST/*.c) $(wildcard $(SRCDIR)/BRIDGE/*.c) $(wildcard $(SRCDIR)/DRVR/*.c) $(wildcard $(SRCDIR)/LEX/*.c) $(wildcard $(SRCDIR)/PARSE/*.c) $(wildcard $(SRCDIR)/UTIL/*.c)
C_OBJECTS=$(C_SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
INSIGHT=../../../obj/insight.a
CFLAGS=-c -Wall -I"../include" -O3 -DADEPT_INSIGHT_BUILD # -fmax-errors=5 -Werror

release: directories $(INSIGHT)

$(INSIGHT): $(C_OBJECTS)
	$(AR) -rcs $(INSIGHT) $(C_OBJECTS)

$(C_OBJECTS): $(OBJDIR)/%.o : $(SRCDIR)/%.c
	$(CC) $(CFLAGS) $< -o $@

ifeq ($(OS), Windows_NT)
directories:
	@if not exist $(OBJDIR) mkdir $(OBJDIR)
	@if not exist $(OBJDIR)\AST mkdir $(OBJDIR)\AST
	@if not exist $(OBJDIR)\BRIDGE mkdir $(OBJDIR)\BRIDGE
	@if not exist $(OBJDIR)\DRVR mkdir $(OBJDIR)\DRVR
	@if not exist $(OBJDIR)\LEX mkdir $(OBJDIR)\LEX
	@if not exist $(OBJDIR)\PARSE mkdir $(OBJDIR)\PARSE
	@if not exist $(OBJDIR)\UTIL mkdir $(OBJDIR)\UTIL
else
directories:
	@mkdir -p $(OBJDIR)
	@mkdir -p $(OBJDIR)/AST
	@mkdir -p $(OBJDIR)/BRIDGE
	@mkdir -p $(OBJDIR)/DRVR
	@mkdir -p $(OBJDIR)/LEX
	@mkdir -p $(OBJDIR)/PARSE
	@mkdir -p $(OBJDIR)/UTIL
endif

clean:
ifeq ($(OS), Windows_NT)
	if exist $(INSIGHT) del $(INSIGHT) /Q
	del $(OBJDIR)\AST\*.* /Q
	del $(OBJDIR)\BRIDGE\*.* /Q
	del $(OBJDIR)\DRVR\*.* /Q
	del $(OBJDIR)\LEX\*.* /Q
	del $(OBJDIR)\PARSE\*.* /Q
	del $(OBJDIR)\UTIL\*.* /Q
else
	rm -f 2> /dev/null $(INSIGHT)
	rm -f 2> /dev/null $(OBJDIR)/AST/*.*
	rm -f 2> /dev/null $(OBJDIR)/BRIDGE/*.*
	rm -f 2> /dev/null $(OBJDIR)/DRVR/*.*
	rm -f 2> /dev/null $(OBJDIR)/LEX/*.*
	rm -f 2> /dev/null $(OBJDIR)/PARSE/*.*
	rm -f 2> /dev/null $(OBJDIR)/UTIL/*.*
endif
