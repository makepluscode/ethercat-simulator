#include "ethercat_sim/communication/dds_text.h"

#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/rtps/transport/shared_mem/SharedMemTransportDescriptor.h>
#include <iostream>

using ethercat_sim::communication::TextMsg;
using ethercat_sim::communication::TextMsgPubSubType;

int main()
{
    eprosima::fastdds::dds::TypeSupport type(new TextMsgPubSubType());
    // SHM-only transport (no UDP) for sandbox friendliness
    auto* factory = eprosima::fastdds::dds::DomainParticipantFactory::get_instance();
    eprosima::fastdds::dds::DomainParticipantQos qos = eprosima::fastdds::dds::PARTICIPANT_QOS_DEFAULT;
    qos.transport().use_builtin_transports = false;
    qos.transport().user_transports.push_back(std::make_shared<eprosima::fastdds::rtps::SharedMemTransportDescriptor>());
    auto* participant = factory->create_participant(0, qos);
    if (!participant) { std::cerr << "participant create failed" << std::endl; return 1; }
    type.register_type(participant);

    eprosima::fastdds::dds::Topic* topic = participant->create_topic("sim_text", type->getName(), eprosima::fastdds::dds::TOPIC_QOS_DEFAULT);
    eprosima::fastdds::dds::Publisher* pub = participant->create_publisher(eprosima::fastdds::dds::PUBLISHER_QOS_DEFAULT, nullptr);
    eprosima::fastdds::dds::DataWriter* writer = pub->create_datawriter(topic, eprosima::fastdds::dds::DATAWRITER_QOS_DEFAULT, nullptr);

    TextMsg msg;
    msg.text = "Hello from simulator";
    writer->write(&msg);
    std::cout << "Published: " << msg.text << std::endl;

    pub->delete_datawriter(writer);
    participant->delete_publisher(pub);
    participant->delete_topic(topic);
    eprosima::fastdds::dds::DomainParticipantFactory::get_instance()->delete_participant(participant);
    return 0;
}
