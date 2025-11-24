#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include <sstream>
#include <cctype>
#include <stdio.h>
#include <filesystem>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
    #include <dirent.h>
#endif

using namespace std;

// variables globales //
// Declarar wide strings con L antes de las comillas
wstring vocales = L"aeiouáéíóúüAEIOUÝÉÝÓÚÜ";
wstring consonantes = L"bcdfghjklmnñpqrstvwxyzBCDFGHJKLMNÑPQRSTVWXYZ";
// // // // // // // // // // // // // // // //

struct Users { // mantenido por compatibilidad futura
    int id;
    char nombre[20];
    char userName[20];
    char password[20];
    char perfil[20];
};


map<int, string> perfiles_opciones = {
    {0, "Salir"},
    {1, "Admin de usuarios y perfiles"},
    {2, "Multiplica matrices NxN"},
    {3, "Juego"},
    {4, "¿Es palíndromo?"},
    {5, "Calcular f(x) = x*x + 2x + 8"},
    {6, "Conteo sobre texto"},
    {7, "Crear índice invertido"},
    {8, "Crear índice invertido paralelo"},
    {9, "BUSCADOR SistOpe"},
    {10, "Prueba de rendimiento Indice Paralelo"}
};

// Usaremos unordered_map<string, pair<string,string>>
// pair.first = password, pair.second = perfil

// funcion para limpiar consola
void limpiarConsola() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
    
}

void print_pid(){
    #ifdef _WIN32
        printf("[PID: %d]\n", GetCurrentProcessId());
    #else
        printf("[PID: %d]\n", getpid());
    #endif
}


// interfaz para indice invertido
bool terminaEnIdx(const string& nombreArchivo) {
    return nombreArchivo.size() >= 4 &&
           nombreArchivo.compare(nombreArchivo.size() - 4, 4, ".idx") == 0;
}

int create_index(string create_index_path, int modo){
    #ifdef _WIN32
        printf("[PID: %d]\n", GetCurrentProcessId());
    #else
        printf("[PID: %d]\n", getpid());
    #endif

    string nombre_indice;
    do{
        cout << "\nIngrese el nombre del indice a crear (debe terminar en .idx): ";
        cin >> nombre_indice;
    }while(terminaEnIdx(nombre_indice) == false);

    string path_carpetas; // ruta fija para libros
    cout << "Ingrese la ruta de la carpeta donde se encuentran los libros: ";
    cin >> path_carpetas;

    if(modo == 0){
        system((create_index_path + " " + nombre_indice + " " + path_carpetas).c_str());
        return 0;
    } else {
        int n_threads = 0;
        int n_lote = 0;

        // Bucle hasta que ambos sean enteros positivos
        while (true) {
            cout << "\nIngrese el número de hilos: ";
            if (!(cin >> n_threads) || n_threads <= 0) {
                cout << "Error: n_threads debe ser un entero positivo. Intente de nuevo." << endl;
                cin.clear();
                cin.ignore(10000, '\n');
                continue;
            }

            cout << "Ingrese el tamaño del lote: ";
            if (!(cin >> n_lote) || n_lote <= 0) {
                cout << "Error: n_lote debe ser un entero positivo. Intente de nuevo." << endl;
                cin.clear();
                cin.ignore(10000, '\n');
                continue;
            }

            // Ambos valores válidos
            break;
        }

        string comando = create_index_path + " " + nombre_indice + " " + path_carpetas + " " + to_string(n_threads) + " " + to_string(n_lote);
        system(comando.c_str());
        return 0;
    }
}
int performance_test(string performance_test_path){
    int n_lote = 0;

    // Bucle hasta que número de lote sea un entero positivo
    while (true) {
            cout << "Ingrese el tamaño del lote: ";
            if (!(cin >> n_lote) || n_lote <= 0) {
                cout << "Error: n_lote debe ser un entero positivo. Intente de nuevo." << endl;
                cin.clear();
                cin.ignore(10000, '\n');
                continue;
            }

            // Ambos valores válidos
            break;
        }
    string comando = performance_test_path + " " + to_string(n_lote);
    system(comando.c_str());
    return 0;
}

