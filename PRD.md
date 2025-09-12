# Product Requirements Document (PRD)
## KickCAT - EtherCAT Master Library & Simulation Environment

### Document Information
- **Project Name**: KickCAT EtherCAT Master Library
- **Version**: 1.0
- **Date**: December 2024
- **Document Type**: Product Requirements Document

---

## 1. Executive Summary

KickCAT is a comprehensive EtherCAT master library designed for real-time communication with EtherCAT slave devices. The project provides both simulation capabilities for development and testing, as well as real hardware support for production environments. The library follows IEC 61158-4-12 standard and implements a complete EtherCAT master functionality with virtual network simulation capabilities.

### Key Value Propositions
- **Development Efficiency**: Virtual simulation environment eliminates need for physical hardware during development
- **Real-time Performance**: Designed for industrial real-time applications with 1ms cycle time support
- **Linux Focus**: Optimized for Linux (WSL) development environment
- **Easy Integration**: Download KickCAT library via Conan package manager for seamless CMake project integration
- **Scalable Architecture**: Progressive deployment from single executable to distributed device architecture

---

## 2. Product Overview

### 2.1 Product Vision
To provide a robust, efficient, and easy-to-use EtherCAT master library that enables developers to build real-time industrial automation applications with comprehensive simulation capabilities for development and testing.

### 2.2 Target Users
- **Industrial Automation Engineers**: Building EtherCAT-based control systems
- **Embedded Systems Developers**: Implementing EtherCAT communication protocols
- **Research and Development Teams**: Prototyping and testing EtherCAT applications
- **Educational Institutions**: Teaching industrial communication protocols

### 2.3 Product Goals
- Enable rapid development of EtherCAT applications through simulation
- Provide production-ready EtherCAT master implementation
- Ensure real-time performance for industrial applications
- Simplify integration with existing CMake-based projects via Conan package download
- Support progressive deployment architecture from single executable to distributed devices
- Optimize for Linux (WSL) development environment

---

## 3. Functional Requirements

### 3.1 EtherCAT Master Implementation

#### 3.1.1 Core Master Functionality
- **REQ-001**: Implement complete EtherCAT master following IEC 61158-4-12 standard
- **REQ-002**: Support EtherCAT frame structure with proper header and payload handling
- **REQ-003**: Provide real-time cyclic data exchange capabilities
- **REQ-004**: Implement slave discovery and configuration management
- **REQ-005**: Support Process Data Object (PDO) mapping and exchange

#### 3.1.2 Master State Management
- **REQ-006**: Track initialization state (uninitialized, initialized, cyclic operation)
- **REQ-007**: Provide graceful startup and shutdown procedures
- **REQ-008**: Implement error recovery mechanisms
- **REQ-009**: Support hot-plugging of slave devices

### 3.2 Network Interface Abstraction

#### 3.2.1 Abstract Network Layer
- **REQ-010**: Provide abstract network interface for hardware independence
- **REQ-011**: Support both real hardware and simulation environments
- **REQ-012**: Implement frame-based communication with configurable timeouts
- **REQ-013**: Ensure thread-safe network operations

#### 3.2.2 Network Operations
- **REQ-014**: Support frame transmission with error detection
- **REQ-015**: Implement frame reception with timeout handling
- **REQ-016**: Provide network initialization and cleanup procedures
- **REQ-017**: Support multiple network interfaces

### 3.3 Simulation Environment

#### 3.3.1 Network Simulator (`NetworkSimulator`)
- **REQ-018**: Implement virtual network environment for testing
- **REQ-019**: Provide thread-safe frame queue with mutex protection
- **REQ-020**: Simulate realistic transmission delays (100μs default)
- **REQ-021**: Support command processing (read/write operations)
- **REQ-022**: Implement slave registry management
- **REQ-023**: Handle EtherCAT commands (0x02=read, 0x03=write)

#### 3.3.2 Virtual Slave (`VirtualSlave`)
- **REQ-024**: Simulate EtherCAT slave device behavior
- **REQ-025**: Support configurable vendor ID, product code, and address
- **REQ-026**: Implement process data simulation with input/output mapping
- **REQ-027**: Provide simple data processing algorithms
- **REQ-028**: Track slave state (online/offline)
- **REQ-029**: Support multiple virtual slaves in single network

#### 3.3.3 Elmo Motor Slave (`ElmoMotorSlave`)
- **REQ-030**: Implement Elmo motor-specific EtherCAT slave behavior
- **REQ-031**: Leverage existing Elmo motor control code
- **REQ-032**: Support Elmo motor parameters (position, velocity, torque)
- **REQ-033**: Implement Elmo-specific PDO mapping
- **REQ-034**: Provide realistic motor physics simulation
- **REQ-035**: Support Elmo motor state machine (enable, disable, fault states)
- **REQ-036**: Implement Elmo motor configuration and calibration
- **REQ-037**: Support multiple Elmo motors in single network

