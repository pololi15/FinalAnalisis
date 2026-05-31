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

struct ResultadoRuta {
    vector<int> camino;
    double costoTotal;
    bool existeRuta;
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

    void reiniciarVisitados();
    ResultadoRuta dijkstraGeneral(int origen, int destino, bool usarTiempo) ;

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

    void leerCSV(const string& nombreArchivo);
    int bfs(int inicio);
    vector<int> encontrarIslasViales();
    bool existeNodo(int nodo) ;
    int cantidadNodos() ;
    int cantidadAristas() ;
    vector<int> obtenerNodos() ;
    const vector<Arista>& getVecinos(int nodo) ;
    const map<int, vector<Arista>>& getAdyacencia() ;
    ResultadoRuta rutaMasCortaPorDistancia(int origen, int destino) ;
    ResultadoRuta rutaMasRapidaPorTiempo(int origen, int destino) ;
    ResultadoAlcance alcanceVehicular(int origen, double radioMetros = 5000.0);
};

#endif
