CC = gcc
CFLAGS = -Wall -Wno-unused-result -O2 -Iinclude
LDFLAGS =
SRC = src/main.c src/preproc.c src/utilidades.c src/imagen.c src/reconocimiento.c
OBJ = $(patsubst src/%.c, obj/%.o, $(SRC))
BIN = EDAcodebar

all: obj $(BIN)

obj:
	mkdir -p obj

$(BIN): $(OBJ)
	$(CC) $(OBJ) -o $(BIN) $(LDFLAGS)

obj/%.o: src/%.c include/%.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f obj/*.o $(BIN)
