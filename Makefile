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
