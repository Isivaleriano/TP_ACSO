#include <iostream>
#include <thread>
#include <chrono>
#include <cassert>
#include <mutex>
#include <atomic>
#include "thread-pool.h"

using namespace std;

mutex oslock;

void sleep_for(int ms) {
    this_thread::sleep_for(chrono::milliseconds(ms));
}

int main() {
    cout << "\n=== TEST: schedule sin wait ===" << endl;
    {
        ThreadPool pool(2);
        pool.schedule([] {
            oslock.lock();
            cout << "Tarea sin wait terminada.\n";
            oslock.unlock();
        });
        sleep_for(200);  
    }

    cout << "\n=== TEST: wait sin tareas ===" << endl;
    {
        ThreadPool pool(2);
        pool.wait(); 
        cout << "wait() sin tareas pasó correctamente." << endl;
    }

    cout << "\n=== TEST: wait llamado dos veces ===" << endl;
    {
        ThreadPool pool(2);
        pool.schedule([] { sleep_for(100); });
        pool.wait();
        pool.wait(); 
        cout << "Doble wait pasó correctamente." << endl;
    }

    cout << "\n=== TEST: muchas tareas con pocos hilos ===" << endl;
    {
        ThreadPool pool(2);
        const int n = 20;
        atomic<int> counter{0};

        for (int i = 0; i < n; ++i) {
            pool.schedule([&]() {
                sleep_for(10);
                counter++;
            });
        }

        pool.wait();
        assert(counter == n);
        cout << "Ejecutadas " << counter << " tareas exitosamente." << endl;
    }

    cout << "\n=== TEST: agregar tareas luego de wait ===" << endl;
    {
        ThreadPool pool(2);
        atomic<int> counter{0};

        pool.schedule([&]() {
            sleep_for(50);
            counter++;
        });
        pool.wait();  

        pool.schedule([&]() {
            sleep_for(50);
            counter++;
        });
        pool.wait();  

        assert(counter == 2);
        cout << "Agregado de tareas luego de wait funciona." << endl;
    }

    cout << "\n=== TEST: tareas largas y simultáneas ===" << endl;
    {
        ThreadPool pool(3);
        atomic<int> counter{0};

        for (int i = 0; i < 6; ++i) {
            pool.schedule([&]() {
                sleep_for(300);
                counter++;
            });
        }

        pool.wait();
        assert(counter == 6);
        cout << "Tareas largas completadas correctamente." << endl;
    }

    cout << "\n=== Todos los tests de casos límite y atípicos pasaron ===" << endl;
    return 0;
}
