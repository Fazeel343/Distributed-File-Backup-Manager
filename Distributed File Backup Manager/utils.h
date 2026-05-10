#pragma once
#include <string>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <sstream>

// ── Simulated file sizes ──────────────────────────────────────
#define NUM_CLIENTS      4
#define NUM_FILES        5       // files each client will submit
#define NUM_WORKERS      3
#define MAX_DISK_WRITES  2       // semaphore limit
#define CHUNK_SIZE_MB    1       // each chunk = 1 MB logically
#define FAILURE_RATE     10      // 1-in-N chance of chunk failure

// ── Backup request ────────────────────────────────────────────
struct BackupRequest {
    int         client_id;
    int         request_id;
    std::string file_name;
    long        file_size_mb;   // logical size in MB
    int         total_chunks;
    int         chunks_done;
    std::string status;         // pending / in_progress / done / failed
    bool        incremental;    // true = only changed chunks
    int         retry_count;

    BackupRequest()
        : client_id(0), request_id(0), file_size_mb(0),
          total_chunks(0), chunks_done(0),
          status("pending"), incremental(false), retry_count(0) {}
};

// ── Colour helpers (ANSI) ─────────────────────────────────────
namespace Color {
    const std::string RESET   = "\033[0m";
    const std::string RED     = "\033[31m";
    const std::string GREEN   = "\033[32m";
    const std::string YELLOW  = "\033[33m";
    const std::string CYAN    = "\033[36m";
    const std::string MAGENTA = "\033[35m";
    const std::string BOLD    = "\033[1m";
    const std::string WHITE   = "\033[37m";
}

// ── Timestamp helper ──────────────────────────────────────────
inline std::string now_str() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm_info;
#ifdef _WIN32
    localtime_s(&tm_info, &t);
#else
    localtime_r(&t, &tm_info);
#endif
    std::ostringstream oss;
    oss << std::put_time(&tm_info, "%H:%M:%S");
    return oss.str();
}