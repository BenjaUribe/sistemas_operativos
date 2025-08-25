#include <iostream>
#include <vector>
#include <cstring>
#include <fstream>
#include <string>
#include <cstdlib>
#include <sstream>
#include <algorithm>

using namespace std;

struct Users {
    int id;
    char nombre[20];
    char userName[20];
    char password[20];
    char perfil[20];
};




void menuPrincipal() {
    cout << "\nM贸dulo - Gesti贸n de Usuarios" << endl;
    cout << "\n0) Salir" << endl;
    cout << "1) Ingresar Usuario" << endl;
    cout << "2) Listar Usuarios" << endl;
    cout << "3) Eliminar Usuario" << endl;
    cout << "\nSeleccione una opci贸n: ";
    return ;
}


void ingresarUsuario(vector<Users>& userList) {
    Users nuevoUsuario;
    int id, opcion;
    char nombre[20], userName[20], password[20], perfil[20];
    
    cout << "ID: ";
    cin >> id;
    cout << "Nombre: ";
    cin >> nombre;
    cout << "username: ";
    cin >> userName;
    cout << "password: ";
    cin >> password;
    cout << "Perfil: ";
    cin >> perfil;

    cout << "1) guardar      2) cancelar" << endl;
    cout  << "Opci贸n: ";
    cin >> opcion;

    if (opcion == 2) {
        cout << "Operaci贸n cancelada." << endl;
        return;
    }
    
    nuevoUsuario.id = id;
    strcpy(nuevoUsuario.nombre, nombre);
    strcpy(nuevoUsuario.userName, userName);
    strcpy(nuevoUsuario.password, password);
    strcpy(nuevoUsuario.perfil, perfil);

    userList.push_back(nuevoUsuario);
    cout << "Usuario ingresado exitosamente." << endl;
    menuPrincipal();
    return;
}

void listarUsuarios(const vector<Users>& userList) {
    cout << "\nLista de Usuarios:" << endl;
    for (const auto& user : userList) {
        cout << "ID: " << user.id << endl;
        cout << "Nombre: " << user.nombre << endl;
        cout << "Username: " << user.userName << endl;
        cout << "Perfil: " << user.perfil << endl;
        cout << "------------------------" << endl;
    }
    menuPrincipal();
    return;
}

string obtenerEnv(const string& nombre_archivo, const string& clave) {
    ifstream archivo(nombre_archivo);
    string linea;
    while (getline(archivo, linea)) {
        // Ignorar comentarios y l韓eas vac韆s
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

//Versi髇 usando getenv
/*const char* obtenerRuta(const string& clave) {
    const char* valor = getenv(clave.c_str());
    if (!valor) return "";

    string ruta(valor);

    replace(ruta.begin(), ruta.end(), '\\', '/');

    return ruta;
}*/


/*
int almacenar(const char* path, const vector<Users>& userList){
    ofstream outFile(path);
    if (!outFile) {
            cerr << "Error al abrir archivo para escribir: " << path << "\n";
            return 1;
        }

    for(const auto& user : userList) {
        outFile << user.id << ","
                << user.nombre << ","
                << user.userName << ","
                << user.password << ","
                << user.perfil << "\n";
    }

    outFile.close();
    return 0;
}*/

int main() {
    vector<Users> userList;

    // leer variable env

    string ruta_usuarios = obtenerEnv("../.env","USER_FILE");
    //(con getenv) string ruta = obtenerRuta("USER_FILE");

    //

    // Men煤 principal
    
    cout << "\nM贸dulo - Gesti贸n de Usuarios" << endl;
    cout << "\n0) Salir" << endl;
    cout << "1) Ingresar Usuario" << endl;
    cout << "2) Listar Usuarios" << endl;
    cout << "3) Eliminar Usuario" << endl;
    cout << "\nSeleccione una opci贸n: ";
    int opcion;
    
    
    do {
        // --> despues de ingresar user estamos aqui
        cin >> opcion;
        
        switch(opcion) {
            case 0:
            cout << "Saliendo..." << endl;
            break;
            case 1:
            ingresarUsuario(userList);
            //almacenar(path, userList);
            break;
            case 2:
            cout << "Listar Usuario" << endl;
            listarUsuarios(userList);
            break;
            case 3:
            cout << "Eliminar Usuario" << endl;
            // Aqu铆 agregar la funci贸n para eliminar usuarios
            break;
            default:
            cout << "Opci贸n no v谩lida, intente de nuevo." << endl;
            }
    } while(opcion != 0);
    
    return 0;
}



