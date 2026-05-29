#include "grafo.h"

namespace {
    const vector<Arista> vecinosVacios;
}

Grafo::Grafo() {
    cantidadAristasOriginales = 0;
}

void Grafo::reiniciarVisitados() {
    visitados.clear();

    for (const auto& nodoConAristas : conectividad) {
        visitados[nodoConAristas.first] = false;
    }
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

void Grafo::leerCSV(const string& nombreArchivo) {
    ifstream archivo(nombreArchivo);

    if (!archivo.is_open()) {
        cerr << "Error: No se pudo abrir el archivo " << nombreArchivo << endl;
        return;
    }

    adyacencia.clear();
    conectividad.clear();
    visitados.clear();
    cantidadAristasOriginales = 0;

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

    reiniciarVisitados();
    cout << "  [CSV] Total de aristas cargadas: " << cantidadAristasOriginales << endl;
}

int Grafo::bfs(int inicio) {
    queue<int> cola;
    visitados[inicio] = true;
    cola.push(inicio);

    int tamanoIsla = 0;

    while (!cola.empty()) {
        int nodoActual = cola.front();
        cola.pop();
        tamanoIsla++;

        for (int vecino : conectividad[nodoActual]) {
            if (!visitados[vecino]) {
                visitados[vecino] = true;
                cola.push(vecino);
            }
        }
    }

    return tamanoIsla;
}

vector<int> Grafo::encontrarIslasViales() {
    vector<int> tamanosIslas;
    reiniciarVisitados();

    for (const auto& nodoConVecinos : conectividad) {
        int nodo = nodoConVecinos.first;

        if (!visitados[nodo]) {
            int tamanoIsla = bfs(nodo);
            tamanosIslas.push_back(tamanoIsla);
        }
    }

    return tamanosIslas;
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

vector<int> Grafo::obtenerNodos()  {
    vector<int> nodos;

    for (const auto& nodoConAristas : adyacencia) {
        nodos.push_back(nodoConAristas.first);
    }

    return nodos;
}

const vector<Arista>& Grafo::getVecinos(int nodo)  {
    auto iterador = adyacencia.find(nodo);

    if (iterador == adyacencia.end()) {
        return vecinosVacios;
    }

    return iterador->second;
}

const map<int, vector<Arista>>& Grafo::getAdyacencia()  {
    return adyacencia;
}

ResultadoRuta Grafo::dijkstraGeneral(int origen, int destino, bool usarTiempo)  {
    ResultadoRuta resultado;
    resultado.costoTotal = numeric_limits<double>::infinity();
    resultado.existeRuta = false;

    if (!existeNodo(origen) || !existeNodo(destino)) {
        return resultado;
    }

    map<int, double> mejorCosto;
    map<int, int> anterior;
    priority_queue<pair<double, int>, vector<pair<double, int>>, greater<pair<double, int>>> colaPrioridad;

    for (const auto& nodoConAristas : adyacencia) {
        mejorCosto[nodoConAristas.first] = numeric_limits<double>::infinity();
    }

    mejorCosto[origen] = 0.0;
    colaPrioridad.push({0.0, origen});

    while (!colaPrioridad.empty()) {
        double costoActual = colaPrioridad.top().first;
        int nodoActual = colaPrioridad.top().second;
        colaPrioridad.pop();

        if (costoActual > mejorCosto[nodoActual]) {
            continue;
        }

        if (nodoActual == destino) {
            break;
        }

        for (const Arista& arista : getVecinos(nodoActual)) {
            double peso = usarTiempo ? arista.tiempoSegundos : arista.distanciaMetros;
            double nuevoCosto = costoActual + peso;

            if (nuevoCosto < mejorCosto[arista.destino]) {
                mejorCosto[arista.destino] = nuevoCosto;
                anterior[arista.destino] = nodoActual;
                colaPrioridad.push({nuevoCosto, arista.destino});
            }
        }
    }

    if (mejorCosto[destino] == numeric_limits<double>::infinity()) {
        return resultado;
    }

    resultado.costoTotal = mejorCosto[destino];
    resultado.existeRuta = true;

    int nodoActual = destino;
    resultado.camino.push_back(nodoActual);

    while (nodoActual != origen) {
        nodoActual = anterior[nodoActual];
        resultado.camino.push_back(nodoActual);
    }

    reverse(resultado.camino.begin(), resultado.camino.end());
    return resultado;
}

ResultadoRuta Grafo::rutaMasCortaPorDistancia(int origen, int destino)  {
    return dijkstraGeneral(origen, destino, false);
}

ResultadoRuta Grafo::rutaMasRapidaPorTiempo(int origen, int destino)  {
    return dijkstraGeneral(origen, destino, true);
}
