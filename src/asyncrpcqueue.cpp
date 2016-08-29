// Copyright (c) 2016 The Zcash developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "asyncrpcqueue.h"

static std::atomic<size_t> workerCounter(0);

AsyncRPCQueue::AsyncRPCQueue() : closed_(false) {
}

AsyncRPCQueue::~AsyncRPCQueue() {
    closeAndWait();     // join on all worker threads
}

/**
 * A worker will execute this method on a new thread
 */
void AsyncRPCQueue::run(size_t workerId) {

    while (!isClosed()) {
        AsyncRPCOperationId key;
        std::shared_ptr<AsyncRPCOperation> operation;
        {
            std::unique_lock< std::mutex > guard(lock_);
            while (operation_id_queue_.empty() && !isClosed()) {
                this->condition_.wait(guard);
            }

            // Exit if the queue is closing.
            if (isClosed()) {
                break;
            }

            // Get operation id
            key = operation_id_queue_.front();
            operation_id_queue_.pop();

            // Search operation map
            AsyncRPCOperationMap::const_iterator iter = operation_map_.find(key);
            if (iter != operation_map_.end()) {
                operation = iter->second;
            }
        }

        if (!operation) {
            // cannot find operation in map, may have been removed
        } else if (operation->isCancelled()) {
            // skip cancelled operation
        } else {
            operation->main();
        }
    }
}


/**
 * Add shared_ptr to operation.
 *
 * To retain polymorphic behaviour, i.e. main() method of derived classes is invoked,
 * caller should create the shared_ptr like this:
 *
 * std::shared_ptr<AsyncRPCOperation> ptr(new MyCustomAsyncRPCOperation(params));
 *
 * Don't use std::make_shared<AsyncRPCOperation>().
 */
void AsyncRPCQueue::addOperation(const std::shared_ptr<AsyncRPCOperation> &ptrOperation) {

    // Don't add if queue is closed
    if (isClosed()) {
        return;
    }

    AsyncRPCOperationId id = ptrOperation->getId();
    std::lock_guard< std::mutex > guard(lock_);
    operation_map_.emplace(id, ptrOperation);
    operation_id_queue_.push(id);
    this->condition_.notify_one();
}

/**
 * Return the operation for a given operation id.
 */
std::shared_ptr<AsyncRPCOperation> AsyncRPCQueue::getOperationForId(AsyncRPCOperationId id) const {
    std::shared_ptr<AsyncRPCOperation> ptr;

    std::lock_guard< std::mutex > guard(lock_);
    AsyncRPCOperationMap::const_iterator iter = operation_map_.find(id);
    if (iter != operation_map_.end()) {
        ptr = iter->second;
    }
    return ptr;
}

/**
 * Return the operation for a given operation id and then remove the operation from internal storage.
 */
std::shared_ptr<AsyncRPCOperation> AsyncRPCQueue::popOperationForId(AsyncRPCOperationId id) {
    std::shared_ptr<AsyncRPCOperation> ptr = getOperationForId(id);
    if (ptr) {
        std::lock_guard< std::mutex > guard(lock_);
        // Note: if the id still exists in the operationIdQueue, when it gets processed by a worker
        // there will no operation in the map to execute, so nothing will happen.
        operation_map_.erase(id);
    }
    return ptr;
}

/**
 * Return true if the queue is closed to new operations.
 */
bool AsyncRPCQueue::isClosed() const {
    return closed_;
}

/**
 * Close the queue and cancel all existing operations
 */
void AsyncRPCQueue::close() {
    this->closed_ = true;
    cancelAllOperations();
}

/**
 *  Call cancel() on all operations
 */
void AsyncRPCQueue::cancelAllOperations() {
    std::unique_lock< std::mutex > guard(lock_);
    for (auto key : operation_map_) {
        key.second->cancel();
    }
    this->condition_.notify_all();
}

/**
 * Return the number of operations in the queue
 */
size_t AsyncRPCQueue::getOperationCount() const {
    std::unique_lock< std::mutex > guard(lock_);
    return operation_id_queue_.size();
}

/**
 * Spawn a worker thread
 */
void AsyncRPCQueue::addWorker() {
    std::unique_lock< std::mutex > guard(lock_); // Todo: could just have a lock on the vector
    workers_.emplace_back( std::thread(&AsyncRPCQueue::run, this, ++workerCounter) );
}

/**
 * Return the number of worker threads spawned by the queue
 */
size_t AsyncRPCQueue::getNumberOfWorkers() const {
    return workers_.size();
}

/**
 * Return a list of all known operation ids found in internal storage.
 */
std::vector<AsyncRPCOperationId> AsyncRPCQueue::getAllOperationIds() const {
    std::unique_lock< std::mutex > guard(lock_);
    std::vector<AsyncRPCOperationId> v;
    for(auto & entry: operation_map_) {
        v.push_back(entry.first);
    }
    return v;
}

/**
 * Calling thread will close and wait for worker threads to join.
 */
void AsyncRPCQueue::closeAndWait() {
    if (!this->closed_) {
        close();
    }
    for (std::thread & t : this->workers_) {
        if (t.joinable()) {
            t.join();
        }
    }
}
