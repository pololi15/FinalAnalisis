#include "grafo.h"

#include <algorithm>
#include <iostream>
#include <map>
#include <vector>

using namespace std;

struct AristaMST {
    int origen;
    int destino;
    double peso;
};

class UnionFind {
private:
    map<int, int> padre;
    map<int, int> rango;

public:
    void agregar(int nodo) {
        padre[nodo] = nodo;
        rango[nodo] = 0;
    }

    int encontrar(int nodo) {
        if (padre[nodo] != nodo) {
            padre[nodo] = encontrar(padre[nodo]);
        }
        return padre[nodo];
    }

    bool unir(int a, int b) {
        int raizA = encontrar(a);
        int raizB = encontrar(b);

        if (raizA == raizB) {
            return false;
        }

        if (rango[raizA] < rango[raizB]) {
            padre[raizA] = raizB;
        } else if (rango[raizA] > rango[raizB]) {
            padre[raizB] = raizA;
        } else {
            padre[raizB] = raizA;
            rango[raizA]++;
        }

        return true;
    }
};

bool compararPorPeso(AristaMST a, AristaMST b) {
    return a.peso < b.peso;
}

int main() {
    string archivoNodos = "nodes_clean.csv";
    string archivoAristas = "edges_clean.csv";

    Grafo grafo;
    grafo.leerCSVs(archivoNodos, archivoAristas);

    vector<int> componenteGigante = grafo.obtenerComponenteGigante();

    if (componenteGigante.empty()) {
        cout << "No se encontro una componente gigante para construir el MST." << endl;
        return 0;
    }

    map<int, bool> estaEnGigante;
    for (int nodo : componenteGigante) {
        estaEnGigante[nodo] = true;
    }

    vector<AristaMST> aristasCandidatas;
    map<string, double> mejorPesoPorPar;

    const map<int, vector<Arista>>& adyacencia = grafo.getAdyacencia();

    for (auto parNodo : adyacencia) {
        int origen = parNodo.first;

        if (!estaEnGigante[origen]) {
            continue;
        }

        for (Arista arista : parNodo.second) {
            int destino = arista.destino;

            if (!estaEnGigante[destino]) {
                continue;
            }

            int menor = min(origen, destino);
            int mayor = max(origen, destino);
            string clave = to_string(menor) + "-" + to_string(mayor);

            if (mejorPesoPorPar.find(clave) == mejorPesoPorPar.end() ||
                arista.distanciaMetros < mejorPesoPorPar[clave]) {
                mejorPesoPorPar[clave] = arista.distanciaMetros;
            }
        }
    }

    for (auto parArista : mejorPesoPorPar) {
        string clave = parArista.first;
        size_t separador = clave.find('-');

        int origen = stoi(clave.substr(0, separador));
        int destino = stoi(clave.substr(separador + 1));
        double peso = parArista.second;

        AristaMST aristaNueva;
        aristaNueva.origen = origen;
        aristaNueva.destino = destino;
        aristaNueva.peso = peso;
        aristasCandidatas.push_back(aristaNueva);
    }

    sort(aristasCandidatas.begin(), aristasCandidatas.end(), compararPorPeso);

    UnionFind conjuntos;
    for (int nodo : componenteGigante) {
        conjuntos.agregar(nodo);
    }

    vector<AristaMST> mst;
    double distanciaTotalMetros = 0.0;

    for (AristaMST arista : aristasCandidatas) {
        if (conjuntos.unir(arista.origen, arista.destino)) {
            mst.push_back(arista);
            distanciaTotalMetros += arista.peso;

            if (mst.size() == componenteGigante.size() - 1) {
                break;
            }
        }
    }

    cout << "\n=== RED DE EMERGENCIA MINIMA (MST) ===" << endl;
    cout << "Se usa la componente gigante del grafo limpio." << endl;
    cout << "Para el MST se modela la red como no dirigida." << endl;
    cout << "Algoritmo usado: Kruskal." << endl;
    cout << "Nodos en la componente gigante: " << componenteGigante.size() << endl;
    cout << "Aristas candidatas no dirigidas: " << aristasCandidatas.size() << endl;
    cout << "Aristas elegidas en el MST: " << mst.size() << endl;
    cout << "Distancia total cubierta: " << distanciaTotalMetros << " m" << endl;
    cout << "Distancia total cubierta: " << distanciaTotalMetros / 1000.0 << " km" << endl;

    if (mst.size() != componenteGigante.size() - 1) {
        cout << "Advertencia: no se pudo completar un arbol de cobertura para toda la componente gigante." << endl;
    }

    return 0;
}
