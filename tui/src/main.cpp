#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>
#include <iostream>
#ifdef ETHERCAT_HAVE_FASTDDS
#include "ethercat_sim/communication/dds_text.h"
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <thread>
#include <atomic>
using namespace eprosima::fastdds::dds;
using ethercat_sim::communication::TextMsg;
using ethercat_sim::communication::TextMsgPubSubType;
#endif

int main()
{
    using namespace ftxui;
    auto doc = vbox({
        text("EtherCAT Simulator") | bold | color(Color::Green),
        separator(),
        text("Hello from TUI (FTXUI)!")
    }) | border;

#ifdef ETHERCAT_HAVE_FASTDDS
    std::atomic<bool> got{false};
    std::string last;

    TypeSupport type(new TextMsgPubSubType());
    DomainParticipant* participant = DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    if (participant)
    {
        type.register_type(participant);
        Topic* topic = participant->create_topic("sim_text", type->getName(), TOPIC_QOS_DEFAULT);
        Subscriber* sub = participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT, nullptr);
        DataReader* reader = sub->create_datareader(topic, DATAREADER_QOS_DEFAULT);

        // Poll for up to ~2s
        for (int i = 0; i < 20 && !got; ++i)
        {
            TextMsg msg; SampleInfo info;
            if (reader->take_next_sample(&msg, &info) == ReturnCode_t::RETCODE_OK && info.instance_state == ALIVE_INSTANCE_STATE)
            {
                got = true; last = msg.text; break;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        sub->delete_datareader(reader);
        participant->delete_subscriber(sub);
        participant->delete_topic(topic);
        DomainParticipantFactory::get_instance()->delete_participant(participant);
    }

    if (got)
    {
        doc = vbox({
            text("EtherCAT Simulator") | bold | color(Color::Green),
            separator(),
            text("DDS received: ") | bold,
            text(last) | color(Color::Yellow)
        }) | border;
    }
#endif

    auto screen = Screen::Create(Dimension::Fit(doc));
    Render(screen, doc);
    std::cout << screen.ToString() << std::endl;
    return 0;
}
