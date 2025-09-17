#pragma once

#include <mutex>
#include <string>
#include <vector>

namespace ethercat_sim::app::main {

struct SubsRow {
    int address {0};
    std::string state;
    std::string al_code;
};

struct MainSnapshot {
    int detected_subs {0};
    bool preop {false};
    bool operational {false};
    std::string status;
    std::vector<SubsRow> subs;
    int selected_subs {0};
    std::string sdo_status;
    std::string sdo_value_hex;
};

class MainModel {
public:
    void setDetectedSubs(int n) { std::lock_guard<std::mutex> l(m_); snap_.detected_subs = n; }
    void setPreop(bool v) { std::lock_guard<std::mutex> l(m_); snap_.preop = v; }
    void setOperational(bool v) { std::lock_guard<std::mutex> l(m_); snap_.operational = v; }
    void setStatus(std::string s) { std::lock_guard<std::mutex> l(m_); snap_.status = std::move(s); }

    void setSubs(std::vector<SubsRow> rows) { std::lock_guard<std::mutex> l(m_); snap_.subs = std::move(rows); }
    void setSelectedSubs(int i) { std::lock_guard<std::mutex> l(m_); snap_.selected_subs = i; }
    void setSdoStatus(std::string s) { std::lock_guard<std::mutex> l(m_); snap_.sdo_status = std::move(s); }
    void setSdoValueHex(std::string v) { std::lock_guard<std::mutex> l(m_); snap_.sdo_value_hex = std::move(v); }
    MainSnapshot snapshot() const { std::lock_guard<std::mutex> l(m_); return snap_; }

private:
    mutable std::mutex m_;
    MainSnapshot snap_{};
};

} // namespace ethercat_sim::app::main
