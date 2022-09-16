LDFLAGS = -L/usr/local/lib -L/usr/X11R6/lib
LIBS = -lglfw -lm -lfreetype
INCS = -Iexternal/file_ops -Iexternal/glad/include -Iexternal/stb -I/usr/X11R6/include/freetype2 -I/usr/local/include
CFLAGS = -std=c99 -O2 -march=native -Wall -Wno-char-subscripts

SRC = src/main.c external/file_ops/file_ops.c src/text.c src/camera.c \
	external/glad/src/glad.c
OBJ = ${SRC:.c=.o}

multiple-shaders: ${OBJ}
	${CC} ${OBJ} ${LDFLAGS} ${LIBS} -o $@

.c.o:
	${CC} ${CFLAGS} ${INCS} -c $< -o $@

src/main.o: src/world_data.h
src/text.o: src/text.h
src/camera.o: src/camera.h

tags:
	ctags `find src -name "*.c" -or -name "*.h"`

clean:
	rm -f multiple-shaders $(OBJ) tags
