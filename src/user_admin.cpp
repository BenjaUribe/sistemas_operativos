#include <iostream>
#include <vector>
#include <cstring>
#include <fstream>
#include <string>
#include <cstdlib>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <limits>
#include <stdio.h>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
#endif


using namespace std;

// Ver si podemos utilizar string
struct Users {
    int id;
    char nombre[40];
    char userName[40];
    char password[20];
    char perfil[8];
};


void limpiarConsola() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

void menuPrincipal() {
    cout << "\n:::::::::: Admin de usuarios y perfiles ::::::::::" << endl;

    #ifdef _WIN32
        printf("[PID: %d]\n", GetCurrentProcessId());
    #else
        printf("[PID: %d]\n", getpid());
    #endif

    cout << "\n0) Salir" << endl;
    cout << "1) Ingresar Usuario" << endl;
    cout << "2) Listar Usuarios" << endl;
    cout << "3) Eliminar Usuario" << endl;
    cout << "\nSeleccione una opción: ";
    return ;
}


// Convierte el perfil a mayúsculas y valida
void compPerfil(char* perfil) {
    char temp[20];
    strncpy(temp, perfil, 19);
    temp[19] = '\0';
    for (int i = 0; temp[i]; ++i) 
        temp[i] = static_cast<char>(toupper(static_cast<unsigned char>(temp[i])));
    if (strcmp(temp, "ADMIN") == 0) {
        strcpy(perfil, "ADMIN");
    } else if (strcmp(temp, "GENERAL") == 0) {
        strcpy(perfil, "GENERAL");
    } else {
        cout << "Perfil inválido. Ingrese 'ADMIN' o 'GENERAL': ";
        cin >> perfil;
        compPerfil(perfil); 
    }
}


void ingresarUsuario(vector<Users>& userList) {
    Users nuevoUsuario;
    int id, opcion;

    #ifdef _WIN32
        printf("[PID: %d]\n", GetCurrentProcessId());
    #else
        printf("[PID: %d]\n", getpid());
    #endif
    
    cout << "\nID: ";
    while (true) {
        cin >> id;
        if (cin.fail() || id <= 0) {
            cin.clear();
            cin.ignore(10000, '\n');
            cout << "ID inválido. Ingrese un ID entero positivo: ";
            continue;
        }
        bool idRepetido = false;
        for (const auto& user : userList) {
            if (user.id == id) {
                idRepetido = true;
                break;
            }
        }
        if (idRepetido) {
            cout << "ID ya existente. Ingrese un ID diferente: ";
            continue;
        }
        break;
    }
    cin.ignore(10000, '\n');

    cout << "Nombre: ";
    cin.getline(nuevoUsuario.nombre, 40);
    if (cin.fail()) {
        cin.clear();
        cin.ignore(10000, '\n');
    }

    cout << "Username: ";
    cin.getline(nuevoUsuario.userName, 40);
    if (cin.fail()) {
        cin.clear();
        cin.ignore(10000, '\n');
    }

    cout << "Password: ";
    cin.getline(nuevoUsuario.password, 20);
    if (cin.fail()) {
        cin.clear();
        cin.ignore(10000, '\n');
    }

    cout << "Perfil: ";
    cin.getline(nuevoUsuario.perfil, 8);
    if (cin.fail()) {
        cin.clear();
        cin.ignore(10000, '\n');
    }

    compPerfil(nuevoUsuario.perfil);

    cout << "\n1) Guardar      2) Cancelar" << endl;
    cout  << "Opción: ";
    cin >> opcion;

    if (opcion == 2) {
        cout << "\nOperación cancelada." << endl;
        menuPrincipal();
        return;
    }
    
    nuevoUsuario.id = id;

    userList.push_back(nuevoUsuario);
    cout << "\nUsuario ingresado exitosamente." << endl;

    //Ordenar por id ascendente
    sort(userList.begin(), userList.end(), [](const Users& a, const Users& b){
        return a.id < b.id;
    });

    menuPrincipal();
    return;
}

