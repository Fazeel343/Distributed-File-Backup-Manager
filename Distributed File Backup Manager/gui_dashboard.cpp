#include "gui_dashboard.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <iomanip>

GUIDashboard::GUIDashboard(MetadataManager&   meta,
                            StorageController& storage,
                            RecoveryModule&    recovery,
                            RequestQueue&      queue)
    : meta_(meta), storage_(storage), recovery_(recovery), queue_(queue) {}

void GUIDashboard::stop()  { running_ = false; }

void GUIDashboard::run() {
    while (running_) {
        render();
        std::this_thread::sleep_for(std::chrono::milliseconds(800));
    }
    render();   // final render after everything finishes
}

void GUIDashboard::print_bar(int done, int total, int width) const {
    int filled = (total > 0) ? (done * width / total) : 0;
    std::cout << "[";
    for (int i = 0; i < width; i++)
        std::cout << (i < filled ? "#" : "-");
    std::cout << "] " << std::setw(3) << done << "/" << total;
}

void GUIDashboard::render() const {
    // Clear screen
#ifdef _WIN32
    system("cls");
#else
    std::cout << "\033[2J\033[H";
#endif

    auto all = meta_.get_all();
    int pending     = meta_.count_by_status("pending");
    int in_progress = meta_.count_by_status("in_progress");
    int done        = meta_.count_by_status("done");
    int failed      = meta_.count_by_status("failed");
    int skipped     = meta_.count_by_status("skipped");

    // ── Header ──────────────────────────────────────────────
    std::cout << Color::BOLD << Color::WHITE
              << "╔══════════════════════════════════════════════════════════╗\n"
              << "║      DISTRIBUTED FILE BACKUP MANAGER  —  Live Status    ║\n"
              << "╚══════════════════════════════════════════════════════════╝\n"
              << Color::RESET;

    // ── Summary bar ─────────────────────────────────────────
    std::cout << Color::CYAN  << "  Pending: "     << std::setw(3) << pending     << "  "
              << Color::YELLOW << "In Progress: "  << std::setw(3) << in_progress << "  "
              << Color::GREEN  << "Done: "         << std::setw(3) << done        << "  "
              << Color::RED    << "Failed: "       << std::setw(3) << failed      << "  "
              << Color::MAGENTA<< "Skipped: "      << std::setw(3) << skipped     << "  "
              << Color::WHITE  << "Queue: "        << std::setw(3) << queue_.size()
              << Color::RESET  << "\n";

    std::cout << "  Total writes to disk: " << Color::GREEN
              << storage_.total_writes() << Color::RESET
              << "  |  Retries: " << Color::YELLOW
              << recovery_.total_retries() << Color::RESET << "\n\n";

    // ── Per-request table ────────────────────────────────────
    std::cout << Color::BOLD
              << std::left
              << std::setw(28) << "  File"
              << std::setw(8)  << "Client"
              << std::setw(12) << "Status"
              << "  Progress"
              << Color::RESET  << "\n";
    std::cout << std::string(72, '-') << "\n";

    for (auto& req : all) {
        // Status colour
        std::string col = Color::WHITE;
        if      (req.status == "done")        col = Color::GREEN;
        else if (req.status == "in_progress") col = Color::YELLOW;
        else if (req.status == "failed")      col = Color::RED;
        else if (req.status == "skipped")     col = Color::MAGENTA;

        std::cout << "  " << std::setw(26) << req.file_name
                  << std::setw(8)           << req.client_id
                  << col << std::setw(12)   << req.status << Color::RESET
                  << "  ";
        print_bar(req.chunks_done, req.total_chunks);
        std::cout << "\n";
    }

    // ── Last 6 log entries ───────────────────────────────────
    std::cout << "\n" << Color::BOLD << "  Recent log:\n" << Color::RESET;
    auto log = meta_.get_log();
    int  start = std::max(0, (int)log.size() - 6);
    for (int i = start; i < (int)log.size(); i++)
        std::cout << Color::WHITE << "  " << log[i] << Color::RESET << "\n";
}