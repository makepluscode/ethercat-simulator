#pragma once

#include <string>

#ifdef ETHERCAT_HAVE_FASTDDS
#include <fastdds/dds/topic/TypeSupport.hpp>
#include <fastdds/dds/topic/TopicDataType.hpp>
#include <fastdds/rtps/common/SerializedPayload.h>
#include <fastcdr/FastBuffer.h>
#include <fastcdr/Cdr.h>

namespace ethercat_sim::communication {

struct TextMsg {
    std::string text;
};

class TextMsgPubSubType : public eprosima::fastdds::dds::TopicDataType {
public:
    TextMsgPubSubType();
    bool serialize(void* data, eprosima::fastrtps::rtps::SerializedPayload_t* payload) override;
    bool deserialize(eprosima::fastrtps::rtps::SerializedPayload_t* payload, void* data) override;
    std::function<uint32_t()> getSerializedSizeProvider(void* data) override;
    bool getKey(void* data, eprosima::fastrtps::rtps::InstanceHandle_t*, bool) override { (void)data; return false; }
};

} // namespace ethercat_sim::communication
#endif // ETHERCAT_HAVE_FASTDDS

