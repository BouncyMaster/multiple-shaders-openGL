LIBS = -lGL -lglfw -ldl -lm -lfreetype
INCS = -Iexternal/glad/include -Iexternal/stb -I/usr/include/freetype2
CFLAGS = -std=c99 -O3 -march=znver1 -Wall -Wno-char-subscripts \
	-Wno-discarded-qualifiers

SRC = src/main.c src/file_ops.c src/text_rendering.c src/camera.c \
	external/glad/src/glad.c
OBJ = $(SRC:.c=.o)

multiple-shaders: $(OBJ)
	$(CC) $(OBJ) $(LIBS) -o $@

.c.o:
	$(CC) $(CFLAGS) $(INCS) -c $< -o $@

src/main.o: src/world_data.h
src/file_ops.o: src/file_ops.h
src/text_rendering.o: src/text_rendering.h
src/camera.o: src/camera.h

gentags:
	ctags `find src -name "*.c" -or -name "*.h"`

clean:
	rm -f multiple-shaders $(OBJ) tags
