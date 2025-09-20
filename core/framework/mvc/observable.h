#pragma once

#include <algorithm>
#include <functional>
#include <mutex>
#include <vector>

namespace ethercat_sim::framework
{

// Generic observer pattern implementation
template <typename T> class Observable
{
  public:
    using Observer   = std::function<void(const T&)>;
    using ObserverId = size_t;

    // Add observer and return ID for removal
    ObserverId addObserver(Observer observer)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        observers_.emplace_back(next_id_++, std::move(observer));
        return next_id_ - 1;
    }

    // Remove observer by ID
    bool removeObserver(ObserverId id)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        auto it = std::find_if(observers_.begin(), observers_.end(),
                               [id](const auto& pair) { return pair.first == id; });

        if (it != observers_.end())
        {
            observers_.erase(it);
            return true;
        }
        return false;
    }

    // Remove all observers
    void clearObservers()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        observers_.clear();
    }

    // Get number of observers
    size_t observerCount() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return observers_.size();
    }

  protected:
    // Notify all observers
    void notifyObservers(const T& data)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        for (const auto& [id, observer] : observers_)
        {
            try
            {
                observer(data);
            }
            catch (...)
            {
                // Ignore observer exceptions to prevent one bad observer
                // from breaking others
            }
        }
    }

  private:
    mutable std::mutex mutex_;
    std::vector<std::pair<ObserverId, Observer>> observers_;
    ObserverId next_id_{0};
};

} // namespace ethercat_sim::framework