### 3.4 Real-time Capabilities

#### 3.4.1 Cyclic Operations
- **REQ-038**: Implement cyclic operation with configurable cycle times
- **REQ-039**: Support 1ms cycle times (typical EtherCAT requirement)
- **REQ-040**: Provide background thread processing
- **REQ-041**: Implement callback mechanisms for data and state changes
- **REQ-042**: Support signal handling for graceful shutdown

#### 3.4.2 Threading and Synchronization
- **REQ-043**: Use detached thread for cyclic operations
- **REQ-044**: Implement proper thread synchronization
- **REQ-045**: Prevent race conditions in shared resources
- **REQ-046**: Provide atomic operations for thread control

#### 3.4.3 Elmo Motor Real-time Control
- **REQ-047**: Support real-time motor control loops
- **REQ-048**: Implement position/velocity/torque control modes
- **REQ-049**: Provide motor feedback processing
- **REQ-050**: Support Elmo motor safety functions

### 3.5 GUI Architecture and Communication

#### 3.5.1 MVC Architecture
- **REQ-051**: Implement Model-View-Controller architecture for GUI separation
- **REQ-052**: Separate GUI from core EtherCAT functionality
- **REQ-053**: Provide modular GUI components
- **REQ-054**: Support multiple GUI frameworks (Qt, web-based)

#### 3.5.2 FastDDS Communication
- **REQ-055**: Implement FastDDS-based communication between GUI and core
- **REQ-056**: Support real-time data publishing/subscribing
- **REQ-057**: Provide topic-based messaging system
- **REQ-058**: Implement data serialization for GUI communication
- **REQ-059**: Support multiple GUI clients simultaneously

#### 3.5.3 Elmo Dashboard
- **REQ-060**: Implement Elmo motor status dashboard
- **REQ-061**: Display motor position, velocity, torque in real-time
- **REQ-062**: Show motor state (enabled/disabled, fault status)
- **REQ-063**: Provide motor control interface (position/velocity commands)
- **REQ-064**: Display network topology and slave status
- **REQ-065**: Support alarm and error message display
- **REQ-066**: Provide motor configuration interface
- **REQ-067**: Support data logging and historical data visualization

---

## 4. Technical Requirements

### 4.1 Data Structures

#### 4.1.1 EtherCATFrame Structure
```cpp
struct EtherCATFrame {
    uint16 length;        // Frame length including header
    uint8 command;        // EtherCAT command (0x02=read, 0x03=write)
    uint8 index;          // Frame index
    uint16 address;       // Slave address
    uint16 interrupt;     // Interrupt register
    uint8 data[1486];     // Maximum EtherCAT payload
    uint16 working_counter; // Working counter for error detection
    uint16 reserved;      // Reserved field
};
```

#### 4.1.2 SlaveInfo Structure
```cpp
struct SlaveInfo {
    uint16 address;       // Slave address
    uint32 vendor_id;     // Vendor identification
    uint32 product_code;  // Product code
    uint32 revision_number; // Revision number
    uint32 serial_number; // Serial number
    std::string name;     // Human-readable name
    bool online;          // Online status
};
```

#### 4.1.3 ElmoMotorInfo Structure
```cpp
struct ElmoMotorInfo {
    uint16 address;           // EtherCAT slave address
    uint32 vendor_id;         // Elmo vendor ID (0x0000009A)
    uint32 product_code;      // Elmo product code
    uint32 revision_number;   // Firmware revision
    uint32 serial_number;     // Motor serial number
    std::string motor_name;   // Motor identifier
    bool online;              // Online status
    
    // Motor parameters
    double position;          // Current position (radians)
    double velocity;          // Current velocity (rad/s)
    double torque;            // Current torque (Nm)
    uint16 status_word;       // Motor status word
    uint16 control_word;      // Motor control word
    
    // Elmo-specific parameters
    uint16 motor_type;        // Motor type identifier
    double max_velocity;     // Maximum velocity
    double max_torque;       // Maximum torque
    bool enabled;            // Motor enable status
    bool fault_detected;     // Fault status
};
```

