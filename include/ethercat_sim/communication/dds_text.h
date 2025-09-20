#pragma once

#include <fastcdr/Cdr.h>
#include <fastcdr/FastBuffer.h>
#include <fastdds/dds/topic/TopicDataType.hpp>
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/rtps/common/SerializedPayload.hpp>
#include <functional>
#include <string>

namespace ethercat_sim::communication
{

struct TextMsg
{
    std::string text;
};

// Upper bound for serialized text to keep DDS payload allocation safe.
// Messages longer than this may be truncated during serialization.
inline constexpr std::size_t TEXTMSG_MAX_LEN = 1024u;

class TextMsgPubSubType : public eprosima::fastdds::dds::TopicDataType
{
  public:
    TextMsgPubSubType();
    bool serialize(const void* const data, eprosima::fastdds::rtps::SerializedPayload_t& payload,
                   eprosima::fastdds::dds::DataRepresentationId_t representation) override;
    bool deserialize(eprosima::fastdds::rtps::SerializedPayload_t& payload, void* data) override;
    uint32_t calculate_serialized_size(
        const void* const data,
        eprosima::fastdds::dds::DataRepresentationId_t representation) override;
    void* create_data() override;
    void delete_data(void* data) override;
    bool compute_key(eprosima::fastdds::rtps::SerializedPayload_t&,
                     eprosima::fastdds::rtps::InstanceHandle_t&, bool) override
    {
        return false;
    }
    bool compute_key(const void* const, eprosima::fastdds::rtps::InstanceHandle_t&, bool) override
    {
        return false;
    }
};

} // namespace ethercat_sim::communication
