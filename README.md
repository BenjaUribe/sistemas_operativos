# sistemas_operativos
## Proyecto asignatura INFO198 (Sistemas Operativos)

---

### ***Propósito del Proyecto***

Desarrollo de un prototipo de sistema operativo, al cual se puede acceder a través de perfiles de usuario, los cuales permiten acceder a distintas funciones. Por el momento se han desarrollado los programas: Administrador de usuarios, en el cual se pueden ingresar, listar o eliminar usuarios; Menú principal, que cuenta con funcionalidades implementadas como cálculo de función, conteo sobre texto y verificación de palíndromos; y Multiplicación de matrices, el cual multiplica matrices de tamaño NxN.

---

### ***Instrucciones de uso***

Para ejecutar, se debe entrar a la carpeta `/src` donde se ubica el archivo `menu_principal.cpp`, `user_admin.cpp` y `matmul.cpp`. Desde la terminal, ejecutar `make` y luego ejecutar de acuerdo al programa deseado:

- `./user_admin` para ejecutar el administrador de usuarios.
- `./menu -u <username> -p <password> -f <nombre de archivo .txt>` para ejecutar el menú principal.
- `./matmul <ruta archivo matriz1> <ruta archivo matriz 2> <separador>` para ejecutar la multiplicación de matrices.

---

### ***Variables de entorno***

- **`USER_FILE`**: ruta del archivo `USUARIOS.txt`, el cual almacena los datos de cada usuario ingresado al sistema.<br/>
- **`PERFIL_FILE`**: ruta del archivo `PERFILES.txt`, el cual almacena los datos de cada PERFIL disponible en el sistema. <br/>
- **`MATRIZ1_FILE`**: ruta del archivo `matriz1.txt`, el cual almacena la matriz para la multiplicacion de matrices. <br/>
- **`MATRIZ2_FILE`**: ruta del archivo `matriz2.txt`, el cual almacena la matriz para la multiplicacion de matrices. <br/>
- **`LIBROS_DIR`**: ruta del archivo `libros`, el cual almacena la ruta de la carpeta donde se almacena todos los libros. <br/>
