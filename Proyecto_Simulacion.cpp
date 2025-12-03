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