#pragma once

#include <mutex>
#include <memory>
#include <functional>
#include <vector>

namespace ethercat_sim::framework {

// Observer pattern for model updates
template<typename T>
class Observable {
public:
    using Observer = std::function<void(const T&)>;
    
    void addObserver(Observer observer) {
        std::lock_guard<std::mutex> lock(mutex_);
        observers_.push_back(std::move(observer));
    }
    
    void removeObserver(const Observer& observer) {
        std::lock_guard<std::mutex> lock(mutex_);
        observers_.erase(
            std::remove(observers_.begin(), observers_.end(), observer),
            observers_.end()
        );
    }
    
protected:
    void notifyObservers(const T& data) {
        std::lock_guard<std::mutex> lock(mutex_);
        for (const auto& observer : observers_) {
            observer(data);
        }
    }

private:
    mutable std::mutex mutex_;
    std::vector<Observer> observers_;
};

// Base model class with thread-safe data access
template<typename DataType>
class BaseModel : public Observable<DataType> {
public:
    using Data = DataType;
    
    // Get current data snapshot
    Data snapshot() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return data_;
    }
    
    // Update data and notify observers
    void updateData(const Data& newData) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            data_ = newData;
        }
        this->notifyObservers(data_);
    }
    
    // Update data without notifying observers
    void setData(const Data& newData) {
        std::lock_guard<std::mutex> lock(mutex_);
        data_ = newData;
    }

protected:
    mutable std::mutex mutex_;
    Data data_{};
};

} // namespace ethercat_sim::framework