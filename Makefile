CC      := clang
SRC     := main.c
TARGET  := raylib-player
CFLAG   := -Wall -pg -g -lc -Wextra -lraylib -lm

SRCS    := $(shell find . -name '*.c' -not -path './.git/*' | grep -v build)

$(TARGET): $(SRC)
	$(CC) $(SRC) -o $(TARGET) ${CFLAG}

tags:
	ctags -f .ctags -R .
	@printf '%s\n' "$(SRCS)" | xargs gcc -M 2>/dev/null | tr ' \\' '\n' | grep '\.h$$' \
		| grep -v "$(CURDIR)/" | grep -vE '/(bits|sys|asm|gnu)/' | sort -u \
		| xargs ctags --quiet --c++-kinds=+p -f .tags.sys
	@echo "tags 生成完毕: $$(wc -l < .ctags) + $$(wc -l < .tags.sys)"

clean:
	-rm -f *.o ${TARGET}

run: ${TARGET}
	@./${TARGET}

.PHONY: tags clean run
