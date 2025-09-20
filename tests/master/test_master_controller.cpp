#include <atomic>
#include <chrono>
#include <filesystem>
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <thread>

#include "bus/slaves_endpoint.h"
#include "logic/master_controller.h"
#include "logic/master_model.h"

using namespace std::chrono_literals;

namespace {
std::string unique_socket_path() {
    auto                  pid = static_cast<long>(::getpid());
    auto                  now = std::chrono::steady_clock::now().time_since_epoch().count();
    std::filesystem::path p =
        std::filesystem::temp_directory_path() /
        ("ethercat_test_" + std::to_string(pid) + "_" + std::to_string(now) + ".sock");
    return p.string();
}

class MasterSlaveFixture : public ::testing::Test {
  protected:
    void SetUp() override {
        socket_path_  = unique_socket_path();
        endpoint_uri_ = std::string("uds://") + socket_path_;
        stop_flag_.store(false);
        slave_ = std::make_unique<ethercat_sim::bus::SlavesEndpoint>(endpoint_uri_);
        slave_->setSlavesCount(1);
        slave_->setStopFlag(&stop_flag_);
        slave_thread_ = std::thread([this] {
            bool ok = slave_->run();
            slave_result_.store(ok);
        });

        // Wait for socket file to exist so master can connect
        for (int i = 0; i < 50; ++i) {
            if (std::filesystem::exists(socket_path_)) {
                break;
            }
            std::this_thread::sleep_for(20ms);
        }
    }

    void TearDown() override {
        stop_flag_.store(true);
        if (slave_thread_.joinable()) {
            slave_thread_.join();
        }
        std::error_code ec;
        std::filesystem::remove(socket_path_, ec);
        EXPECT_TRUE(slave_result_.load()) << "Slave endpoint run() returned false";
    }

    std::shared_ptr<ethercat_sim::app::master::MasterController> makeController() {
        auto controller =
            std::make_shared<ethercat_sim::app::master::MasterController>(endpoint_uri_, 1000);
        controller->start();
        std::this_thread::sleep_for(50ms);
        return controller;
    }

    std::string                                        socket_path_;
    std::string                                        endpoint_uri_;
    std::unique_ptr<ethercat_sim::bus::SlavesEndpoint> slave_;
    std::thread                                        slave_thread_;
    std::atomic_bool                                   stop_flag_{false};
    std::atomic_bool                                   slave_result_{false};
};

} // namespace

TEST_F(MasterSlaveFixture, ScanDetectsSingleSlave) {
    auto controller = makeController();

    controller->scan();
    auto snap = controller->model()->snapshot();
    EXPECT_EQ(1, snap.detected_slaves);
    EXPECT_EQ("scan ok", snap.status);

    controller->stop();
}

TEST_F(MasterSlaveFixture, InitPreopTransitionsToPreop) {
    auto controller = makeController();

    controller->initPreop();
    auto snap = controller->model()->snapshot();
    EXPECT_EQ(1, snap.detected_slaves);
    EXPECT_TRUE(snap.preop);
    EXPECT_EQ("preop ok", snap.status);

    controller->stop();
}
