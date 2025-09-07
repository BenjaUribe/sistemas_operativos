#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include <sstream>
#include <cctype>

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
    {1, "Admin de usuarios y perfiles (en construcción)"},
    {2, "Multiplica matrices NxN"},
    {3, "Juego (en construcción)"},
    {4, "¿Es palíndromo?"},
    {5, "Calcular f(x) = x*x + 2x + 8"},
    {6, "Conteo sobre texto"}
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

// Función para verificar si una cadena es palíndromo
void palindromo(string str){
    int left = 0;
    int right = str.length() - 1;

    while (left < right) {
        if (str[left] != str[right]) {
            cout << "No es palíndromo";
            return;
        }
        left++;
        right--;
    }
    cout << "Es palíndromo";
}

void interfaz_palindromo(){
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
            cout << " ===" << endl;
            break;
        case 2:
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
            cout << "\n=== Resultado de f(" << x << "): " << funcion(x) << " ===" << endl;
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
    cout << "\n:::::::::: Menu principal ::::::::::\n" << endl;
    cout << "Usuario: " << usuario << " | Perfil: " << perfil << "\n" << endl;
    
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

    // Verificación de limpieza
    string user_file = env_vars["USER_FILE"];
    string perfil_file = env_vars["PERFIL_FILE"];
    string M1 = env_vars["MATRIZ1_FILE"];
    string M2 = env_vars["MATRIZ2_FILE"];

    string libros_dir = env_vars["LIBROS_DIR"];
    
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
                cout << "\nSaliendo..." << endl;
                break;
            case 1:
                limpiarConsola();
                cout << "\n:::::::::: Admin de usuarios y perfiles ::::::::::" << endl;
                cout << "\n === Funcionalidad en desarrollo ===" << endl;
                break;
            case 2:
                limpiarConsola();
                cout << "\n:::::::::: Multiplica matrices NxN ::::::::::" << endl;
                //multi_matrices(M1, M2);
                break;
            case 3:
                limpiarConsola();
                cout << "\n:::::::::: Juego ::::::::::" << endl;
                cout << "\n === Funcionalidad en desarrollo ===" << endl;
                break;
            case 4:
                limpiarConsola();
                cout << "\n:::::::::: ¿Es palíndromo? ::::::::::" << endl;
                interfaz_palindromo();
                break;
            case 5:
                limpiarConsola();
                cout << "\n:::::::::: Calcular f(x) = x*x + 2x + 8 ::::::::::" << endl;
                interfaz_funcion();
                break;
            case 6:
                limpiarConsola();
                cout << "\n:::::::::: Conteo sobre texto ::::::::::" << endl;
                conteoTexto(ruta_libro);
                break;
            default:
                limpiarConsola();
                cout << "\nOpción no válida, intente de nuevo." << endl;
        }
    } while(opcion != 0);

    return 0;
}