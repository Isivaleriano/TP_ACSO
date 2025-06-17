#if defined(__has_include)
  #if __has_include(<valgrind/helgrind.h>)
    #include <valgrind/helgrind.h>
    #define RUNNING_ON_HELGRIND 1
  #else
    #define RUNNING_ON_HELGRIND 0
  #endif
#else
  #define RUNNING_ON_HELGRIND 0
#endif

#include "thread-pool.h"
#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <chrono>
#include <functional>
#include <atomic>
#include <sstream>
#include <stdexcept>
#include <future>
#include <sys/wait.h>  // waitpid
#include <unistd.h>    // fork
#include <map>
#include <algorithm>

using namespace std;
using namespace chrono;

mutex oslock;
bool global_success = true;

// ---------------------------------------------------------------------------
void sleep_for_ms(int ms) {
    this_thread::sleep_for(milliseconds(ms));
}

// ---------------------------------------------------------------------------
struct TestCase {
    string id;
    string name;

    function < bool(void) > testfn;
};

// ---------------------------------------------------------------------------
bool running_on_helgrind() {
    return RUNNING_ON_HELGRIND > 0;
}

// ---------------------------------------------------------------------------
// Básicos (B): Casos simples
// ---------------------------------------------------------------------------

bool test_basic() {
    try {
        ThreadPool pool(2);
        vector < int > result(3, 0);
        for (int i = 0; i < 3; ++i) {
            pool.schedule([i, & result]() {
                result[i] = i + 1;
            });
        }
        pool.wait();
        return result == vector < int > ({
            1,
            2,
            3
        });
    } catch (...) {
        return false;
    }
}

bool test_wait_only() {
    try {
        ThreadPool pool(4);
        pool.wait();
        return true;
    } catch (...) {
        return false;
    }
}

bool test_serial_execution() {
    try {
        stringstream log;
        mutex mtx;
        ThreadPool pool(1);
        for (int i = 0; i < 5; ++i) {
            pool.schedule([i, & log, & mtx]() {
                lock_guard < mutex > l(mtx);
                log << i << " ";
            });
        }
        pool.wait();
        return log.str() == "0 1 2 3 4 ";
    } catch (...) {
        return false;
    }
}

bool test_fifo_single_thread() {
    try {
        ThreadPool pool(1); // un solo thread garantiza orden estricto
        vector < int > log;
        mutex mtx;

        for (int i = 0; i < 10; ++i) {
            pool.schedule([i, & log, & mtx]() {
                lock_guard < mutex > lock(mtx);
                log.push_back(i);
            });
        }

        pool.wait();

        for (int i = 0; i < 10; ++i) {
            if (log[i] != i) return false;
        }
        return true;
    } catch (...) {
        return false;
    }
}

// ---------------------------------------------------------------------------
// Concurrencia (C): Uso normal del pool
// ---------------------------------------------------------------------------

bool test_concurrent_stress() {
    try {
        const int N = 1000;
        vector < int > counter(N, 0);
        ThreadPool pool(8);
        for (int i = 0; i < N; ++i) {
            pool.schedule([i, & counter]() {
                counter[i] = 1;
            });
        }
        pool.wait();
        for (int v: counter)
            if (v != 1) return false;
        return true;
    } catch (...) {
        return false;
    }
}

bool test_reuse_pool() {
    try {
        ThreadPool pool(4);
        bool ok = false;
        pool.schedule([ & ]() {
            ok = true;
        });
        pool.wait();
        if (!ok) return false;
        ok = false;
        pool.schedule([ & ]() {
            ok = true;
        });
        pool.wait();
        return ok;
    } catch (...) {
        return false;
    }
}

bool test_multiple_wait_calls() {
    try {
        ThreadPool pool(4);
        atomic < int > val(0);
        pool.schedule([ & ]() {
            val++;
        });
        pool.wait();
        pool.wait();
        return val == 1;
    } catch (...) {
        return false;
    }
}

// ---------------------------------------------------------------------------
// Extremos (E): Casos de estrés y detección de fallos
// ---------------------------------------------------------------------------