int run_game(string game_app_path){
    system((game_app_path).c_str());
    return 0;
}

int buscador(string search_app_path){
    namespace fs = std::filesystem;
    
    // Buscar archivos .idx en la carpeta data/
    vector<string> archivos_idx;
    string data_path = "data/";
    
    // Verificar si la carpeta data/ existe
    if (!fs::exists(data_path) || !fs::is_directory(data_path)) {
        cout << "\nError: La carpeta 'data/' no existe." << endl;
        return 0;
    }
    
    // Iterar sobre todos los archivos en data/
    for (const auto& entry : fs::directory_iterator(data_path)) {
        if (entry.is_regular_file()) {
            string filename = entry.path().filename().string();
            if (terminaEnIdx(filename)) {
                archivos_idx.push_back(entry.path().string());
            }
        }
    }
    
    // Verificar si se encontraron archivos .idx
    if (archivos_idx.empty()) {
        cout << "\nNo se encontraron archivos .idx en la carpeta data/" << endl;
        cout << "Por favor, cree primero un indice invertido usando la opcion 7 u 8." << endl;
        return 0;
    }
    
    // Mostrar archivos disponibles
    cout << "\n=== Archivos de indice disponibles ===" << endl;
    for (size_t i = 0; i < archivos_idx.size(); i++) {
        cout << i + 1 << ". " << archivos_idx[i] << endl;
    }
    
    // Seleccionar archivo
    int seleccion;
    do {
        cout << "\nSeleccione el numero del archivo a usar (1-" << archivos_idx.size() << "): ";
        cin >> seleccion;
        
        if (cin.fail() || seleccion < 1 || seleccion > (int)archivos_idx.size()) {
            cout << "Error: Seleccion invalida." << endl;
            cin.clear();
            cin.ignore(10000, '\n');
            seleccion = 0;
        }
    } while (seleccion < 1 || seleccion > (int)archivos_idx.size());
    
    string nombre_indice = archivos_idx[seleccion - 1];
    cout << "\nUsando archivo: " << nombre_indice << endl;

    // Ejecutar buscador_sistOpe con el archivo seleccionado
    system((search_app_path + " " + nombre_indice).c_str());
  
    return 0;
}




// funciones para las matrices
int matrizNxN(string M_path, char separador){
    //cout << "Verificando matriz en: " << M_path << endl;
    ifstream M(M_path);
    //cout << "Abriendo archivo..." << endl;
    if(!M.is_open()){
        cout << "Error al abrir " << M_path<< endl;
        return -1; // Error al abrir archivo
    }
    //cout << "Archivo abierto correctamente." << endl;
    string linea;
    int numLineas = 0;
    unsigned int ancho = 0;

    //cout << "Leyendo lineas..." << endl;
    while(getline(M, linea)){
        numLineas++;
        //cout << "Linea " << numLineas << ": " << linea << endl;

        // Contar números en la línea usando el separador especificado
        unsigned int cantidadNumeros = 0;
        stringstream ss(linea);
        string token;
        
        // Si el separador es espacio, usa metodo que cuenta tokens distintos de espacio
        if(separador == ' ') {
            while(ss >> token) {
                cantidadNumeros++;
            }
        } else {
            // Para cualquier otro separador, separa la cadena por los caracteres
            while(getline(ss, token, separador)) {
                token.erase(0, token.find_first_not_of(" \t\n\r"));
                token.erase(token.find_last_not_of(" \t\n\r") + 1);
                if(!token.empty()) {
                    cantidadNumeros++;
                }
            }
        }
        
        if(ancho == 0) {
            ancho = cantidadNumeros; // Establecer ancho con la primera línea
        } else if(cantidadNumeros != ancho) {
            //cout << "Error: Línea " << numLineas << " tiene " << cantidadNumeros << " números, pero se esperaba " << ancho << "." << endl;
            //cout << "Error: Líneas con diferente cantidad de números." << endl;
            return -1; // Líneas de diferente longitud
        }
    }
    
    M.close();
    //cout << M_path << endl;
    //cout << numLineas << " " << (int)ancho << endl;
    // Verificar si es cuadrada (n×n)
    if(numLineas == (int)ancho) {
        return numLineas; // Retorna n (dimensión de la matriz)
    }
    
    return -1; // No es cuadrada
}


