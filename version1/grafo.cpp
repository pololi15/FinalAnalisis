#include "grafo.h"

#include <chrono>
#include <unordered_map>

namespace {
    const vector<Arista> vecinosVacios;
}

Grafo::Grafo() {
    cantidadAristasOriginales = 0;
}

void Grafo::limpiarEstructuras() {
    adyacencia.clear();
    conectividad.clear();
    visitados.clear();
    cantidadAristasOriginales = 0;
}

void Grafo::reiniciarVisitados() {
    visitados.clear();

    for (const auto& nodoConAristas : conectividad) {
        visitados[nodoConAristas.first] = false;
    }
}

void Grafo::registrarNodo(int nodo) {
    adyacencia[nodo];
    conectividad[nodo];
    visitados[nodo] = false;
}

void Grafo::agregarArista(
    int origen,
    int destino,
    double distanciaMetros,
    bool esOneWay,
    double velocidadKmh,
    double tiempoSegundos,
    const string& tipoVia,
    long long idOsm
) {
    registrarNodo(origen);
    registrarNodo(destino);

    Arista arista;
    arista.destino = destino;
    arista.distanciaMetros = distanciaMetros;
    arista.velocidadKmh = velocidadKmh;
    arista.tiempoSegundos = tiempoSegundos;
    arista.esOneWay = esOneWay;
    arista.tipoVia = tipoVia;
    arista.idOsm = idOsm;

    adyacencia[origen].push_back(arista);
    conectividad[origen].push_back(destino);
    conectividad[destino].push_back(origen);

    if (!esOneWay) {
        Arista aristaRegreso = arista;
        aristaRegreso.destino = origen;
        adyacencia[destino].push_back(aristaRegreso);
    } else {
        adyacencia[destino];
    }
}

void Grafo::leerNodosCSV(const string& nombreArchivo) {
    ifstream archivo(nombreArchivo);

    if (!archivo.is_open()) {
        cerr << "Error: No se pudo abrir el archivo " << nombreArchivo << endl;
        return;
    }

    string linea;
    getline(archivo, linea);

    while (getline(archivo, linea)) {
        stringstream separador(linea);
        vector<string> columnas;
        string valorTexto;

        while (getline(separador, valorTexto, ',')) {
            columnas.push_back(valorTexto);
        }

        if (columnas.size() < 1) {
            continue;
        }

        int nodeId = stoi(columnas[0]);
        registrarNodo(nodeId);
    }
}

void Grafo::cargarAristasCSV(const string& nombreArchivo) {
    ifstream archivo(nombreArchivo);

    if (!archivo.is_open()) {
        cerr << "Error: No se pudo abrir el archivo " << nombreArchivo << endl;
        return;
    }

    string linea;
    getline(archivo, linea);

    while (getline(archivo, linea)) {
        stringstream separador(linea);
        string valorTexto;
        vector<string> columnas;

        while (getline(separador, valorTexto, ',')) {
            columnas.push_back(valorTexto);
        }

        if (columnas.size() < 11) {
            continue;
        }

        long long idOsm = stoll(columnas[0]);
        int origen = stoi(columnas[1]);
        int destino = stoi(columnas[2]);
        double distanciaMetros = stod(columnas[3]);
        string tipoVia = columnas[4];
        bool esOneWay = columnas[5] == "1";
        double velocidadKmh = stod(columnas[9]);
        double tiempoSegundos = stod(columnas[10]);

        agregarArista(
            origen,
            destino,
            distanciaMetros,
            esOneWay,
            velocidadKmh,
            tiempoSegundos,
            tipoVia,
            idOsm
        );

        cantidadAristasOriginales++;

        if (cantidadAristasOriginales % 100000 == 0) {
            cout << "  [CSV] Procesadas " << cantidadAristasOriginales << " aristas..." << endl;
        }
    }
}

void Grafo::leerCSVs(const string& archivoNodos, const string& archivoAristas) {
    limpiarEstructuras();
    leerNodosCSV(archivoNodos);
    cargarAristasCSV(archivoAristas);

    reiniciarVisitados();
    cout << "  [CSV] Total de nodos cargados: " << cantidadNodos() << endl;
    cout << "  [CSV] Total de aristas cargadas: " << cantidadAristasOriginales << endl;
}

