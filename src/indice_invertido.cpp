#include <iostream>
#include <stdio.h>
#include <fstream>
#include <string>
#include <filesystem>
#include <map>
#include <unordered_map>
#include <algorithm>
#include <regex>
#include <vector>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
#endif

using namespace std;

// Función para crear un archivo vacío
int crear_archivo(const string& nombre_indice) {
    ofstream indice(nombre_indice);
    if (!indice.is_open()) {
        cerr << "Error al crear el archivo: " << nombre_indice << endl;
        return -1; // Error al crear archivo
    }
    indice.close();
    return 0; // Archivo creado exitosamente
}

// Función para procesar un archivo y actualizar el índice invertido
void procesar_archivo(const string& archivo, unordered_map<string, map<string, int>>& indice_invertido) {
    
    ifstream inFile(archivo);
    if (!inFile.is_open()) {
        cerr << "Error al abrir archivo: " << archivo << endl;
        return;
    }
    string linea;
    regex caracteres_validos("[A-Za-zÝÉÝÓÚáéíóúÑñÜü]+", regex_constants::ECMAScript);

    while (getline(inFile, linea)) {
        for (sregex_iterator it(linea.begin(), linea.end(), caracteres_validos), end_it; it != end_it; ++it) {
            string palabra = it->str();
            transform(palabra.begin(), palabra.end(), palabra.begin(), ::tolower);
            // Filtrar palabras que solo contengan letras válidas
            const string letras_validas = "abcdefghijklmnopqrstuvwxyzáéíóúñü";
            bool solo_letras = true;
            for (char c : palabra) {
                if (letras_validas.find(c) == string::npos) {
                    solo_letras = false;
                    break;
                }
            }
            if (solo_letras && !palabra.empty()) {
                indice_invertido[palabra][archivo]++;
            }
        }
    }
}

// Función para escribir el índice invertido en un archivo
void escribir_indice(const string& nombre_indice, const unordered_map<string, map<string, int>>& indice_invertido) {
    ofstream outFile(nombre_indice, ios::app);
    if (!outFile.is_open()) {
        cerr << "Error al abrir el archivo para escribir: " << nombre_indice << endl;
        return;
    }

    for (const auto& [palabra, archivos] : indice_invertido) {
        outFile << palabra;

        // Copiar y ordenar los archivos por cantidad descendente
        vector<pair<string, int>> archivos_vec(archivos.begin(), archivos.end());
        sort(archivos_vec.begin(), archivos_vec.end(),
            [](const auto& a, const auto& b) { return a.second > b.second; });

        for (const auto& [archivo, count] : archivos_vec) {
            filesystem::path p(archivo);
            outFile << ";(" << p.filename().string() << "," << count << ")";
        }
        outFile << "\n";
    }
    outFile.close();
}

// Función principal para crear el índice invertido
void crear_indice(const string& nombre_indice, const string& path_carpeta) {
    unordered_map<string, map<string, int>> indice_invertido;

    namespace fs = filesystem;
    for (const auto& entry : fs::directory_iterator(path_carpeta)) {
        if (entry.is_regular_file()) {
            procesar_archivo(entry.path().string(), indice_invertido);
        }
    }

    escribir_indice(nombre_indice, indice_invertido);
}

// Función principal
int main(int argc, char* argv[]) {
    if (argc != 3) {
        cout << "Uso correcto: " << argv[0] << " <nombre_indice.idx> <ruta_carpetas>" << endl;
        return 1;
    }

    string nombre_indice = "data/" + string(argv[1]);
    string path_carpeta = argv[2];

    cout << "\nCreando índice invertido..." << endl;
    #ifdef _WIN32
        printf("[PID: %d]\n", GetCurrentProcessId());
    #else
        printf("[PID: %d]\n", getpid());
    #endif

    // Crear el archivo del índice
    if (crear_archivo(nombre_indice) != 0) {
        return 1; // Error al crear el archivo
    }

    // Crear el índice invertido
    crear_indice(nombre_indice, path_carpeta);

    cout << "\n=== Índice invertido creado exitosamente en: " << nombre_indice << " ===\n" << endl;
    return 0;
}