#### 4.1.4 ElmoMotorPDO Structure
```cpp
struct ElmoMotorPDO {
    // Input PDO (from motor to master)
    uint16 status_word;      // Motor status
    int32 position_feedback; // Position feedback
    int32 velocity_feedback; // Velocity feedback
    int16 torque_feedback;   // Torque feedback
    uint16 motor_temperature; // Motor temperature
    uint16 error_code;       // Error code
    
    // Output PDO (from master to motor)
    uint16 control_word;     // Motor control
    int32 position_command;  // Position command
    int32 velocity_command;  // Velocity command
    int16 torque_command;    // Torque command
    uint16 operation_mode;   // Operation mode
    uint16 reserved;         // Reserved field
};
```

#### 4.1.5 FastDDS Communication Structures
```cpp
// Motor Status Topic
struct MotorStatusTopic {
    uint16 motor_id;         // Motor identifier
    double position;          // Current position (radians)
    double velocity;         // Current velocity (rad/s)
    double torque;           // Current torque (Nm)
    uint16 status_word;      // Motor status word
    uint16 control_word;     // Motor control word
    bool enabled;            // Motor enable status
    bool fault_detected;     // Fault status
    uint16 error_code;       // Error code
    uint16 temperature;     // Motor temperature
    uint64 timestamp;       // Timestamp (microseconds)
};

// Motor Command Topic
struct MotorCommandTopic {
    uint16 motor_id;         // Motor identifier
    double position_command;  // Position command (radians)
    double velocity_command; // Velocity command (rad/s)
    double torque_command;   // Torque command (Nm)
    uint16 control_word;     // Control word
    uint16 operation_mode;   // Operation mode
    uint64 timestamp;       // Timestamp (microseconds)
};

// Network Status Topic
struct NetworkStatusTopic {
    uint16 total_slaves;     // Total number of slaves
    uint16 online_slaves;    // Number of online slaves
    bool master_state;       // Master operational state
    uint32 cycle_time;       // Current cycle time (μs)
    uint32 frame_count;      // Frame count
    uint64 timestamp;       // Timestamp (microseconds)
};
```

### 4.2 Performance Requirements

#### 4.2.1 Timing Requirements
- **REQ-051**: Support cycle times as low as 1ms
- **REQ-052**: Minimize frame transmission latency
- **REQ-053**: Support maximum EtherCAT frame size (1486 bytes)
- **REQ-054**: Implement efficient memory usage with minimal allocations

#### 4.2.2 Throughput Requirements
- **REQ-055**: Support high-frequency cyclic data exchange
- **REQ-056**: Handle multiple slaves simultaneously
- **REQ-057**: Process frames within real-time constraints

#### 4.2.3 Elmo Motor Performance Requirements
- **REQ-068**: Support 1ms motor control cycle times
- **REQ-069**: Minimize motor command latency
- **REQ-070**: Support multiple Elmo motors simultaneously
- **REQ-071**: Process motor feedback within real-time constraints

#### 4.2.4 GUI Communication Performance Requirements
- **REQ-072**: Support GUI update rates up to 100Hz
- **REQ-073**: Minimize GUI communication latency (< 10ms)
- **REQ-074**: Support multiple GUI clients (up to 10)
- **REQ-075**: Handle GUI disconnection/reconnection gracefully

### 4.3 Platform Requirements

#### 4.3.1 Programming Language
- **REQ-076**: Implement in C++17 or higher
- **REQ-077**: Use modern C++ features (RAII, smart pointers)
- **REQ-078**: Follow exception-safe design principles
- **REQ-079**: Integrate existing Elmo motor control code

#### 4.3.2 Build System
- **REQ-080**: Use CMake 3.11+ for build configuration
- **REQ-081**: Support Conan package manager integration
- **REQ-082**: Provide cross-platform build support
- **REQ-083**: Include Elmo motor libraries and dependencies
- **REQ-084**: Include FastDDS libraries and dependencies
- **REQ-085**: Include GUI framework dependencies (Qt, etc.)

#### 4.3.3 Platform Support
- **REQ-086**: Support Linux (WSL) as primary and only target platform
- **REQ-087**: Use POSIX threads (pthread) for threading
- **REQ-088**: Optimize for Linux kernel and system libraries
- **REQ-089**: Support Linux-specific real-time features

#### 4.3.4 GUI Framework Support
- **REQ-090**: Support Qt framework for Linux desktop GUI
- **REQ-091**: Support web-based GUI (HTML5/JavaScript)
- **REQ-092**: Optimize GUI for Linux environment
- **REQ-093**: Support responsive GUI design on Linux

---

## 5. Architecture Design

### 5.1 Core Architecture

#### 5.1.1 Network Interface Layer (`NetworkInterface`)
- **Purpose**: Abstract interface for network communication
- **Implementation**: Pure virtual base class defining frame-based communication
- **Key Methods**:
  - `initialize()`: Initialize network resources
  - `sendFrame()`: Send EtherCAT frame to network
  - `receiveFrame()`: Receive EtherCAT frame with timeout
  - `close()`: Clean up network resources