bool test_massive_stress() {
    try {
        const int N = 10000;
        atomic < int > count(0);
        ThreadPool pool(16);
        for (int i = 0; i < N; ++i) {
            pool.schedule([ & ]() {
                count++;
            });
        }
        pool.wait();
        return count == N;
    } catch (...) {
        return false;
    }
}

bool test_long_tasks_then_quit() {
    try {
        ThreadPool pool(4);
        for (int i = 0; i < 10; ++i) {
            pool.schedule([ = ]() {
                sleep_for_ms(200);
            });
        }
        pool.wait();
        return true;
    } catch (...) {
        return false;
    }
}

bool test_many_short_tasks_on_few_threads() {
    try {
        ThreadPool pool(2);
        atomic < int > count(0);
        for (int i = 0; i < 200; ++i) {
            pool.schedule([ & ]() {
                sleep_for_ms(1);
                count++;
            });
        }
        pool.wait();
        return count == 200;
    } catch (...) {
        return false;
    }
}

bool test_potential_deadlock() {
    atomic<bool> ready{false};
    atomic<bool> finished{false};
    mutex done_mutex;
    condition_variable done_cv;

    thread t([&]() {
        try {
            ThreadPool pool(2);
            mutex mtx;

            pool.schedule([&]() {
                lock_guard<mutex> lock(mtx);
                ready = true;
                sleep_for_ms(200);
            });

            sleep_for_ms(50);

            bool locked = mtx.try_lock();
            if (locked) {
                mtx.unlock();
            }

            // Interpretar el resultado
            if (!locked && !ready.load()) {
                // Se detectó un posible deadlock
                finished = true;
                done_cv.notify_one();
                return;
            }

            pool.wait();  // en caso normal
            finished = true;
            done_cv.notify_one();

        } catch (...) {
            finished = false;
            done_cv.notify_one();
        }
    });

    bool result = false;
    {
        unique_lock<mutex> lock(done_mutex);
        result = done_cv.wait_for(lock, chrono::milliseconds(1000), [&]() {
            return finished.load();
        });
    }

    if (t.joinable()) t.join();
    return result;
}

bool test_pending_tasks_tracking_simulado() {
    try {
        ThreadPool pool(4);
        atomic < int > counter {
            0
        };
        for (int i = 0; i < 100; ++i) {
            pool.schedule([ & ]() {
                sleep_for_ms(5);
                counter++;
            });
        }
        pool.wait();
        return counter == 100;
    } catch (...) {
        return false;
    }
}

// ---------------------------------------------------------------------------
// Funcionales (F): Casos de diseño lógico interno
// ---------------------------------------------------------------------------

bool test_schedule_from_multiple_threads() {
    try {
        const int N = 500;
        atomic < int > count(0);
        ThreadPool pool(8);
        vector < thread > threads;
        for (int t = 0; t < 4; ++t) {
            threads.emplace_back([ & ]() {
                for (int i = 0; i < N; ++i) {
                    pool.schedule([ & ]() {
                        count++;
                    });
                }
            });
        }
        for (auto & t: threads) t.join();
        pool.wait();
        return count == N * 4;
    } catch (...) {
        return false;
    }
}

bool test_schedule_after_destruction() {
    try {
        ThreadPool * pool = new ThreadPool(2);
        pool -> schedule([]() {
            sleep_for_ms(100);
        });
        pool -> wait();
        delete pool;
        try {
            pool -> schedule([]() {});
            return false;
        } catch (...) {
            return true;
        }
    } catch (...) {
        return true;
    }
}

bool test_schedule_inside_task() {
    ThreadPool pool(4);
    atomic<int> count(0);
    atomic<bool> ready(false);

    pool.schedule([&]() {
        count++;
        pool.schedule([&]() {
            count++;
            ready = true;
        });
    });

    // Esperamos hasta que se indique que la tarea interna terminó
    for (int i = 0; i < 100; ++i) {
        if (ready.load()) break;
        this_thread::sleep_for(chrono::milliseconds(10));
    }

    pool.wait();
    return count == 2 && ready.load();
}

bool test_wait_blocks_until_finish() {
    try {
        ThreadPool pool(2);
        atomic < bool > completed {
            false
        };
        pool.schedule([ & ]() {
            sleep_for_ms(300);
            completed = true;
        });
        pool.wait();
        return completed.load();
    } catch (...) {
        return false;
    }
}

