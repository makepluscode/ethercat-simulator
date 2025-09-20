#include <chrono>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/rtps/transport/shared_mem/SharedMemTransportDescriptor.hpp>
#include <iostream>
#include <thread>

#include "ethercat_sim/communication/dds_text.h"

using ethercat_sim::communication::TextMsg;
using ethercat_sim::communication::TextMsgPubSubType;

int main(int argc, char** argv) {
    const char* text = (argc > 1) ? argv[1] : "Hello from fastdds_sim_pub";

    eprosima::fastdds::dds::TypeSupport type(new TextMsgPubSubType());
    auto* factory = eprosima::fastdds::dds::DomainParticipantFactory::get_instance();
    // Prefer SHM-only transport; if not permitted, fall back to builtin (UDP)
    eprosima::fastdds::dds::DomainParticipantQos qos =
        eprosima::fastdds::dds::PARTICIPANT_QOS_DEFAULT;
    qos.transport().use_builtin_transports = false;
    qos.transport().user_transports.push_back(
        std::make_shared<eprosima::fastdds::rtps::SharedMemTransportDescriptor>());
    auto* participant = factory->create_participant(0, qos);
    if (!participant) {
        std::cerr << "SHM transport unavailable; falling back to builtin UDP" << std::endl;
        eprosima::fastdds::dds::DomainParticipantQos qos_fallback =
            eprosima::fastdds::dds::PARTICIPANT_QOS_DEFAULT;
        qos_fallback.transport().use_builtin_transports = true; // UDPv4 defaults
        participant = factory->create_participant(0, qos_fallback);
    }
    if (!participant) {
        std::cerr << "participant create failed" << std::endl;
        return 1;
    }
    type.register_type(participant);

    auto* topic = participant->create_topic("sim_text", type.get_type_name(),
                                            eprosima::fastdds::dds::TOPIC_QOS_DEFAULT);
    auto* pub =
        participant->create_publisher(eprosima::fastdds::dds::PUBLISHER_QOS_DEFAULT, nullptr);
    auto* writer =
        pub->create_datawriter(topic, eprosima::fastdds::dds::DATAWRITER_QOS_DEFAULT, nullptr);

    // Publish a few messages to increase chance of reception
    for (int i = 0; i < 5; ++i) {
        TextMsg msg;
        msg.text = text;
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