// interfaz intermedia para matmul
void interfaz_matmul(string matmul_path){
    string matriz1_path;
    string matriz2_path;
    char separador;
    int n, opcion;

    #ifdef _WIN32
        printf("[PID: %d]\n", GetCurrentProcessId());
    #else
        printf("[PID: %d]\n", getpid());
    #endif

    // Validar primera matriz
    do {
        cout << "\nIngrese la ruta de la primera matriz: ";
        cin >> matriz1_path;
        
        ifstream M1(matriz1_path);
        if (!M1.is_open()) {
            cout << "Error: No se pudo abrir el archivo '" << matriz1_path << "'. Intente de nuevo." << endl;
        } else {
            M1.close();
            break; // Salir del bucle si el archivo se abrió correctamente
        }
    } while (true);

    // Validar segunda matriz
    do {
        cout << "Ingrese la ruta de la segunda matriz: ";
        cin >> matriz2_path;
        
        ifstream M2(matriz2_path);
        if (!M2.is_open()) {
            cout << "Error: No se pudo abrir el archivo '" << matriz2_path << "'. Intente de nuevo." << endl;
        } else {
            M2.close();
            break; // Salir del bucle si el archivo se abrió correctamente
        }
    } while (true);


    cout << "Ingrese la dimensión de las matrices (NxN): ";
    cin >> n;
    
    cout << "Ingrese el separador (por defecto es espacio, presione Enter para espacio): ";
    cin.ignore(); // Limpiar el buffer después de cin >> n
    
    string separador_input;
    getline(cin, separador_input);
    
    // Si no ingresa nada (Enter), usar espacio por defecto
    if (separador_input.empty()) {
        separador = ' ';
        cout << "\nUsando separador por defecto: espacio" << endl;
    } else {
        separador = separador_input[0]; // Tomar el primer carácter
        cout << "\nUsando separador: '" << separador << "'" << endl;
    }

    if(matrizNxN(matriz1_path, separador) == -1 || matrizNxN(matriz2_path, separador) == -1){
        cout << "\nMatrices de diferente tamaño\n" << endl;
    return;
    }
    if(matrizNxN(matriz1_path, separador) != n || matrizNxN(matriz2_path, separador) != n){
        cout << "\nError: Las matrices no son de dimensión " << n << "x" << n << "\n" << endl;
        return;
    }

    cout << "\n1) Multiplicar matrices" << endl;
    cout << "2) Cancelar" << endl;
    cout << "\nSeleccione una opción: ";
    cin >> opcion;

    string comando = matmul_path + " " + matriz1_path + " " + matriz2_path + " " + separador + " " + to_string(n);
    switch(opcion) {
        case 1:
            cout << "\nEjecutando programa de multiplicación de matrices..." << endl;
            cout << ">> " << comando << endl;
            system(comando.c_str());
            return;
        case 2:
            cout << "\nOperación cancelada\n" << endl;
            return;
        default:
            cout << "\nOpción no válida.\n" << endl;
            break;
    }
}


// Función para verificar si una cadena es palíndromo
void palindromo(string str){
    int left = 0;
    int right = str.length() - 1;

    while (left < right) {
        if (str[left] != str[right]) {
            cout << "No es palíndromo\n";
            return;
        }
        left++;
        right--;
    }
    cout << "Es palíndromo";
}