#### 5.1.2 Master Implementation (`Master`)
- **Purpose**: Main EtherCAT master functionality
- **Key Features**:
  - Slave lifecycle management (scan, configure, monitor)
  - Cyclic operation control with background thread
  - Process data exchange (read/write operations)
  - Callback system for data and state changes
- **Threading Model**: Uses detached thread for cyclic operations
- **State Management**: Tracks initialization and cyclic operation states

### 5.2 Simulation Components

#### 5.2.1 NetworkSimulator Implementation
- **Thread-safe Frame Queue**: Mutex-protected queue for frame handling
- **Simulated Transmission Delays**: Configurable delay simulation (100μs default)
- **Command Processing**: Handle read/write operations
- **Slave Registry**: Manage virtual slave devices
- **Frame Processing**: Process EtherCAT commands

#### 5.2.2 VirtualSlave Implementation
- **Configurable Parameters**: Vendor ID, product code, address
- **Process Data Simulation**: Input/output mapping
- **Data Processing**: Simple algorithms for realistic behavior
- **State Tracking**: Online/offline status management

#### 5.2.3 ElmoMotorSlave Implementation
- **Elmo Motor Integration**: Leverage existing Elmo code for motor control
- **Motor Parameters**: Position, velocity, torque control
- **EtherCAT PDO Mapping**: Elmo-specific process data objects
- **Motor State Machine**: Elmo motor operational states
- **Realistic Motor Behavior**: Physics-based motor simulation

### 5.3 Threading and Synchronization

#### 5.3.1 Cyclic Operation Thread
- **Purpose**: Background thread for real-time data exchange
- **Implementation**: Detached thread with 1ms sleep cycle
- **Synchronization**: Uses atomic boolean for thread control
- **Error Handling**: Graceful shutdown on master destruction

#### 5.3.2 Network Simulator Threading
- **Frame Queue**: Thread-safe queue with mutex protection
- **Timeout Handling**: Configurable timeout for frame reception
- **Race Condition Prevention**: Proper locking for shared resources

### 5.4 GUI Architecture

#### 5.4.1 MVC Architecture Implementation
- **Model**: Data layer for motor status, network state, and configuration
- **View**: GUI components for displaying motor information and controls
- **Controller**: Business logic for handling user interactions and data flow
- **Separation**: Complete separation between GUI and core EtherCAT functionality

#### 5.4.2 FastDDS Communication Layer
- **Publisher**: Core system publishes motor status and network information
- **Subscriber**: GUI subscribes to motor status and publishes commands
- **Topics**: MotorStatusTopic, MotorCommandTopic, NetworkStatusTopic
- **QoS**: Reliable communication with appropriate latency settings

#### 5.4.3 Elmo Dashboard Components
- **Motor Status Panel**: Real-time display of position, velocity, torque
- **Network Topology**: Visual representation of EtherCAT network
- **Control Panel**: Motor command interface (position/velocity/torque)
- **Alarm Panel**: Error messages and fault status display
- **Configuration Panel**: Motor parameter configuration interface
- **Data Logging**: Historical data visualization and export

---

## 6. Project Structure

```
KickCAT/
├── include/                    # Header files
│   ├── kickcat.h              # Main library header
│   └── communication/        # FastDDS communication
│       ├── topics.h          # FastDDS topic definitions
│       └── publisher.h        # Data publisher interface
├── core/                       # Core source files
│   ├── master.cpp             # Master implementation
│   ├── slave.cpp              # Slave implementation
│   ├── network.cpp            # Network interface
│   ├── simulation/            # Simulation components
│   │   ├── network_simulator.cpp
│   │   └── virtual_slave.cpp
│   └── communication/        # FastDDS communication
│       ├── publisher.cpp      # Data publisher implementation
│       ├── subscriber.cpp     # Data subscriber implementation
│       └── topics.cpp         # Topic implementations
├── lib/                        # Library files
│   ├── libkickcat.a           # Static library
│   └── libkickcat.so          # Shared library
├── gui/                       # GUI applications
│   ├── include/              # GUI headers
│   │   ├── model.h            # MVC Model classes
│   │   ├── view.h             # MVC View classes
│   │   ├── controller.h       # MVC Controller classes
│   │   └── dashboard.h         # Elmo dashboard components
│   ├── src/                  # GUI implementation
│   │   ├── model/             # MVC Model implementation
│   │   │   ├── motor_model.cpp
│   │   │   └── network_model.cpp
│   │   ├── view/              # MVC View implementation
│   │   │   ├── motor_view.cpp
│   │   │   └── dashboard_view.cpp
│   │   ├── controller/        # MVC Controller implementation
│   │   │   ├── motor_controller.cpp
│   │   │   └── dashboard_controller.cpp
│   │   └── dashboard/         # Elmo dashboard components
│   │       ├── motor_panel.cpp
│   │       ├── network_panel.cpp
│   │       └── control_panel.cpp
│   ├── desktop/              # Desktop GUI (Qt)
│   │   ├── main.cpp
│   │   ├── CMakeLists.txt
│   │   └── resources/         # GUI resources
│   └── web/                  # Web-based GUI
│       ├── index.html
│       ├── dashboard.js
│       └── styles.css
├── examples/                  # Example applications
│   ├── main.cpp              # Master-slave communication example
│   └── CMakeLists.txt
├── cmake_example/            # CMake integration example
│   ├── main.cpp
│   ├── CMakeLists.txt
│   ├── conanfile.txt
│   └── build_example.sh
├── conanfile.py              # Conan package configuration
├── CMakeLists.txt            # Main CMake configuration
├── build_kickcat.sh          # Build script
└── PRD.md                    # This Product Requirements Document
```

