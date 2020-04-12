CC=gcc
CFLAGS=-Wall -Werror -Wextra -g
LFLAGS=-lSDL2 -lSDL_net -lSDL2_ttf -lSDL2_image -lm
SERVER_LFLAGS=-lSDL2 -lSDL_net -lm

CLIENT_SRC= client/*.c client/*/*.c utils/*.c gui/*.c gui/renderers/*.c editor/MapRenderer.c game/*.c SDL_FontCache/*.o

EDITOR_SRC=editor/*.c utils/Vector.c gui/*.c game/*.c gui/renderers/*.c SDL_FontCache/*.o

SERVER_SRC=server/*.c game/*.c utils/*.c 

CLIENT_OUT=havoc
SERVER_OUT=havoc_server
EDITOR_OUT=havoc_editor

client:
	$(CC) $(CFLAGS) $(LFLAGS) $(CLIENT_SRC) -o $(CLIENT_OUT)

server:
	$(CC) $(CFLAGS) $(SERVER_LFLAGS) $(SERVER_SRC) -o $(SERVER_OUT)

editor:
	$(CC) $(CFLAGS) $(LFLAGS) $(EDITOR_SRC) -o $(EDITOR_OUT)

.PHONY: server client editor
