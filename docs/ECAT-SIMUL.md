# EtherCAT 시뮬레이터 완전 가이드

## 목차
1. [EtherCAT 프로토콜 완전 이해](#ethercat-프로토콜-완전-이해)
2. [시뮬레이터 아키텍처 상세 분석](#시뮬레이터-아키텍처-상세-분석)
3. [빌드 및 설치 완전 가이드](#빌드-및-설치-완전-가이드)
4. [사용법 상세 설명](#사용법-상세-설명)
5. [실제 예제 및 시나리오](#실제-예제-및-시나리오)
6. [고급 설정 및 커스터마이징](#고급-설정-및-커스터마이징)
7. [문제 해결 및 디버깅](#문제-해결-및-디버깅)

## EtherCAT 프로토콜 완전 이해

### EtherCAT란 무엇인가?

EtherCAT(Ethernet for Control Automation Technology)는 현대 산업 자동화 분야에서 널리 사용되는 실시간 이더넷 통신 프로토콜입니다. 이 프로토콜은 기존의 표준 이더넷 기술을 기반으로 하되, 산업용 자동화 시스템에서 요구되는 극도로 빠른 응답 시간과 높은 정확성을 제공하도록 특별히 설계되었습니다. EtherCAT의 가장 큰 특징은 마이크로초 단위의 결정적 지연시간을 보장하면서도, 기존 이더넷 인프라를 그대로 활용할 수 있다는 점입니다.

전통적인 이더넷 통신에서는 각 디바이스가 개별적으로 프레임을 처리하고 응답을 보내는 방식이었지만, EtherCAT는 혁신적인 "온더플라이(On-the-fly)" 처리 방식을 도입하여 하나의 프레임이 네트워크를 순회하면서 모든 슬레이브 디바이스의 데이터를 동시에 처리할 수 있게 했습니다. 이는 네트워크 효율성을 극대화하고 통신 지연시간을 최소화하는 핵심 기술입니다.

```mermaid
graph TB
    subgraph "EtherCAT 네트워크 구조"
        MASTER[EtherCAT Master<br/>마스터]
        
        subgraph "EtherCAT 네트워크"
            SLAVE1[Slave 1<br/>EL1258<br/>디지털 입력]
            SLAVE2[Slave 2<br/>EL2004<br/>디지털 출력]
            SLAVE3[Slave 3<br/>EL3104<br/>아날로그 입력]
            SLAVEN[Slave N<br/>기타 디바이스]
        end
        
        subgraph "물리적 연결"
            ETH_CABLE[이더넷 케이블<br/>RJ45]
        end
    end
    
    MASTER --> ETH_CABLE
    ETH_CABLE --> SLAVE1
    SLAVE1 --> SLAVE2
    SLAVE2 --> SLAVE3
    SLAVE3 --> SLAVEN
    SLAVEN --> MASTER
    
    style MASTER fill:#e3f2fd
    style SLAVE1 fill:#e8f5e8
    style SLAVE2 fill:#fff3e0
    style SLAVE3 fill:#f3e5f5
    style SLAVEN fill:#fce4ec
```

### EtherCAT의 핵심 특징과 기술적 우위

#### 1. 실시간 성능의 혁신적 구현

EtherCAT의 가장 중요한 특징은 마이크로초 단위의 극도로 빠른 응답 시간을 보장한다는 점입니다. 일반적인 EtherCAT 네트워크에서는 100마이크로초 이하의 결정적 지연시간을 달성할 수 있으며, 이는 기존의 다른 산업용 통신 프로토콜들과 비교했을 때 혁신적인 성능입니다. 이러한 빠른 응답 시간은 산업용 로봇, CNC 머신, 자동화 생산라인과 같은 실시간 제어가 필수적인 애플리케이션에서 매우 중요한 요소입니다.

또한 EtherCAT는 분산 클록(Distributed Clock) 기술을 통해 네트워크상의 모든 디바이스가 동일한 시간 기준을 공유할 수 있게 합니다. 이는 여러 슬레이브 디바이스가 정확히 동시에 동작해야 하는 복잡한 제어 시스템에서 매우 중요한 기능입니다. 예를 들어, 여러 개의 모터가 정확히 동시에 시작되거나 정지되어야 하는 경우, 분산 클록을 통해 마이크로초 단위의 정밀한 동기화가 가능합니다.

#### 2. 효율적인 데이터 전송 메커니즘

EtherCAT의 혁신적인 데이터 전송 방식은 기존의 이더넷 통신과는 완전히 다른 접근 방식을 사용합니다. 전통적인 이더넷에서는 각 디바이스가 개별적으로 프레임을 수신하고 처리한 후 별도의 응답 프레임을 전송하는 방식이었지만, EtherCAT는 하나의 프레임이 네트워크를 순회하면서 모든 슬레이브 디바이스의 데이터를 동시에 처리하는 "온더플라이" 방식을 사용합니다.

이 방식에서는 마스터가 하나의 EtherCAT 프레임을 전송하면, 이 프레임이 네트워크상의 첫 번째 슬레이브에 도착하는 순간 해당 슬레이브는 자신의 데이터를 프레임에 추가하고 즉시 다음 슬레이브로 전달합니다. 이 과정이 모든 슬레이브를 거쳐 마지막 슬레이브에서 마스터로 다시 돌아올 때까지 계속되며, 결과적으로 하나의 프레임으로 모든 슬레이브와의 데이터 교환이 완료됩니다.

#### 3. 네트워크 토폴로지의 유연성

EtherCAT는 다양한 네트워크 토폴로지를 지원하여 다양한 산업 환경에 적합한 네트워크 구성을 가능하게 합니다. 가장 일반적으로 사용되는 선형 토폴로지는 슬레이브 디바이스들을 직렬로 연결하는 방식으로, 설치가 간단하고 케이블 비용을 최소화할 수 있습니다. 트리 토폴로지는 중앙에서 여러 분기로 나뉘는 구조로, 복잡한 생산라인에서 여러 섹션을 독립적으로 제어할 때 유용합니다.

스타 토폴로지는 모든 슬레이브가 중앙의 스위치나 허브에 직접 연결되는 방식으로, 각 슬레이브의 독립적인 제어가 중요한 경우에 사용됩니다. 마지막으로 링 토폴로지는 네트워크가 순환 구조를 이루는 방식으로, 높은 신뢰성이 요구되는 애플리케이션에서 사용됩니다.

### EtherCAT 통신 원리의 상세한 작동 메커니즘

EtherCAT의 통신 원리를 이해하기 위해서는 실제 데이터가 네트워크를 통해 어떻게 흐르는지 단계별로 살펴볼 필요가 있습니다. EtherCAT 통신은 매우 정교하게 설계된 프로세스로, 각 단계가 마이크로초 단위로 정확히 실행됩니다.

```mermaid
sequenceDiagram
    participant M as Master
    participant S1 as Slave 1
    participant S2 as Slave 2
    participant S3 as Slave 3
    
    Note over M,S3: EtherCAT 사이클 시작<br/>(마스터가 주기적으로 실행)
    
    M->>S1: EtherCAT 프레임 전송<br/>(명령 + 출력 데이터)
    Note right of S1: 프레임 수신 즉시<br/>데이터 처리 시작
    S1->>S1: 데이터 처리<br/>(읽기/쓰기 연산)
    Note right of S1: 처리 완료 후<br/>입력 데이터를 프레임에 추가
    S1->>S2: 수정된 프레임 전달<br/>(응답 데이터 포함)
    Note right of S2: 다음 슬레이브에서<br/>동일한 과정 반복
    S2->>S2: 데이터 처리
    S2->>S3: 수정된 프레임 전달
    S3->>S3: 데이터 처리
    S3->>M: 최종 응답 프레임<br/>(모든 슬레이브 응답 포함)
    
    Note over M,S3: 사이클 완료<br/>(일반적으로 < 100μs)
```

EtherCAT 통신 사이클의 첫 번째 단계는 마스터가 EtherCAT 프레임을 생성하고 네트워크로 전송하는 것입니다. 이 프레임에는 각 슬레이브에 대한 명령어와 출력 데이터가 포함되어 있습니다. 마스터는 미리 정의된 주기(일반적으로 1밀리초 이하)에 따라 이 프레임을 정기적으로 전송합니다.

프레임이 첫 번째 슬레이브에 도착하면, 해당 슬레이브는 프레임을 수신하는 동시에 자신에게 할당된 명령을 즉시 처리하기 시작합니다. 이 과정에서 슬레이브는 자신의 입력 데이터를 프레임에 추가하고, 출력 데이터가 있다면 이를 자신의 레지스터에 저장합니다. 이 모든 과정은 프레임이 다음 슬레이브로 전달되기 전에 완료되어야 하므로, 각 슬레이브는 매우 빠른 처리 속도를 가져야 합니다.

이 과정이 네트워크상의 모든 슬레이브를 거쳐 마지막 슬레이브에 도달하면, 마지막 슬레이브는 모든 슬레이브의 응답 데이터가 포함된 완전한 프레임을 마스터로 다시 전송합니다. 마스터는 이 응답 프레임을 수신하여 각 슬레이브의 상태와 입력 데이터를 확인하고, 다음 사이클을 위한 준비를 합니다.

### EtherCAT 프레임 구조의 상세한 분석

EtherCAT 프레임의 구조를 이해하는 것은 EtherCAT 통신의 핵심을 파악하는 데 매우 중요합니다. EtherCAT 프레임은 표준 이더넷 프레임을 기반으로 하되, EtherCAT 통신에 특화된 헤더와 데이터 구조를 가지고 있습니다.

```mermaid
graph LR
    subgraph "EtherCAT 프레임 구조"
        ETH_HDR[Ethernet Header<br/>14 bytes<br/>목적지/소스 MAC 주소]
        ECAT_HDR[EtherCAT Header<br/>2 bytes<br/>Length + Type 필드]
        ECAT_DATA[EtherCAT Data<br/>가변 길이<br/>명령어 + 데이터 페이로드]
        ETH_CRC[Ethernet CRC<br/>4 bytes<br/>순환 중복 검사]
    end
    
    ETH_HDR --> ECAT_HDR
    ECAT_HDR --> ECAT_DATA
    ECAT_DATA --> ETH_CRC
    
    style ETH_HDR fill:#e1f5fe
    style ECAT_HDR fill:#f3e5f5
    style ECAT_DATA fill:#e8f5e8
    style ETH_CRC fill:#fff3e0
```

EtherCAT 프레임의 첫 번째 구성 요소는 표준 이더넷 헤더입니다. 이 14바이트 헤더에는 목적지 MAC 주소(6바이트), 소스 MAC 주소(6바이트), 그리고 이더넷 타입 필드(2바이트)가 포함되어 있습니다. EtherCAT의 경우 이더넷 타입 필드는 0x88A4로 설정되어 이 프레임이 EtherCAT 프레임임을 나타냅니다.

EtherCAT 헤더는 2바이트로 구성되며, 첫 번째 바이트는 EtherCAT 데이터의 길이를 나타내고, 두 번째 바이트는 프레임 타입을 나타냅니다. 이 헤더는 EtherCAT 네트워크상의 모든 디바이스가 프레임을 올바르게 처리할 수 있도록 필요한 정보를 제공합니다.

EtherCAT 데이터 섹션은 실제 명령어와 데이터가 포함되는 부분으로, 길이가 가변적입니다. 이 섹션에는 각 슬레이브에 대한 명령어, 출력 데이터, 그리고 슬레이브로부터 받은 입력 데이터가 포함됩니다. 마지막으로 이더넷 CRC는 프레임의 무결성을 보장하기 위해 사용됩니다.

### PDO (Process Data Object) 시스템의 상세한 작동 원리

PDO(Process Data Object)는 EtherCAT에서 실시간 데이터 교환을 담당하는 핵심 메커니즘입니다. PDO 시스템을 이해하기 위해서는 마스터와 슬레이브 간의 데이터 흐름, 논리적 메모리 매핑, 그리고 실시간 데이터 교환 과정을 상세히 살펴볼 필요가 있습니다.

```mermaid
graph TB
    subgraph "PDO 데이터 교환 시스템"
        MASTER[EtherCAT Master<br/>마스터 애플리케이션]
        
        subgraph "논리적 메모리 공간"
            LOGICAL[논리적 주소 공간<br/>0x0000 ~ 0xFFFF<br/>마스터 관점의 통합 메모리]
        end
        
        subgraph "Slave 1 - EL1258 디지털 입력"
            S1_INPUT[Input PDO<br/>디지털 입력 상태<br/>8비트 데이터]
            S1_OUTPUT[Output PDO<br/>설정 데이터<br/>상태 제어]
        end
        
        subgraph "Slave 2 - EL2004 디지털 출력"
            S2_INPUT[Input PDO<br/>출력 상태 피드백<br/>8비트 데이터]
            S2_OUTPUT[Output PDO<br/>디지털 출력 명령<br/>8비트 데이터]
        end
        
        subgraph "Slave 3 - EL3104 아날로그 입력"
            S3_INPUT[Input PDO<br/>아날로그 입력 값<br/>16비트 × 4채널]
            S3_OUTPUT[Output PDO<br/>필터 설정<br/>게인 제어]
        end
    end
    
    MASTER -->|Output PDO<br/>논리적 주소 0x1000| S1_OUTPUT
    MASTER -->|Output PDO<br/>논리적 주소 0x1008| S2_OUTPUT
    MASTER -->|Output PDO<br/>논리적 주소 0x1010| S3_OUTPUT
    
    S1_INPUT -->|Input PDO<br/>논리적 주소 0x2000| MASTER
    S2_INPUT -->|Input PDO<br/>논리적 주소 0x2008| MASTER
    S3_INPUT -->|Input PDO<br/>논리적 주소 0x2010| MASTER
    
    LOGICAL -.->|주소 매핑<br/>0x1000-0x1007| S1_OUTPUT
    LOGICAL -.->|주소 매핑<br/>0x1008-0x100F| S2_OUTPUT
    LOGICAL -.->|주소 매핑<br/>0x1010-0x101F| S3_OUTPUT
    
    LOGICAL -.->|주소 매핑<br/>0x2000-0x2007| S1_INPUT
    LOGICAL -.->|주소 매핑<br/>0x2008-0x200F| S2_INPUT
    LOGICAL -.->|주소 매핑<br/>0x2010-0x201F| S3_INPUT
    
    style MASTER fill:#e3f2fd
    style LOGICAL fill:#f3e5f5
    style S1_INPUT fill:#e8f5e8
    style S1_OUTPUT fill:#fff3e0
    style S2_INPUT fill:#e8f5e8
    style S2_OUTPUT fill:#fff3e0
    style S3_INPUT fill:#e8f5e8
    style S3_OUTPUT fill:#fff3e0
```

PDO 시스템의 핵심은 논리적 메모리 매핑입니다. 마스터 애플리케이션은 물리적으로 분산된 여러 슬레이브의 데이터를 하나의 연속된 논리적 메모리 공간으로 인식할 수 있습니다. 예를 들어, 논리적 주소 0x1000부터 0x1007까지는 첫 번째 슬레이브의 출력 데이터에 매핑되고, 0x2000부터 0x2007까지는 첫 번째 슬레이브의 입력 데이터에 매핑됩니다.

이러한 매핑을 통해 마스터 애플리케이션은 복잡한 네트워크 구조를 신경 쓰지 않고도 단순한 메모리 읽기/쓰기 연산으로 모든 슬레이브와 통신할 수 있습니다. 이는 프로그래밍을 크게 단순화하고 실시간 성능을 향상시키는 중요한 요소입니다.

## 시뮬레이터 아키텍처 상세 분석

### 전체 시스템 구조와 컴포넌트 간의 상호작용

EtherCAT 시뮬레이터는 실제 EtherCAT 네트워크를 소프트웨어로 시뮬레이션하는 복합적인 시스템입니다. 이 시스템의 아키텍처를 이해하기 위해서는 각 계층의 역할과 컴포넌트 간의 상호작용을 상세히 살펴볼 필요가 있습니다.

```mermaid
graph TB
    subgraph "EtherCAT 시뮬레이터 전체 시스템"
        subgraph "사용자 인터페이스 계층"
            TUI_MAIN[터미널 UI<br/>a-main TUI<br/>실시간 모니터링]
            TUI_SUBS[터미널 UI<br/>a-subs TUI<br/>슬레이브 상태 표시]
            CLI[명령줄 인터페이스<br/>CLI 옵션<br/>설정 및 제어]
        end
        
        subgraph "애플리케이션 계층"
            A_MAIN[a-main 애플리케이션<br/>EtherCAT 마스터 시뮬레이션<br/>사이클 관리 및 제어]
            A_SUBS[a-subs 애플리케이션<br/>EtherCAT 슬레이브 시뮬레이션<br/>디바이스 모델링]
        end
        
        subgraph "비즈니스 로직 계층"
            MAIN_CONTROLLER[Main Controller<br/>마스터 제어 로직<br/>사이클 처리]
            SUBS_CONTROLLER[Subs Controller<br/>슬레이브 제어 로직<br/>디바이스 시뮬레이션]
            MAIN_MODEL[Main Model<br/>마스터 데이터 모델<br/>상태 관리]
            SUBS_MODEL[Subs Model<br/>슬레이브 데이터 모델<br/>디바이스 상태]
        end
        
        subgraph "프레임워크 계층"
            MVC_FRAMEWORK[MVC Framework<br/>Model-View-Controller<br/>기본 클래스 제공]
            BASE_CONTROLLER[Base Controller<br/>스레드 관리<br/>생명주기 제어]
            BASE_MODEL[Base Model<br/>옵저버 패턴<br/>데이터 동기화]
        end
        
        subgraph "시뮬레이션 엔진 계층"
            NET_SIM[Network Simulator<br/>네트워크 시뮬레이션<br/>프레임 처리 및 지연시간]
            VIRTUAL_SLAVES[Virtual Slaves<br/>가상 슬레이브 관리<br/>디바이스 모델]
            FRAME_QUEUE[Frame Queue<br/>프레임 큐잉<br/>지연시간 시뮬레이션]
        end
        
        subgraph "통신 추상화 계층"
            SOCKET_FACTORY[Socket Factory<br/>소켓 생성 팩토리<br/>프로토콜 추상화]
            ENDPOINT_PARSER[Endpoint Parser<br/>엔드포인트 파싱<br/>URL 해석]
            UDS_HANDLER[UDS Handler<br/>Unix Domain Socket<br/>로컬 통신]
            TCP_HANDLER[TCP Handler<br/>TCP Socket<br/>네트워크 통신]
        end
        
        subgraph "프로토콜 어댑터 계층"
            KICKCAT_ADAPTER[KickCAT Adapter<br/>프로토콜 어댑터<br/>EtherCAT 구현]
            SIM_SOCKET[Sim Socket<br/>시뮬레이션 소켓<br/>가상 네트워크 인터페이스]
        end
        
        subgraph "외부 라이브러리 계층"
            KICKCAT[KickCAT Library<br/>EtherCAT 프로토콜<br/>실제 EtherCAT 구현]
            FTXUI[FTXUI Library<br/>터미널 UI<br/>사용자 인터페이스]
            FASTDDS[FastDDS Library<br/>분산 통신<br/>DDS 프로토콜]
        end
        
        subgraph "시스템 통신 버스"
            UDS_BUS[Unix Domain Socket<br/>uds:///tmp/ethercat_bus.sock<br/>로컬 프로세스 간 통신]
            TCP_BUS[TCP Socket<br/>tcp://localhost:8080<br/>네트워크 통신]
        end
    end
    
    %% 사용자 인터페이스 연결
    TUI_MAIN --> A_MAIN
    TUI_SUBS --> A_SUBS
    CLI --> A_MAIN
    CLI --> A_SUBS
    
    %% 애플리케이션 계층 연결
    A_MAIN --> MAIN_CONTROLLER
    A_SUBS --> SUBS_CONTROLLER
    MAIN_CONTROLLER --> MAIN_MODEL
    SUBS_CONTROLLER --> SUBS_MODEL
    
    %% 비즈니스 로직과 프레임워크 연결
    MAIN_CONTROLLER --> BASE_CONTROLLER
    SUBS_CONTROLLER --> BASE_CONTROLLER
    MAIN_MODEL --> BASE_MODEL
    SUBS_MODEL --> BASE_MODEL
    
    %% 프레임워크와 시뮬레이션 엔진 연결
    BASE_CONTROLLER --> NET_SIM
    BASE_MODEL --> NET_SIM
    NET_SIM --> VIRTUAL_SLAVES
    NET_SIM --> FRAME_QUEUE
    
    %% 통신 추상화 연결
    A_MAIN --> SOCKET_FACTORY
    A_SUBS --> SOCKET_FACTORY
    SOCKET_FACTORY --> ENDPOINT_PARSER
    ENDPOINT_PARSER --> UDS_HANDLER
    ENDPOINT_PARSER --> TCP_HANDLER
    
    %% 프로토콜 어댑터 연결
    SOCKET_FACTORY --> KICKCAT_ADAPTER
    KICKCAT_ADAPTER --> SIM_SOCKET
    SIM_SOCKET --> KICKCAT
    
    %% 통신 버스 연결
    UDS_HANDLER --> UDS_BUS
    TCP_HANDLER --> TCP_BUS
    UDS_BUS --> NET_SIM
    TCP_BUS --> NET_SIM
    
    %% 외부 라이브러리 연결
    TUI_MAIN -.-> FTXUI
    TUI_SUBS -.-> FTXUI
    KICKCAT_ADAPTER -.-> FASTDDS
    
    %% 스타일링
    style A_MAIN fill:#e3f2fd
    style A_SUBS fill:#e8f5e8
    style NET_SIM fill:#f3e5f5
    style UDS_BUS fill:#fff3e0
    style TCP_BUS fill:#fff3e0
    style KICKCAT fill:#fce4ec
    style FTXUI fill:#e0f2f1
```

### 시뮬레이션 원리

실제 EtherCAT 네트워크를 소프트웨어로 시뮬레이션합니다:

1. **가상 네트워크**: 실제 이더넷 대신 소켓 통신 사용
2. **가상 슬레이브**: 실제 하드웨어 대신 소프트웨어 모델
3. **프레임 시뮬레이션**: EtherCAT 프레임을 소프트웨어로 처리
4. **지연시간 모델링**: 네트워크 지연시간 시뮬레이션

## 빌드 및 설치 완전 가이드

### 시스템 요구사항과 환경 설정

EtherCAT 시뮬레이터를 성공적으로 빌드하고 실행하기 위해서는 적절한 개발 환경이 필요합니다. 이 섹션에서는 시스템 요구사항부터 실제 빌드 과정까지 단계별로 상세히 설명합니다.

#### 기본 시스템 요구사항

- **운영체제**: Linux (Ubuntu 20.04 LTS 이상 권장, CentOS 8+ 지원)
- **컴파일러**: GCC 9.0 이상 또는 Clang 10.0 이상
- **CMake**: 버전 3.20 이상 (최신 기능 활용을 위해)
- **Conan**: 버전 2.0 이상 (의존성 관리 및 패키지 빌드)
- **메모리**: 최소 4GB RAM (빌드 시 8GB 권장)
- **디스크 공간**: 최소 2GB 여유 공간 (의존성 포함 시 5GB 권장)

### 빌드 프로세스 전체 흐름

```mermaid
flowchart TD
    START([프로젝트 시작]) --> CHECK_SYS{시스템 요구사항<br/>확인}
    
    CHECK_SYS -->|요구사항 미충족| INSTALL_DEPS[의존성 설치<br/>GCC, CMake, Conan]
    CHECK_SYS -->|요구사항 충족| CLONE_REPO[저장소 클론<br/>git clone]
    
    INSTALL_DEPS --> CLONE_REPO
    CLONE_REPO --> CONAN_PROFILE[Conan 프로필 설정<br/>conan profile detect]
    
    CONAN_PROFILE --> CONAN_INSTALL[Conan 의존성 설치<br/>conan install]
    CONAN_INSTALL --> CMAKE_CONFIG[CMake 설정<br/>cmake configure]
    
    CMAKE_CONFIG --> CMAKE_BUILD[CMake 빌드<br/>cmake --build]
    CMAKE_BUILD --> BUILD_SUCCESS{빌드 성공?}
    
    BUILD_SUCCESS -->|실패| DEBUG_BUILD[빌드 오류 디버깅<br/>로그 확인]
    BUILD_SUCCESS -->|성공| RUN_TESTS[테스트 실행<br/>ctest]
    
    DEBUG_BUILD --> CMAKE_CONFIG
    RUN_TESTS --> TEST_SUCCESS{테스트 통과?}
    
    TEST_SUCCESS -->|실패| DEBUG_TESTS[테스트 오류 디버깅]
    TEST_SUCCESS -->|통과| INSTALL_APPS[애플리케이션 설치<br/>make install]
    
    DEBUG_TESTS --> RUN_TESTS
    INSTALL_APPS --> READY([시뮬레이터 준비 완료])
    
    %% 스타일링
    style START fill:#e8f5e8
    style READY fill:#e8f5e8
    style CHECK_SYS fill:#fff3e0
    style BUILD_SUCCESS fill:#fff3e0
    style TEST_SUCCESS fill:#fff3e0
    style DEBUG_BUILD fill:#ffebee
    style DEBUG_TESTS fill:#ffebee
```

### 의존성 설치

#### Ubuntu/Debian
```bash
# 기본 개발 도구
sudo apt update
sudo apt install -y build-essential cmake git

# Conan 설치
pip install conan

# 선택적 의존성 (터미널 UI)
sudo apt install -y libncurses-dev
```

#### CentOS/RHEL
```bash
# 기본 개발 도구
sudo yum groupinstall -y "Development Tools"
sudo yum install -y cmake3 git

# Conan 설치
pip install conan

# 선택적 의존성
sudo yum install -y ncurses-devel
```

### 프로젝트 빌드

#### 1. 저장소 클론
```bash
git clone https://github.com/your-repo/ethercat-simulator.git
cd ethercat-simulator
```

#### 2. Conan 의존성 설치
```bash
# Conan 프로필 설정
conan profile detect --force

# 의존성 설치
conan install . --output-folder=build --build=missing
```

#### 3. CMake 빌드
```bash
# 빌드 디렉토리 생성
mkdir build && cd build

# CMake 설정
cmake .. -DCMAKE_TOOLCHAIN_FILE=../build/conan_toolchain.cmake

# 빌드 실행
cmake --build . --parallel
```

#### 4. 빌드 스크립트 사용 (권장)
```bash
# 전체 빌드 (Release 모드)
./build.sh

# Debug 모드 빌드
./build.sh --debug

# 클린 빌드
./build.sh --clean
```

### 빌드 옵션

```bash
# CMake 옵션 설정
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_EXAMPLES=ON \
    -DBUILD_GUI=ON \
    -DENABLE_COVERAGE=ON \
    -DFORCE_NO_FTXUI=OFF
```

**주요 옵션**:
- `BUILD_EXAMPLES`: 예제 애플리케이션 빌드
- `BUILD_GUI`: Qt GUI 애플리케이션 빌드
- `ENABLE_COVERAGE`: 코드 커버리지 활성화
- `FORCE_NO_FTXUI`: FTXUI 비활성화

## 사용법 상세 설명

### 기본 실행 과정과 시뮬레이터 동작 원리

EtherCAT 시뮬레이터를 사용하는 과정은 실제 EtherCAT 네트워크를 구성하고 운영하는 과정과 매우 유사합니다. 이 섹션에서는 시뮬레이터의 기본 실행 방법부터 고급 사용법까지 단계별로 상세히 설명합니다.

### 시뮬레이터 실행 흐름도

```mermaid
sequenceDiagram
    participant USER as 사용자
    participant SUBS as a-subs<br/>(슬레이브 시뮬레이터)
    participant MAIN as a-main<br/>(마스터 시뮬레이터)
    participant BUS as 시뮬레이션 버스<br/>(UDS/TCP)
    participant SIM as 네트워크 시뮬레이터
    participant SLAVES as 가상 슬레이브들
    
    Note over USER,SLAVES: 1단계: 슬레이브 시뮬레이터 시작
    
    USER->>SUBS: 슬레이브 실행 명령<br/>./a-subs --count 3
    SUBS->>SIM: 네트워크 시뮬레이터 초기화
    SIM->>SLAVES: 가상 슬레이브 생성<br/>(EL1258, EL2004 등)
    SUBS->>BUS: 통신 버스 연결<br/>uds:///tmp/ethercat_bus.sock
    SUBS-->>USER: 슬레이브 준비 완료<br/>3개 슬레이브 온라인
    
    Note over USER,SLAVES: 2단계: 마스터 시뮬레이터 시작
    
    USER->>MAIN: 마스터 실행 명령<br/>./a-main --cycle 1000
    MAIN->>BUS: 통신 버스 연결<br/>동일한 소켓 사용
    MAIN->>SIM: 네트워크 시뮬레이터 연결
    MAIN-->>USER: 마스터 준비 완료<br/>사이클 시간: 1000μs
    
    Note over USER,SLAVES: 3단계: EtherCAT 통신 시작
    
    loop EtherCAT 사이클 (1ms마다)
        MAIN->>BUS: EtherCAT 프레임 전송<br/>(슬레이브 스캔 + PDO 교환)
        BUS->>SIM: 프레임 처리<br/>(지연시간 시뮬레이션)
        SIM->>SLAVES: 슬레이브별 데이터 처리<br/>(읽기/쓰기 연산)
        SLAVES->>SIM: 응답 데이터 생성<br/>(입력 상태, 출력 피드백)
        SIM->>BUS: 수정된 프레임 전달
        BUS->>MAIN: 응답 프레임 수신<br/>(모든 슬레이브 데이터 포함)
        MAIN->>MAIN: 데이터 분석 및 처리<br/>(상태 업데이트, 제어 로직)
    end
    
    Note over USER,SLAVES: 4단계: 모니터링 및 제어
    
    USER->>MAIN: TUI를 통한 실시간 모니터링<br/>ESC 키로 종료
    MAIN->>USER: 슬레이브 상태 표시<br/>(온라인/오프라인, 데이터 값)
    USER->>SUBS: 슬레이브 상태 확인<br/>TUI를 통한 디바이스 모니터링
    
    Note over USER,SLAVES: 5단계: 우아한 종료
    
    USER->>MAIN: 종료 신호 (ESC 또는 Ctrl+C)
    USER->>SUBS: 종료 신호
    MAIN->>BUS: 연결 해제
    SUBS->>BUS: 연결 해제
    MAIN-->>USER: 마스터 종료 완료
    SUBS-->>USER: 슬레이브 종료 완료
```

### 기본 실행 방법

#### 1. 슬레이브 시뮬레이터 시작 과정

슬레이브 시뮬레이터는 실제 EtherCAT 슬레이브 디바이스들의 동작을 소프트웨어로 시뮬레이션하는 애플리케이션입니다. 이 애플리케이션을 시작하면 지정된 개수만큼의 가상 슬레이브가 생성되고, 마스터의 명령을 기다리게 됩니다.

```bash
# 기본 설정으로 슬레이브 1개 실행
# 이 명령은 Unix Domain Socket을 사용하여 로컬 통신을 수행합니다
./build/src/a-subs/a-subs

# 여러 슬레이브 실행 (3개의 가상 슬레이브 생성)
# 각 슬레이브는 고유한 주소를 가지며 독립적으로 동작합니다
./build/src/a-subs/a-subs --count 3

# TCP 연결을 사용한 원격 통신
# 네트워크를 통해 다른 시스템의 마스터와 통신할 수 있습니다
./build/src/a-subs/a-subs --tcp localhost:8080 --count 2
```

#### 2. 마스터 시뮬레이터 시작 과정

마스터 시뮬레이터는 EtherCAT 네트워크의 제어 중심이 되는 애플리케이션입니다. 이 애플리케이션은 주기적으로 EtherCAT 프레임을 전송하여 슬레이브들과 통신하고, 실시간 데이터 교환을 수행합니다.

```bash
# 기본 설정으로 마스터 실행
# 기본 사이클 시간은 1000마이크로초(1밀리초)로 설정됩니다
./build/src/a-main/a-main

# 사이클 시간을 500마이크로초로 설정
# 더 빠른 응답이 필요한 애플리케이션에 적합합니다
./build/src/a-main/a-main --cycle 500

# TCP 연결을 사용한 원격 통신
# 네트워크를 통해 다른 시스템의 슬레이브들과 통신합니다
./build/src/a-main/a-main --tcp localhost:8080 --cycle 1000
```

### 고급 사용법

#### 동시 실행 스크립트
```bash
# 터미널 1: 슬레이브 실행
./build/src/a-subs/a-subs --count 3

# 터미널 2: 마스터 실행
./build/src/a-main/a-main --cycle 1000
```

#### 자동화 스크립트
```bash
# 제공된 실행 스크립트 사용
./a-subs.sh  # 슬레이브 실행
./a-main.sh  # 마스터 실행
```

### 명령줄 옵션

#### a-subs (슬레이브)
```bash
a-subs [옵션]

옵션:
  --uds PATH        Unix Domain Socket 경로 (기본: /tmp/ethercat_bus.sock)
  --tcp HOST:PORT   TCP 연결 (예: localhost:8080)
  --count N         가상 슬레이브 개수 (기본: 1)
  -h, --help        도움말 표시
```

#### a-main (마스터)
```bash
a-main [옵션]

옵션:
  --uds PATH        Unix Domain Socket 경로 (기본: /tmp/ethercat_bus.sock)
  --tcp HOST:PORT   TCP 연결 (예: localhost:8080)
  --cycle us        EtherCAT 사이클 시간 (마이크로초, 기본: 1000)
  -h, --help        도움말 표시
```

## 실제 예제

### 예제 1: 기본 디지털 입출력 시뮬레이션

#### 1단계: 슬레이브 실행
```bash
# 터미널 1에서 실행
./build/src/a-subs/a-subs --count 2
```

#### 2단계: 마스터 실행
```bash
# 터미널 2에서 실행
./build/src/a-main/a-main --cycle 1000
```

#### 3단계: 동작 확인
- 마스터가 슬레이브를 스캔하고 PDO 교환을 시작합니다
- TUI에서 실시간 상태를 모니터링할 수 있습니다
- ESC 키로 우아하게 종료할 수 있습니다

### 예제 2: 네트워크 지연시간 시뮬레이션

#### 설정 파일 생성
```json
{
  "network": {
    "latency_ms": 5,
    "jitter_ms": 1,
    "packet_loss": 0.001
  },
  "slaves": [
    {
      "type": "EL1258",
      "address": 1,
      "digital_inputs": 8
    },
    {
      "type": "EL2004", 
      "address": 2,
      "digital_outputs": 8
    }
  ]
}
```

#### 실행
```bash
# 설정 파일과 함께 실행
./build/src/a-subs/a-subs --config config.json --count 2
./build/src/a-main/a-main --config config.json --cycle 500
```

### 예제 3: 분산 시뮬레이션

#### 원격 슬레이브 실행
```bash
# 원격 서버에서 슬레이브 실행
./build/src/a-subs/a-subs --tcp 0.0.0.0:8080 --count 3
```

#### 로컬 마스터 실행
```bash
# 로컬에서 마스터 실행
./build/src/a-main/a-main --tcp remote-server:8080 --cycle 1000
```

## 고급 설정

### 네트워크 시뮬레이터 설정

#### 지연시간 설정
```cpp
// 네트워크 시뮬레이터에서 지연시간 설정
NetworkSimulator simulator;
simulator.setLatencyMs(10);  // 10ms 지연시간
simulator.setLinkUp(true);   // 링크 활성화
```

#### 슬레이브 구성
```cpp
// 가상 슬레이브 추가
auto slave1 = std::make_shared<EL1258Subs>(1);  // 주소 1
auto slave2 = std::make_shared<EL2004Subs>(2);  // 주소 2

simulator.addVirtualSlave(slave1);
simulator.addVirtualSlave(slave2);
```

### 커스텀 슬레이브 개발

#### 새로운 슬레이브 타입 추가
```cpp
// sim/custom_slave.h
class CustomSlave : public VirtualSlave {
public:
    CustomSlave(std::uint16_t address) 
        : VirtualSlave(address, 0x00000002, 0x12345678, "Custom") {
        initializeCustomFeatures();
    }
    
    // 커스텀 기능 구현
    bool readCustomData(uint8_t* data, std::size_t len) override {
        // 커스텀 데이터 읽기 로직
        return true;
    }
    
    bool writeCustomData(const uint8_t* data, std::size_t len) override {
        // 커스텀 데이터 쓰기 로직
        return true;
    }
    
private:
    void initializeCustomFeatures() {
        // 커스텀 기능 초기화
    }
};
```

### 성능 튜닝

#### 사이클 시간 최적화
```bash
# 고성능 모드 (500μs 사이클)
./build/src/a-main/a-main --cycle 500

# 실시간 모드 (100μs 사이클)
./build/src/a-main/a-main --cycle 100
```

#### 메모리 최적화
```bash
# 메모리 사용량 모니터링
valgrind --tool=massif ./build/src/a-main/a-main

# 메모리 누수 검사
valgrind --leak-check=full ./build/src/a-main/a-main
```

## 문제 해결

### 일반적인 문제

#### 1. 소켓 연결 실패
```bash
# 오류: 소켓 연결 실패
# 해결: 소켓 파일 권한 확인
ls -la /tmp/ethercat_bus.sock
sudo chmod 666 /tmp/ethercat_bus.sock
```

#### 2. 빌드 실패
```bash
# 오류: KickCAT 라이브러리 없음
# 해결: Conan 의존성 재설치
conan install . --build=missing --update
```

#### 3. 런타임 오류
```bash
# 오류: 세그멘테이션 폴트
# 해결: 디버그 모드로 빌드
./build.sh --debug
gdb ./build/src/a-main/a-main
```

### 로그 및 디버깅

#### 로그 레벨 설정
```bash
# 환경 변수로 로그 레벨 설정
export ETHERCAT_LOG_LEVEL=DEBUG
./build/src/a-main/a-main
```

#### 상세 로그 출력
```bash
# 상세 로그와 함께 실행
./build/src/a-main/a-main --verbose 2>&1 | tee master.log
./build/src/a-subs/a-subs --verbose 2>&1 | tee slaves.log
```

### 성능 모니터링

#### 시스템 리소스 모니터링
```bash
# CPU 및 메모리 사용량 모니터링
htop

# 네트워크 통계
netstat -i

# 소켓 통계
ss -tuln
```

#### 프로파일링
```bash
# CPU 프로파일링
perf record -g ./build/src/a-main/a-main
perf report

# 메모리 프로파일링
valgrind --tool=massif ./build/src/a-main/a-main
```

### 테스트 및 검증

#### 단위 테스트 실행
```bash
# 모든 테스트 실행
./test.sh

# 특정 테스트 실행
cd build-tests
ctest -R test_communication
```

#### 통합 테스트
```bash
# 예제 애플리케이션 테스트
cd examples
./run_examples.sh
```

### FAQ (자주 묻는 질문)

#### Q: 시뮬레이터가 실제 EtherCAT 하드웨어와 호환되나요?
A: 아니요. 이 시뮬레이터는 소프트웨어 시뮬레이션으로, 실제 EtherCAT 하드웨어와는 통신하지 않습니다.

#### Q: 최대 몇 개의 슬레이브를 시뮬레이션할 수 있나요?
A: 이론적으로는 수천 개까지 가능하지만, 성능상 권장은 100개 이하입니다.

#### Q: 실시간 성능을 보장하나요?
A: 일반적인 Linux 환경에서는 하드 실시간을 보장하지 않습니다. 실시간 커널(RT kernel) 사용을 권장합니다.

#### Q: 다른 EtherCAT 마스터와 호환되나요?
A: KickCAT 라이브러리를 사용하므로 KickCAT 호환 마스터와는 호환됩니다.

## 추가 리소스

### 문서
- [EtherCAT 기술 사양서](https://www.ethercat.org/)
- [KickCAT 라이브러리 문서](https://github.com/OpenEtherCATsociety/SOEM)
- [CMake 공식 문서](https://cmake.org/documentation/)

### 커뮤니티
- [EtherCAT 기술 포럼](https://www.ethercat.org/en/community.html)
- [GitHub Issues](https://github.com/your-repo/ethercat-simulator/issues)

### 라이선스
이 프로젝트는 MIT 라이선스 하에 배포됩니다. 자세한 내용은 LICENSE 파일을 참조하세요.

---

**주의사항**: 이 시뮬레이터는 교육 및 개발 목적으로만 사용되어야 하며, 실제 산업 환경에서의 사용 전 충분한 테스트가 필요합니다.