---

## 7. Error Handling and Logging

### 7.1 Error Handling Strategy
- **REQ-056**: Use boolean return values for operation success/failure
- **REQ-057**: Implement exception-safe design with RAII and smart pointers
- **REQ-058**: Provide automatic resource cleanup in destructors
- **REQ-059**: Implement proper state validation before operations

### 7.2 Logging Implementation
- **REQ-060**: Use standard C++ iostream for console output
- **REQ-061**: Provide detailed operation logging for debugging
- **REQ-062**: Implement clear error reporting with context
- **REQ-063**: Support performance monitoring (cycle time and operation timing)

---

## 8. Build System Integration

### 8.1 CMake Configuration
- **REQ-064**: Organize source files by functionality (master, network, simulation)
- **REQ-065**: Implement automatic Linux library linking
- **REQ-066**: Provide proper library and header installation
- **REQ-067**: Support configurable build options for simulation and examples
- **REQ-068**: Generate library files in lib/ directory
- **REQ-069**: Support both static and shared library builds for Linux
- **REQ-070**: Implement proper library installation to lib/ directory

### 8.2 Conan Package Distribution
- **REQ-068**: Define complete package metadata and dependencies
- **REQ-069**: Support Linux system library linking
- **REQ-070**: Provide configurable options for shared/static builds
- **REQ-071**: Implement proper source and header file export
- **REQ-072**: Provide easy download and integration via Conan
- **REQ-073**: Support automatic dependency resolution
- **REQ-074**: Include comprehensive usage examples
- **REQ-075**: Package library files in lib/ directory
- **REQ-076**: Support Linux library naming conventions

---

## 9. Testing Requirements

### 9.1 Unit Testing
- **REQ-072**: Test individual components in isolation
- **REQ-073**: Verify frame structure handling
- **REQ-074**: Test slave management functionality
- **REQ-075**: Validate network interface operations

### 9.2 Integration Testing
- **REQ-076**: Test master-slave communication
- **REQ-077**: Verify simulation environment functionality
- **REQ-078**: Test real-time performance requirements
- **REQ-079**: Validate cross-platform compatibility

### 9.3 Performance Testing
- **REQ-080**: Measure cycle time performance
- **REQ-081**: Test throughput under load
- **REQ-082**: Validate memory usage efficiency
- **REQ-083**: Test latency requirements

---

## 10. Documentation Requirements

### 10.1 API Documentation
- **REQ-084**: Provide comprehensive API documentation
- **REQ-085**: Include usage examples for all major features
- **REQ-086**: Document configuration options and parameters
- **REQ-087**: Provide troubleshooting guide

### 10.2 User Documentation
- **REQ-088**: Create getting started guide
- **REQ-089**: Provide integration examples
- **REQ-090**: Document simulation environment setup
- **REQ-091**: Include performance tuning guidelines

---

## 11. Deployment and Distribution

### 11.1 Internal Package Distribution
- **REQ-092**: Provide Conan package for internal distribution
- **REQ-093**: Support multiple build configurations
- **REQ-094**: Include Linux binaries in lib/ directory
- **REQ-095**: Provide source code distribution
- **REQ-096**: Enable easy download via Conan package manager
- **REQ-097**: Support automatic dependency management
- **REQ-098**: Provide clear installation and usage instructions
- **REQ-099**: Package library files in standardized lib/ directory structure
- **REQ-100**: Support Linux library file naming conventions

