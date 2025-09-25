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

using namespace std;

// Ver si podemos utilizar string
struct Users {
    int id;
    char nombre[40];
    char userName[40];
    char password[20];
    char perfil[8];
};

//Funci髇 limpiar consola 
void limpiarConsola() {
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

void menuPrincipal() {
    cout << "\n:::::::::: M贸dulo - Gesti贸n de Usuarios ::::::::::" << endl;
    cout << "\n0) Salir" << endl;
    cout << "1) Ingresar Usuario" << endl;
    cout << "2) Listar Usuarios" << endl;
    cout << "3) Eliminar Usuario" << endl;
    cout << "\nSeleccione una opci贸n: ";
    return ;
}


// Convierte el perfil a may煤sculas y valida
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
        cout << "Perfil inv谩lido. Ingrese 'ADMIN' o 'GENERAL': ";
        cin >> perfil;
        compPerfil(perfil); 
    }
}


void ingresarUsuario(vector<Users>& userList) {
    Users nuevoUsuario;
    int id, opcion;
    
    cout << "\nID: ";
    while (true) {
        cin >> id;
        if (cin.fail() || id <= 0) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "ID inv谩lido. Ingrese un ID entero positivo: ";
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
    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    cout << "Nombre: ";
    cin.getline(nuevoUsuario.nombre, 40);
    if (cin.fail()) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }

    cout << "Username: ";
    cin.getline(nuevoUsuario.userName, 40);
    if (cin.fail()) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }

    cout << "Password: ";
    cin.getline(nuevoUsuario.password, 20);
    if (cin.fail()) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }

    cout << "Perfil: ";
    cin.getline(nuevoUsuario.perfil, 8);
    if (cin.fail()) {
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }

    compPerfil(nuevoUsuario.perfil);

    cout << "\n1) Guardar      2) Cancelar" << endl;
    cout  << "Opci贸n: ";
    cin >> opcion;

    if (opcion == 2) {
        cout << "\nOperaci贸n cancelada." << endl;
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
    cout << "\nID  |Nombre                                  |Username                                |Perfil" << endl;
    cout <<   "----|----------------------------------------|----------------------------------------|-------" << endl;
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
        // Ignorar comentarios y l锟絥eas vac锟絘s
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


int almacenar(string path, const vector<Users>& userList) {
    ofstream outFile(path, ios::trunc); // trunc para reescribir siempre
    if (!outFile) {
        cerr << "\nError al abrir archivo para escribir: " << path << "\n";
        return 1;
    }

    for (const auto& user : userList) {
        //ID
        outFile << setw(5) << setfill('0') << right << user.id;

        //Nombre
        outFile << setw(40) << setfill(' ') << left << user.nombre;

        //Username
        outFile << setw(40) << setfill(' ') << left << user.userName;

        //Password
        outFile << setw(20) << setfill(' ') << left << user.password;

        //Perfil
        outFile << setw(8) << setfill(' ') << left << user.perfil;

        outFile << "\n";
    }

    outFile.close();
    return 0;
}


int cargarDatos(const string& path, vector<Users>& userList) {
    ifstream archivo(path, ios::binary);
    if (!archivo.is_open()) {
        cerr << "Error al abrir archivo: " << path << endl;
        return 1;
    }

    char linea[114]; 
    while (archivo.read(linea, 113)) {
        linea[113] = '\0';

        Users user;

        char idStr[6];
        strncpy(idStr, linea, 5);
        idStr[5] = '\0';
        user.id = atoi(idStr);

        strncpy(user.nombre, linea + 5, 39);
        user.nombre[39] = '\0';

        strncpy(user.userName, linea + 45, 39);
        user.userName[39] = '\0';

        strncpy(user.password, linea + 85, 19);
        user.password[19] = '\0';

        strncpy(user.perfil, linea + 105, 7);
        user.perfil[7] = '\0';

        for (char* p : {user.nombre, user.userName, user.password, user.perfil}) {
            int len = strlen(p);
            while (len > 0 && p[len - 1] == ' ') {
                p[len - 1] = '\0';
                --len;
            }
        }

        userList.push_back(user);

        archivo.ignore(1); //ignorar el salto de l韓ea
    }

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
    int id, opcion;
    cin >> id;
    
    auto it = find_if(userList.begin(), userList.end(), [id](const Users& user) {
        return user.id == id;
    });

    
    if(strcmp(it -> perfil, "ADMIN") == 0){
        cout << "\nAdvertencia: el usuario a eliminar es admin.\n驴Desea continuar?" << endl;
        cout << "\n1) Si      2) No" << endl;
        cout  << "Opci贸n: ";
        cin >> opcion;
    }

    if (opcion == 2) {
        cout << "\nOperaci贸n cancelada." << endl;
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
    

    // Men煤 principal
    cout << "\n:::::::::: M贸dulo - Gesti贸n de Usuarios ::::::::::" << endl;
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
            cout << "\nSaliendo..." << endl;
            break;
            case 1:
            limpiarConsola();
            cout << "\n:::::::::: Ingresar Usuario ::::::::::" << endl;
            ingresarUsuario(userList);
            limpiarUsuarios(ruta_usuarios);
            almacenar(ruta_usuarios, userList);
            break;
            case 2:
            limpiarConsola();
            cout << "\n:::::::::: Listado de Usuarios ::::::::::" << endl;
            listarUsuarios(userList);
            break;
            case 3:
            limpiarConsola();
            cout << "\n:::::::::: Eliminar Usuario ::::::::::" << endl;
            eliminarUsuario(userList);
            limpiarUsuarios(ruta_usuarios);
            almacenar(ruta_usuarios, userList);
            break;
            default:
            limpiarConsola();
            cout << "\nOpci贸n no v谩lida, intente de nuevo." << endl;
            menuPrincipal();
            }
    } while(opcion != 0);
    
    return 0;
}



