#include "ethercat_sim/communication/dds_text.h"
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <chrono>
#include <iostream>
#include <thread>

using ethercat_sim::communication::TextMsg;
using ethercat_sim::communication::TextMsgPubSubType;

int main(int argc, char** argv)
{
    const char* text = (argc > 1) ? argv[1] : "Hello from fastdds_sim_pub";

    eprosima::fastdds::dds::TypeSupport type(new TextMsgPubSubType());
    auto* factory = eprosima::fastdds::dds::DomainParticipantFactory::get_instance();
    auto* participant = factory->create_participant(0, eprosima::fastdds::dds::PARTICIPANT_QOS_DEFAULT);
    if (!participant) { std::cerr << "participant create failed" << std::endl; return 1; }
    type.register_type(participant);

    auto* topic = participant->create_topic("sim_text", type->getName(), eprosima::fastdds::dds::TOPIC_QOS_DEFAULT);
    auto* pub = participant->create_publisher(eprosima::fastdds::dds::PUBLISHER_QOS_DEFAULT, nullptr);
    auto* writer = pub->create_datawriter(topic, eprosima::fastdds::dds::DATAWRITER_QOS_DEFAULT, nullptr);

    // Publish a few messages to increase chance of reception
    for (int i = 0; i < 5; ++i) {
        TextMsg msg; msg.text = text;
        writer->write(&msg);
        std::cout << "Published: " << msg.text << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }

    pub->delete_datawriter(writer);
    participant->delete_publisher(pub);
    participant->delete_topic(topic);
    factory->delete_participant(participant);
    return 0;
}
