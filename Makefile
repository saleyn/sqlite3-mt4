#CC=i686-pc-mingw32-gcc
#CC=i586-mingw32msvc-gcc
CC:=$(shell ls /bin/*-mingw32-gcc.exe)
CC:=$(CC:%.exe=%)
METALANG=mql /mql4
CFLAGS=-O2 -m32
LDFLAGS=-shared -Wl,--add-stdcall-alias
LDFLAGS_LIBS=-lshlwapi -lshell32

SQLITE3_OBJS=sqlite3.o
SQLITE3_EXT_OBJS=extension-functions.o
WRAPPER_OBJS=sqlite3_wrapper.o
OBJS=$(SQLITE3_OBJS) $(SQLITE3_EXT_OBJS) $(WRAPPER_OBJS)
TARGET_DLL=MQL4/Libraries/sqlite3_wrapper.dll
MQ4_FILES=$(shell find MQL4/ -type f -name '*.mq4')
HAVE_MQL=$(shell if command -v mql; then true; fi)
EX4_FILES=$(if $(HAVE_MQL),$(MQ4_FILES:MQL4/%.mq4=%.ex4))

.SUFFIXES: .c .o .mq4 .ex4

.PHONEY: all dll
all: $(TARGET_DLL) $(EX4_FILES)

dll: $(TARGET_DLL)

.c.o:
	$(CC) $(CFLAGS) -c -o $@ $<

.mq4.ex4:
	$(METALANG) $<

sqlite3.o: sqlite3.c sqlite3.h

$(TARGET_DLL): $(OBJS)
	$(CC) $(LDFLAGS) -o $(TARGET_DLL) $(OBJS) $(LDFLAGS_LIBS)

.PHONEY: clean
clean:
	rm -rf $(TARGET_DLL) $(OBJS) project/build

.PHONEY: clean-ex4
clean-ex4:
	rm -f *.ex4

.PHONEY: distclean
distclean: clean clean-ex4
	rm -f *.o
