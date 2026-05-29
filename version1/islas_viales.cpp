#include <iostream>
#include <vector>
#include <fstream>
#include <queue>
#include <map>
using namespace std;


class Grafo {
    private : 
    map <int, vector<int>> adyacencia;
    map <int, bool> visitados;
    public :
    void agregarArista(int u, int v);
    void leerCSV(const string& nombreArchivo);
    int bfs(int inicio);
    vector <int> encontrarIslasViales();
        
    
}