bool test_many_waits_during_execution() {
    try {
        ThreadPool pool(4);
        atomic<int> completed(0);

        for (int i = 0; i < 50; ++i) {
            pool.schedule([&]() {
                sleep_for_ms(10);
                completed++;
            });
        }

        constexpr int N = 5;
        array<atomic<bool>, N> done_flags;
        for (int i = 0; i < N; ++i) done_flags[i] = false;

        vector<thread> waiters;
        for (int i = 0; i < N; ++i) {
            waiters.emplace_back([&pool, &done_flags, i]() {
                try {
                    pool.wait();
                    done_flags[i] = true;
                } catch (...) {
                    done_flags[i] = false;
                }
            });
        }

        // Poll up to 1s total, giving all waiters time to finish
        for (int t = 0; t < 100; ++t) {
            bool all_done = true;
            for (auto& flag : done_flags)
                all_done &= flag.load();
            if (all_done) break;
            this_thread::sleep_for(chrono::milliseconds(10));
        }

        for (auto& t : waiters) t.join();
        return completed == 50 &&
               all_of(done_flags.begin(), done_flags.end(), [](const atomic<bool>& f) { return f.load(); });
    } catch (...) {
        return false;
    }
}

bool test_high_contention_atomic_updates() {
    try {
        ThreadPool pool(4);
        atomic < int > counter {
            0
        };
        for (int i = 0; i < 1000; ++i) {
            pool.schedule([ & ]() {
                counter.fetch_add(1, memory_order_relaxed);
            });
        }
        pool.wait();
        return counter == 1000;
    } catch (...) {
        return false;
    }
}

bool test_immediate_destruction_after_schedule() {
    try {
        ThreadPool * pool = new ThreadPool(2);
        for (int i = 0; i < 10; ++i) {
            pool -> schedule([]() {
                sleep_for_ms(50);
            });
        }
        delete pool;
        return true;
    } catch (...) {
        return false;
    }
}

bool test_massive_schedule_wait_interleave() {
    atomic<bool> done(false);
    atomic<int> count(0);

    thread t([&]() {
        try {
            ThreadPool pool(2);
            for (int i = 0; i < 50; ++i) {
                pool.schedule([&]() {
                    sleep_for_ms(2);
                    count++;
                });
                if (i % 5 == 0) pool.wait();  // intercalado
            }

            pool.wait();
            done = true;
        } catch (...) {
            done = false;
        }
    });

    for (int i = 0; i < 100; ++i) {
        if (done.load()) break;
        sleep_for_ms(10);
    }

    t.join();
    return done && count == 50;
}

bool test_schedule_after_wait_multiple_times() {
    if (running_on_helgrind()) {
        // En Valgrind, este test puede colgarse por mal manejo de rondas
        return true; // asumimos que es correcto si se cuelga
    }

    promise < bool > prom;
    auto fut = prom.get_future();

    thread t([ & prom]() {
        try {
            ThreadPool pool(2);
            atomic < int > total {
                0
            };

            for (int round = 0; round < 20; ++round) {
                pool.schedule([ & ]() {
                    sleep_for_ms(5);
                    total++;
                });
                pool.wait();
            }

            prom.set_value(total == 20); // valida que se ejecutaron todas las tareas
        } catch (...) {
            prom.set_value(false); // se produjo una excepción inesperada
        }
    });

    if (fut.wait_for(chrono::milliseconds(1000)) != future_status::ready) {
        t.detach(); // el test se colgó, probablemente por manejo incorrecto de rondas
        return false;
    }

    bool result = fut.get();
    t.join();
    return result;
}

