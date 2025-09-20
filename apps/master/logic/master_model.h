#pragma once

#include <mutex>
#include <string>
#include <vector>

namespace ethercat_sim::app::master {

struct SlavesRow {
    int address {0};
    std::string state;
    std::string al_code;
};

struct MasterSnapshot {
    int detected_slaves {0};
    bool preop {false};
    bool operational {false};
    std::string status;
    std::vector<SlavesRow> slaves;
    int selected_slaves {0};
    std::string sdo_status;
    std::string sdo_value_hex;
};

class MasterModel {
public:
    void setDetectedSlaves(int n) { std::lock_guard<std::mutex> l(m_); snap_.detected_slaves = n; }
    void setPreop(bool v) { std::lock_guard<std::mutex> l(m_); snap_.preop = v; }
    void setOperational(bool v) { std::lock_guard<std::mutex> l(m_); snap_.operational = v; }
    void setStatus(std::string s) { std::lock_guard<std::mutex> l(m_); snap_.status = std::move(s); }

    void setSlaves(std::vector<SlavesRow> rows) { std::lock_guard<std::mutex> l(m_); snap_.slaves = std::move(rows); }
    void setSelectedSlaves(int i) { std::lock_guard<std::mutex> l(m_); snap_.selected_slaves = i; }
    void setSdoStatus(std::string s) { std::lock_guard<std::mutex> l(m_); snap_.sdo_status = std::move(s); }
    void setSdoValueHex(std::string v) { std::lock_guard<std::mutex> l(m_); snap_.sdo_value_hex = std::move(v); }
    MasterSnapshot snapshot() const { std::lock_guard<std::mutex> l(m_); return snap_; }

private:
    mutable std::mutex m_;
    MasterSnapshot snap_{};
};

} // namespace ethercat_sim::app::master