void listarUsuarios(const vector<Users>& userList) {
    #ifdef _WIN32
        printf("[PID: %d]\n", GetCurrentProcessId());
    #else
        printf("[PID: %d]\n", getpid());
    #endif

    cout << "\nID  |Nombre                                  |Username                                |Perfil" << endl;
    cout <<   "====|========================================|========================================|=======" << endl;
    for (const auto& user : userList) {
        std::string idStr = std::to_string(user.id);
        cout << setw(4) << left << idStr;        // ancho fijo 4
        cout << "|" << setw(40) << left << user.nombre; // ancho fijo 40
        cout << "|" << setw(40) << left << user.userName; // ancho fijo 40
        cout << "|" << setw(8) << left << user.perfil; // ancho fijo 8
        cout << endl;
    }
    menuPrincipal();
    return;
}

string obtenerEnv(const string& nombre_archivo, const string& clave) {
    ifstream archivo(nombre_archivo);
    string linea;
    while (getline(archivo, linea)) {
        // Ignorar comentarios y l�neas vac�as
        if (linea.empty() || linea[0] == '#') continue;

        if (linea.rfind(clave + "=", 0) == 0) { // si empieza con "clave="
            string valor = linea.substr(clave.size() + 1);

            // Reemplazar '\' por '/' en la ruta (Windows)
            replace(valor.begin(), valor.end(), '\\', '/');

            return valor;
        }
    }
    return "";
}

//Versi�n usando getenv
/*const char* obtenerRuta(const string& clave) {
    const char* valor = getenv(clave.c_str());
    if (!valor) return "";

    string ruta(valor);

    replace(ruta.begin(), ruta.end(), '\\', '/');

    return ruta;
}*/


int almacenar(string path, const vector<Users>& userList) {
    ofstream outFile(path, ios::trunc); // trunc para reescribir siempre
    if (!outFile) {
        cerr << "\nError al abrir archivo para escribir: " << path << "\n";
        return 1;
    }

    for (const auto& user : userList) {
        // ID: 5 caracteres, con ceros a la izquierda
        outFile << setw(5) << setfill('0') << right << user.id;

        // Nombre: 40 caracteres, alineado a la izquierda y relleno con espacios
        outFile << setw(40) << setfill(' ') << left << user.nombre;

        // Username: 40 caracteres
        outFile << setw(40) << setfill(' ') << left << user.userName;

        // Password: 20 caracteres
        outFile << setw(20) << setfill(' ') << left << user.password;

        // Perfil: 20 caracteres
        outFile << setw(8) << setfill(' ') << left << user.perfil;

        outFile << "\n";
    }

    outFile.close();
    return 0;
}


int cargarDatos(string path, vector<Users>& userList) {
    ifstream archivo(path);
    if (!archivo.is_open()) {
        cerr << "Error al abrir archivo: " << path << endl;
        return 1;
    }

    string linea;
    while (getline(archivo, linea)) {
        if (linea.empty()) continue;
        if (linea.size() < 113) continue;

        Users user;

        // ID (primeros 5 caracteres)
        string idStr = linea.substr(0, 5);

        // Validar que el ID tenga solo d�gitos
        if (!all_of(idStr.begin(), idStr.end(), ::isdigit)) {
            cerr << "ID inválido en línea: [" << linea << "]" << endl;
            continue;
        }

        user.id = stoi(idStr);

        // Nombre [5, 45)
        string nombre = linea.substr(5, 40);
        strncpy(user.nombre, nombre.c_str(), 39);
        user.nombre[39] = '\0';

        // Username [45, 85)
        string userName = linea.substr(45, 40);
        strncpy(user.userName, userName.c_str(), 39);
        user.userName[39] = '\0';

        // Password [85, 105)
        string password = linea.substr(85, 20);
        strncpy(user.password, password.c_str(), 19);
        user.password[19] = '\0';

        // Perfil [105, 113)
        string perfil = linea.substr(105, 8);
        strncpy(user.perfil, perfil.c_str(), 7);
        user.perfil[7] = '\0';

        // Quitar espacios finales en cada campo
        for (char* p : {user.nombre, user.userName, user.password, user.perfil}) {
            int len = strlen(p);
            while (len > 0 && p[len - 1] == ' ') {
                p[len - 1] = '\0';
                --len;
            }
        }

        userList.push_back(user);
    }

    archivo.close();
    return 0;
}


