# EtherCAT Simulator Architecture

## Overview

The EtherCAT Simulator is a comprehensive simulation framework for EtherCAT networks, designed with a modular architecture that separates concerns between core simulation logic, communication layers, and application-specific components. The architecture follows modern C++17+ practices with clear separation of concerns and dependency injection patterns.

## Core Architecture Principles

- **Modular Design**: Clear separation between core libraries, applications, and optional components
- **MVC Pattern**: Model-View-Controller architecture for applications
- **Dependency Injection**: Factory patterns for socket creation and component instantiation
- **Thread Safety**: Atomic operations and mutex-protected shared state
- **RAII**: Resource management through smart pointers and RAII principles

## Directory Structure

```
ethercat-simulator/
├── core/                    # Core simulation libraries
│   ├── communication/       # Communication abstractions
│   ├── framework/          # Application framework (MVC base classes)
│   ├── kickcat/           # KickCAT adapter integration
│   └── simulation/        # Network simulation engine
├── src/                    # Main applications
│   ├── a-main/            # EtherCAT Master application
│   └── a-subs/            # EtherCAT Slaves application
├── include/               # Public headers
├── examples/              # Example applications
├── tests/                 # Unit tests
└── docs/                  # Documentation
```

## Core Libraries (`core/`)

### 1. Communication Layer (`core/communication/`)

**Purpose**: Provides abstraction for different communication protocols and socket management.

**Key Components**:
- `SocketFactory`: Factory pattern for creating sockets based on endpoint type (TCP/UDS)
- `EndpointParser`: Parses endpoint strings (e.g., `uds:///tmp/socket`, `tcp://host:port`)
- `DdsTextType`: FastDDS integration for distributed communication (optional)

**Architecture**:
```cpp
class SocketFactory {
    static int createSocket(const std::string& endpoint);
    static void registerCreator(const std::string& type, SocketCreator creator);
};
```

### 2. Framework Layer (`core/framework/`)

**Purpose**: Provides base classes for MVC architecture and application lifecycle management.

**Key Components**:
- `BaseApplication`: Common application lifecycle (signal handling, terminal setup)
- `BaseController<T>`: Template base class for controllers with worker thread management
- `BaseModel<T>`: Template base class for models with observer pattern
- `Observable<T>`: Observer pattern implementation for model updates

**Architecture**:
```cpp
template<typename ModelType>
class BaseController {
    void start();  // Starts worker thread
    void stop();   // Stops worker thread gracefully
    virtual void run() = 0;  // Pure virtual worker function
};

template<typename DataType>
class BaseModel : public Observable<DataType> {
    Data snapshot() const;
    void updateData(const Data& newData);
};
```

### 3. Simulation Engine (`core/simulation/`)

**Purpose**: Core EtherCAT network simulation logic.

**Key Components**:
- `NetworkSimulator`: Main simulation engine managing virtual slaves and network state
- Virtual slave management with station addressing
- Frame queuing with configurable latency
- Logical memory mapping for PDO data

**Architecture**:
```cpp
class NetworkSimulator {
    // Slave management
    void setVirtualSlaveCount(std::size_t n);
    void addVirtualSlave(std::shared_ptr<VirtualSlave> slave);
    
    // Frame processing
    bool sendFrame(const EtherCATFrame& frame);
    bool receiveFrame(EtherCATFrame& out);
    
    // Memory operations
    bool writeToSlave(std::uint16_t addr, std::uint16_t reg, const uint8_t* data, std::size_t len);
    bool readFromSlave(std::uint16_t addr, std::uint16_t reg, uint8_t* out, std::size_t len);
};
```

### 4. KickCAT Adapter (`core/kickcat/`)

**Purpose**: Integration layer between KickCAT library and simulation environment.

**Key Components**:
- `SimSocket`: Custom socket implementation for KickCAT integration
- Bridges KickCAT's socket interface with simulation network

## Main Applications (`src/`)

### 1. EtherCAT Master (`src/a-main/`)

**Purpose**: Implements EtherCAT Master functionality for controlling and monitoring slave devices.

**Architecture**:
```
a-main/
├── app/main.cpp              # Application entry point
├── bus/main_socket.cpp       # Master-side socket implementation
├── mvc/
│   ├── controller/
│   │   └── main_controller.cpp    # Master controller logic
│   ├── model/
│   │   └── main_model.h          # Master data model
│   └── view/
│       └── main_tui.cpp           # Terminal UI (optional)
```

**Key Features**:
- EtherCAT cycle management with configurable timing
- Real-time communication with virtual slaves
- Optional TUI for monitoring and control
- Signal handling for graceful shutdown

