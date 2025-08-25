#include <iostream>
#include <vector>
#include <cstring>
#include <fstream>
#include <string>
#include <cstdlib>
#include <sstream>

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




    //

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
            // Aquí agregar la función para eliminar usuarios
            break;
            default:
            cout << "Opción no válida, intente de nuevo." << endl;
            }
    } while(opcion != 0);
    
    return 0;
}