void interfaz_palindromo(){
    print_pid();
    string str;
    cout << "\nIngrese una cadena: ";
    cin.ignore();
    getline(cin, str);
    int opcion;

    cout << "\n1) Validar" << endl;
    cout << "2) Cancelar" << endl;
    cout << "\nSeleccione una opción: ";
    cin >> opcion;

    switch(opcion) {
        case 1:
            cout << "\n=== ";
            palindromo(str);
            cout << " ===\n" << endl;
            break;
        case 2:
            cout << "\nOperación cancelada.\n" << endl;
            return;
        default:
            cout << "Opción no válida." << endl;
            break;
    }
}


// funcion para evaluar funcion f(x)=x*x + 2x + 8
int funcion(int x){
    int res;
    res = x*x + 2*x + 8;
    return res;
}

void interfaz_funcion(){
    print_pid();
    cout << "\nIngrese el valor de X: ";
    int x;
    cin >> x;
    int opcion;
    cout << "\n1) Calcular f(" << x << ") = " << x << "^2 + (2*" << x << ") + 8" << endl;
    cout << "2) Cancelar" << endl;
    cout << "\nSeleccione una opción: ";
    cin >> opcion;

    switch(opcion) {
        case 1:
            cout << "\n=== Resultado de f(" << x << "): " << funcion(x) << " ===\n" << endl;
            return;
        case 2:
            return;
        default:
            cout << "\nOpción no válida.\n" << endl;
            break;
    }

}

// funcion que retorna true si c es vocal y false si no
bool esVocal(wchar_t c) {
    // wcout << L"caracter: "<< c <<" esVocal: " << (vocales.find(c) != wstring::npos) << endl; // Línea de depuración
    return vocales.find(c) != wstring::npos;
}

// funcion que retorna true si c es consonante y false si no
bool esConsonante(wchar_t c) {
    // wcout << "caracter: "<< c << " esConsonante: " << (consonantes.find(c) != wstring::npos) << endl; // Línea de depuración
    return consonantes.find(c) != wstring::npos;
}

void conteoTexto(string ruta) {
    print_pid();
    int cont_vocales = 0, cont_consonantes = 0, cont_especiales = 0, cont_palabras = 0;
    bool dentroPalabra = false;
    wifstream archivo(ruta);
    archivo.imbue(locale(""));
    wstring linea;

    if (archivo.is_open()){
        while (getline(archivo, linea)){
            for (wchar_t c : linea) {
                if (esVocal(c)) {
                    cont_vocales++;
                if (!dentroPalabra) {
                    dentroPalabra = true;
                    cont_palabras++;
                }
                } 
                else if (esConsonante(c)) {
                cont_consonantes++;
                if (!dentroPalabra) {
                    dentroPalabra = true;
                    cont_palabras++;
                }
                } 
            else {
                // Todo lo que no sea vocal ni consonante es especial
                cont_especiales++;
                dentroPalabra = false;
                }
            }
        }

        cout << "\n=== Resumen de conteo ===\n" << endl;
        cout << "Archivo: " << ruta.substr(12) << "\n" << endl;
        cout << "Cantidad de vocales: " << cont_vocales << endl;
        cout << "Cantidad de consonantes: " << cont_consonantes << endl;
        cout << "Cantidad de caracteres especiales: " << cont_especiales << endl;
        cout << "Cantidad de palabras: " << cont_palabras << endl;
    }
    else {
        cout << "\nNo se puedo abrir el archivo" << endl;
    }

    // Opción volver
    int opcion;
    do{
        cout << "\n0) Volver al menú principal: ";
        cin >> opcion;
        switch (opcion)
        {
        case 0:
            break;
        }
    }while(opcion != 0);
}

// Función para leer variables del .env
map<string, string> leer_env(const string& env_path) {
    map<string, string> env_vars;
    ifstream env(env_path);
    string line;
    while (getline(env, line)) {
        if (line.empty() || line[0] == '#') continue;
        size_t eq = line.find('=');
        if (eq != string::npos) {
            string key = line.substr(0, eq);
            string value = line.substr(eq + 1);
            env_vars[key] = value;
        }
    }
    return env_vars;
}