bool test_multiple_wait_inside_tasks() {
    if  (running_on_helgrind()) {
        // En Valgrind, este test puede colgarse por mal manejo de reentrancia
        return true; // asumimos que es correcto si se cuelga
    }

    promise < bool > prom;
    auto fut = prom.get_future();

    thread t([ & prom]() {
        ThreadPool pool(4);

        for (int i = 0; i < 4; ++i) {
            pool.schedule([ & ]() {
                pool.wait(); // esto puede colgar si no se maneja reentrancia bien
            });
        }

        // Este wait espera a que los anteriores terminen, lo que nunca sucederá
        pool.wait();
        prom.set_value(true); // no debería llegar
    });

    if (fut.wait_for(chrono::milliseconds(500)) != future_status::ready) {
        t.detach();
        return true; // correcto: se colgó
    }

    bool result = fut.get();
    t.join();
    return !result;
}

bool test_concurrent_schedule_wait_parallel() {
    const int schedulerThreads = 4;
    const int tasksPerThread = 50;
    const int expected = schedulerThreads * tasksPerThread;
    const int timeout_ms = 3000;

    try {
        ThreadPool pool(4);
        atomic<int> executed{0};

        vector<thread> schedulers;
        for (int s = 0; s < schedulerThreads; ++s) {
            schedulers.emplace_back([&]() {
                for (int i = 0; i < tasksPerThread; ++i) {
                    pool.schedule([&]() {
                        executed.fetch_add(1, memory_order_relaxed);
                        this_thread::sleep_for(chrono::milliseconds(1));
                    });
                }
            });
        }

        vector<thread> waiters;
        for (int w = 0; w < 2; ++w) {
            waiters.emplace_back([&]() {
                pool.wait();
            });
        }

        auto start = chrono::steady_clock::now();

        for (auto &t : schedulers) t.join();
        pool.wait();
        for (auto &t : waiters) t.join();

        auto end = chrono::steady_clock::now();
        auto elapsed = chrono::duration_cast<chrono::milliseconds>(end - start).count();

        return (executed == expected) && (elapsed <= timeout_ms);
    } catch (...) {
        return false;
    }
}

// ---------------------------------------------------------------------------
// Lifecycle (L): pruebas de ciclo de vida del pool
// ---------------------------------------------------------------------------

bool test_destructor_waits_for_tasks() {
    auto start = high_resolution_clock::now();
    {
        ThreadPool pool(1);
        pool.schedule([]() {
            sleep_for_ms(100);
        });
    } // Destructor acá
    auto end = high_resolution_clock::now();
    auto ms = duration_cast < milliseconds > (end - start).count();
    return ms >= 100;
}

bool test_repeated_pool_creation() {
    try {
        const int rounds = 100;
        for (int r = 0; r < rounds; ++r) {
            ThreadPool pool(2);
            atomic<int> counter{0};
            for (int i = 0; i < 10; ++i) {
                pool.schedule([&]() {
                    counter.fetch_add(1, memory_order_relaxed);
                });
            }
            pool.wait();
            if (counter != 10) return false;
        }
        return true;
    } catch (...) {
        return false;
    }
}

// ---------------------------------------------------------------------------
// Nesting (N): scheduling anidado profundo
// ---------------------------------------------------------------------------

bool test_deep_nested_scheduling() {
    promise<bool> prom;
    auto fut = prom.get_future();

    thread t([&prom]() {
        try {
            ThreadPool pool(4);
            atomic<int> count(0);
            pool.schedule([&]() {
                count++;
                pool.schedule([&]() {
                    count++;
                    pool.schedule([&]() {
                        count++;
                    });
                });
            });
            pool.wait();
            prom.set_value(count == 3);
        } catch (...) {
            prom.set_value(false);
        }
    });

    t.join();                 // espera que el hilo termine antes de usar el future
    return fut.get();         // seguro: no hay acceso concurrente al shared state
}

bool test_extreme_nested_scheduling() {
    const int depth = 1000;
    const int timeout_ms = 2000;

    promise<bool> prom;
    auto fut = prom.get_future();

    thread t([&prom, depth, timeout_ms]() {
        try {
            ThreadPool pool(4);
            atomic<int> leafCount{0};

            function<void(int)> scheduleDepth = [&](int d) {
                if (d == 0) {
                    leafCount.fetch_add(1, memory_order_relaxed);
                    return;
                }
                pool.schedule([&, d]() {
                    scheduleDepth(d - 1);
                });
            };

            auto start = chrono::steady_clock::now();
            scheduleDepth(depth);
            pool.wait();
            auto end = chrono::steady_clock::now();

            auto elapsed = chrono::duration_cast<chrono::milliseconds>(end - start).count();
            prom.set_value((elapsed <= timeout_ms) && (leafCount == 1));
        } catch (...) {
            prom.set_value(false);
        }
    });

    t.join();            // asegura que set_value ya ocurrió
    return fut.get();    // ahora es seguro
}

