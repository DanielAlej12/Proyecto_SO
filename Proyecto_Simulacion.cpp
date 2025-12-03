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

// Round Robin
SimulationResult runRR(const vector<Activity>& originalActivities, int quantum) {
    if (quantum <= 0) {
        SimulationResult result;
        return result;
    }

    struct RR_Activity_State {
        string name;
        int ti;
        int t_initial;
        int t_remaining;
        int tf = 0;
        bool isCompleted = false;
    };

    vector<RR_Activity_State> all_activities;
    for (const auto& act : originalActivities) {
        all_activities.push_back({act.name, act.ti, act.t, act.t, 0, false});
    }

    int currentTime = 0;
    int completedCount = 0;
    size_t currentIndex = 0;
    int n = all_activities.size();

    while (completedCount < n) {
        RR_Activity_State& act = all_activities[currentIndex];

        if (!act.isCompleted && act.ti <= currentTime && act.t_remaining > 0) {
            int execTime = min(act.t_remaining, quantum);
            act.t_remaining -= execTime;
            currentTime += execTime;

            if (act.t_remaining == 0) {
                act.tf = currentTime;
                act.isCompleted = true;
                completedCount++;
            }
        } else {
            // Verifica si hay alguna tarea lista en currentTime
            bool anyReady = false;
            for (const auto& a : all_activities) {
                if (!a.isCompleted && a.ti <= currentTime) {
                    anyReady = true;
                    break;
                }
            }
            if (!anyReady) {
                // Avanzar tiempo al próximo ti pendiente para evitar bloqueo
                int minTi = INT_MAX;
                for (const auto& a : all_activities) {
                    if (!a.isCompleted && a.ti > currentTime && a.ti < minTi) {
                        minTi = a.ti;
                    }
                }
                if (minTi != INT_MAX) currentTime = minTi;
            }
        }

        currentIndex = (currentIndex + 1) % n;
    }

    vector<Activity> completedActivities;
    for (const auto& s : all_activities) {
        Activity act;
        act.name = s.name;
        act.ti = s.ti;
        act.t = s.t_initial;
        act.tf = s.tf;
        calculateMetrics(act);
        completedActivities.push_back(act);
    }

    SimulationResult result;
    result.completedActivities = completedActivities;
    calculateAverages(result);
    return result;
}

// --- 6. ESCRITURA DE RESULTADOS ---
void writeResults(ofstream& outfile, const string& method, const SimulationResult& result) {
    const int W_NAME = 12;
    const int W_TI = 5;
    const int W_T = 5;
    const int W_TF = 5;
    const int W_METRIC = 10; // Para T y E
    const int W_RATIO = 12; // Para I

    outfile << "========================================================\n";
    outfile << "           RESULTADOS DEL MÉTODO: " << method << "\n";
    outfile << "========================================================\n";

    // Encabezados de la Tabla
    outfile << left << setw(W_NAME) << "Actividad"
            << "| " << right << setw(W_TI) << "ti"
            << " | " << right << setw(W_T) << "t"
            << " | " << right << setw(W_TF) << "tf"
            << " | " << right << setw(W_METRIC) << "T (Total)" 
            << " | " << right << setw(W_METRIC) << "E (Espera)"
            << " | " << right << setw(W_RATIO) << "I (Servicio)"
            << "\n";
    
    outfile << "------------" << "+-------" << "+-------" << "+-------" << "+------------" << "+------------" << "+-------------\n";

    // Datos de Actividades
    for (const auto& act : result.completedActivities) {
        outfile << left << setw(W_NAME) << act.name 
                << "| " << right << setw(W_TI) << act.ti 
                << " | " << right << setw(W_T) << act.t 
                << " | " << right << setw(W_TF) << act.tf 
                << " | " << right << setw(W_METRIC) << act.T 
                << " | " << right << setw(W_METRIC) << act.E 
                << " | " << right << setw(W_RATIO) << fixed << setprecision(4) << act.I 
                << "\n";
    }

    // Promedios
    outfile << "\n--- Promedios ---\n";
    outfile << "Promedio T (Total): " << fixed << setprecision(4) << result.avg_T << "\n";
    outfile << "Promedio E (Espera):  " << fixed << setprecision(4) << result.avg_E << "\n";
    outfile << "Promedio I (Servicio): " << fixed << setprecision(4) << result.avg_I << "\n";
    outfile << "Tiempo de Ejecución: " << fixed << setprecision(6) << result.executionTime << " segundos.\n";
    outfile << "\n\n";
}

