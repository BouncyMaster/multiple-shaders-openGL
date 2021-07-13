LIBS = -lGL -lglfw -ldl -lm -lfreetype
INCS = -Iexternal/glad/include -I/usr/include/freetype2
CFLAGS = -std=c99 -O2 -march=znver1 -Wall -Wno-char-subscripts

SRC = src/main.c src/file_ops.c src/text_rendering.c external/glad/src/glad.c
OBJ = $(SRC:.c=.o)

text-rendering: $(OBJ)
	$(CC) $(OBJ) $(LIBS) -o $@

.c.o:
	$(CC) $(CFLAGS) $(INCS) -c $< -o $@

src/text_rendering.o: src/text_rendering.h
src/file_ops.o: src/file_ops.h

gentags:
	ctags `find src -name "*.c" -or -name "*.h"`

clean:
	rm -f text-rendering $(OBJ) tags
