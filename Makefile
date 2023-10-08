CC = gcc
CFLAGS = -std=c17 -Wall
LDFLAGS = -lmingw32 -lSDL2main -lSDL2 -lSDL2_ttf
INCLUDES = -I./SDL2/include
LIBS = -L./SDL2/lib
SRC_DIR = src
SRC = $(wildcard $(SRC_DIR)/*.c)
OBJ = textures/icon.o
OUTPUT = network

all: $(OUTPUT)

$(OUTPUT): $(OBJ)
	$(CC) $(CFLAGS) $(SRC) $(OBJ) $(INCLUDES) $(LIBS) $(LDFLAGS) -o $(OUTPUT)

clean:
	rm -f $(OUTPUT)
