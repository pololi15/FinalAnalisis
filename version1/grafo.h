#ifndef GRAFO_H
#define GRAFO_H

#include <algorithm>
#include <chrono>
#include <fstream>
#include <functional>
#include <iostream>
#include <limits>
#include <map>
#include <queue>
#include <sstream>
#include <string>
#include <unordered_map>
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

struct ResultadoRuta {
    vector<int> ruta;
    double distanciaTotal;
    double tiempoTotal;
    bool existe;
};

struct ResultadoAlcance {
    int nodoOrigen;
    double radioMetros;
    unordered_map<int, double> distancias;
    int nodosAlcanzables;
    double distanciaMaxima;
    double distanciaPromedio;
    double tiempoMs;
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
    ResultadoRuta dijkstraGeneral(int origen, int destino, bool porTiempo);

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
    ResultadoRuta rutaMasCortaPorDistancia(int origen, int destino);
    ResultadoRuta rutaMasRapidaPorTiempo(int origen, int destino);
    ResultadoAlcance alcanceVehicular(int origen, double radioMetros = 5000.0);
};

#endif
