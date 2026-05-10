#pragma once
#include <string>
#include <sstream>
#include <thread>
#include <atomic>
#include <netinet/in.h>
#include <unistd.h>
#include "metadata_manager.h"
#include "storage_controller.h"
#include "recovery.h"
#include "request_queue.h"

class WebServer {
public:
    WebServer(MetadataManager& meta, StorageController& storage,
              RecoveryModule& recovery, RequestQueue& queue, int port = 8765)
        : meta_(meta), storage_(storage), recovery_(recovery),
          queue_(queue), port_(port), running_(false) {}

    void start() {
        running_ = true;
        thread_ = std::thread(&WebServer::serve_loop, this);
    }

    void stop() {
        running_ = false;
        if (thread_.joinable()) thread_.join();
    }

private:
    MetadataManager&   meta_;
    StorageController& storage_;
    RecoveryModule&    recovery_;
    RequestQueue&      queue_;
    int                port_;
    std::atomic<bool>  running_;
    std::thread        thread_;

    std::string build_json() {
        auto all     = meta_.get_all();
        auto log     = meta_.get_log();
        int pending  = meta_.count_by_status("pending");
        int inprog   = meta_.count_by_status("in_progress");
        int done     = meta_.count_by_status("done");
        int failed   = meta_.count_by_status("failed");
        int skipped  = meta_.count_by_status("skipped");

        std::ostringstream j;
        j << "{";
        j << "\"pending\":"    << pending  << ",";
        j << "\"in_progress\":" << inprog  << ",";
        j << "\"done\":"       << done     << ",";
        j << "\"failed\":"     << failed   << ",";
        j << "\"skipped\":"    << skipped  << ",";
        j << "\"queue_size\":" << queue_.size() << ",";
        j << "\"total_writes\":" << storage_.total_writes() << ",";
        j << "\"total_retries\":" << recovery_.total_retries() << ",";

        // requests array
        j << "\"requests\":[";
        bool first = true;
        for (auto& r : all) {
            if (!first) j << ",";
            first = false;
            j << "{";
            j << "\"id\":"          << r.request_id   << ",";
            j << "\"client_id\":"   << r.client_id    << ",";
            j << "\"file_name\":\"" << r.file_name    << "\",";
            j << "\"status\":\""    << r.status       << "\",";
            j << "\"chunks_done\":" << r.chunks_done  << ",";
            j << "\"total_chunks\":" << r.total_chunks << ",";
            j << "\"retry_count\":" << r.retry_count  << ",";
            j << "\"incremental\":" << (r.incremental ? "true" : "false");
            j << "}";
        }
        j << "],";

        // last 8 log entries
        j << "\"log\":[";
        int start = std::max(0, (int)log.size() - 8);
        for (int i = start; i < (int)log.size(); i++) {
            if (i > start) j << ",";
            // escape quotes in log string
            std::string entry = log[i];
            std::string escaped;
            for (char c : entry) {
                if (c == '"') escaped += "\\\"";
                else if (c == '\\') escaped += "\\\\";
                else escaped += c;
            }
            j << "\"" << escaped << "\"";
        }
        j << "]}";
        return j.str();
    }

    void handle_client(int client_fd) {
        char buf[4096] = {};
        read(client_fd, buf, sizeof(buf)-1);
        std::string req(buf);

        std::string body;
        std::string content_type;

        if (req.find("GET /api/status") != std::string::npos) {
            body = build_json();
            content_type = "application/json";
        } else {
            // serve a redirect hint
            body = "{\"error\":\"use /api/status\"}";
            content_type = "application/json";
        }

        std::string response =
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: " + content_type + "\r\n"
            "Access-Control-Allow-Origin: *\r\n"
            "Connection: close\r\n"
            "Content-Length: " + std::to_string(body.size()) + "\r\n"
            "\r\n" + body;

        write(client_fd, response.c_str(), response.size());
        close(client_fd);
    }

    void serve_loop() {
        int server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd < 0) return;

        int opt = 1;
        setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        sockaddr_in addr{};
        addr.sin_family      = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port        = htons(port_);

        if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
            close(server_fd); return;
        }
        listen(server_fd, 10);

        // non-blocking accept loop
        fd_set fds;
        timeval tv;
        while (running_) {
            FD_ZERO(&fds);
            FD_SET(server_fd, &fds);
            tv = {0, 200000}; // 200ms timeout
            if (select(server_fd+1, &fds, nullptr, nullptr, &tv) > 0) {
                int client_fd = accept(server_fd, nullptr, nullptr);
                if (client_fd >= 0) {
                    std::thread(&WebServer::handle_client, this, client_fd).detach();
                }
            }
        }
        close(server_fd);
    }
};
