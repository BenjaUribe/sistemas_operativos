# sistemas_operativos
## Proyecto asignatura INFO198 (Sistemas Operativos)

---

### ***Propósito del Proyecto***

La aplicación consiste en un prototipo de sistema operativo, cuyo propósito actual es crear perfiles de usuario, con la opción de listarlos y eliminarlos, a la espera de implementar nuevas funcionalidades.

---

### ***Instrucciones de uso***

Para ejecutar, se debe entrar a la carpeta `/src` donde se ubica el archivo `menu_principal.cpp`, `user_admin.cpp` y `matmul.cpp`. Desde la terminal, ejecutar `make` para compilar todo el proyecto o `make battleship` para ejecutar solo el juego `battleship` y luego `./menu` para ejecuta el menu principal, `./user_admin` para ejecutar el administrador de usuarios. Para iniciar el juego, seleccionar la opcion desde `menu` o ejecutar directamente `./battleship_server` para comenzar a ejecutar el servidor del juego y a continuacion ejecutar `battleship_client` la cantidad de veces necesaria para tener todos los clientes necesarios.

---

### ***Variables de entorno***

- **`USER_FILE`**: ruta del archivo `USUARIOS.txt`, el cual almacena los datos de cada usuario ingresado al sistema.<br/>
- **`PERFIL_FILE`**: ruta del archivo `PERFILES.txt`, el cual almacena los datos de cada PERFIL disponible en el sistema. <br/>
- **`MATRIZ1_FILE`**: ruta del archivo `matriz1.txt`, el cual almacena la matriz para la multiplicacion de matrices. <br/>
- **`MATRIZ2_FILE`**: ruta del archivo `matriz2.txt`, el cual almacena la matriz para la multiplicacion de matrices. <br/>
- **`LIBROS_DIR`**: ruta del archivo `libros`, el cual almacena la ruta de la carpeta donde se almacena todos los libros. <br/>
- **`ADMIN_SYS`**: ruta al archivo `user_admin`, el cual se encarga de administrar todos los usuarios del sistema. <br/>
- **`MUTLI_M`**: ruta al archivo `matmul`, el cual multiplica dos matrices entregadas como parametros. <br/>
- **`CREATE_INDEX`**: ruta al archivo `indice_invertido`, el cual crea un indice invertido usando la carpeta libros. <br/>
- **`INDICE_INVERT_PARALELO`**: ruta al archivo `indice_invertido_paralelo`, el cual crea un indice invertido usando threads. <br/>
- **`GAME_APP`**: ruta al archivo `battleship_server`, el crea un socket para que los jugadores puedan conectarse para iniciar el juego <br/>


