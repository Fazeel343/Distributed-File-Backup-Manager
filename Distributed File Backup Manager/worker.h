#pragma once
#include "utils.h"
#include "request_queue.h"
#include "metadata_manager.h"
#include "storage_controller.h"
#include "recovery.h"

class Worker {
public:
    Worker(int id,
           RequestQueue&      queue,
           MetadataManager&   meta,
           StorageController& storage,
           RecoveryModule&    recovery);

    void run();   // thread entry point

private:
    int                id_;
    RequestQueue&      queue_;
    MetadataManager&   meta_;
    StorageController& storage_;
    RecoveryModule&    recovery_;
};