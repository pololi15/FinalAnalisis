#include "grafo.h"

#include <iostream>
#include <map>

using namespace std;

int main() {
    string archivoNodos = "nodes_clean.csv";
    string archivoAristas = "edges_clean.csv";
    int nodoOrigen;
    double distanciaMaxima = 5000.0;

    Grafo grafo;
    grafo.leerCSVs(archivoNodos, archivoAristas);

    cout << "\n=== ALCANCE VEHICULAR ===" << endl;
    cout << "Ingrese el nodo origen: ";
    cin >> nodoOrigen;

    if (!grafo.existeNodo(nodoOrigen)) {
        cout << "El nodo origen no existe en el grafo cargado." << endl;
        return 0;
    }

    ResultadoDijkstra resultado = grafo.dijkstraDistanciasDesde(nodoOrigen);

    int cantidadAlcanzables = 0;

    for (auto parDistancia : resultado.distancias) {
        double distancia = parDistancia.second;

        if (distancia <= distanciaMaxima) {
            cantidadAlcanzables++;
        }
    }

    cout << "\nNodo origen: " << nodoOrigen << endl;
    cout << "Distancia maxima permitida: " << distanciaMaxima << " m" << endl;
    cout << "Nodos alcanzables por red vial: " << cantidadAlcanzables << endl;
    cout << "\nNota: se usa distancia de calle, no distancia euclidiana." << endl;

    return 0;
}