### 11.2 Conan Integration and Usage
- **REQ-101**: Support standard Conan installation procedures
- **REQ-102**: Provide CMake integration examples with Conan
- **REQ-103**: Include automatic dependency management via Conan
- **REQ-104**: Support internal development and production environments
- **REQ-105**: Provide clear Conan download and usage documentation
- **REQ-106**: Install library files to lib/ directory via Conan
- **REQ-107**: Support linking to libkickcat library files

---

## 12. Success Criteria

### 12.1 Functional Success Criteria
- ✅ Complete EtherCAT master implementation following IEC 61158-4-12
- ✅ Functional simulation environment with virtual slaves
- ✅ Real-time cyclic operation with 1ms cycle time support
- ✅ Linux (WSL) compatibility and optimization

### 12.2 Performance Success Criteria
- ✅ Support for 1ms cycle times
- ✅ Minimal frame transmission latency
- ✅ Support for maximum EtherCAT frame size (1486 bytes)
- ✅ Efficient memory usage with minimal allocations

### 12.3 Integration Success Criteria
- ✅ Easy CMake project integration via Conan download
- ✅ Conan package manager support for internal distribution
- ✅ Simple library download and integration process on Linux
- ✅ Comprehensive documentation and examples
- ✅ Robust error handling and logging

---

## 13. Risk Assessment

### 13.1 Technical Risks
- **Real-time Performance**: Ensuring deterministic timing in simulation
- **Cross-platform Compatibility**: Managing platform-specific differences
- **Threading Complexity**: Avoiding race conditions and deadlocks

### 13.2 Mitigation Strategies
- Comprehensive testing on all target platforms
- Careful thread synchronization design
- Performance profiling and optimization
- Extensive documentation and examples

---

## 14. Development Phases

### 14.1 Phase 1: Core Foundation
**목표**: 기본 EtherCAT 마스터 라이브러리 구조 구축

#### Phase 1 Requirements
- **REQ-001 to REQ-009**: EtherCAT Master 기본 구현
- **REQ-010 to REQ-017**: Network Interface 추상화
- **REQ-046 to REQ-055**: 플랫폼 및 빌드 시스템 설정
- **REQ-056 to REQ-063**: 기본 에러 처리 및 로깅

#### Phase 1 Deliverables
- ✅ 기본 EtherCAT 프레임 구조 구현
- ✅ NetworkInterface 추상 클래스
- ✅ Master 클래스 기본 구조
- ✅ CMake 빌드 시스템
- ✅ 기본 로깅 시스템

#### Phase 1 Success Criteria
- 라이브러리가 컴파일되고 링크됨
- 기본 프레임 구조가 정의됨
- 네트워크 인터페이스 추상화 완료

### 14.2 Phase 2: Simulation Environment
**목표**: 가상 네트워크 시뮬레이터 및 Elmo Motor 슬레이브 구현

#### Phase 2 Requirements
- **REQ-018 to REQ-037**: 시뮬레이션 환경 및 Elmo Motor 구현
- **REQ-072 to REQ-075**: 기본 단위 테스트

#### Phase 2 Deliverables
- ✅ NetworkSimulator 클래스 구현
- ✅ VirtualSlave 클래스 구현
- ✅ ElmoMotorSlave 클래스 구현
- ✅ 스레드 안전 프레임 큐
- ✅ 가상 슬레이브 등록 관리
- ✅ Elmo Motor PDO 매핑
- ✅ 기본 단위 테스트

#### Phase 2 Success Criteria
- 가상 네트워크에서 프레임 전송/수신 가능
- 여러 가상 슬레이브 동시 시뮬레이션
- Elmo Motor 슬레이브 시뮬레이션
- 기본 EtherCAT 명령어 처리 (0x02, 0x03)
- Elmo Motor 제어 명령 처리

### 14.3 Phase 3: Real-time Operations
**목표**: 실시간 사이클릭 운영 및 Elmo Motor 제어 구현

#### Phase 3 Requirements
- **REQ-038 to REQ-050**: 실시간 기능 및 Elmo Motor 제어 구현
- **REQ-076 to REQ-079**: 통합 테스트

#### Phase 3 Deliverables
- ✅ 사이클릭 운영 스레드 구현
- ✅ 1ms 사이클 타임 지원
- ✅ Elmo Motor 실시간 제어
- ✅ 콜백 메커니즘
- ✅ 시그널 핸들링
- ✅ 통합 테스트 스위트

#### Phase 3 Success Criteria
- 1ms 사이클 타임 달성
- 안정적인 실시간 데이터 교환
- Elmo Motor 실시간 제어 성공
- 우아한 종료 처리

### 14.4 Phase 4: Advanced Features
**목표**: 고급 기능, Elmo Motor 최적화 및 성능 튜닝

