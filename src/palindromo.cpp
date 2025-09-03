#include <iostream>
#include <string>

using namespace std;

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
    getline(cin, str);
     int opcion;

    cout << "1) Validar" << endl;
    cout << "2) Cancelar" << endl;
    cin >> opcion;

    switch(opcion) {
        case 1:
            palindromo(str);
            break;
        case 2:
            return;
        default:
            cout << "Opción no válida." << endl;
            break;
    }
}


int main() {
    interfaz_palindromo();
    return 0;
}