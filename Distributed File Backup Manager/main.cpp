#include <iostream>
#include <thread>
#include <vector>
#include <cstdlib>
#include <ctime>

#include "utils.h"
#include "request_queue.h"
#include "metadata_manager.h"
#include "storage_controller.h"
#include "recovery.h"
#include "client.h"
#include "worker.h"
#include "web_server.h"

int main() {
    srand(static_cast<unsigned>(time(nullptr)));

    std::cout << Color::BOLD << Color::CYAN
              << "\n=== Distributed File Backup Manager Starting ===\n"
              << "=== Open dashboard.html in your browser        ===\n"
              << "=== API: http://localhost:8765/api/status       ===\n\n"
              << Color::RESET;

    RequestQueue      queue;
    MetadataManager   meta;
    StorageController storage(MAX_DISK_WRITES);
    RecoveryModule    recovery;

    WebServer web(meta, storage, recovery, queue, 8765);
    web.start();
    std::cout << Color::GREEN << "[Web] Dashboard API started on port 8765\n" << Color::RESET;

    std::vector<Worker> workers;
    workers.reserve(NUM_WORKERS);
    for (int i = 1; i <= NUM_WORKERS; i++)
        workers.emplace_back(i, queue, meta, storage, recovery);

    std::vector<std::thread> worker_threads;
    for (auto& w : workers)
        worker_threads.emplace_back(&Worker::run, &w);

    std::vector<Client> clients;
    clients.reserve(NUM_CLIENTS);
    for (int i = 1; i <= NUM_CLIENTS; i++)
        clients.emplace_back(i, queue, meta);

    std::vector<std::thread> client_threads;
    for (auto& c : clients)
        client_threads.emplace_back(&Client::run, &c);

    for (auto& t : client_threads) t.join();
    std::cout << Color::CYAN << "\n[Main] All clients done. Waiting for workers...\n" << Color::RESET;

    while (true) {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        int active = meta.count_by_status("pending") + meta.count_by_status("in_progress");
        if (active == 0 && queue.size() == 0) break;
    }

    queue.shutdown();
    for (auto& t : worker_threads) t.join();

    std::cout << Color::BOLD << Color::GREEN << "\n=== Backup Session Complete ===\n" << Color::RESET;
    std::cout << "  Files done    : " << meta.count_by_status("done")    << "\n";
    std::cout << "  Files failed  : " << meta.count_by_status("failed")  << "\n";
    std::cout << "  Files skipped : " << meta.count_by_status("skipped") << "\n";
    std::cout << "  Total writes  : " << storage.total_writes()           << "\n";
    std::cout << "  Total retries : " << recovery.total_retries()         << "\n";
    std::cout << Color::CYAN << "\n[Web] Server still running. Press Ctrl+C to exit.\n" << Color::RESET;

    std::this_thread::sleep_for(std::chrono::seconds(60));
    web.stop();
    return 0;
}
