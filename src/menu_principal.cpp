#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include <sstream>

using namespace std;



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
    {2, "Multiplica matrices NxN (en construcción)"},
    {3, "Juego (en construcción)"},
    {4, "es palíndromo?"},
    {5, "Calcular f(x)=x*x + 2x + 8 "},
    {6, "CONTEO SOBRE TEXTO (en construcción)"}
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
            cout << "No es palíndromo." << endl;
            return;
        }
        left++;
        right--;
    }
    cout << "Es palíndromo." << endl;
}

void interfaz_palindromo(){
    string str;
    cout << "Ingrese una cadena: ";
    cin.ignore();
    getline(cin, str);
    int opcion;

    cout << "1) Validar" << endl;
    cout << "2) Cancelar" << endl;
    cout << "Seleccione una opción: ";
    cin >> opcion;

    switch(opcion) {
        case 1:
            cout << "===========================" << endl;
            palindromo(str);
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
    cout << "Ingrese el valor de X: ";
    int x;
    cin >> x;
    int opcion;
    cout << "1) Calcular f(x) = x^2 + 2x + 8" << endl;
    cout << "2) Cancelar" << endl;
    cout << "Seleccione una opción: ";
    cin >> opcion;

    switch(opcion) {
        case 1:
            cout << "Resultado: " << funcion(x) << endl;
            return;
        case 2:
            return;
        default:
            cout << "Opción no válida." << endl;
            break;
    }

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
    if(!archivo.is_open()) {
        cerr << "Error al abrir archivo: " << path << endl;
        return usuarios;
    }
    string linea;
    while(getline(archivo, linea)) {
        if (linea.empty() || linea[0] == '#') continue;
        stringstream ss(linea);
        string id, nombre, userName, password, perfil;
        if(!getline(ss, id, ',')) continue;
        if(!getline(ss, nombre, ',')) continue;
        if(!getline(ss, userName, ',')) continue;
        if(!getline(ss, password, ',')) continue;
        getline(ss, perfil, ','); // puede venir vacío
    usuarios[userName] = {password, perfil}; // sobrescribe si duplicado
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

    // Verificación de limpieza
    string user_file = env_vars["USER_FILE"];
    string perfil_file = env_vars["PERFIL_FILE"];

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
                cout << "Funcionalidad en desarrollo..." << endl;
                break;
            case 2:
                limpiarConsola();
                cout << "\n:::::::::: Multiplica matrices NxN ::::::::::" << endl;
                cout << "Funcionalidad en desarrollo..." << endl;
                break;
            case 3:
                limpiarConsola();
                cout << "\n:::::::::: Juego ::::::::::" << endl;
                cout << "Funcionalidad en desarrollo..." << endl;
                break;
            case 4:
                limpiarConsola();
                cout << "\n:::::::::: ¿Es palíndromo? ::::::::::" << endl;
                interfaz_palindromo();
                break;
            case 5:
                limpiarConsola();
                cout << "\n:::::::::: Calcular f(x)=x*x + 2x + 8 ::::::::::" << endl;
                interfaz_funcion();
                break;
            case 6:
                limpiarConsola();
                cout << "\n:::::::::: CONTEO SOBRE TEXTO ::::::::::" << endl;
                cout << "Funcionalidad en desarrollo..." << endl;
                break;
            default:
                limpiarConsola();
                cout << "\nOpción no válida, intente de nuevo." << endl;
        }
    } while(opcion != 0);

    return 0;
}