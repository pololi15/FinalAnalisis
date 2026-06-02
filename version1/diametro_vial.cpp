#include "grafo.h"

#include <chrono>
#include <iostream>
#include <limits>
#include <vector>

using namespace std;

int encontrarNodoMasLejano(
    vector<int>& nodosComponente,
    map<int, double>& distancias,
    double& distanciaMaxima
) {
    int nodoMasLejano = -1;
    distanciaMaxima = -1.0;

    for (int nodo : nodosComponente) {
        auto it = distancias.find(nodo);
        if (it == distancias.end()) {
            continue;
        }

        double distancia = it->second;
        if (distancia == numeric_limits<double>::infinity()) {
            continue;
        }

        if (distancia > distanciaMaxima) {
            distanciaMaxima = distancia;
            nodoMasLejano = nodo;
        }
    }

    return nodoMasLejano;
}

int main() {
    string archivoNodos = "nodes_clean.csv";
    string archivoAristas = "edges_clean.csv";

    Grafo grafo;
    grafo.leerCSVs(archivoNodos, archivoAristas);

    vector<int> componenteGigante = grafo.obtenerComponenteGigante();

    if (componenteGigante.empty()) {
        cout << "No se encontro una componente gigante para analizar." << endl;
        
    }

    cout << "\n=== DIAMETRO VIAL APROXIMADO ===" << endl;
    cout << "Se usan nodes_clean.csv y edges_clean.csv." << endl;
    cout << "La componente gigante se obtiene con BFS sobre conectividad debil." << endl;
    cout << "Las distancias minimas se calculan con Dijkstra usando distance_m." << endl;
    cout << "Se aplica la tecnica de doble Dijkstra para aproximar el diametro." << endl;
    cout << "Nodos en la componente gigante: " << componenteGigante.size() << endl;
    cout << "Iniciando evaluacion aproximada..." << endl;

    auto tiempoInicio = chrono::steady_clock::now();

    int nodoSemilla = componenteGigante.front();
    cout << "Nodo semilla inicial: " << nodoSemilla << endl;

    ResultadoDijkstra primerDijkstra = grafo.dijkstraDistanciasDesde(nodoSemilla);
    double distanciaHastaA = -1.0;
    int nodoA = encontrarNodoMasLejano(
        componenteGigante,
        primerDijkstra.distancias,
        distanciaHastaA
    );

    cout << "Extremo provisional A encontrado: " << nodoA
         << " | distancia desde semilla: "
         << distanciaHastaA << " m" << endl;

    ResultadoDijkstra segundoDijkstra = grafo.dijkstraDistanciasDesde(nodoA);
    double diametroAproximado = -1.0;
    int nodoB = encontrarNodoMasLejano(
        componenteGigante,
        segundoDijkstra.distancias,
        diametroAproximado
    );

    auto tiempoFin = chrono::steady_clock::now();
    double tiempoTotalSegundos = chrono::duration<double>(tiempoFin - tiempoInicio).count();

    cout << "\n=== RESULTADO FINAL ===" << endl;
    cout << "Nodo extremo A: " << nodoA << endl;
    cout << "Nodo extremo B: " << nodoB << endl;
    cout << "Diametro vial aproximado: " << diametroAproximado << " m" << endl;
    cout << "Diametro vial aproximado: " << diametroAproximado / 1000.0 << " km" << endl;
    cout << "Tiempo total de ejecucion: " << tiempoTotalSegundos << " s" << endl;
    cout << "\nNota: el calculo respeta el sentido de las aristas del grafo cargado." << endl;
    cout << "Nota: este resultado es una aproximacion eficiente, no el diametro exacto." << endl;

    return 0;
}
