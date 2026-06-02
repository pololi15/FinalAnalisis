#include "grafo.h"

#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

int main() {
    string archivoNodos = "nodes_clean.csv";
    string archivoAristas = "edges_clean.csv";


    Grafo grafo;
    grafo.leerCSVs(archivoNodos, archivoAristas);

    vector<int> tamanosIslas = grafo.encontrarIslasViales();
    sort(tamanosIslas.begin(), tamanosIslas.end(), greater<int>());

    cout << "\n=== ANALISIS DE ISLAS VIALES ===" << endl;
    cout << "Nodos del grafo: " << grafo.cantidadNodos() << endl;
    cout << "Aristas originales cargadas: " << grafo.cantidadAristas() << endl;
    cout << "Numero total de islas: " << tamanosIslas.size() << endl;

    if (!tamanosIslas.empty()) {
        cout << "Tamano de la componente gigante: " << tamanosIslas.front() << endl;
    } else {
        cout << "Tamano de la componente gigante: 0" << endl;
    }

    cout << "\nTop 10 tamanos de islas:" << endl;
    for (size_t i = 0; i < tamanosIslas.size() && i < 10; ++i) {
        cout << "  Isla " << (i + 1) << ": " << tamanosIslas[i] << " nodos" << endl;
    }

    return 0;
}
