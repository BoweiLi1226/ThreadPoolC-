  #include "ThreadPool.hpp"

ThreadPool::ThreadPool(size_t thread_count) : data_(std::make_shared<data>()) {
    for (size_t i = 0; i < thread_count; ++i) {
        std::thread([data = data_] {
            std::unique_lock<std::mutex> lk(data->mtx_);
            for (;;) {
                if (!data->tasks_.empty()) {
                auto current = std::move(data->tasks_.front());
                data->tasks_.pop();
                lk.unlock();
                current();
                lk.lock();
                } else if (data->is_shutdown_) {
                    break;
                } else {
                    data->cond_.wait(lk);
                }
            }
        }).detach();
    }
}

ThreadPool::~ThreadPool() {
        if ((bool) data_) {
        {
            std::lock_guard<std::mutex> lk(data_->mtx_);
            data_->is_shutdown_ = true;
        }
        data_->cond_.notify_all();
        }
    }