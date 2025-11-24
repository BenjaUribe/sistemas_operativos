# Compilar todo por defecto
all: user_admin menu matmul indice_invertido battleship indice_paralelo rendimiento buscador_sistema

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
  
# Sistema de búsqueda (Motor, Cache y Buscador)
buscador_sistema: motor_busqueda cache buscador_sistOpe

# Motor de búsqueda (servidor)
motor_busqueda:
	g++ -o motor_busqueda -Wall -std=c++11 motor_busqueda.cpp 

# Cache (servidor + cliente del motor)
cache:
	g++ -o cache -Wall -std=c++11 cache.cpp 

# Buscador (cliente)
buscador_sistope:
	g++ -o buscador_sistOpe -Wall -std=c++11 buscador_sistOpe.cpp 

# Limpiar archivos ejecutables
clean:
	rm -f user_admin menu matmul indice_invertido indice_paralelo battleship_server battleship_client rendimiento motor_busqueda cache buscador_sistOpe

.PHONY: all battleship server client buscador_sistema motor_busqueda cache buscador_sistope clean

