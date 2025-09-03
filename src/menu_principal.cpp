#include <iostream>
#include <fstream>
#include <string>
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

// Usaremos unordered_map<string, pair<string,string>>
// pair.first = password, pair.second = perfil

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

void mostrar_menu(const string& usuario, const string& perfil) {
    cout << "\n:::::::::: Menu principal ::::::::::\n" << endl;
    if (perfil == "ADMIN") {
        cout << "1) Admin de usuarios y perfiles (en construcción)" << endl;
    }
    cout << "2) Multiplica matrices NxN (en construcción)" << endl;
    cout << "3) Juego (en construcción)" << endl;
    cout << "4) ¿es palíndromo? (en construcción)" << endl;
    cout << "5) Calcular f(x)=x*x + 2x + 8 (en construcción)" << endl;
    cout << "6) CONTEO SOBRE TEXTO (en construcción)" << endl;
    cout << "7) Salir" << endl;
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

    // Cargar perfiles (aun no implementado)
    

    // Validar usuario y password desde memoria
    string perfil = validar_usuario(Usuarios, usuario, password);
    if (perfil.empty()) {
        cout << "\nError: Usuario o contraseña incorrectos.\n";
        return 1;
    }

    // Mostrar menú principal
    mostrar_menu(usuario, perfil);

    // No se implementan las funciones, solo se muestra el menú
    return 0;
}