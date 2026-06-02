#include "grafo.h"

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <utility>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        cerr << "Uso: " << argv[0] << " <nodo_origen> [radio_metros]" << endl;
        return 1;
    }

    int nodoOrigen = stoi(argv[1]);
    double radioMetros = (argc >= 3) ? stod(argv[2]) : 5000.0;

    Grafo grafo;
    grafo.leerCSV("edges_clean.csv");

    ResultadoAlcance resultado = grafo.alcanceVehicular(nodoOrigen, radioMetros);

    cout << fixed << setprecision(3);
    cout << "Nodo origen: " << resultado.nodoOrigen << endl;
    cout << "Radio (m): " << resultado.radioMetros << endl;
    cout << "Nodos alcanzables: " << resultado.nodosAlcanzables << endl;
    cout << "Distancia maxima (m): " << resultado.distanciaMaxima << endl;
    cout << "Distancia promedio (m): " << resultado.distanciaPromedio << endl;
    cout << "Tiempo de ejecucion (ms): " << resultado.tiempoMs << endl;

    vector<pair<int, double>> distanciasOrdenadas;
    distanciasOrdenadas.reserve(resultado.distancias.size());

    for (const auto& par : resultado.distancias) {
        distanciasOrdenadas.push_back(par);
    }

    sort(distanciasOrdenadas.begin(), distanciasOrdenadas.end(), [](const auto& lhs, const auto& rhs) {
        if (lhs.second != rhs.second) {
            return lhs.second < rhs.second;
        }

        return lhs.first < rhs.first;
    });

    string nombreArchivo = "alcance_nodo_" + to_string(nodoOrigen) + ".csv";
    ofstream salida(nombreArchivo);

    if (!salida.is_open()) {
        cerr << "Error: No se pudo crear " << nombreArchivo << endl;
        return 1;
    }

    salida << "node_id,distance_m\n";
    salida << fixed << setprecision(6);

    for (const auto& par : distanciasOrdenadas) {
        salida << par.first << ',' << par.second << '\n';
    }

    cout << "CSV generado: " << nombreArchivo << endl;

    return 0;
}
