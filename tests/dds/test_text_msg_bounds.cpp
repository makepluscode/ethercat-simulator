// GoogleTest
#include <gtest/gtest.h>

// Fast DDS
#include <fastdds/dds/core/ReturnCode.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/publisher/DataWriter.hpp>
#include <fastdds/dds/publisher/Publisher.hpp>
#include <fastdds/dds/subscriber/DataReader.hpp>
#include <fastdds/dds/subscriber/SampleInfo.hpp>
#include <fastdds/dds/subscriber/Subscriber.hpp>
#include <fastdds/dds/topic/Topic.hpp>
#include <fastdds/rtps/transport/shared_mem/SharedMemTransportDescriptor.hpp>

// STL
#include <chrono>
#include <string>
#include <thread>
#include <unistd.h>

// Project
#include "ethercat_sim/communication/dds_text.h"

using namespace std::chrono_literals;

namespace
{
std::string unique_topic()
{
    return std::string("sim_text_bounds_") + std::to_string(::getpid());
}
} // namespace

TEST(DdsTextBounds, LongMessage_Truncated_NotCrashing)
{
    using namespace eprosima::fastdds::dds;
    using ethercat_sim::communication::TextMsg;
    using ethercat_sim::communication::TextMsgPubSubType;

    TypeSupport type(new TextMsgPubSubType());

    DomainParticipantQos qos               = PARTICIPANT_QOS_DEFAULT;
    qos.transport().use_builtin_transports = false;
    qos.transport().user_transports.push_back(
        std::make_shared<eprosima::fastdds::rtps::SharedMemTransportDescriptor>());
    DomainParticipant* participant =
        DomainParticipantFactory::get_instance()->create_participant(0, qos);
    if (!participant)
    {
        GTEST_SKIP() << "SHM transport unavailable; skipping bounds test";
    }

    type.register_type(participant);

    auto topic_name = unique_topic();
    Topic* topic = participant->create_topic(topic_name, type.get_type_name(), TOPIC_QOS_DEFAULT);
    ASSERT_NE(topic, nullptr);
    Publisher* pub = participant->create_publisher(PUBLISHER_QOS_DEFAULT, nullptr);
    ASSERT_NE(pub, nullptr);
    DataWriter* writer = pub->create_datawriter(topic, DATAWRITER_QOS_DEFAULT, nullptr);
    ASSERT_NE(writer, nullptr);
    Subscriber* sub = participant->create_subscriber(SUBSCRIBER_QOS_DEFAULT, nullptr);
    ASSERT_NE(sub, nullptr);
    DataReader* reader = sub->create_datareader(topic, DATAREADER_QOS_DEFAULT);
    ASSERT_NE(reader, nullptr);

    std::this_thread::sleep_for(100ms);

    // Prepare a message longer than TEXTMSG_MAX_LEN
    std::string long_text(ethercat_sim::communication::TEXTMSG_MAX_LEN + 200, 'A');
    TextMsg msg;
    msg.text = long_text;

    auto rc = writer->write(&msg);
    EXPECT_EQ(rc, eprosima::fastdds::dds::RETCODE_OK);

    // Receive and verify it is truncated to TEXTMSG_MAX_LEN
    TextMsg received{};
    SampleInfo info{};
    bool got   = false;
    auto start = std::chrono::steady_clock::now();
    while (std::chrono::steady_clock::now() - start < 2s)
    {
        if (reader->take_next_sample(&received, &info) == eprosima::fastdds::dds::RETCODE_OK &&
            info.instance_state == ALIVE_INSTANCE_STATE)
        {
            got = true;
            break;
        }
        std::this_thread::sleep_for(20ms);
    }

    ASSERT_TRUE(got) << "No DDS sample received";
    EXPECT_EQ(received.text.size(), ethercat_sim::communication::TEXTMSG_MAX_LEN);
    // All chars 'A'
    if (!received.text.empty())
    {
        EXPECT_EQ(received.text.find_first_not_of('A'), std::string::npos);
    }

    // Cleanup
    sub->delete_datareader(reader);
    participant->delete_subscriber(sub);
    pub->delete_datawriter(writer);
    participant->delete_publisher(pub);
    participant->delete_topic(topic);
    DomainParticipantFactory::get_instance()->delete_participant(participant);
}