#### Phase 4 Requirements
- **REQ-080 to REQ-093**: 빌드 시스템 고도화
- **REQ-080 to REQ-083**: 성능 테스트
- **REQ-084 to REQ-091**: 문서화
- **REQ-068 to REQ-071**: Elmo Motor 성능 최적화

#### Phase 4 Deliverables
- ✅ Conan 패키지 지원 (사내 배포용)
- ✅ 크로스 플랫폼 빌드
- ✅ Elmo Motor 성능 최적화
- ✅ API 문서화
- ✅ Conan 다운로드 및 사용 가이드
- ✅ Elmo Motor 사용자 가이드
- ✅ Elmo Motor 예제 애플리케이션

#### Phase 4 Success Criteria
- Conan을 통한 사내 패키지 배포
- Conan을 통한 라이브러리 다운로드 성공
- Linux (WSL)에서 빌드 성공
- Elmo Motor 성능 요구사항 충족
- 다중 Elmo Motor 동시 제어 성공

### 14.5 Phase 5: GUI Architecture and Communication
**목표**: MVC 아키텍처 기반 GUI 분리 및 FastDDS 통신 구현

#### Phase 5 Requirements
- **REQ-051 to REQ-067**: GUI 아키텍처 및 Elmo 대시보드 구현
- **REQ-072 to REQ-075**: GUI 통신 성능 요구사항

#### Phase 5 Deliverables
- ✅ MVC 아키텍처 구현
- ✅ FastDDS 통신 레이어
- ✅ Elmo 대시보드 GUI
- ✅ Motor Status Panel
- ✅ Network Topology Display
- ✅ Motor Control Interface
- ✅ Alarm and Error Display
- ✅ Configuration Panel

#### Phase 5 Success Criteria
- GUI와 Core 시스템 완전 분리
- FastDDS를 통한 실시간 통신 성공
- Elmo 대시보드 실시간 모니터링
- 다중 GUI 클라이언트 지원

### 14.6 Phase 6: Virtual Ethernet Architecture
**목표**: 2개 실행파일로 분리 (가상 이더넷)

#### Phase 6 Requirements
- 마스터와 슬레이브 실행파일 분리
- 가상 이더넷 통신 구현
- 프로세스 간 EtherCAT 프레임 전송

#### Phase 6 Deliverables
- ✅ Master Executable (EtherCAT 마스터)
- ✅ Slave Executable (EtherCAT 슬레이브 시뮬레이션)
- ✅ Virtual Ethernet 통신 레이어
- ✅ 분산 아키텍처 예제

#### Phase 6 Success Criteria
- 마스터와 슬레이브 실행파일 분리 성공
- 가상 이더넷을 통한 통신 성공
- 분산 아키텍처 시뮬레이션 동작

### 14.7 Phase 7: Separate Device Architecture
**목표**: 별도 디바이스로 마스터와 슬레이브 분리

#### Phase 7 Requirements
- 전용 마스터 디바이스 개발
- 전용 슬레이브 디바이스 개발
- 실제 이더넷 네트워크 통신

#### Phase 7 Deliverables
- ✅ Master Device (전용 하드웨어)
- ✅ Slave Device (전용 하드웨어)
- ✅ Real Ethernet 통신
- ✅ 산업용 펌웨어

#### Phase 7 Success Criteria
- 별도 디바이스에서 마스터/슬레이브 동작
- 실제 이더넷 네트워크 통신 성공
- 산업 환경에서 안정적 동작

### 14.8 Phase 8: Production Ready
**목표**: 프로덕션 배포 준비

#### Phase 8 Requirements
- **REQ-092 to REQ-099**: 배포 및 설치
- 모든 이전 요구사항 검증
- Conan을 통한 KickCAT 라이브러리 배포

#### Phase 8 Deliverables
- ✅ 완전한 사내 패키지 배포 시스템
- ✅ Conan을 통한 KickCAT 라이브러리 다운로드 제공
- ✅ Conan 설치 및 사용 가이드
- ✅ 예제 애플리케이션 (단일 실행파일)
- ✅ 문제 해결 가이드
- ✅ 성능 벤치마크

#### Phase 8 Success Criteria
- Linux (WSL) 환경에서 안정적 동작
- Conan을 통한 KickCAT 라이브러리 다운로드 및 사용 가능
- 완전한 문서화
- 사내 사용자 피드백 수용 가능

---

## 15. Development Planning

### 개발 단계별 계획

