#include "ethercat_sim/communication/dds_text.h"

#ifdef ETHERCAT_HAVE_FASTDDS
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <iostream>

using namespace eprosima::fastdds::dds;
using ethercat_sim::communication::TextMsg;
using ethercat_sim::communication::TextMsgPubSubType;

int main()
{
    TypeSupport type(new TextMsgPubSubType());
    auto* participant = DomainParticipantFactory::get_instance()->create_participant(0, PARTICIPANT_QOS_DEFAULT);
    if (!participant) { std::cerr << "participant create failed" << std::endl; return 1; }
    type.register_type(participant);

    Topic* topic = participant->create_topic("sim_text", type->getName(), TOPIC_QOS_DEFAULT);
    Publisher* pub = participant->create_publisher(PUBLISHER_QOS_DEFAULT, nullptr);
    DataWriter* writer = pub->create_datawriter(topic, DATAWRITER_QOS_DEFAULT, nullptr);

    TextMsg msg;
    msg.text = "Hello from simulator";
    writer->write(&msg);
    std::cout << "Published: " << msg.text << std::endl;

    pub->delete_datawriter(writer);
    participant->delete_publisher(pub);
    participant->delete_topic(topic);
    DomainParticipantFactory::get_instance()->delete_participant(participant);
    return 0;
}
#else
int main(){return 0;}
#endif

