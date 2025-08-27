#include <iostream>
#include <vector>
#include <cstring>
#include <fstream>
#include <string>
#include <cstdlib>
#include <sstream>
#include <algorithm>
#include <iomanip>

using namespace std;

struct Users {
    int id;
    char nombre[20];
    char userName[20];
    char password[20];
    char perfil[20];
};


void menuPrincipal() {
    cout << "\n:::::::::: Módulo - Gestión de Usuarios ::::::::::" << endl;
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
    
    cout << "\nID: ";
    cin >> id;
    cout << "Nombre: ";
    cin >> nombre;
    cout << "username: ";
    cin >> userName;
    cout << "password: ";
    cin >> password;
    cout << "Perfil: ";
    cin >> perfil;

    cout << "\n1) guardar      2) cancelar" << endl;
    cout  << "Opción: ";
    cin >> opcion;

    if (opcion == 2) {
        cout << "\nOperación cancelada." << endl;
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
    cout << "\nUsuario ingresado exitosamente." << endl;

    //Ordenar por id ascendente
    sort(userList.begin(), userList.end(), [](const Users& a, const Users& b){
        return a.id < b.id;
    });

    menuPrincipal();
    return;
}

void listarUsuarios(const vector<Users>& userList) {
    cout << "\nID   Nombre               Username             Perfil" << endl;
    cout << "-------------------------------------------------------------------" << endl;
    for (const auto& user : userList) {
        std::string idStr = std::to_string(user.id);
        cout << setw(4) << left << idStr;        // ancho fijo 4
        cout << " " << setw(20) << left << user.nombre; // ancho fijo 20
        cout << " " << setw(20) << left << user.userName; // ancho fijo 20
        cout << " " << setw(20) << left << user.perfil; // ancho fijo 20
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



int almacenar(string path, vector<Users>& userList){
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

int cargarDatos(string path, vector<Users>& userList) {
    ifstream archivo(path);
    if(!archivo.is_open()) {
        cerr << "Error al abrir archivo: " << path << endl;
        return 1;
    }
    string linea;
    while(getline(archivo, linea)){
        if (linea.empty() || linea[0] == '#') continue;
        stringstream ss(linea);
        string id, nombre, userName, password, perfil;
        getline(ss, id, ',');
        getline(ss, nombre, ',');
        getline(ss, userName, ',');
        getline(ss, password, ',');
        getline(ss, perfil, ',');

        int userId = stoi(id);
        Users user;
        user.id = userId;
        strncpy(user.nombre, nombre.c_str(), 19);
        user.nombre[19] = '\0';
        strncpy(user.userName, userName.c_str(), 19);
        user.userName[19] = '\0';
        strncpy(user.password, password.c_str(), 19);
        user.password[19] = '\0';
        strncpy(user.perfil, perfil.c_str(), 19);
        user.perfil[19] = '\0';
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
    cout << "\nIndique el id del usuario que desea eliminar: ";
    int id;
    cin >> id;
    
    auto it = find_if(userList.begin(), userList.end(), [id](const Users& user) {
        return user.id == id;
    });

    
    if(strcmp(it -> perfil, "admin") == 0){
        cout << "\nAdvertencia: el usuario a eliminar es admin." << endl;
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
    cout << "Cargando datos de usuarios desde el archivo..." << endl;
    cargarDatos(ruta_usuarios, userList);
    

    // Menú principal
    cout << "\n:::::::::: Módulo - Gestión de Usuarios ::::::::::" << endl;
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
            limpiarUsuarios(ruta_usuarios);
            almacenar(ruta_usuarios, userList);
            break;
            case 1:
            cout << "\n:::::::::: Ingresar Usuario ::::::::::" << endl;
            ingresarUsuario(userList);
            break;
            case 2:
            cout << "\n:::::::::: Listado de Usuarios ::::::::::" << endl;
            listarUsuarios(userList);
            break;
            case 3:
            cout << "\n:::::::::: Eliminar Usuario ::::::::::" << endl;
            eliminarUsuario(userList);
            break;
            default:
            cout << "\nOpción no válida, intente de nuevo." << endl;
            }
    } while(opcion != 0);
    
    return 0;
}