| Phase | 주요 마일스톤 | 의존성 |
|-------|---------------|--------|
| Phase 1 | 기본 라이브러리 구조 | 없음 |
| Phase 2 | 시뮬레이션 환경 | Phase 1 완료 |
| Phase 3 | 실시간 운영 | Phase 1,2 완료 |
| Phase 4 | 고급 기능 | Phase 1,2,3 완료 |
| Phase 5 | GUI 아키텍처 및 통신 | Phase 4 완료 |
| Phase 6 | 가상 이더넷 아키텍처 | Phase 5 완료 |
| Phase 7 | 별도 디바이스 아키텍처 | Phase 6 완료 |
| Phase 8 | 프로덕션 준비 (Conan 라이브러리) | 모든 Phase 완료 |

### 병렬 개발 가능 영역
- **문서화**: Phase 2부터 병렬 진행 가능
- **테스트**: 각 Phase와 병렬 진행
- **예제 코드**: Phase 3부터 병렬 진행 가능

---

## 16. Risk Mitigation by Phase

### Phase 1 Risks
- **기술적 복잡성**: EtherCAT 프로토콜 이해 부족
- **완화책**: 프로토타입 개발, 전문가 자문

### Phase 2 Risks
- **스레드 동기화**: 경쟁 상태 및 데드락
- **완화책**: 철저한 코드 리뷰, 동시성 테스트

### Phase 3 Risks
- **실시간 성능**: 1ms 사이클 타임 달성 어려움
- **완화책**: 성능 프로파일링, 최적화

### Phase 4 Risks
- **크로스 플랫폼**: 플랫폼별 차이점
- **완화책**: CI/CD 파이프라인, 자동화된 테스트

### Phase 5 Risks
- **사용자 채택**: 복잡한 통합 과정
- **완화책**: 상세한 문서화, 예제 제공

---

## 17. Deployment Architecture Evolution

### 17.1 Phase 1: Single Executable (초기 단계)
- **목표**: 단일 실행파일로 KickCAT 라이브러리 사용
- **구현**: Conan을 통해 KickCAT 라이브러리 다운로드 및 통합
- **특징**: 마스터와 슬레이브가 같은 프로세스에서 실행
- **용도**: 개발 및 테스트, 단순한 EtherCAT 애플리케이션
- **사용법**: `conan install kickcat/1.0.0@company/stable` 명령으로 라이브러리 다운로드
- **라이브러리 위치**: 설치 후 `lib/libkickcat.a` 또는 `lib/libkickcat.so` 파일 사용
- **플랫폼**: Linux (WSL) 환경에서 실행

### 17.2 Phase 2: Virtual Ethernet (중기 단계)
- **목표**: 2개의 실행파일로 분리 (가상 이더넷 통신)
- **구현**: 
  - Master Executable: EtherCAT 마스터 기능 (Conan으로 다운로드)
  - Slave Executable: EtherCAT 슬레이브 시뮬레이션 (Conan으로 다운로드)
  - Virtual Ethernet: 프로세스 간 통신
- **특징**: 네트워크 시뮬레이션을 통한 분산 아키텍처
- **용도**: 네트워크 테스트, 분산 시스템 개발
- **사용법**: 각각 별도의 Conan 패키지로 다운로드하여 사용

### 17.3 Phase 3: Separate Devices (장기 단계)
- **목표**: 별도의 디바이스로 마스터와 슬레이브 분리
- **구현**:
  - Master Device: 전용 EtherCAT 마스터 하드웨어 (Conan으로 펌웨어 다운로드)
  - Slave Device: 전용 EtherCAT 슬레이브 하드웨어 (Conan으로 펌웨어 다운로드)
  - Real Ethernet: 실제 이더넷 네트워크 통신
- **특징**: 실제 산업 환경과 동일한 분산 아키텍처
- **용도**: 프로덕션 환경, 실제 산업 자동화 시스템
- **사용법**: 디바이스별 Conan 패키지로 펌웨어 다운로드 및 설치

### 17.4 Conan Integration Strategy
- **Phase 1**: KickCAT 라이브러리를 Conan 패키지로 제공하여 다운로드 가능
- **Phase 2**: Master/Slave 컴포넌트를 별도 Conan 패키지로 분리하여 개별 다운로드
- **Phase 3**: 디바이스별 Conan 패키지 및 펌웨어 지원
- **사용 방식**: 개발자는 Conan을 통해 KickCAT 라이브러리를 다운로드하여 Linux 프로젝트에 통합
- **라이브러리 위치**: Conan 설치 후 lib/ 디렉토리에 libkickcat.a, libkickcat.so가 설치됨
- **패키지 구조**: Conan 패키지 내부에 include/, lib/, bin/ 디렉토리 구조 제공
- **플랫폼**: Linux (WSL) 환경에 최적화

---

*This PRD serves as the foundation for the KickCAT EtherCAT Master Library development and will be updated as requirements evolve.*