#pragma once
#include "utils.h"
#include "request_queue.h"
#include "metadata_manager.h"

class Client {
public:
    Client(int id, RequestQueue& queue, MetadataManager& meta);
    void run();   // called as a thread entry point

private:
    int               id_;
    RequestQueue&     queue_;
    MetadataManager&  meta_;
    int               next_request_id_;   // global counter assigned in main
};