// Valida usuario
string validar_usuario(const unordered_map<string, pair<string,string>>& usuarios,
                       const string& usuario, const string& password) {
    auto it = usuarios.find(usuario);
    if (it == usuarios.end()) return "";
    if (it->second.first == password) return it->second.second;
    return "";
}


// Carga hash userName -> (password, perfil)
unordered_map<string, pair<string,string>> cargarUsuarios(const string& path) {
    unordered_map<string, pair<string,string>> usuarios;
    ifstream archivo(path);
    if (!archivo.is_open()) {
        cerr << "Error al abrir archivo: " << path << endl;
        return usuarios;
    }

    string linea;
    while (getline(archivo, linea)) {
        if (linea.empty() || linea[0] == '#') continue;
        if (linea.size() < 113) continue;

        string id = linea.substr(0, 5);
        string nombre = linea.substr(5, 40);
        string userName = linea.substr(45, 40);
        string password = linea.substr(85, 20);
        string perfil = linea.substr(105, 8);

        // Funci�n trim (quita espacios sobrantes)
        auto trim = [](string &s) {
            while (!s.empty() && isspace((unsigned char)s.back())) s.pop_back();
            while (!s.empty() && isspace((unsigned char)s.front())) s.erase(s.begin());
        };

        trim(id);
        trim(nombre);
        trim(userName);
        trim(password);
        trim(perfil);

        usuarios[userName] = {password, perfil};
    }
    return usuarios;
}

// Carga perfiles y sus opciones válidas desde PERFILES.txt
map<string, vector<int>> cargarPerfiles(const string& path) {
    map<string, vector<int>> perfiles;
    ifstream archivo(path);
    if(!archivo.is_open()) {
        cerr << "Error al abrir archivo de perfiles: " << path << endl;
        return perfiles;
    }
    string linea;
    while(getline(archivo, linea)) {
        if (linea.empty() || linea[0] == '#') continue;
        size_t punto_coma = linea.find(';');
        if (punto_coma == string::npos) continue;
        
        string perfil = linea.substr(0, punto_coma);
        string opciones_str = linea.substr(punto_coma + 1);
        
        vector<int> opciones;
        stringstream ss(opciones_str);
        string opcion;
        while(getline(ss, opcion, ',')) {
            try {
                opciones.push_back(stoi(opcion));
            } catch(...) {
                // Ignorar opciones inválidas
            }
        }
        perfiles[perfil] = opciones;
    }
    return perfiles;
}

void mostrar_menu(const string& usuario, const string& perfil, const map<string, vector<int>>& perfiles) {
    cout << ":::::::::: Menú principal ::::::::::" << endl;
    print_pid();
    cout << "\nUsuario: " << usuario << " | Perfil: " << perfil << "\n" << endl;
    
    auto it = perfiles.find(perfil);
    if (it == perfiles.end()) {
        cout << "Error: Perfil no encontrado" << endl;
        return;
    }
    
    const vector<int>& opciones_validas = it->second;
    
    for (int opcion : opciones_validas) {
        cout << opcion << ") " << perfiles_opciones[opcion] << endl;
    }
    
    cout << "\nSeleccione una opción: ";
}