**Controller Logic**:
```cpp
class MainController : public BaseController<MainModel> {
    void run() override {
        while (!shouldStop()) {
            // EtherCAT cycle processing
            processEtherCATCycle();
            sleepFor(std::chrono::microseconds(cycle_us_));
        }
    }
};
```

### 2. EtherCAT Slaves (`src/a-subs/`)

**Purpose**: Implements virtual EtherCAT slave devices for simulation.

**Architecture**:
```
a-subs/
├── app/main.cpp              # Application entry point
├── bus/subs_endpoint.cpp     # Slave-side endpoint implementation
├── mvc/
│   ├── controller/
│   │   └── subs_controller.cpp    # Slaves controller logic
│   ├── model/
│   │   └── subs_model.h           # Slaves data model
│   └── view/
│       └── subs_tui.cpp          # Terminal UI (optional)
└── sim/
    └── el1258_subs.h             # Specific slave device simulation
```

**Key Features**:
- Multiple virtual slave instances
- Device-specific simulation (e.g., EL1258 digital input module)
- Real-time response to master commands
- Optional TUI for monitoring slave states

## Build System Architecture

### CMake Structure

The build system uses a hierarchical CMake structure:

1. **Root CMakeLists.txt**: Project configuration, dependency management, feature flags
2. **Core Libraries**: `core/CMakeLists.txt` builds static libraries:
   - `ethercat_core`: Core simulation and communication
   - `ethercat_framework`: MVC framework base classes
   - `ethercat_kickcat_adapter`: KickCAT integration (optional)
   - `ethercat_dds`: FastDDS integration (optional)

3. **Applications**: Each application has its own CMakeLists.txt:
   - `src/a-main/CMakeLists.txt`: Master application
   - `src/a-subs/CMakeLists.txt`: Slaves application

### Dependency Management

- **KickCAT**: Required EtherCAT protocol library (via Conan or vendored)
- **FTXUI**: Optional terminal UI library (via Conan)
- **FastDDS**: Optional distributed communication (via Conan)

## Communication Flow

### Master-Slave Communication

1. **Master (`a-main`)**:
   - Creates socket connection to simulation bus
   - Sends EtherCAT frames via KickCAT
   - Receives responses and processes slave data

2. **Slaves (`a-subs`)**:
   - Connects to same simulation bus
   - Receives EtherCAT frames from master
   - Processes commands and updates virtual device state
   - Sends responses back to master

3. **Simulation Bus**:
   - Unix Domain Socket (`uds:///tmp/ethercat_bus.sock`) or TCP
   - Frame queuing with configurable latency
   - Virtual slave registry and addressing

### Data Flow Architecture

```
Master Application
    ↓ (EtherCAT Frames)
KickCAT Library
    ↓ (Socket I/O)
Simulation Bus (UDS/TCP)
    ↓ (Frame Processing)
Network Simulator
    ↓ (Slave Operations)
Virtual Slaves
    ↓ (Device Simulation)
EL1258/Other Devices
```

## Threading Model

### Controller Threading

- Each application uses `BaseController` with dedicated worker thread
- Atomic stop flags for graceful shutdown
- Thread-safe model updates with mutex protection

### Communication Threading

- Socket operations are thread-safe
- Frame queuing uses mutex-protected containers
- Observer pattern notifications are thread-safe

## Configuration and Runtime

### Command Line Interface

**Master (`a-main`)**:
```bash
a-main [--uds PATH | --tcp HOST:PORT] [--cycle us]
```

**Slaves (`a-subs`)**:
```bash
a-subs [--uds PATH | --tcp HOST:PORT] [--count N]
```

### Runtime Features

- Signal handling (SIGINT, SIGTERM, SIGTSTP)
- ESC key support for graceful shutdown
- Optional TUI when running in terminal
- Smoke test mode for CI/CD

## Extension Points

### Adding New Slave Types

1. Create device-specific header in `src/a-subs/sim/`
2. Implement virtual slave behavior in controller
3. Add device-specific PDO mapping

### Adding New Communication Protocols

1. Extend `SocketFactory` with new creator
2. Implement protocol-specific socket handling
3. Update endpoint parser for new URL schemes

### Adding New Applications

1. Follow MVC pattern in `src/`
2. Inherit from `BaseController` and `BaseModel`
3. Add CMakeLists.txt following existing pattern

## Testing Architecture

- Unit tests in `tests/` using GoogleTest
- Example applications in `examples/` for integration testing
- Smoke tests for CI/CD validation
- Coverage reporting with GCC/Clang support

This architecture provides a solid foundation for EtherCAT simulation with clear separation of concerns, extensibility, and maintainability.