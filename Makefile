#CC=i686-pc-mingw32-gcc
#CC=i586-mingw32msvc-gcc
CC:=$(shell ls /bin/*-mingw32-gcc.exe)
CC:=$(CC:%.exe=%)
METALANG=mql /mql4
CFLAGS=-O2 -m32 -Ilib -Isrc
LDXFLAGS=-shared -Wl,--add-stdcall-alias
LDXFLAGS_LIBS=-lshlwapi -lshell32

UNAME=$(shell uname -o)

ifeq ($(UNAME), Linux)
EXT=
DLLEXT=.so
else
EXT=.exe
DLLEXT=.dll
endif
SQLITE3_OBJS=build/sqlite3.o
SQLITE3_EXT_OBJS=build/extension-functions.o
SHELL_OBJS=build/shell.o $(SQLITE3_OBJS) $(SQLITE3_EXT_OBJS)
WRAPPER_OBJS=build/sqlite3_wrapper.o $(SQLITE3_OBJS) $(SQLITE3_EXT_OBJS)
TARGET_DLL=MQL4/Libraries/sqlite3_wrapper$(DLLEXT)
SQLITE3_SHELL=build/sqlite$(EXT)
MQ4_FILES=$(shell find MQL4/ -type f -name '*.mq4')
HAVE_MQL=$(shell if command -v mql; then true; fi)
EX4_FILES=$(if $(HAVE_MQL),$(MQ4_FILES:MQL4/%.mq4=%.ex4))

.SUFFIXES: .c .o .mq4 .ex4

.PHONEY: all dll
all: build $(TARGET_DLL) $(EX4_FILES) $(SQLITE3_SHELL)

dll: $(TARGET_DLL)

build/%.o:: lib/%.c
	$(CC) $(CFLAGS) -c -o $@ $<
build/%.o:: src/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

.mq4.ex4:
	$(METALANG) $<

build/sqlite3.o:               lib/sqlite3.c lib/sqlite3.h
build/shell.o:                 lib/shell.c   lib/sqlite3.h
build/extension-functions.o:   lib/extension-functions.c lib/sqlite3.h

build/sqlite3_wrapper.o:       src/sqlite3_wrapper.c lib/sqlite3.h
#	$(CC) $(CFLAGS) -c -o $@ $<

$(TARGET_DLL): $(WRAPPER_OBJS)
	$(CC) $(LDXFLAGS) -o $(TARGET_DLL) $(WRAPPER_OBJS) $(LDXFLAGS_LIBS)
	cp $(TARGET_DLL) build

$(SQLITE3_SHELL): $(SHELL_OBJS)
	$(CC) -o $(SQLITE3_SHELL) $(SHELL_OBJS)

build:
	mkdir build

.PHONEY: clean
clean:
	rm -rf $(TARGET_DLL) build

.PHONEY: clean-ex4
clean-ex4:
	rm -f *.ex4

.PHONEY: distclean
distclean: clean clean-ex4
	rm -f *.o
