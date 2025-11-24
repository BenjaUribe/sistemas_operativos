# Compilar todo por defecto
all: user_admin menu matmul indice_invertido battleship indice_paralelo rendimiento

# Programas individuales
user_admin:
	g++ -o user_admin -Wall user_admin.cpp

menu:
	g++ -o menu -Wall menu_principal.cpp

matmul:
	g++ -o matmul -Wall matmul.cpp

indice_invertido:
	g++ -o indice_invertido -Wall indice_invertido.cpp

indice_paralelo:
	g++ -o indice_paralelo -Wall indice_paralelo.cpp

rendimiento:
	g++ -o rendimiento -Wall rendimiento.cpp 


# Compilar battleship completo (servidor y cliente)
battleship: server client

# Compilar servidor
server:
	g++ -o battleship_server -Wall -std=c++11 battleship/server.cpp battleship/board.cpp battleship/ship.cpp battleship/player.cpp battleship/game_logic.cpp

# Compilar cliente
client:
	g++ -o battleship_client -Wall -std=c++11 battleship/client.cpp battleship/board.cpp battleship/ship.cpp battleship/player.cpp battleship/game_logic.cpp

# Limpiar archivos ejecutables
clean:
	rm -f user_admin menu matmul indice_invertido indice_paralelo battleship_server battleship_client rendimiento

.PHONY: all battleship server client clean

