#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>

#include <stdio.h>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
#endif

using namespace std;

// Funcion multiplicar matrices

int matrizNxN(string M_path, char separador){
    //cout << "Verificando matriz en: " << M_path << endl;
    ifstream M(M_path);
    //cout << "Abriendo archivo..." << endl;
    if(!M.is_open()){
        cout << "Error al abrir\n" << M_path<< endl;
        return -1; // Error al abrir archivo
    }
    //cout << "Archivo abierto correctamente." << endl;
    string linea;
    int numLineas = 0;
    unsigned int ancho = 0;

    //cout << "Leyendo lineas..." << endl;
    while(getline(M, linea)){
        numLineas++;
        //cout << "Linea " << numLineas << ": " << linea << endl;

        // Contar números en la línea usando el separador especificado
        unsigned int cantidadNumeros = 0;
        stringstream ss(linea);
        string token;
        
        // Si el separador es espacio, usa metodo que cuenta tokens distintos de espacio
        if(separador == ' ') {
            while(ss >> token) {
                cantidadNumeros++;
            }
        } else {
            // Para cualquier otro separador, separa la cadena por los caracteres
            while(getline(ss, token, separador)) {
                token.erase(0, token.find_first_not_of(" \t\n\r"));
                token.erase(token.find_last_not_of(" \t\n\r") + 1);
                if(!token.empty()) {
                    cantidadNumeros++;
                }
            }
        }
        
        if(ancho == 0) {
            ancho = cantidadNumeros; // Establecer ancho con la primera línea
        } else if(cantidadNumeros != ancho) {
            //cout << "Error: Línea " << numLineas << " tiene " << cantidadNumeros << " números, pero se esperaba " << ancho << "." << endl;
            //cout << "Error: Líneas con diferente cantidad de números." << endl;
            return -1; // Líneas de diferente longitud
        }
    }
    
    M.close();
    //cout << M_path << endl;
    //cout << numLineas << " " << (int)ancho << endl;
    // Verificar si es cuadrada (n×n)
    if(numLineas == (int)ancho) {
        return numLineas; // Retorna n (dimensión de la matriz)
    }
    
    return -1; // No es cuadrada
}

int convert_matriz(string path, vector<vector<int>> &matriz, char separador){
    ifstream archivo(path);

    if(!archivo.is_open()) {
        cerr << "Error al abrir archivo: " << path << "\n" << endl;
        return -1;
    }

    string linea;
    while(getline(archivo, linea)){
        vector<int> fila;
        
        // Si el separador es espacio, usa metodo que cuenta tokens distintos de espacio
        if(separador == ' ') {
            stringstream ss(linea);
            int num;
            while(ss >> num){
                fila.push_back(num);
            }
        } else {
            // Para cualquier otro separador, separa la cadena por los caracteres
            stringstream ss(linea);
            string token;
            while(getline(ss, token, separador)) {
                token.erase(0, token.find_first_not_of(" \t\n\r"));
                token.erase(token.find_last_not_of(" \t\n\r") + 1);
                if(!token.empty()) {
                    try {
                        int num = stoi(token);
                        fila.push_back(num);
                    } catch(const exception& e) {
                        cerr << "Error: No se pudo convertir '" << token << "' a número\n" << endl;
                    }
                }
            }
        }
        
        if(!fila.empty())
            matriz.push_back(fila);
            
    }
    archivo.close();
    return 1;
}

void multi_matrices(string matriz1_path, string matriz2_path, char separador, int n){
    ifstream M1(matriz1_path);
    ifstream M2(matriz2_path);

    /*if(!M1.is_open()) {
        cerr << "Error al abrir archivo: " << matriz1_path << endl;
        return;
    }
    if(!M2.is_open()) {
        cerr << "Error al abrir archivo: " << matriz2_path << endl;
        return;
    }*/

    

    cout << "\nMultiplicando matrices..." << endl;
    #ifdef _WIN32
        printf("[PID: %d]\n", GetCurrentProcessId());
    #else
        printf("[PID: %d]\n", getpid());
    #endif

    /*if(matrizNxN(matriz1_path, separador) == -1 || matrizNxN(matriz2_path, separador) == -1){
        cout << "Error al multiplicar las matrices\n";
        return;
    }

    int n = matrizNxN(matriz1_path, separador);*/

    vector<vector<int>> matriz1;
    vector<vector<int>> matriz2;
    vector<vector<int>> result;

    convert_matriz(matriz1_path, matriz1, separador);
    convert_matriz(matriz2_path, matriz2, separador);

    // Inicializar matriz resultado
    result.resize(n, vector<int>(n, 0));

    // multiplicacion optimizada pipeline
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            int sum[2] = {0}; 
            int l = 0;
            
            // Procesar de 2 en 2 para múltiplos de 2
            for (; l <= n-2; l += 2) {
                sum[0] += matriz1[i][l]   * matriz2[l][j];
                sum[1] += matriz1[i][l+1] * matriz2[l+1][j];
            }
            
            // Manejo del elemento restante si n es impar
            int extra = 0;
            if (l < n) {
                extra += matriz1[i][l] * matriz2[l][j];
            }
            
            result[i][j] = sum[0] + sum[1] + extra;
        }
    }

    cout << "\n=== Resultado ===" << endl;
    for (const auto& fila : result) {
        for (int elemento : fila) {
            cout << elemento << " ";
        }
        cout << endl;
    }

    cout << endl;
}


int main(int argc, char* argv[]){

    // Verificar que se pasaron los argumentos correctos
    if (argc != 5) {
        cout << "Uso correcto: " << argv[0] << " <ruta_matriz1> <ruta_matriz2> <caracter> <n>" << endl;
        return 1;
    }

    // Obtener los argumentos
    string matriz1_path = argv[1];
    string matriz2_path = argv[2];
    char caracter = argv[3][0]; // Tomar el primer caracter del tercer argumento
    int n = stoi(argv[4]); // Convertir el cuarto argumento a entero

    /*
    cout << "Matriz 1: " << matriz1_path << endl;
    cout << "Matriz 2: " << matriz2_path << endl;
    cout << "Caracter separador: " << caracter << endl;
    cout << "----------------------------------------" << endl;
    */
    
    multi_matrices(matriz1_path, matriz2_path, caracter, n);
    return 0;
}