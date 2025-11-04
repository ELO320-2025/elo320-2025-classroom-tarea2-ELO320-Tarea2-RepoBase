CC = gcc
CFLAGS = -Wall -O2 -Iinclude
LDFLAGS = 
SRC = src/main.c src/preproc.c src/utilidades.c src/imagen.c src/reconocimiento.c
OBJ = $(SRC:.c=.o)
BIN = EDAcodebar

all: $(BIN)

$(BIN): $(OBJ)
	$(CC) $(OBJ) -o $(BIN) $(LDFLAGS)

src/%.o: src/%.c include/%.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(BIN)

