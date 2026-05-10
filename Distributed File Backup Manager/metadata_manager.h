#pragma once
#include <map>
#include <vector>
#include <mutex>
#include <string>
#include "utils.h"

class MetadataManager {
public:
    // Called by workers to update request state
    void register_request(const BackupRequest& req);
    void update_status(int request_id, const std::string& status, int chunks_done = -1);

    // Incremental: returns true if file has changed since last backup
    bool is_modified(const BackupRequest& req);
    void record_backup_hash(const BackupRequest& req);

    // Dashboard reads
    std::vector<BackupRequest> get_all() const;
    std::vector<std::string>   get_log()  const;
    int count_by_status(const std::string& status) const;

    void add_log(const std::string& msg);

private:
    mutable std::mutex                  mtx_;
    std::map<int, BackupRequest>        index_;       // request_id -> request
    std::map<std::string, long>         last_hash_;   // file_name  -> last size
    std::vector<std::string>            log_;
};