int main(int argc, char* argv[]) {
    unordered_map<string, pair<string,string>> Usuarios; // user -> (password, perfil)
    string usuario, password, file;
    bool u_ok = false, p_ok = false, f_ok = false;

    for (int i = 1; i < argc; ++i) {
        string arg = argv[i];
        if (arg == "-u" && i + 1 < argc) {
            usuario = argv[++i];
            u_ok = true;
        } else if (arg == "-p" && i + 1 < argc) {
            password = argv[++i];
            p_ok = true;
        } else if (arg == "-f" && i + 1 < argc) {
            file = argv[++i];
            f_ok = true;
        } else if (arg == "-u" || arg == "-p" || arg == "-f") {
            cout << "Error: Falta el valor para " << arg << endl;
            u_ok = p_ok = f_ok = false;
            break;
        }
    }

    if (!u_ok || !p_ok || !f_ok) {
        cout << "\nUso correcto: ./menu -u <usuario> -p <password> -f <file>\n";
        return 1;
    }

    // Leer variables del .env
    map<string, string> env_vars = leer_env("../.env");
    /* verificacion de lectura de .env
    cout << "\n[DEBUG] Variables de entorno leídas:" << endl;
    for(const auto& [key, value] : env_vars) {
        cout << key << " = " << value << endl;
    }
    */
    env_vars["USER_FILE"].erase(0, env_vars["USER_FILE"].find_first_not_of(" \n\r\t"));
    env_vars["USER_FILE"].erase(env_vars["USER_FILE"].find_last_not_of(" \n\r\t") + 1);

    env_vars["PERFIL_FILE"].erase(0, env_vars["PERFIL_FILE"].find_first_not_of(" \n\r\t"));
    env_vars["PERFIL_FILE"].erase(env_vars["PERFIL_FILE"].find_last_not_of(" \n\r\t") + 1);

    env_vars["MATRIZ1_FILE"].erase(0, env_vars["MATRIZ1_FILE"].find_first_not_of(" \n\r\t"));
    env_vars["MATRIZ1_FILE"].erase(env_vars["MATRIZ1_FILE"].find_last_not_of(" \n\r\t") + 1);

    env_vars["MATRIZ2_FILE"].erase(0, env_vars["MATRIZ2_FILE"].find_first_not_of(" \n\r\t"));
    env_vars["MATRIZ2_FILE"].erase(env_vars["MATRIZ2_FILE"].find_last_not_of(" \n\r\t") + 1);

    env_vars["LIBROS_DIR"].erase(0, env_vars["LIBROS_DIR"].find_first_not_of(" \n\r\t"));
    env_vars["LIBROS_DIR"].erase(env_vars["LIBROS_DIR"].find_last_not_of(" \n\r\t") + 1);

    env_vars["ADMIN_SYS"].erase(0, env_vars["ADMIN_SYS"].find_first_not_of(" \n\r\t"));
    env_vars["ADMIN_SYS"].erase(env_vars["ADMIN_SYS"].find_last_not_of(" \n\r\t") + 1);

    env_vars["MUTLI_M"].erase(0, env_vars["MUTLI_M"].find_first_not_of(" \n\r\t"));
    env_vars["MUTLI_M"].erase(env_vars["MUTLI_M"].find_last_not_of(" \n\r\t") + 1);

    env_vars["CREATE_INDEX"].erase(0, env_vars["CREATE_INDEX"].find_first_not_of(" \n\r\t"));
    env_vars["CREATE_INDEX"].erase(env_vars["CREATE_INDEX"].find_last_not_of(" \n\r\t") + 1);

    env_vars["INDICE_INVERT_PARALELO"].erase(0, env_vars["INDICE_INVERT_PARALELO"].find_first_not_of(" \n\r\t"));
    env_vars["INDICE_INVERT_PARALELO"].erase(env_vars["INDICE_INVERT_PARALELO"].find_last_not_of(" \n\r\t") + 1);

    env_vars["GAME_APP"].erase(0, env_vars["GAME_APP"].find_first_not_of(" \n\r\t"));
    env_vars["GAME_APP"].erase(env_vars["GAME_APP"].find_last_not_of(" \n\r\t") + 1);

    env_vars["PERFORMANCE_TEST"].erase(0, env_vars["PERFORMANCE_TEST"].find_first_not_of(" \n\r\t"));
    env_vars["PERFORMANCE_TEST"].erase(env_vars["PERFORMANCE_TEST"].find_last_not_of(" \n\r\t") + 1);
  
    env_vars["SEARCH_APP"].erase(0, env_vars["SEARCH_APP"].find_first_not_of(" \n\r\t"));
    env_vars["SEARCH_APP"].erase(env_vars["SEARCH_APP"].find_last_not_of(" \n\r\t") + 1);

    

    // Verificación de limpieza
    string user_file = env_vars["USER_FILE"];
    string perfil_file = env_vars["PERFIL_FILE"];
    string M1 = env_vars["MATRIZ1_FILE"];
    string M2 = env_vars["MATRIZ2_FILE"];

    string libros_dir = env_vars["LIBROS_DIR"];
    string user_admin_path = env_vars["ADMIN_SYS"];
    string matmul_path = env_vars["MUTLI_M"];
    string create_index_path = env_vars["CREATE_INDEX"];
    string create_index_parallel_path = env_vars["INDICE_INVERT_PARALELO"];
    string game_app_path = env_vars["GAME_APP"];
    string performance_test_path = env_vars["PERFORMANCE_TEST"];
    string search_app_path = env_vars["SEARCH_APP"];

    // Concatenar directorio de libros con el archivo específico
    string ruta_libro = libros_dir + "/" + file;


    // Cargar mapa user->(password, perfil)
    Usuarios = cargarUsuarios(user_file);

    // Cargar perfiles y sus opciones válidas
    map<string, vector<int>> perfiles = cargarPerfiles(perfil_file);

    // Validar usuario y password desde memoria
    string perfil = validar_usuario(Usuarios, usuario, password);
    if (perfil.empty()) {
        cout << "\nError: Usuario o contraseña incorrectos.\n";
        return 1;
    }

    // Obtener opciones válidas para este perfil
    auto it = perfiles.find(perfil);
    if (it == perfiles.end()) {
        cout << "\nError: Perfil no encontrado en configuración.\n";
        return 1;
    }
    const vector<int>& opciones_validas = it->second;

    limpiarConsola();

    int opcion;
    do {
        // Mostrar menú principal con opciones según perfil
        mostrar_menu(usuario, perfil, perfiles);
        cin >> opcion;

        // Verificar si la opción seleccionada es válida para este perfil
        bool opcion_valida = false;
        for (int op_valida : opciones_validas) {
            if (opcion == op_valida) {
                opcion_valida = true;
                break;
            }
        }

        if (!opcion_valida && opcion != 0) {
            cout << "\nOpción no válida para su perfil. Intente de nuevo." << endl;
            continue;
        }

        switch(opcion) {
            case 0:
                cout << "\nSaliendo...\n" << endl;
                break;
            case 1:
                limpiarConsola();
                cout << "Ejecutando programa de administración..." << endl;
                system(user_admin_path.c_str());
                break;
            case 2:
                limpiarConsola();
                cout << ":::::::::: Multiplica matrices NxN ::::::::::" << endl;
                interfaz_matmul(matmul_path);
                break;
            case 3:
                limpiarConsola();
                cout << ":::::::::: Juego ::::::::::" << endl;
                run_game(game_app_path);
                break;
            case 4:
                limpiarConsola();
                cout << ":::::::::: ¿Es palíndromo? ::::::::::" << endl;
                interfaz_palindromo();
                break;
            case 5:
                limpiarConsola();
                cout << ":::::::::: Calcular f(x) = x*x + 2x + 8 ::::::::::" << endl;
                interfaz_funcion();
                break;
            case 6:
                limpiarConsola();
                cout << ":::::::::: Conteo sobre texto ::::::::::" << endl;
                conteoTexto(ruta_libro);
                cout << endl;
                break;
            case 7:
                limpiarConsola();
                cout << ":::::::::: Crear índice invertido ::::::::::" << endl;
                create_index(create_index_path, 0);
                break;
            case 8:
                limpiarConsola();
                cout << ":::::::::: Crear índice invertido paralelo ::::::::::" << endl;
                create_index(create_index_parallel_path, 1);
                break;
            case 10:
                limpiarConsola();
                cout << ":::::::::: Prueba de rendimiento ::::::::::" << endl;
                performance_test(performance_test_path);
            case 9:
                limpiarConsola();
                cout << ":::::::::: BUSCADOR SistOpe ::::::::::" << endl;
                buscador(search_app_path);
                break;
            default:
                limpiarConsola();
                cout << "\nOpción no válida, intente de nuevo." << endl;
        }
    } while(opcion != 0);

    return 0;
}