vector<vector<int>> Grafo::obtenerComponentesDebiles() {
    vector<vector<int>> componentes;
    reiniciarVisitados();

    for (const auto& nodoConVecinos : conectividad) {
        int nodoInicio = nodoConVecinos.first;

        if (visitados[nodoInicio]) {
            continue;
        }

        vector<int> componenteActual;
        queue<int> cola;
        visitados[nodoInicio] = true;
        cola.push(nodoInicio);

        while (!cola.empty()) {
            int nodoActual = cola.front();
            cola.pop();
            componenteActual.push_back(nodoActual);

            for (int vecino : conectividad[nodoActual]) {
                if (!visitados[vecino]) {
                    visitados[vecino] = true;
                    cola.push(vecino);
                }
            }
        }

        componentes.push_back(componenteActual);
    }

    return componentes;
}

vector<int> Grafo::encontrarIslasViales() {
    vector<int> tamanosIslas;
    vector<vector<int>> componentes = obtenerComponentesDebiles();

    for (const vector<int>& componente : componentes) {
        tamanosIslas.push_back(componente.size());
    }

    return tamanosIslas;
}

vector<int> Grafo::obtenerComponenteGigante() {
    vector<vector<int>> componentes = obtenerComponentesDebiles();
    vector<int> componenteGigante;

    for (const vector<int>& componente : componentes) {
        if (componente.size() > componenteGigante.size()) {
            componenteGigante = componente;
        }
    }

    return componenteGigante;
}

bool Grafo::existeNodo(int nodo)  {
    return adyacencia.find(nodo) != adyacencia.end();
}

int Grafo::cantidadNodos()  {
    return static_cast<int>(adyacencia.size());
}

int Grafo::cantidadAristas()  {
    return cantidadAristasOriginales;
}

const vector<Arista>& Grafo::getVecinos(int nodo) {
    auto iterador = adyacencia.find(nodo);

    if (iterador == adyacencia.end()) {
        return vecinosVacios;
    }

    return iterador->second;
}

const map<int, vector<Arista>>& Grafo::getAdyacencia()  {
    return adyacencia;
}

ResultadoDijkstra Grafo::dijkstraDistanciasDesde(int origen) {
    ResultadoDijkstra resultado;

    if (!existeNodo(origen)) {
        return resultado;
    }

    priority_queue<pair<double, int>, vector<pair<double, int>>, greater<pair<double, int>>> colaPrioridad;

    for (const auto& nodoConAristas : adyacencia) {
        resultado.distancias[nodoConAristas.first] = numeric_limits<double>::infinity();
    }

    resultado.distancias[origen] = 0.0;
    colaPrioridad.push({0.0, origen});

    while (!colaPrioridad.empty()) {
        double costoActual = colaPrioridad.top().first;
        int nodoActual = colaPrioridad.top().second;
        colaPrioridad.pop();

        if (costoActual > resultado.distancias[nodoActual]) {
            continue;
        }

        for (const Arista& arista : getVecinos(nodoActual)) {
            double nuevoCosto = costoActual + arista.distanciaMetros;

            if (nuevoCosto < resultado.distancias[arista.destino]) {
                resultado.distancias[arista.destino] = nuevoCosto;
                resultado.anteriores[arista.destino] = nodoActual;
                colaPrioridad.push({nuevoCosto, arista.destino});
            }
        }
    }

    return resultado;
}

