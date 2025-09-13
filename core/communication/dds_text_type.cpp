#include "ethercat_sim/communication/dds_text.h"

#ifdef ETHERCAT_HAVE_FASTDDS
using eprosima::fastdds::dds::TopicDataType;
using eprosima::fastrtps::rtps::SerializedPayload_t;

namespace ethercat_sim::communication {

TextMsgPubSubType::TextMsgPubSubType()
{
    setName("TextMsg");
    m_typeSize = static_cast<uint32_t>(4 /*encapsulation*/ + 4 /*len*/ + 256 /*approx*/ + 4 /*alignment*/);
    m_isGetKeyDefined = false;
}

bool TextMsgPubSubType::serialize(void* data, SerializedPayload_t* payload)
{
    TextMsg* msg = static_cast<TextMsg*>(data);
    eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(payload->data), payload->max_size);
    eprosima::fastcdr::Cdr ser(fastbuffer);
    payload->encapsulation = ser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;
    ser.serialize_encapsulation();
    ser << msg->text;
    payload->length = static_cast<uint32_t>(ser.getSerializedDataLength());
    return true;
}

bool TextMsgPubSubType::deserialize(SerializedPayload_t* payload, void* data)
{
    TextMsg* msg = static_cast<TextMsg*>(data);
    eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(payload->data), payload->length);
    eprosima::fastcdr::Cdr deser(fastbuffer);
    deser.read_encapsulation();
    deser >> msg->text;
    return true;
}

std::function<uint32_t()> TextMsgPubSubType::getSerializedSizeProvider(void* data)
{
    return [data]() -> uint32_t {
        auto* msg = static_cast<TextMsg*>(data);
        // 4 bytes for encapsulation + 4 for string length + content size
        return static_cast<uint32_t>(4 + 4 + msg->text.size() + 4);
    };
}

} // namespace ethercat_sim::communication
#endif