void compareAndWriteBest(ofstream& outfile, const SimulationResult& fifo, const SimulationResult& lifo, const SimulationResult& rr) {
    double best_I = fifo.avg_I;
    string best_method = "FIFO";

    if (lifo.avg_I > best_I) {
        best_I = lifo.avg_I;
        best_method = "LIFO";
    }

    if (rr.avg_I > best_I) {
        best_I = rr.avg_I;
        best_method = "Round Robin";
    }

    outfile << "========================================================\n";
    outfile << "          COMPARACIÓN FINAL DE ALGORITMOS\n";
    outfile << "========================================================\n";
    outfile << "Métrica de Comparación Principal: Mayor Índice de Servicio (I)\n\n";
    outfile << "Resultados: \n";
    outfile << "  FIFO (Promedio I): " << fixed << setprecision(4) << fifo.avg_I << "\n";
    outfile << "  LIFO (Promedio I): " << fixed << setprecision(4) << lifo.avg_I << "\n";
    outfile << "  Round Robin (Promedio I): " << fixed << setprecision(4) << rr.avg_I << "\n\n";

    outfile << "EL MEJOR MÉTODO DE PLANIFICACIÓN ES: " << best_method << " con un Promedio I de "
            << fixed << setprecision(4) << best_I << ".\n";
}

// --- 7. FUNCIÓN PRINCIPAL (MAIN) ---
int main() {
    string csv_path;
    int quantum;
    const string output_filename = "resultados_planificacion.txt";

    cout << "--- Simulador de Planificacion de CPU ---\n";
    cout << "Ingrese la ruta del archivo CSV: "; cin >> csv_path;

    cout << "Ingrese el Quantum (Q) para Round Robin (entero positivo): ";
    if (!(cin >> quantum) || quantum <= 0) {
        cerr << "Error: El Quantum debe ser un número entero positivo.\n";
        return 1;
    }

    vector<Activity> activities = readActivities(csv_path);
    if (activities.empty()) {
        cerr << "No hay actividades válidas para procesar. Saliendo.\n";
        return 1;
    }
    cout << "\nSe leyeron " << activities.size() << " actividades.\n";

    cout << "Simulando FIFO..." << endl;
    SimulationResult fifo_res = runMultipleTimes(runScanningFIFO, activities, 1000);
    
    cout << "Simulando LIFO..." << endl;
    SimulationResult lifo_res = runMultipleTimes(runScanningLIFO, activities, 1000);
    
    cout << "Simulando Round Robin..." << endl;
    SimulationResult rr_res = runMultipleTimes([&](const vector<Activity>& acts) { 
        return runRR(acts, quantum); 
    }, activities, 1000);

    ofstream outfile(output_filename);
    if (!outfile.is_open()) {
        cerr << "Error: No se pudo crear o abrir el archivo de salida: " << output_filename << endl;
        return 1;
    }
    
    writeResults(outfile, "FIFO", fifo_res);
    writeResults(outfile, "LIFO", lifo_res);
    writeResults(outfile, "Round Robin", rr_res);

    compareAndWriteBest(outfile, fifo_res, lifo_res, rr_res);

    outfile.close();
    
    cout << "\n--- Proceso Completado ---\n";
    cout << "Resultados y comparacion guardados en: " << output_filename << endl;

    return 0;
}