int limpiarUsuarios(string path){
    ofstream outFile(path, ios::trunc);
    if (!outFile) {
        cerr << "\nError al abrir archivo para limpiar: " << path << "\n";
        return 1;
    }
    outFile.close();
    return 0;
}

int eliminarUsuario(vector<Users>& userList){
    #ifdef _WIN32
        printf("[PID: %d]\n", GetCurrentProcessId());
    #else
        printf("[PID: %d]\n", getpid());
    #endif

    cout << "\nIndique el id del usuario que desea eliminar: ";
    int id, opcion;
    cin >> id;
    
    auto it = find_if(userList.begin(), userList.end(), [id](const Users& user) {
        return user.id == id;
    });

    
    if(strcmp(it -> perfil, "ADMIN") == 0){
        cout << "\nAdvertencia: el usuario a eliminar es admin.\n¿Desea continuar?" << endl;
        cout << "\n1) Si      2) No" << endl;
        cout << "Opción: ";
        cin >> opcion;
    }

    if (opcion == 2) {
        cout << "\nOperación cancelada." << endl;
        menuPrincipal();
        return 0;
    }

    if(it != userList.end()){
        userList.erase(it);
        cout << "\nUsuario eliminado." << endl;
    } else {
        cout << "\nUsuario no encontrado." << endl;
    }
    menuPrincipal();
    return 0;
}

int main() {
    vector<Users> userList;

    // leer variable env
    string ruta_usuarios = obtenerEnv("../.env","USER_FILE");
    //cout << "[DEPURACION] Valor retornado por obtenerEnv: '" << ruta_usuarios << "'" << endl;

    //(con getenv) string ruta = obtenerRuta("USER_FILE");

    // Elimina los espacion en blancos de la ruta
    ruta_usuarios.erase(0, ruta_usuarios.find_first_not_of(" \n\r\t"));
    ruta_usuarios.erase(ruta_usuarios.find_last_not_of(" \n\r\t") + 1);

    
    // Prueba de escritura en el archivo
    /*cout << "[PRUEBA] Intentando abrir y escribir en data/USUARIOS.txt..." << endl;
    ofstream testOut("data/USUARIOS.txt", ios::app);
    if (testOut.is_open()) {
        testOut << "PRUEBA_ESCRITURA\n";
        testOut.close();
        cout << "[PRUEBA] Escritura exitosa en data/USUARIOS.txt" << endl;
    } else {
        cout << "[PRUEBA] Error al abrir data/USUARIOS.txt para escribir" << endl;
    }*/

    //Carga inicial
    cargarDatos(ruta_usuarios, userList);
    

    // Menú principal
    cout << "\n:::::::::: Admin de usuarios y perfiles ::::::::::" << endl;
    #ifdef _WIN32
        printf("[PID: %d]\n", GetCurrentProcessId());
    #else
        printf("[PID: %d]\n", getpid());
    #endif
    cout << "\n0) Salir" << endl;
    cout << "1) Ingresar Usuario" << endl;
    cout << "2) Listar Usuarios" << endl;
    cout << "3) Eliminar Usuario" << endl;
    cout << "\nSeleccione una opción: ";
    int opcion;
    
    
    do {
        // --> despues de ingresar user estamos aqui
        cin >> opcion;
        
        switch(opcion) {
            case 0:
            cout << "\nSaliendo..." << endl;
            limpiarConsola();
            break;
            case 1:
            limpiarConsola();
            cout << ":::::::::: Ingresar Usuario ::::::::::" << endl;
            ingresarUsuario(userList);
            limpiarUsuarios(ruta_usuarios);
            almacenar(ruta_usuarios, userList);
            break;
            case 2:
            limpiarConsola();
            cout << ":::::::::: Listado de Usuarios ::::::::::" << endl;
            listarUsuarios(userList);
            break;
            case 3:
            limpiarConsola();
            cout << ":::::::::: Eliminar Usuario ::::::::::" << endl;
            eliminarUsuario(userList);
            limpiarUsuarios(ruta_usuarios);
            almacenar(ruta_usuarios, userList);
            break;
            default:
            limpiarConsola();
            cout << "\nOpción no válida, intente de nuevo." << endl;
            menuPrincipal();
            }
    } while(opcion != 0);
    
    return 0;
}



