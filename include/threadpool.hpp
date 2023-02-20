#pragma once

#include <deque>
#include <thread>
#include <future>
#include <condition_variable>
#include <vector>

class ThreadPool {
    struct F_BASE {
        virtual void call() = 0;
        virtual ~F_BASE()   = default;
    };
    std::vector<std::thread> threads;
    std::deque<std::unique_ptr<F_BASE>> tasks_queue;

    std::condition_variable update;
    std::mutex tasks_mtx;

    std::atomic_bool stop_flag = false;

    void thread_func() {

        while (1) {
            std::unique_lock lock(tasks_mtx);
            update.wait(lock,
                        [this]() { return stop_flag || !tasks_queue.empty(); });
            if (stop_flag) return;

            auto task = std::move(tasks_queue.front());
            tasks_queue.pop_front();
            lock.unlock();
            task->call();
        }
    }

public:
    template<class F, class... As>
    std::future<std::invoke_result_t<F, As...>> queue(F&& f, As&&... as) {
        using R = std::invoke_result_t<F, As...>;

        std::promise<R> pr;
        auto ft = pr.get_future();

        struct FUNC: F_BASE {
            std::promise<R> pr_;
            std::decay_t<F> f_;
            std::tuple<std::decay_t<As>...> as_;

            void call() override {
                try {
                    if constexpr (std::is_void_v<R>) {
                        std::apply(std::move(f_), std::move(as_));
                        pr_.set_value();
                    } else {
                        pr_.set_value(
                            std::apply(std::move(f_), std::move(as_)));
                    }
                } catch (...) { pr_.set_exception(std::current_exception()); }
            }

            FUNC(std::promise<R>&& pr, F&& f, As&&... as)
                : pr_(std::move(pr)), f_(std::forward<F>(f)),
                  as_(std::forward_as_tuple(std::forward<As>(as)...)) {}
        };

        auto func = std::make_unique<FUNC>(std::move(pr), std::forward<F>(f),
                                           std::forward<As>(as)...);

        {
            std::lock_guard g(tasks_mtx);
            tasks_queue.push_back(std::move(func));
        }
        update.notify_one();

        return ft;
    }
    ~ThreadPool() {
        stop_flag = true;
        update.notify_all();
        for (auto& t : threads) t.join();
    }
    explicit ThreadPool(int n) {
        threads.reserve(n);
        for (int i = 0; i < n; ++i)
            threads.push_back(std::thread(&ThreadPool::thread_func, this));
    }
    explicit ThreadPool(): ThreadPool(std::thread::hardware_concurrency()) {}
};
