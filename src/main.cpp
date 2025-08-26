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
    cout << "\nMódulo - Gestión de Usuarios" << endl;
    cout << "\n0) Salir" << endl;
    cout << "1) Ingresar Usuario" << endl;
    cout << "2) Listar Usuarios" << endl;
    cout << "3) Eliminar Usuario" << endl;
    cout << "\nSeleccione una opción: ";
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
    cout  << "Opción: ";
    cin >> opcion;

    if (opcion == 2) {
        cout << "Operación cancelada." << endl;
        return;
    }
    
    nuevoUsuario.id = id;
    strncpy(nuevoUsuario.nombre, nombre, 19);
    nuevoUsuario.nombre[19] = '\0';
    strncpy(nuevoUsuario.userName, userName, 19);
    nuevoUsuario.userName[19] = '\0';
    strncpy(nuevoUsuario.password, password, 19);
    nuevoUsuario.password[19] = '\0';
    strncpy(nuevoUsuario.perfil, perfil, 19);
    nuevoUsuario.perfil[19] = '\0';

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



int almacenar(string path, const vector<Users>& userList){
    // Verificar si el archivo existe antes de abrirlo
    /*cout << "\nAlmacenando usuarios en: " << path << endl;
    ifstream testFile(path);
    if (testFile.good()) {
        cout << "[VERIFICACION] El archivo existe: " << path << endl;
    } else {
        cout << "[VERIFICACION] El archivo NO existe, se creará: " << path << endl;
    }
    testFile.close();*/

    ofstream outFile(path, ios::app);
    cout << "Almacenando usuarios en: " << path << endl;
    if (!outFile) {
            cerr << "\nError al abrir archivo para escribir: " << path << "\n";
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


    // Menú principal
    cout << "\nMódulo - Gestión de Usuarios" << endl;
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
            cout << "Saliendo..." << endl;
            almacenar(ruta_usuarios, userList);
            break;
            case 1:
            ingresarUsuario(userList);
            break;
            case 2:
            cout << "Listar Usuario" << endl;
            listarUsuarios(userList);
            break;
            case 3:
            cout << "Eliminar Usuario" << endl;
            // Aquí agregar la función para eliminar usuarios
            break;
            default:
            cout << "Opción no válida, intente de nuevo." << endl;
            }
    } while(opcion != 0);
    
    return 0;
}



