CC := clang
SRC := main.c
TARGET := raylib-player
CFLAG := -Wall -pg -g -lc -Wextra -lraylib -lm

$(TARGET): $(SRC)
	$(CC) $(SRC) -o $(TARGET) ${CFLAG}

clean:
	-rm -f *.o ${TARGET}

run: ${TARGET}
	@./${TARGET}

tags:
	ctags -R --languages=C,C++ --exclude=.git -f .ctags --c-kinds=+p+l --extras=+q /usr/include/raylib.h /usr/include/raymath.h
