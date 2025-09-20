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
#include <cstdlib>
#include <memory>
#include <string>
#include <thread>
#include <unistd.h>

// Project
#include "ethercat_sim/communication/dds_text.h"

using namespace std::chrono_literals;

namespace {

std::string unique_topic() {
    // Unique topic per test run to avoid clashes across processes
    return std::string("sim_text_test_") + std::to_string(::getpid());
}

} // namespace

TEST(DdsTextPubSub, PublishReceive_ShmOnly_Succeeds) {
    using namespace eprosima::fastdds::dds;
    using ethercat_sim::communication::TextMsg;
    using ethercat_sim::communication::TextMsgPubSubType;

    TypeSupport type(new TextMsgPubSubType());

    // Participant with SHM-only transport (no UDP)
    DomainParticipantQos qos               = PARTICIPANT_QOS_DEFAULT;
    qos.transport().use_builtin_transports = false;
    qos.transport().user_transports.push_back(
        std::make_shared<eprosima::fastdds::rtps::SharedMemTransportDescriptor>());
    DomainParticipant* participant =
        DomainParticipantFactory::get_instance()->create_participant(0, qos);
    if (!participant) {
        GTEST_SKIP() << "SHM transport unavailable; skipping pub/sub test";
    }

    type.register_type(participant);

    auto   topic_name = unique_topic();
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

    // Give discovery a brief moment
    std::this_thread::sleep_for(100ms);

    // Publish a sample
    TextMsg msg;
    msg.text = "hello_dds";
    auto ret = writer->write(&msg);
    EXPECT_EQ(ret, eprosima::fastdds::dds::RETCODE_OK);

    // Poll for reception up to ~2s
    TextMsg    received{};
    SampleInfo info{};
    bool       got   = false;
    auto       start = std::chrono::steady_clock::now();
    while (std::chrono::steady_clock::now() - start < 2s) {
        if (reader->take_next_sample(&received, &info) == eprosima::fastdds::dds::RETCODE_OK &&
            info.instance_state == ALIVE_INSTANCE_STATE) {
            got = true;
            break;
        }
        std::this_thread::sleep_for(20ms);
    }

    EXPECT_TRUE(got) << "Did not receive DDS sample within timeout";
    if (got) {
        EXPECT_EQ(received.text, msg.text);
    }

    // Cleanup
    sub->delete_datareader(reader);
    participant->delete_subscriber(sub);
    pub->delete_datawriter(writer);
    participant->delete_publisher(pub);
    participant->delete_topic(topic);
    DomainParticipantFactory::get_instance()->delete_participant(participant);
}
