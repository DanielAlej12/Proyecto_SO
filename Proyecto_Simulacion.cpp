#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <chrono>
#include <iomanip>
#include <climits>
#include <functional>

using namespace std;

// --- 1. ESTRUCTURAS DE DATOS ---
struct Activity {
    string name;
    int ti;  // Tiempo Inicial
    int t;   // Tiempo de Actividad
    bool isCompleted = false;

    // Métricas calculadas
    int tf;  // Tiempo Final de completado
    int T;   // Tiempo Total: tf - ti
    int E;   // Tiempo de Espera: T - t
    double I; // Índice de Servicio: t / T
};

struct SimulationResult {
    vector<Activity> completedActivities;
    double avg_T;
    double avg_E;
    double avg_I;
    double executionTime; // Tiempo de ejecución promedio por iteración
};

// --- FUNCIÓN AUXILIAR: EJECUTAR MÚLTIPLES VECES ---
SimulationResult runMultipleTimes(function<SimulationResult(vector<Activity>)> algorithm, 
                                  const vector<Activity>& activities, int iterations = 1000) {
    vector<SimulationResult> results;
    
    auto START = chrono::high_resolution_clock::now();

    for(int i = 0; i < iterations; i++) {
        SimulationResult single_result = algorithm(activities);
        results.push_back(single_result);
    }
    
    auto END = chrono::high_resolution_clock::now();
    chrono::duration<double> duration = END - START;
    
    // Promediar métricas
    SimulationResult avg_result;
    avg_result.completedActivities = results[0].completedActivities; // Usar primera iteración
    double total_T = 0, total_E = 0, total_I = 0;
    
    for(const auto& res : results) {
        total_T += res.avg_T;
        total_E += res.avg_E;
        total_I += res.avg_I;
    }
    
    avg_result.avg_T = total_T / iterations;
    avg_result.avg_E = total_E / iterations;
    avg_result.avg_I = total_I / iterations;
    avg_result.executionTime = duration.count() / iterations; // Tiempo promedio
    
    return avg_result;
}