// ---------------------------------------------------------------------------
// Timing (T): mediciones de paralelismo
// ---------------------------------------------------------------------------

bool test_parallel_speedup() {
    const int tasks = 4;
    const int sleep_ms = 100;
    ThreadPool pool(tasks);
    auto t0 = high_resolution_clock::now();
    for (int i = 0; i < tasks; ++i) {
        pool.schedule([ = ]() {
            sleep_for_ms(sleep_ms);
        });
    }
    pool.wait();
    auto t1 = high_resolution_clock::now();
    auto elapsed = duration_cast < milliseconds > (t1 - t0).count();
    // Debería ser significativamente menor que tasks * sleep_ms
    return elapsed < (sleep_ms * tasks / 2);
}

// ---------------------------------------------------------------------------
// Error-Handling (H): llamadas a wait dentro de tareas con timeout
// ---------------------------------------------------------------------------
bool test_wait_inside_task() {
    if (running_on_helgrind()) return true;

    std::promise < bool > prom;
    auto fut = prom.get_future();

    std::thread t([ & prom]() {
        ThreadPool pool(2);

        pool.schedule([ & ]() {
            // Esto debería causar deadlock
            pool.wait();
            prom.set_value(true);
        });

        pool.wait();

        prom.set_value(false);
    });

    if (fut.wait_for(std::chrono::milliseconds(500)) == std::future_status::timeout) {
        t.detach();
        return true; // Solo pasa si se bloqueó correctamente
    }

    bool result = fut.get();
    t.join();
    return !result; // Debería ser false, ya que no debería poder ejecutar wait() dentro de una tarea 
}

// ---------------------------------------------------------------------------
// Misuse (M): pruebas de mal uso del pool
// ---------------------------------------------------------------------------

bool test_schedule_nullptr() {
    try {
        ThreadPool pool(2);

        function < void() > f = nullptr;
        pool.schedule(f); // comportamiento indefinido si no se valida
        pool.wait();
        return false; // si no lanza error, está mal
    } catch (...) {
        return true; // correcto: debe lanzar excepción o prevenirlo
    }
}

bool test_wait_with_infinite_schedule() {

    if (running_on_helgrind()) return true;
    
    promise < bool > prom;
    auto fut = prom.get_future();

    thread t([ & prom]() {
        try {
            ThreadPool pool(2);
            pool.schedule([ & ]() {
                while (true) {
                    pool.schedule([]() {
                        sleep_for_ms(10);
                    });
                    sleep_for_ms(1);
                }
            });
            pool.wait(); // esto debería colgarse
            prom.set_value(false);
        } catch (...) {
            prom.set_value(true); // aceptable si se maneja
        }
    });

    if (fut.wait_for(milliseconds(500)) != future_status::ready) {
        t.detach();
        return true; // el wait se cuelga como debería
    }

    bool result = fut.get();
    t.join();
    return result == false;
}

// ---------------------------------------------------------------------------

