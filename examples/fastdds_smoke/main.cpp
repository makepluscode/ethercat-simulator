#include <fastdds/dds/domain/DomainParticipantFactory.hpp>
#include <fastdds/dds/domain/DomainParticipant.hpp>
#include <iostream>

int main()
{
    std::cout << "FastDDS include OK" << std::endl;

    // Avoid creating a participant to keep it sandbox/network friendly.
    // Just touch the factory singleton to ensure link is proper.
    auto* factory = eprosima::fastdds::dds::DomainParticipantFactory::get_instance();
    if (factory == nullptr) {
        std::cerr << "FastDDS factory is null" << std::endl;
        return 1;
    }
    std::cout << "FastDDS smoke OK" << std::endl;
    return 0;
}
