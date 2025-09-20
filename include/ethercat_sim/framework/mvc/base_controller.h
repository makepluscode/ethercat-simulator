#pragma once

#include <atomic>
#include <chrono>
#include <memory>
#include <thread>

namespace ethercat_sim::framework {

// Base controller class with common lifecycle management
template <typename ModelType> class BaseController {
  public:
    using Model = ModelType;

    BaseController() = default;
    virtual ~BaseController() {
        stop();
    }

    // Start the controller
    void start() {
        if (worker_thread_.joinable()) {
            return; // Already running
        }

        stop_flag_.store(false);
        worker_thread_ = std::thread(&BaseController::run, this);
    }

    // Stop the controller
    void stop() {
        stop_flag_.store(true);
        if (worker_thread_.joinable()) {
            worker_thread_.join();
        }
    }

    // Check if controller is running
    bool isRunning() const {
        return worker_thread_.joinable() && !stop_flag_.load();
    }

    // Get model reference
    std::shared_ptr<Model> model() const {
        return model_;
    }

  protected:
    // Main worker thread function - must be implemented by derived classes
    virtual void run() = 0;

    // Check if stop was requested
    bool shouldStop() const {
        return stop_flag_.load();
    }

    // Sleep for specified duration, respecting stop flag
    void sleepFor(std::chrono::milliseconds duration) {
        auto start = std::chrono::steady_clock::now();
        while (!stop_flag_.load()) {
            auto elapsed = std::chrono::steady_clock::now() - start;
            if (elapsed >= duration) {
                break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    // Model instance
    std::shared_ptr<Model> model_{std::make_shared<Model>()};

  private:
    std::atomic_bool stop_flag_{false};
    std::thread      worker_thread_;
};

} // namespace ethercat_sim::framework