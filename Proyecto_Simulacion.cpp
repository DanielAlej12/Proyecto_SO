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

// --- 2. FUNCIÓN DE CÁLCULO DE MÉTRICAS ---
void calculateMetrics(Activity& act) {
    act.T = act.tf - act.ti;
    act.E = act.T - act.t;
    act.I = (double)act.t / act.T;
}

// --- 3. LECTURA DEL ARCHIVO CSV ---
vector<Activity> readActivities(const string& filename) {
    vector<Activity> activities;
    ifstream file(filename);

    if (!file.is_open()) {
        cerr << "Error: No se pudo abrir el archivo CSV: " << filename << endl;
        return activities;
    }

    string line;
    while (getline(file, line)) {
        if (line.empty()) continue;

        stringstream ss(line);
        string segment;
        Activity act;

        try {
            if (!getline(ss, segment, ',')) continue;
            act.name = segment;

            if (!getline(ss, segment, ',')) continue;
            act.ti = stoi(segment);

            if (!getline(ss, segment, ',')) continue;
            act.t = stoi(segment);

            if (act.t <= 0) continue; 

            act.tf = 0; act.T = 0; act.E = 0; act.I = 0.0;
            act.isCompleted = false; 
            activities.push_back(act);
        } catch (const exception& e) {
            cerr << "Error de formato en la línea: '" << line << "'. Detalle: " << e.what() << endl;
        }
    }
    return activities;
}

// --- 4. CÁLCULO DE PROMEDIOS ---
void calculateAverages(SimulationResult& result) {
    double total_T = 0.0;
    double total_E = 0.0;
    double total_I = 0.0;
    int count = result.completedActivities.size();

    for (const auto& act : result.completedActivities) {
        total_T += act.T;
        total_E += act.E;
        total_I += act.I;
    }

    result.avg_T = total_T / count;
    result.avg_E = total_E / count;
    result.avg_I = total_I / count;
}

// --- 5. ALGORITMOS DE PLANIFICACIÓN ---

// FIFO
SimulationResult runScanningFIFO(vector<Activity> activities) {
    int currentTime = 0;
    int completedCount = 0;
    int n = activities.size();

    while (completedCount < n) {
        bool found = false;
        for (int i = 0; i < n; ++i) {
            Activity& act = activities[i];
            if (!act.isCompleted && act.ti <= currentTime) {
                act.tf = currentTime + act.t;
                currentTime = act.tf;
                act.isCompleted = true;
                completedCount++;
                found = true;
                break; // Sale al encontrar la primera lista
            }
        }

        if (!found) {
            // Avanza al siguiente ti mínimo pendiente
            int minTi = INT_MAX;
            for (const auto& act : activities) {
                if (!act.isCompleted && act.ti < minTi) {
                    minTi = act.ti;
                }
            }
            if (minTi != INT_MAX) currentTime = minTi;
            else break; // No quedan actividades
        }
    }   

    vector<Activity> completedActivities;
    for (const auto& s : activities) {
        Activity act;
        act.name = s.name;
        act.ti = s.ti;
        act.t = s.t;
        act.tf = s.tf;
        calculateMetrics(act);
        completedActivities.push_back(act);
    }
    
    SimulationResult result;
    result.completedActivities = completedActivities;
    calculateAverages(result);
    return result;
}

// LIFO
SimulationResult runScanningLIFO(vector<Activity> activities) {
    int currentTime = 0;
    int completedCount = 0;
    int n = activities.size();

    while (completedCount < n) {
        bool found = false;
        for (int i = n - 1; i >= 0; --i) {
            Activity& act = activities[i];
            if (!act.isCompleted && act.ti <= currentTime) {
                act.tf = currentTime + act.t;
                currentTime = act.tf;
                act.isCompleted = true;
                completedCount++;
                found = true;
                break; // Sale al encontrar la primera lista
            }
        }
        
        if (!found) {
            // Avanza al siguiente ti mínimo pendiente
            int minTi = INT_MAX;
            for (const auto& act : activities) {
                if (!act.isCompleted && act.ti < minTi) {
                    minTi = act.ti;
                }
            }
            if (minTi != INT_MAX) currentTime = minTi;
            else break; // No quedan actividades
        }
    }

    vector<Activity> completedActivities;
    for (const auto& s : activities) {
        Activity act;
        act.name = s.name;
        act.ti = s.ti;
        act.t = s.t;
        act.tf = s.tf;
        calculateMetrics(act);
        completedActivities.push_back(act);
    }
    
    SimulationResult result;
    result.completedActivities = completedActivities;
    calculateAverages(result);
    return result;
}