ResultadoRuta Grafo::dijkstraGeneral(int origen, int destino, bool porTiempo) {
    ResultadoRuta resultado;
    resultado.existe = false;

    if (!existeNodo(origen) || !existeNodo(destino)) {
        return resultado;
    }

    unordered_map<int, double> distancias;
    unordered_map<int, int> anteriores;

    for (const auto& nodoConAristas : adyacencia) {
        distancias[nodoConAristas.first] = numeric_limits<double>::infinity();
    }

    distancias[origen] = 0.0;

    priority_queue<pair<double, int>, vector<pair<double, int>>, greater<pair<double, int>>> cola;
    cola.push({0.0, origen});

    while (!cola.empty()) {
        double costoActual = cola.top().first;
        int nodoActual = cola.top().second;
        cola.pop();

        auto it = distancias.find(nodoActual);
        if (it == distancias.end() || costoActual > it->second) {
            continue;
        }

        if (nodoActual == destino) {
            break;
        }

        for (const Arista& arista : getVecinos(nodoActual)) {
            double peso = porTiempo ? arista.tiempoSegundos : arista.distanciaMetros;
            double nuevoCosto = costoActual + peso;

            auto itVec = distancias.find(arista.destino);
            if (itVec == distancias.end() || nuevoCosto < itVec->second) {
                distancias[arista.destino] = nuevoCosto;
                anteriores[arista.destino] = nodoActual;
                cola.push({nuevoCosto, arista.destino});
            }
        }
    }

    auto itDestino = distancias.find(destino);
    if (itDestino == distancias.end() || itDestino->second == numeric_limits<double>::infinity()) {
        return resultado;
    }

    // Reconstruir ruta
    vector<int> ruta;
    int actual = destino;
    ruta.push_back(actual);
    while (actual != origen) {
        auto itAnt = anteriores.find(actual);
        if (itAnt == anteriores.end()) {
            // No hay camino
            resultado.existe = false;
            return resultado;
        }
        actual = itAnt->second;
        ruta.push_back(actual);
    }

    reverse(ruta.begin(), ruta.end());

    // Calcular distancia total y tiempo total recorriendo la ruta
    double distanciaTotal = 0.0;
    double tiempoTotal = 0.0;

    for (size_t i = 0; i + 1 < ruta.size(); ++i) {
        int u = ruta[i];
        int v = ruta[i + 1];
        bool encontrado = false;

        for (const Arista& ar : getVecinos(u)) {
            if (ar.destino == v) {
                distanciaTotal += ar.distanciaMetros;
                tiempoTotal += ar.tiempoSegundos;
                encontrado = true;
                break;
            }
        }

        if (!encontrado) {
            // Si no encontramos la arista, seguiremos con lo que tengamos
            continue;
        }
    }

    resultado.ruta = std::move(ruta);
    resultado.distanciaTotal = distanciaTotal;
    resultado.tiempoTotal = tiempoTotal;
    resultado.existe = true;

    return resultado;
}
ResultadoRuta Grafo::rutaMasCortaPorDistancia(int origen, int destino)  {
    return dijkstraGeneral(origen, destino, false);
}

ResultadoRuta Grafo::rutaMasRapidaPorTiempo(int origen, int destino)  {
    return dijkstraGeneral(origen, destino, true);
}

ResultadoAlcance Grafo::alcanceVehicular(int origen, double radioMetros) {
    ResultadoAlcance resultado;
    resultado.nodoOrigen = origen;
    resultado.radioMetros = radioMetros;
    resultado.nodosAlcanzables = 0;
    resultado.distanciaMaxima = 0.0;
    resultado.distanciaPromedio = 0.0;
    resultado.tiempoMs = 0.0;

    auto inicio = chrono::high_resolution_clock::now();

    if (!existeNodo(origen) || radioMetros < 0.0) {
        auto fin = chrono::high_resolution_clock::now();
        resultado.tiempoMs = chrono::duration<double, std::milli>(fin - inicio).count();
        return resultado;
    }

    unordered_map<int, double> distancias;
    distancias.reserve(adyacencia.size());

    priority_queue<pair<double, int>, vector<pair<double, int>>, greater<pair<double, int>>> colaPrioridad;

    distancias[origen] = 0.0;
    colaPrioridad.push({0.0, origen});

    while (!colaPrioridad.empty()) {
        double costoActual = colaPrioridad.top().first;
        int nodoActual = colaPrioridad.top().second;
        colaPrioridad.pop();

        auto itCosto = distancias.find(nodoActual);
        if (itCosto == distancias.end() || costoActual > itCosto->second) {
            continue;
        }

        for (const Arista& arista : getVecinos(nodoActual)) {
            double nuevoCosto = costoActual + arista.distanciaMetros;

            if (nuevoCosto > radioMetros) {
                continue;
            }

            auto itVecino = distancias.find(arista.destino);
            if (itVecino == distancias.end() || nuevoCosto < itVecino->second) {
                distancias[arista.destino] = nuevoCosto;
                colaPrioridad.push({nuevoCosto, arista.destino});
            }
        }
    }

    double sumaDistancias = 0.0;
    double distanciaMaxima = 0.0;

    for (const auto& par : distancias) {
        sumaDistancias += par.second;
        if (par.second > distanciaMaxima) {
            distanciaMaxima = par.second;
        }
    }

    auto fin = chrono::high_resolution_clock::now();

    resultado.distancias = std::move(distancias);
    resultado.nodosAlcanzables = static_cast<int>(resultado.distancias.size());
    resultado.distanciaMaxima = distanciaMaxima;
    resultado.distanciaPromedio = resultado.nodosAlcanzables > 0
        ? sumaDistancias / static_cast<double>(resultado.nodosAlcanzables)
        : 0.0;
    resultado.tiempoMs = chrono::duration<double, std::milli>(fin - inicio).count();

    return resultado;
}
