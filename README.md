# ethertemp

A project to implement a TCP/IP stack in order to gather temperature, humidity and other data periodically. Based on *STM32F411RET6*

### Currently data avaliable

Currently, device can measure only 2 environment parameters:
>
> - Temperature in °C
> - Relative Humidity level (in percentage)
>

Going to implement atmospheric pressure measurement using GY-68, and Geiger counter based on SBM-21 in the future

### Ethernet connection

This project uses Ethernet as a communication protocol between device and server. Ethernet port is implemented in ENC28J60 module from **Microchip**, and is communicating with STM32 through SPI.
ENC28J60 fully manages physical layer, and could partially manage channel layer, as it has features for error checking, basic filtering, etc.

On the STM32 side, it implements:

>
> - Partly the channel layer (as for instance, ARP requests and responses)
> - Network layer (IPv4, ICMP)
> - Transport layer (UDP, virtual ports table, TCP - todo)
> - A small part of application layer, particularly - DHCP discovery
>

### Ethernet implementation and operation

Firstly, STM32 initializes SPI and ENC28J60 module. This operation includes setting MAC address, mapping RX/TX buffer and setting other required register for operating in Half Duplex Mode, and is located in `Core/Src/main.c` calling `initialize_enc28j60()`.
ENC28J60 initialization source code is located here: [Core/Src/enc28j60.c](https://github.com/CeSiumUA/ethertemp/blob/master/Core/Src/enc28j60.c).

Afterwards, the ARP request is sent to find a server MAC address by given IP address (`arp_search_server()` in `Core/Src/main.c`) and process Ethernet packages to get ARP response with the desired MAC address. All ARP entries are stored in memory as `IP<->MAC` address as linked list (`arp_table` in source code).
ARP source code: [Core/Src/arp.c](https://github.com/CeSiumUA/ethertemp/blob/master/Core/Src/arp.c)

In an infinite `while` loop devices is checking ENC28J60 for new frames, and then processes them (`eth_process(&frame)`). It could be a better approach to use DMA here, so this is a subject to change in the nearest future.

Environmental parameters are measured every 5 seconds, triggered by configured timer: [Timer interrupt handler](https://github.com/CeSiumUA/ethertemp/blob/master/Core/Src/main.c#L158), and then the measured data is sent to server via UDP. As a better approach, I would like to implement a TCP protocol and behave this device as a server itself. UDP server is implemented as a very simple [Go application](https://github.com/CeSiumUA/ethertemp-server), which receives data via UDP and writes temperature and humidity to MongoDB database.

The data on the server:

![Data on the server](/img/img1.png "Data on the server")

22.... is a temperature, and 35.... is relative humidity in percentage

The device is discoverable via ARP requests, so it's IP address is mapped to a MAC address, and could handle ICMP requests, therefore it is possible to ping the device in order to check its condition:

![Ping command to device](/img/img2.png "Ping to the device")

Sending 230 packages gives pretty good results, not loosing anything:

![Ping results](/img/img3.png "Ping results")

Additionally, there is some work-in-progress on DHCP. Currently, the device could send DHCP DISCOVER commands, but still could not handle it. Current and future implementation is located here: [Core/Src/dhcp.c](https://github.com/CeSiumUA/ethertemp/blob/master/Core/Src/dhcp.c)

### Environmental measurements

Device measures temperature and relative humidity using HTU-21 (GY-21), communicating with it through I2C interface.
Source code: [Core/Src/htu.c](https://github.com/CeSiumUA/ethertemp/blob/master/Core/Src/htu.c)

### Logging

The logging is performed in a straight simple way, via USART. There are functions to print predefined messages, and also to print any piece of data. Logger source code: [Core/Src/logger.c](https://github.com/CeSiumUA/ethertemp/blob/master/Core/Src/logger.c)

### Board connections

>!Warning! This paragraph is specific to *STM32F411RET6*

For HTU-21:

    HTU21  | STM32
    ------ | ------
    VIN    | 3.3V
    GND    | GND
    SCL    | D6 (PB10)
    SDA    | D3 (PB3)

For ENC28J60:

    ENC28J60 | STM32
    -------- | -----
    VCC      | 3.3V
    GND      | GND
    CS       | D8 (PA9)
    RST      | D7 (PA8)
    SI       | D11 (PA7)
    SCK      | D13 (PA5)
    SO       | D12 (PA6)

### Future improvements

As the future improvement, it would be nice to implement a TCP server on board (or communicate with server via MQTT), add more sensors, move from HAL to CMSIS (as I love it more) and may be to use some real time operating system such as FreeRTOS.