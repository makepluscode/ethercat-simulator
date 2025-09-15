#include "ethercat_sim/communication/dds_text.h"

namespace ethercat_sim::communication {

TextMsgPubSubType::TextMsgPubSubType()
{
    set_name("TextMsg");
    // Bound the maximum serialized size to ensure Fast DDS allocates
    // a sufficiently large payload buffer for serialize().
    // Encapsulation(4) + length(4) + data(TEXTMSG_MAX_LEN) + padding(~4)
    max_serialized_type_size = static_cast<uint32_t>(4 + 4 + TEXTMSG_MAX_LEN + 4);
    is_compute_key_provided = false;
}

bool TextMsgPubSubType::serialize(const void* const data,
                                  eprosima::fastdds::rtps::SerializedPayload_t& payload,
                                  eprosima::fastdds::dds::DataRepresentationId_t /*representation*/)
{
    const TextMsg* msg = static_cast<const TextMsg*>(data);
    // Enforce maximum length to avoid overruns
    std::string data_str = msg->text;
    if (data_str.size() > TEXTMSG_MAX_LEN) {
        data_str.resize(TEXTMSG_MAX_LEN);
    }
    // Serialize into provided payload buffer
    eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(payload.data), payload.max_size);
    eprosima::fastcdr::Cdr ser(fastbuffer);
    payload.encapsulation = ser.endianness() == eprosima::fastcdr::Cdr::BIG_ENDIANNESS ? CDR_BE : CDR_LE;
    ser.serialize_encapsulation();
    ser << data_str;
    payload.length = static_cast<uint32_t>(ser.get_serialized_data_length());
    return true;
}

bool TextMsgPubSubType::deserialize(eprosima::fastdds::rtps::SerializedPayload_t& payload,
                                    void* data)
{
    TextMsg* msg = static_cast<TextMsg*>(data);
    eprosima::fastcdr::FastBuffer fastbuffer(reinterpret_cast<char*>(payload.data), payload.length);
    eprosima::fastcdr::Cdr deser(fastbuffer);
    deser.read_encapsulation();
    deser >> msg->text;
    return true;
}

uint32_t TextMsgPubSubType::calculate_serialized_size(const void* const data,
                                                      eprosima::fastdds::dds::DataRepresentationId_t /*representation*/)
{
    auto* msg = static_cast<const TextMsg*>(data);
    const std::size_t n = std::min<std::size_t>(msg->text.size(), TEXTMSG_MAX_LEN);
    return static_cast<uint32_t>(4 /*encapsulation*/ + 4 /*len*/ + n + 4 /*align*/);
}

void* TextMsgPubSubType::create_data()
{
    return new TextMsg();
}

void TextMsgPubSubType::delete_data(void* data)
{
    delete static_cast<TextMsg*>(data);
}

} // namespace ethercat_sim::communication
