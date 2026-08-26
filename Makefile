CC := clang
SRC := main.c
TARGET := program
CFLAG := -Wall -pg -g -lc -Wextra -lraylib -lm

$(TARGET): $(SRC)
	$(CC) $(SRC) -o $(TARGET) ${CFLAG}

clean:
	-rm -f *.o ${TARGET}

run: ${TARGET}
	@./${TARGET}
