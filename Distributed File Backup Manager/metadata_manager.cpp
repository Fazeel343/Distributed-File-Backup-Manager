#include "metadata_manager.h"

void MetadataManager::register_request(const BackupRequest& req) {
    std::lock_guard<std::mutex> lock(mtx_);
    index_[req.request_id] = req;
    log_.push_back("[" + now_str() + "] Registered: " + req.file_name +
                   " (client " + std::to_string(req.client_id) + ")");
}

void MetadataManager::update_status(int request_id,
                                     const std::string& status,
                                     int chunks_done) {
    std::lock_guard<std::mutex> lock(mtx_);
    auto it = index_.find(request_id);
    if (it == index_.end()) return;

    it->second.status = status;
    if (chunks_done >= 0)
        it->second.chunks_done = chunks_done;

    log_.push_back("[" + now_str() + "] " +
                   it->second.file_name + " -> " + status);
}

bool MetadataManager::is_modified(const BackupRequest& req) {
    std::lock_guard<std::mutex> lock(mtx_);
    auto it = last_hash_.find(req.file_name);
    if (it == last_hash_.end()) return true;       // never backed up
    return it->second != req.file_size_mb;         // size changed
}

void MetadataManager::record_backup_hash(const BackupRequest& req) {
    std::lock_guard<std::mutex> lock(mtx_);
    last_hash_[req.file_name] = req.file_size_mb;
}

std::vector<BackupRequest> MetadataManager::get_all() const {
    std::lock_guard<std::mutex> lock(mtx_);
    std::vector<BackupRequest> result;
    for (auto& kv : index_) result.push_back(kv.second);
    return result;
}

std::vector<std::string> MetadataManager::get_log() const {
    std::lock_guard<std::mutex> lock(mtx_);
    return log_;
}

int MetadataManager::count_by_status(const std::string& status) const {
    std::lock_guard<std::mutex> lock(mtx_);
    int count = 0;
    for (auto& kv : index_)
        if (kv.second.status == status) count++;
    return count;
}

void MetadataManager::add_log(const std::string& msg) {
    std::lock_guard<std::mutex> lock(mtx_);
    log_.push_back("[" + now_str() + "] " + msg);
}