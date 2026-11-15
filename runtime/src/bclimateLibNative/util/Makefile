SOURCE_DIRS=.
SOURCE_DIRS+=../native
SOURCE_DIRS+=../../inet/native/sha1

CFLAGS=
CFLAGS += $(patsubst %,-I%,$(SOURCE_DIRS))
CFLAGS +=-g

SRC=
SRC+=bclimateLibNativeCrc32.c
SRC+=bclimateLibNativeRtea.c
SRC+=bclimateLibNative_License_common.c
SRC+=inet_sha1.c
SRC+=strptime_win32.c


VPATH=${SOURCE_DIRS}
NAME=license_generator

GENERATOR=${NAME}${EXT}
TEST=${NAME}_test${EXT}

all: ${GENERATOR} ${TEST}

check: ${TEST}
	./${TEST}

release: all windows

windows:
	make CROSS_PREFIX=i686-w64-mingw32- EXT='.exe' all

${GENERATOR}: ${SRC} main.c
	${CROSS_PREFIX}gcc -o $@ ${CFLAGS} $^

${TEST}: ${SRC} main_test.c
	${CROSS_PREFIX}gcc -o $@ ${CFLAGS} $^


clean:
	-@rm ${TEST} ${GENERATOR}