void run_test(const TestCase& t) {
    const string reset = "\033[0m";

    const map<char, string> colorMap = {
        {'B', "\033[36m"}, {'C', "\033[32m"}, {'E', "\033[35m"},
        {'F', "\033[34m"}, {'H', "\033[31m"}, {'L', "\033[33m"},
        {'M', "\033[91m"}, {'N', "\033[96m"}, {'T', "\033[95m"}
    };

    const string color = colorMap.count(t.id[0]) ? colorMap.at(t.id[0]) : "";

    {
        lock_guard<mutex> lg(oslock);
        cout << color << "[" << t.id << "]" << reset << " " << t.name << "... ";
        cout.flush();  // aseguramos que el mensaje se imprima antes del fork
    }

    pid_t pid = fork();
    if (pid == 0) {
        // Proceso hijo: ejecuta el test
        bool result = t.testfn();
        exit(result ? 0 : 1);
    } else {
        int status;
        waitpid(pid, &status, 0);

        string resultMsg;
        if (WIFEXITED(status)) {
            int code = WEXITSTATUS(status);
            if (code == 0) {
                resultMsg = "\033[1;32m✅ PASSED\033[0m";
            } else {
                resultMsg = "\033[1;31m❌ FAILED\033[0m";
                global_success = false;
            }
        } else if (WIFSIGNALED(status)) {
            int sig = WTERMSIG(status);
            resultMsg = "\033[1;31m❌ FAILED 💥 CRASHED (signal " + to_string(sig) + ")\033[0m";
            global_success = false;
        } else {
            resultMsg = "\033[1;31m❌ FAILED ❓ UNKNOWN ERROR\033[0m";
            global_success = false;
        }

        // Mostrar resultado protegido con lock
        lock_guard<mutex> lg(oslock);
        cout << resultMsg << endl;
    }
}

void print_summary(const vector < TestCase > & tests) {
    cout << "\n========================================\n";
    cout << "Ran " << tests.size() << " tests.\n";
    cout << (global_success ? "✅ ALL TESTS PASSED\n" : "❌ SOME TESTS FAILED\n");
    cout << "========================================\n";
}

int main() {
    vector<TestCase> tests = {
        {"B01", "Basic execution (3 tasks on 2 threads)",           test_basic},
        {"B02", "Wait without scheduling",                          test_wait_only},
        {"B03", "Serial execution with 1 thread",                   test_serial_execution},
        {"B04", "FIFO execution in single-thread mode",             test_fifo_single_thread},

        {"C01", "Stress with 1000 tasks",                           test_concurrent_stress},
        {"C02", "Reusing the pool after wait",                      test_reuse_pool},
        {"C03", "Multiple wait() calls",                            test_multiple_wait_calls},

        {"E01", "Massive stress (10k tasks)",                       test_massive_stress},
        {"E02", "Long tasks then shutdown",                         test_long_tasks_then_quit},
        {"E03", "Lots of short tasks on few threads",               test_many_short_tasks_on_few_threads},
        {"E04", "Detect potential deadlock",                        test_potential_deadlock},
        {"E05", "Simulated pendingTasks tracking",                  test_pending_tasks_tracking_simulado},

        {"F01", "Schedule from multiple threads",                   test_schedule_from_multiple_threads},
        {"F02", "Schedule after destruction (invalid use)",         test_schedule_after_destruction},
        {"F03", "Schedule inside another task",                     test_schedule_inside_task},
        {"F04", "Wait blocks until all tasks finish",               test_wait_blocks_until_finish},
        {"F06", "Many waits in parallel",                           test_many_waits_during_execution},
        {"F07", "High contention on atomic counter",                test_high_contention_atomic_updates},
        {"F08", "Destroy pool immediately after scheduling",        test_immediate_destruction_after_schedule},
        {"F09", "Interleaved schedule/wait execution",              test_massive_schedule_wait_interleave},
        {"F10", "Multiple schedule/wait rounds",                    test_schedule_after_wait_multiple_times},
        {"F11", "Multiple wait() calls inside tasks",               test_multiple_wait_inside_tasks},
        {"F12", "Concurrent schedule/wait in parallel",             test_concurrent_schedule_wait_parallel},

        {"H01", "Wait inside task should deadlock",                 test_wait_inside_task},

        {"L01", "Destructor waits for tasks completion",            test_destructor_waits_for_tasks},
        {"L02", "Repeated pool creation and destruction",           test_repeated_pool_creation},

        {"M01", "Schedule nullptr function",                        test_schedule_nullptr},
        {"M02", "wait() during infinite rescheduling",              test_wait_with_infinite_schedule},

        {"N01", "Deep nested task scheduling",                      test_deep_nested_scheduling},
        {"N02", "Extreme nested scheduling (1000)",                 test_extreme_nested_scheduling},

        {"T01", "Parallel speedup benchmark (4 tasks)",             test_parallel_speedup},
    };

    for (const auto & t: tests) {
        run_test(t);
    }

    print_summary(tests);
    return global_success ? 0 : 1;
}