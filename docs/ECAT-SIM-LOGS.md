2025-09-20 20:11:22.499 [INFO ] [slaves] SlavesEndpoint::run() started with endpoint: uds:///tmp/ethercat_bus.sock
2025-09-20 20:11:22.499 [INFO ] [slaves] Parsed UDS endpoint, path: /tmp/ethercat_bus.sock
2025-09-20 20:11:22.499 [INFO ] [slaves] Socket bound and listening, fd=3
2025-09-20 20:11:22.499 [INFO ] [slaves] Successfully bound and listening UDS /tmp/ethercat_bus.sock
2025-09-20 20:11:22.499 [INFO ] [slaves] Entering accept loop, waiting for connections...
2025-09-20 20:11:32.602 [INFO ] [slaves] Accept returned fd=4
2025-09-20 20:11:32.602 [INFO ] [slaves] Client connected
2025-09-20 20:11:32.602 [INFO ] [core  ] NetworkSimulator initialized
2025-09-20 20:11:32.602 [DEBUG] [core  ] VirtualSlave constructor: addr=1, vendor=0x2, product=0x82456658, name='EL1258'
2025-09-20 20:11:32.602 [DEBUG] [core  ] VirtualSlave[1] initialized AL state to INIT
2025-09-20 20:11:32.602 [DEBUG] [core  ] VirtualSlave[1] initialization complete
2025-09-20 20:11:32.602 [DEBUG] [core  ] El1258Slave[1] constructor: vendor=0x2, product=0x82456658
2025-09-20 20:11:32.602 [DEBUG] [core  ] El1258Slave[1] applying multi-timestamping TxPDO mapping
2025-09-20 20:11:32.602 [DEBUG] [core  ] El1258Slave[1] Multi-timestamping TxPDO mapping applied: map_count=8, assigned=true, input_pdo_mapped=true
2025-09-20 20:11:32.602 [DEBUG] [core  ] El1258Slave[1] constructor complete
2025-09-20 20:11:32.602 [DEBUG] [core  ] VirtualSlave[1] starting
2025-09-20 20:11:32.809 [DEBUG] [core  ] SlavesEndpoint FPR  adp=0 ado=0x0130 len=8 ok=0
2025-09-20 20:11:32.809 [DEBUG] [core  ] SlavesEndpoint FPRD AL_STATUS adp=0 value=0x0000
2025-09-20 20:11:33.018 [DEBUG] [core  ] SlavesEndpoint B W ado=0x0101 len=1 ok=1
2025-09-20 20:11:33.225 [DEBUG] [core  ] SlavesEndpoint B W ado=0x0300 len=20 ok=1
2025-09-20 20:11:33.433 [DEBUG] [core  ] SlavesEndpoint B W ado=0x0600 len=256 ok=1
2025-09-20 20:11:33.641 [DEBUG] [core  ] SlavesEndpoint B W ado=0x0800 len=128 ok=1
2025-09-20 20:11:33.849 [DEBUG] [core  ] SlavesEndpoint B W ado=0x0910 len=8 ok=1
2025-09-20 20:11:34.057 [DEBUG] [core  ] SlavesEndpoint B W ado=0x0981 len=1 ok=1
2025-09-20 20:11:34.265 [DEBUG] [core  ] SlavesEndpoint B W ado=0x0200 len=2 ok=1
2025-09-20 20:11:34.473 [DEBUG] [core  ] SlavesEndpoint B W ado=0x0930 len=2 ok=1
2025-09-20 20:11:34.681 [DEBUG] [core  ] SlavesEndpoint B W ado=0x0934 len=2 ok=1
2025-09-20 20:11:35.513 [DEBUG] [core  ] SlavesEndpoint B W ado=0x0400 len=2 ok=1
2025-09-20 20:11:35.721 [DEBUG] [core  ] SlavesEndpoint B W ado=0x0410 len=2 ok=1
2025-09-20 20:11:35.929 [DEBUG] [core  ] SlavesEndpoint B W ado=0x0420 len=2 ok=1
2025-09-20 20:11:36.137 [DEBUG] [core  ] SlavesEndpoint B W ado=0x0500 len=2 ok=1
2025-09-20 20:11:36.346 [DEBUG] [core  ] SlavesEndpoint APW  idx=0 ado=0x0010 len=2 ok=1
2025-09-20 20:11:36.553 [DEBUG] [core  ] idx=0 AL_CONTROL=0x11 len=2
2025-09-20 20:11:36.553 [DEBUG] [core  ] VirtualSlave[1001] AL_CONTROL write: 0x17 current state: 1
2025-09-20 20:11:36.553 [DEBUG] [core  ] SlavesEndpoint B W ado=0x0120 len=2 ok=1
2025-09-20 20:11:36.553 [DEBUG] [core  ] SlavesEndpoint B W AL_CONTROL value=0x0011
2025-09-20 20:11:36.781 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0130 len=6 ok=1
2025-09-20 20:11:36.782 [DEBUG] [core  ] SlavesEndpoint FPRD AL_STATUS adp=1001 value=0x0001
[slave 1001] EEPROM control write cmd=0x100 addr=0x0
2025-09-20 20:11:36.782 [DEBUG] [core  ] SlavesEndpoint B W ado=0x0502 len=6 ok=1
2025-09-20 20:11:36.986 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0502 len=2 ok=1
[slave 1001] EEPROM read addr=0 data=0x180
2025-09-20 20:11:36.987 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0508 len=4 ok=1
[slave 1001] EEPROM control write cmd=0x100 addr=0x2
2025-09-20 20:11:36.987 [DEBUG] [core  ] SlavesEndpoint B W ado=0x0502 len=6 ok=1
2025-09-20 20:11:37.195 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0502 len=2 ok=1
[slave 1001] EEPROM read addr=2 data=0x0
2025-09-20 20:11:37.196 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0508 len=4 ok=1
[slave 1001] EEPROM control write cmd=0x100 addr=0x4
2025-09-20 20:11:37.196 [DEBUG] [core  ] SlavesEndpoint B W ado=0x0502 len=6 ok=1
2025-09-20 20:11:37.402 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0502 len=2 ok=1
[slave 1001] EEPROM read addr=4 data=0x0
2025-09-20 20:11:37.403 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0508 len=4 ok=1
[slave 1001] EEPROM control write cmd=0x100 addr=0x6
2025-09-20 20:11:37.403 [DEBUG] [core  ] SlavesEndpoint B W ado=0x0502 len=6 ok=1
2025-09-20 20:11:37.610 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0502 len=2 ok=1
[slave 1001] EEPROM read addr=6 data=0x0
2025-09-20 20:11:37.611 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0508 len=4 ok=1
[slave 1001] EEPROM control write cmd=0x100 addr=0x8
2025-09-20 20:11:37.611 [DEBUG] [core  ] SlavesEndpoint B W ado=0x0502 len=6 ok=1
2025-09-20 20:11:37.818 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0502 len=2 ok=1
[slave 1001] EEPROM read addr=8 data=0x2
2025-09-20 20:11:37.819 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0508 len=4 ok=1
[slave 1001] EEPROM control write cmd=0x100 addr=0xa
2025-09-20 20:11:37.819 [DEBUG] [core  ] SlavesEndpoint B W ado=0x0502 len=6 ok=1
2025-09-20 20:11:38.026 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0502 len=2 ok=1
[slave 1001] EEPROM read addr=10 data=0x4ea3052
2025-09-20 20:11:38.027 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0508 len=4 ok=1
[slave 1001] EEPROM control write cmd=0x100 addr=0xc
2025-09-20 20:11:38.027 [DEBUG] [core  ] SlavesEndpoint B W ado=0x0502 len=6 ok=1
2025-09-20 20:11:38.234 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0502 len=2 ok=1
[slave 1001] EEPROM read addr=12 data=0x1
2025-09-20 20:11:38.235 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0508 len=4 ok=1
[slave 1001] EEPROM control write cmd=0x100 addr=0xe
2025-09-20 20:11:38.235 [DEBUG] [core  ] SlavesEndpoint B W ado=0x0502 len=6 ok=1
2025-09-20 20:11:38.442 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0502 len=2 ok=1
[slave 1001] EEPROM read addr=14 data=0x1
2025-09-20 20:11:38.443 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0508 len=4 ok=1
[slave 1001] EEPROM control write cmd=0x100 addr=0x10
2025-09-20 20:11:38.443 [DEBUG] [core  ] SlavesEndpoint B W ado=0x0502 len=6 ok=1
2025-09-20 20:11:38.650 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0502 len=2 ok=1
[slave 1001] EEPROM read addr=16 data=0x0
2025-09-20 20:11:38.651 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0508 len=4 ok=1
[slave 1001] EEPROM control write cmd=0x100 addr=0x12
2025-09-20 20:11:38.651 [DEBUG] [core  ] SlavesEndpoint B W ado=0x0502 len=6 ok=1
2025-09-20 20:11:38.858 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0502 len=2 ok=1
[slave 1001] EEPROM read addr=18 data=0x0
2025-09-20 20:11:38.859 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0508 len=4 ok=1
[slave 1001] EEPROM control write cmd=0x100 addr=0x14
2025-09-20 20:11:38.859 [DEBUG] [core  ] SlavesEndpoint B W ado=0x0502 len=6 ok=1
2025-09-20 20:11:39.066 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0502 len=2 ok=1
[slave 1001] EEPROM read addr=20 data=0x100000
2025-09-20 20:11:39.067 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0508 len=4 ok=1
[slave 1001] EEPROM control write cmd=0x100 addr=0x16
2025-09-20 20:11:39.067 [DEBUG] [core  ] SlavesEndpoint B W ado=0x0502 len=6 ok=1
2025-09-20 20:11:39.274 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0502 len=2 ok=1
[slave 1001] EEPROM read addr=22 data=0x200
2025-09-20 20:11:39.275 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0508 len=4 ok=1
[slave 1001] EEPROM control write cmd=0x100 addr=0x18
2025-09-20 20:11:39.275 [DEBUG] [core  ] SlavesEndpoint B W ado=0x0502 len=6 ok=1
2025-09-20 20:11:39.482 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0502 len=2 ok=1
[slave 1001] EEPROM read addr=24 data=0x120000
2025-09-20 20:11:39.483 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0508 len=4 ok=1
[slave 1001] EEPROM control write cmd=0x100 addr=0x1a
2025-09-20 20:11:39.483 [DEBUG] [core  ] SlavesEndpoint B W ado=0x0502 len=6 ok=1
2025-09-20 20:11:39.691 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0502 len=2 ok=1
[slave 1001] EEPROM read addr=26 data=0x200
2025-09-20 20:11:39.692 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0508 len=4 ok=1
[slave 1001] EEPROM control write cmd=0x100 addr=0x1c
2025-09-20 20:11:39.692 [DEBUG] [core  ] SlavesEndpoint B W ado=0x0502 len=6 ok=1
2025-09-20 20:11:39.898 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0502 len=2 ok=1
[slave 1001] EEPROM read addr=28 data=0x100000
2025-09-20 20:11:39.899 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0508 len=4 ok=1
[slave 1001] EEPROM control write cmd=0x100 addr=0x1e
2025-09-20 20:11:39.899 [DEBUG] [core  ] SlavesEndpoint B W ado=0x0502 len=6 ok=1
2025-09-20 20:11:40.107 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0502 len=2 ok=1
[slave 1001] EEPROM read addr=30 data=0x200
2025-09-20 20:11:40.108 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0508 len=4 ok=1
[slave 1001] EEPROM control write cmd=0x100 addr=0x20
2025-09-20 20:11:40.108 [DEBUG] [core  ] SlavesEndpoint B W ado=0x0502 len=6 ok=1
2025-09-20 20:11:40.315 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0502 len=2 ok=1
[slave 1001] EEPROM read addr=32 data=0x120000
2025-09-20 20:11:40.316 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0508 len=4 ok=1
[slave 1001] EEPROM control write cmd=0x100 addr=0x22
2025-09-20 20:11:40.316 [DEBUG] [core  ] SlavesEndpoint B W ado=0x0502 len=6 ok=1
2025-09-20 20:11:40.522 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0502 len=2 ok=1
[slave 1001] EEPROM read addr=34 data=0x200
2025-09-20 20:11:40.523 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0508 len=4 ok=1
[slave 1001] EEPROM control write cmd=0x100 addr=0x24
2025-09-20 20:11:40.523 [DEBUG] [core  ] SlavesEndpoint B W ado=0x0502 len=6 ok=1
2025-09-20 20:11:40.730 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0502 len=2 ok=1
[slave 1001] EEPROM read addr=36 data=0x4
2025-09-20 20:11:40.731 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0508 len=4 ok=1
[slave 1001] EEPROM control write cmd=0x100 addr=0x26
2025-09-20 20:11:40.731 [DEBUG] [core  ] SlavesEndpoint B W ado=0x0502 len=6 ok=1
2025-09-20 20:11:40.938 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0502 len=2 ok=1
[slave 1001] EEPROM read addr=38 data=0x0
2025-09-20 20:11:40.939 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0508 len=4 ok=1
[slave 1001] EEPROM control write cmd=0x100 addr=0x28
2025-09-20 20:11:40.939 [DEBUG] [core  ] SlavesEndpoint B W ado=0x0502 len=6 ok=1
2025-09-20 20:11:41.146 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0502 len=2 ok=1
[slave 1001] EEPROM read addr=40 data=0x0
2025-09-20 20:11:41.147 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0508 len=4 ok=1
[slave 1001] EEPROM control write cmd=0x100 addr=0x2a
2025-09-20 20:11:41.147 [DEBUG] [core  ] SlavesEndpoint B W ado=0x0502 len=6 ok=1
2025-09-20 20:11:41.354 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0502 len=2 ok=1
[slave 1001] EEPROM read addr=42 data=0x0
2025-09-20 20:11:41.355 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0508 len=4 ok=1
[slave 1001] EEPROM control write cmd=0x100 addr=0x2c
2025-09-20 20:11:41.355 [DEBUG] [core  ] SlavesEndpoint B W ado=0x0502 len=6 ok=1
2025-09-20 20:11:41.562 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0502 len=2 ok=1
[slave 1001] EEPROM read addr=44 data=0x0
2025-09-20 20:11:41.563 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0508 len=4 ok=1
[slave 1001] EEPROM control write cmd=0x100 addr=0x2e
2025-09-20 20:11:41.563 [DEBUG] [core  ] SlavesEndpoint B W ado=0x0502 len=6 ok=1
2025-09-20 20:11:41.770 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0502 len=2 ok=1
[slave 1001] EEPROM read addr=46 data=0x1
2025-09-20 20:11:41.771 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0508 len=4 ok=1
[slave 1001] EEPROM control write cmd=0x100 addr=0x30
2025-09-20 20:11:41.771 [DEBUG] [core  ] SlavesEndpoint B W ado=0x0502 len=6 ok=1
2025-09-20 20:11:41.979 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0502 len=2 ok=1
[slave 1001] EEPROM read addr=48 data=0x1
2025-09-20 20:11:41.980 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0508 len=4 ok=1
[slave 1001] EEPROM control write cmd=0x100 addr=0x32
2025-09-20 20:11:41.980 [DEBUG] [core  ] SlavesEndpoint B W ado=0x0502 len=6 ok=1
2025-09-20 20:11:42.186 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0502 len=2 ok=1
[slave 1001] EEPROM read addr=50 data=0x0
2025-09-20 20:11:42.187 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0508 len=4 ok=1
[slave 1001] EEPROM control write cmd=0x100 addr=0x34
2025-09-20 20:11:42.187 [DEBUG] [core  ] SlavesEndpoint B W ado=0x0502 len=6 ok=1
2025-09-20 20:11:42.394 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0502 len=2 ok=1
[slave 1001] EEPROM read addr=52 data=0x0
2025-09-20 20:11:42.395 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0508 len=4 ok=1
[slave 1001] EEPROM control write cmd=0x100 addr=0x36
2025-09-20 20:11:42.395 [DEBUG] [core  ] SlavesEndpoint B W ado=0x0502 len=6 ok=1
2025-09-20 20:11:42.602 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0502 len=2 ok=1
[slave 1001] EEPROM read addr=54 data=0x0
2025-09-20 20:11:42.603 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0508 len=4 ok=1
[slave 1001] EEPROM control write cmd=0x100 addr=0x38
2025-09-20 20:11:42.603 [DEBUG] [core  ] SlavesEndpoint B W ado=0x0502 len=6 ok=1
2025-09-20 20:11:42.810 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0502 len=2 ok=1
[slave 1001] EEPROM read addr=56 data=0x0
2025-09-20 20:11:42.811 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0508 len=4 ok=1
[slave 1001] EEPROM control write cmd=0x100 addr=0x3a
2025-09-20 20:11:42.811 [DEBUG] [core  ] SlavesEndpoint B W ado=0x0502 len=6 ok=1
2025-09-20 20:11:43.018 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0502 len=2 ok=1
[slave 1001] EEPROM read addr=58 data=0x0
2025-09-20 20:11:43.019 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0508 len=4 ok=1
[slave 1001] EEPROM control write cmd=0x100 addr=0x3c
2025-09-20 20:11:43.019 [DEBUG] [core  ] SlavesEndpoint B W ado=0x0502 len=6 ok=1
2025-09-20 20:11:43.227 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0502 len=2 ok=1
[slave 1001] EEPROM read addr=60 data=0x0
2025-09-20 20:11:43.228 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0508 len=4 ok=1
[slave 1001] EEPROM control write cmd=0x100 addr=0x3e
2025-09-20 20:11:43.228 [DEBUG] [core  ] SlavesEndpoint B W ado=0x0502 len=6 ok=1
2025-09-20 20:11:43.434 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0502 len=2 ok=1
[slave 1001] EEPROM read addr=62 data=0x0
2025-09-20 20:11:43.435 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0508 len=4 ok=1
[slave 1001] EEPROM control write cmd=0x100 addr=0x40
2025-09-20 20:11:43.435 [DEBUG] [core  ] SlavesEndpoint B W ado=0x0502 len=6 ok=1
2025-09-20 20:11:43.642 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0502 len=2 ok=1
[slave 1001] EEPROM read addr=64 data=0x0
2025-09-20 20:11:43.643 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0508 len=4 ok=1
[slave 1001] EEPROM control write cmd=0x100 addr=0x42
2025-09-20 20:11:43.643 [DEBUG] [core  ] SlavesEndpoint B W ado=0x0502 len=6 ok=1
2025-09-20 20:11:43.850 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0502 len=2 ok=1
[slave 1001] EEPROM read addr=66 data=0x0
2025-09-20 20:11:43.851 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0508 len=4 ok=1
[slave 1001] EEPROM control write cmd=0x100 addr=0x44
2025-09-20 20:11:43.851 [DEBUG] [core  ] SlavesEndpoint B W ado=0x0502 len=6 ok=1
2025-09-20 20:11:44.058 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0502 len=2 ok=1
[slave 1001] EEPROM read addr=68 data=0x0
2025-09-20 20:11:44.059 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0508 len=4 ok=1
[slave 1001] EEPROM control write cmd=0x100 addr=0x46
2025-09-20 20:11:44.059 [DEBUG] [core  ] SlavesEndpoint B W ado=0x0502 len=6 ok=1
2025-09-20 20:11:44.267 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0502 len=2 ok=1
[slave 1001] EEPROM read addr=70 data=0x0
2025-09-20 20:11:44.268 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0508 len=4 ok=1
[slave 1001] EEPROM control write cmd=0x100 addr=0x48
2025-09-20 20:11:44.268 [DEBUG] [core  ] SlavesEndpoint B W ado=0x0502 len=6 ok=1
2025-09-20 20:11:44.475 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0502 len=2 ok=1
[slave 1001] EEPROM read addr=72 data=0x0
2025-09-20 20:11:44.476 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0508 len=4 ok=1
[slave 1001] EEPROM control write cmd=0x100 addr=0x4a
2025-09-20 20:11:44.476 [DEBUG] [core  ] SlavesEndpoint B W ado=0x0502 len=6 ok=1
2025-09-20 20:11:44.682 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0502 len=2 ok=1
[slave 1001] EEPROM read addr=74 data=0x0
2025-09-20 20:11:44.683 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0508 len=4 ok=1
[slave 1001] EEPROM control write cmd=0x100 addr=0x4c
2025-09-20 20:11:44.683 [DEBUG] [core  ] SlavesEndpoint B W ado=0x0502 len=6 ok=1
2025-09-20 20:11:44.890 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0502 len=2 ok=1
[slave 1001] EEPROM read addr=76 data=0x0
2025-09-20 20:11:44.891 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0508 len=4 ok=1
[slave 1001] EEPROM control write cmd=0x100 addr=0x4e
2025-09-20 20:11:44.891 [DEBUG] [core  ] SlavesEndpoint B W ado=0x0502 len=6 ok=1
2025-09-20 20:11:45.098 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0502 len=2 ok=1
[slave 1001] EEPROM read addr=78 data=0x0
2025-09-20 20:11:45.099 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0508 len=4 ok=1
[slave 1001] EEPROM control write cmd=0x100 addr=0x50
2025-09-20 20:11:45.099 [DEBUG] [core  ] SlavesEndpoint B W ado=0x0502 len=6 ok=1
2025-09-20 20:11:45.306 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0502 len=2 ok=1
[slave 1001] EEPROM read addr=80 data=0x0
2025-09-20 20:11:45.307 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0508 len=4 ok=1
[slave 1001] EEPROM control write cmd=0x100 addr=0x52
2025-09-20 20:11:45.307 [DEBUG] [core  ] SlavesEndpoint B W ado=0x0502 len=6 ok=1
2025-09-20 20:11:45.514 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0502 len=2 ok=1
[slave 1001] EEPROM read addr=82 data=0x0
2025-09-20 20:11:45.515 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0508 len=4 ok=1
[slave 1001] EEPROM control write cmd=0x100 addr=0x54
2025-09-20 20:11:45.515 [DEBUG] [core  ] SlavesEndpoint B W ado=0x0502 len=6 ok=1
2025-09-20 20:11:45.723 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0502 len=2 ok=1
[slave 1001] EEPROM read addr=84 data=0x0
2025-09-20 20:11:45.724 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0508 len=4 ok=1
[slave 1001] EEPROM control write cmd=0x100 addr=0x56
2025-09-20 20:11:45.724 [DEBUG] [core  ] SlavesEndpoint B W ado=0x0502 len=6 ok=1
2025-09-20 20:11:45.930 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0502 len=2 ok=1
[slave 1001] EEPROM read addr=86 data=0x0
2025-09-20 20:11:45.931 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0508 len=4 ok=1
[slave 1001] EEPROM control write cmd=0x100 addr=0x58
2025-09-20 20:11:45.931 [DEBUG] [core  ] SlavesEndpoint B W ado=0x0502 len=6 ok=1
2025-09-20 20:11:46.138 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0502 len=2 ok=1
[slave 1001] EEPROM read addr=88 data=0x0
2025-09-20 20:11:46.139 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0508 len=4 ok=1
[slave 1001] EEPROM control write cmd=0x100 addr=0x5a
2025-09-20 20:11:46.139 [DEBUG] [core  ] SlavesEndpoint B W ado=0x0502 len=6 ok=1
2025-09-20 20:11:46.346 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0502 len=2 ok=1
[slave 1001] EEPROM read addr=90 data=0x0
2025-09-20 20:11:46.347 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0508 len=4 ok=1
[slave 1001] EEPROM control write cmd=0x100 addr=0x5c
2025-09-20 20:11:46.347 [DEBUG] [core  ] SlavesEndpoint B W ado=0x0502 len=6 ok=1
2025-09-20 20:11:46.554 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0502 len=2 ok=1
[slave 1001] EEPROM read addr=92 data=0x0
2025-09-20 20:11:46.555 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0508 len=4 ok=1
[slave 1001] EEPROM control write cmd=0x100 addr=0x5e
2025-09-20 20:11:46.555 [DEBUG] [core  ] SlavesEndpoint B W ado=0x0502 len=6 ok=1
2025-09-20 20:11:46.763 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0502 len=2 ok=1
[slave 1001] EEPROM read addr=94 data=0x0
2025-09-20 20:11:46.764 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0508 len=4 ok=1
[slave 1001] EEPROM control write cmd=0x100 addr=0x60
2025-09-20 20:11:46.764 [DEBUG] [core  ] SlavesEndpoint B W ado=0x0502 len=6 ok=1
2025-09-20 20:11:46.971 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0502 len=2 ok=1
[slave 1001] EEPROM read addr=96 data=0xffffffff
2025-09-20 20:11:46.972 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0508 len=4 ok=1
2025-09-20 20:11:46.972 [DEBUG] [core  ] SlavesEndpoint FPW  adp=1001 ado=0x0800 len=16 ok=1
2025-09-20 20:11:46.972 [DEBUG] [core  ] idx=0 AL_CONTROL=0x12 len=2
2025-09-20 20:11:46.972 [DEBUG] [core  ] VirtualSlave[1001] AL_CONTROL write: 0x18 current state: 1
2025-09-20 20:11:46.972 [DEBUG] [core  ] VirtualSlave[1001] preconditionsMet_(2): true (no special requirements)
2025-09-20 20:11:46.972 [DEBUG] [core  ] VirtualSlave[1001] New AL state: 2
2025-09-20 20:11:46.972 [DEBUG] [core  ] SlavesEndpoint B W ado=0x0120 len=2 ok=1
2025-09-20 20:11:46.972 [DEBUG] [core  ] SlavesEndpoint B W AL_CONTROL value=0x0012
2025-09-20 20:11:47.197 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0130 len=6 ok=1
2025-09-20 20:11:47.197 [DEBUG] [core  ] SlavesEndpoint FPRD AL_STATUS adp=1001 value=0x0002
2025-09-20 20:11:47.198 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0805 len=1 ok=1
2025-09-20 20:11:47.198 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x080D len=1 ok=1
2025-09-20 20:11:47.198 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0130 len=8 ok=1
2025-09-20 20:11:47.198 [DEBUG] [core  ] SlavesEndpoint FPRD AL_STATUS adp=1001 value=0x0002
2025-09-20 20:11:47.198 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0134 len=8 ok=1
2025-09-20 20:11:47.198 [DEBUG] [core  ] SlavesEndpoint FPRD AL_STATUS_CODE adp=1001 value=0x0000
2025-09-20 20:11:47.198 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0130 len=8 ok=1
2025-09-20 20:11:47.198 [DEBUG] [core  ] SlavesEndpoint FPRD AL_STATUS adp=1001 value=0x0002
2025-09-20 20:11:47.198 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0134 len=8 ok=1
2025-09-20 20:11:47.198 [DEBUG] [core  ] SlavesEndpoint FPRD AL_STATUS_CODE adp=1001 value=0x0000
2025-09-20 20:11:47.198 [DEBUG] [core  ] idx=0 AL_CONTROL=0x18 len=2
2025-09-20 20:11:47.198 [DEBUG] [core  ] VirtualSlave[1001] AL_CONTROL write: 0x24 current state: 2
2025-09-20 20:11:47.198 [DEBUG] [core  ] VirtualSlave[1001] preconditionsMet_(4): true (input_pdo_mapped_=true)
2025-09-20 20:11:47.198 [DEBUG] [core  ] VirtualSlave[1001] preconditionsMet_(8): true (input_pdo_mapped_=true)
2025-09-20 20:11:47.198 [DEBUG] [core  ] VirtualSlave[1001] New AL state: 8
2025-09-20 20:11:47.198 [DEBUG] [core  ] SlavesEndpoint B W ado=0x0120 len=2 ok=1
2025-09-20 20:11:47.198 [DEBUG] [core  ] SlavesEndpoint B W AL_CONTROL value=0x0018
2025-09-20 20:11:47.421 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0130 len=6 ok=1
2025-09-20 20:11:47.422 [DEBUG] [core  ] SlavesEndpoint FPRD AL_STATUS adp=1001 value=0x0008
2025-09-20 20:11:47.422 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0130 len=8 ok=1
2025-09-20 20:11:47.422 [DEBUG] [core  ] SlavesEndpoint FPRD AL_STATUS adp=1001 value=0x0008
2025-09-20 20:11:47.422 [DEBUG] [core  ] SlavesEndpoint FPR  adp=1001 ado=0x0134 len=8 ok=1
2025-09-20 20:11:47.422 [DEBUG] [core  ] SlavesEndpoint FPRD AL_STATUS_CODE adp=1001 value=0x0000
2025-09-20 20:11:50.108 [INFO ] [core  ] Graceful shutdown




2025-09-20 20:11:32.602 [INFO ] [master] Running automatic scan/init/op sequence
2025-09-20 20:11:32.602 [INFO ] [master] MasterSocket::open() endpoint: uds:///tmp/ethercat_bus.sock
2025-09-20 20:11:32.602 [INFO ] [master] Connecting to UDS: /tmp/ethercat_bus.sock
2025-09-20 20:11:32.602 [INFO ] [master] Successfully connected to UDS
2025-09-20 20:11:32.810 [INFO ] [master] Auto sequence step 'scan' ok
2025-09-20 20:11:32.810 [INFO ] [master] Starting bus->init()
2025-09-20 20:11:47.198 [INFO ] [master] bus->init() completed
2025-09-20 20:11:47.198 [INFO ] [master] After bus->init(): preopReached = true
2025-09-20 20:11:47.198 [INFO ] [master] After bus->init(): initReached = true
2025-09-20 20:11:47.198 [INFO ] [master] Auto sequence step 'preop' ok
2025-09-20 20:11:47.422 [INFO ] [master] Auto sequence step 'operational' ok
2025-09-20 20:11:47.422 [INFO ] [master] Headless mode - OP state reached
2025-09-20 20:11:50.778 [INFO ] [master] Graceful shutdown
