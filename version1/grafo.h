#ifndef GRAFO_H
#define GRAFO_H

#include <algorithm>
#include <fstream>
#include <functional>
#include <iostream>
#include <limits>
#include <map>
#include <queue>
#include <sstream>
#include <string>
#include <vector>

using namespace std;

struct Arista {
    int destino;
    double distanciaMetros;
    double velocidadKmh;
    double tiempoSegundos;
    bool esOneWay;
    string tipoVia;
    long long idOsm;
};

struct ResultadoDijkstra {
    map<int, double> distancias;
    map<int, int> anteriores;
};

class Grafo {
private:
    map<int, vector<Arista>> adyacencia;
    map<int, vector<int>> conectividad;
    map<int, bool> visitados;
    int cantidadAristasOriginales;

    void limpiarEstructuras();
    void reiniciarVisitados();
    void registrarNodo(int nodo);
    void leerNodosCSV(const string& nombreArchivo);
    void cargarAristasCSV(const string& nombreArchivo);
    vector<vector<int>> obtenerComponentesDebiles();
    const vector<Arista>& getVecinos(int nodo);

public:
    Grafo();

    void agregarArista(
        int origen,
        int destino,
        double distanciaMetros,
        bool esOneWay,
        double velocidadKmh,
        double tiempoSegundos,
        const string& tipoVia,
        long long idOsm
    );

    void leerCSVs(const string& archivoNodos, const string& archivoAristas);
    vector<int> encontrarIslasViales();
    vector<int> obtenerComponenteGigante();
    bool existeNodo(int nodo) ;
    int cantidadNodos() ;
    int cantidadAristas() ;
    const map<int, vector<Arista>>& getAdyacencia() ;
    ResultadoDijkstra dijkstraDistanciasDesde(int origen);
};

#endif
