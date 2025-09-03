#include <iostream>
#include <string>

using namespace std;

int funcion(int x){
    int res;
    res = x*x + 2*x + 8;
    return res;
}

void interfaz_funcion(){
    cout << "Ingrese el valor de X: " << endl;
    int x;
    cin >> x;
    int opcion;
    cout << "1) Calcular f(x) = x^2 + 2x + 8" << endl;
    cout << "2) Cancelar" << endl;
    cin >> opcion;

    switch(opcion) {
        case 1:
            cout << funcion(x) << endl;
            return;
        case 2:
            return;
        default:
            cout << "Opción no válida." << endl;
            break;
    }

    
}


int main() {
    interfaz_funcion();
    return 0;
}