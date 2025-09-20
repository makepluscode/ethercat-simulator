// FTXUI
#include <ftxui/component/component.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>

// STD
#include <atomic>
#include <csignal>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

// Project
#include "ethercat_sim/communication/dds_text.h"

// FastDDS
#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/rtps/transport/shared_mem/SharedMemTransportDescriptor.hpp>

namespace
{
std::atomic<bool> g_running{true};

void handle_signal(int) noexcept
{
    g_running.store(false, std::memory_order_relaxed);
}
} // namespace

int main()
{
    // Smoke-test mode for CI/CTest: render once and exit
    if (std::getenv("TUI_SMOKE_TEST"))
    {
        auto doc = ftxui::vbox({ftxui::text("EtherCAT Simulator") | ftxui::bold |
                                    ftxui::color(ftxui::Color::Green),
                                ftxui::separator(), ftxui::text("TUI smoke test")}) |
                   ftxui::border;
        auto screen = ftxui::Screen::Create(ftxui::Dimension::Fit(doc));
        ftxui::Render(screen, doc);
        std::cout << screen.ToString() << std::endl;
        return 0;
    }

    // Install signal handlers for Ctrl+C / Ctrl+Z / SIGTERM
    std::signal(SIGINT, handle_signal);
    std::signal(SIGTSTP, handle_signal);
    std::signal(SIGTERM, handle_signal);

    // Shared state for received messages
    std::mutex mtx;
    std::vector<std::string> messages;
    const std::size_t max_messages = 100;

    // FastDDS setup
    eprosima::fastdds::dds::DomainParticipant* participant = nullptr;
    {
        eprosima::fastdds::dds::DomainParticipantQos qos =
            eprosima::fastdds::dds::PARTICIPANT_QOS_DEFAULT;
        qos.transport().use_builtin_transports = false;
        qos.transport().user_transports.push_back(
            std::make_shared<eprosima::fastdds::rtps::SharedMemTransportDescriptor>());
        participant =
            eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->create_participant(
                0, qos);
    }
    if (!participant)
    {
        participant =
            eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->create_participant(
                0, eprosima::fastdds::dds::PARTICIPANT_QOS_DEFAULT);
    }

    eprosima::fastdds::dds::Subscriber* subscriber = nullptr;
    eprosima::fastdds::dds::Topic* topic           = nullptr;
    eprosima::fastdds::dds::DataReader* reader     = nullptr;
    if (participant)
    {
        eprosima::fastdds::dds::TypeSupport type(
            new ethercat_sim::communication::TextMsgPubSubType());
        type.register_type(participant);
        topic = participant->create_topic("sim_text", type.get_type_name(),
                                          eprosima::fastdds::dds::TOPIC_QOS_DEFAULT);
        subscriber =
            participant->create_subscriber(eprosima::fastdds::dds::SUBSCRIBER_QOS_DEFAULT, nullptr);
        reader =
            subscriber->create_datareader(topic, eprosima::fastdds::dds::DATAREADER_QOS_DEFAULT);
    }

    // Create screen now so background threads can notify redraw.
    ftxui::ScreenInteractive screen = ftxui::ScreenInteractive::TerminalOutput();

    // DDS polling thread (append received messages)
    std::thread dds_thread;
    if (reader)
    {
        dds_thread = std::thread(
            [&]()
            {
                while (g_running.load(std::memory_order_relaxed))
                {
                    ethercat_sim::communication::TextMsg msg;
                    eprosima::fastdds::dds::SampleInfo info;
                    auto rc = reader->take_next_sample(&msg, &info);
                    if (rc == eprosima::fastdds::dds::RETCODE_OK &&
                        info.instance_state == eprosima::fastdds::dds::ALIVE_INSTANCE_STATE)
                    {
                        {
                            std::lock_guard<std::mutex> lk(mtx);
                            messages.emplace_back(msg.text);
                            if (messages.size() > max_messages)
                            {
                                messages.erase(messages.begin(),
                                               messages.begin() + (messages.size() - max_messages));
                            }
                        }
                        screen.PostEvent(ftxui::Event::Custom);
                    }
                    else
                    {
                        std::this_thread::sleep_for(std::chrono::milliseconds(50));
                    }
                }
            });
    }

    // UI component
    auto component = ftxui::Renderer(
        [&]
        {
            std::vector<ftxui::Element> lines;
            {
                std::lock_guard<std::mutex> lk(mtx);
                lines.reserve(messages.size());
                for (const auto& s : messages)
                {
                    lines.push_back(ftxui::text(s) | ftxui::color(ftxui::Color::Yellow));
                }
            }
            if (lines.empty())
            {
                lines.push_back(ftxui::text("Waiting for DDS messages on 'sim_text'...") |
                                ftxui::dim);
            }
            auto content = ftxui::vbox({
                ftxui::text("EtherCAT Simulator") | ftxui::bold | ftxui::color(ftxui::Color::Green),
                ftxui::separator(),
                ftxui::text("Press ESC/Ctrl+C/Ctrl+Z to exit") | ftxui::dim,
                ftxui::separator(),
                ftxui::vbox(std::move(lines)) | ftxui::yframe,
            });
            return content | ftxui::border;
        });

    component = ftxui::CatchEvent(component,
                                  [&](ftxui::Event ev)
                                  {
                                      if (ev == ftxui::Event::Escape)
                                      {
                                          g_running.store(false, std::memory_order_relaxed);
                                          screen.Exit();
                                          return true;
                                      }
                                      return false;
                                  });

    // Background thread to exit the UI loop when a signal is caught.
    std::thread exit_watcher(
        [&]()
        {
            while (g_running.load(std::memory_order_relaxed))
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
            // Wake the UI and exit.
            screen.PostEvent(ftxui::Event::Custom);
            screen.Exit();
        });

    // Run UI loop until exit is requested
    screen.Loop(component);

    // Cleanup threads and DDS
    g_running.store(false, std::memory_order_relaxed);
    if (exit_watcher.joinable())
        exit_watcher.join();
    if (dds_thread.joinable())
        dds_thread.join();

    if (subscriber && reader)
    {
        subscriber->delete_datareader(reader);
    }
    if (participant)
    {
        if (subscriber)
            participant->delete_subscriber(subscriber);
        if (topic)
            participant->delete_topic(topic);
        eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->delete_participant(
            participant);
    }
    return 0;
}
