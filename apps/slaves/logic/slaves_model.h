#pragma once

#include <mutex>
#include <string>

namespace ethercat_sim::app::slaves {

struct SlavesSnapshot {
    std::string endpoint;
    int count {1};
    bool listening {false};
    bool connected {false};
    std::string status;
};

class SlavesModel {
public:
    void setEndpoint(std::string v) { std::lock_guard<std::mutex> l(m_); snap_.endpoint = std::move(v); }
    void setCount(int n) { std::lock_guard<std::mutex> l(m_); snap_.count = n; }
    void setListening(bool v) { std::lock_guard<std::mutex> l(m_); snap_.listening = v; }
    void setConnected(bool v) { std::lock_guard<std::mutex> l(m_); snap_.connected = v; }
    void setStatus(std::string s) { std::lock_guard<std::mutex> l(m_); snap_.status = std::move(s); }
    SlavesSnapshot snapshot() const { std::lock_guard<std::mutex> l(m_); return snap_; }

private:
    mutable std::mutex m_;
    SlavesSnapshot snap_{};
};

} // namespace ethercat_sim::app::slaves

