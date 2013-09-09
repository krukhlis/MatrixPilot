

#define NETWORK_INTERFACE_NONE                  0
#define NETWORK_INTERFACE_WIFI_MRF24WG          1
#define NETWORK_INTERFACE_ETHERNET_ENC624J600   2
#define NETWORK_INTERFACE_ETHERNET_ENC28J60     3

////////////////////////////////////////////////////////////////////////////////
// TCP/UDP/IP protocols with Network interface
// Enable a network interface over SPI for internet access.
// WiFi is for short range use. For testing use the home WiFi and then a cell phone hotspot on-board.
// For Ethernet a wired router with a high-gain WiFi antenna can work quite far with a directional basestation antenna
// For additional IP tweaks see TCPIPConfig.h, HardwareProfile.h, MyIpOptions.h and edit MyTelemetry[]
// Select a network interface by defining NETWORK_INTERFACE to one of these options:
// NETWORK_INTERFACE_NONE
// NETWORK_INTERFACE_WIFI_MRF24WG           // 802.11g 54 MBit
// NETWORK_INTERFACE_ETHERNET_ENC624J600    // 10/100 MBit
// NETWORK_INTERFACE_ETHERNET_ENC28J60      // 10 MBit

//#define NETWORK_INTERFACE               (NETWORK_INTERFACE_NONE)
//#define NETWORK_INTERFACE               (NETWORK_INTERFACE_WIFI_MRF24WG)
#define NETWORK_INTERFACE               (NETWORK_INTERFACE_ETHERNET_ENC28J60)
//#define NETWORK_INTERFACE               (NETWORK_INTERFACE_ETHERNET_ENC624J600)

/*
// Select which Network modules you would like to Enable. Set to (1) to enable
#define NETWORK_USE_UART1               (0) // Forward UART1 data
#define NETWORK_USE_UART2               (0) // Forward UART2 data
#define NETWORK_USE_FLYBYWIRE           (0) // Joystick -> flight surfaces (over the internet!)
#define NETWORK_USE_MAVLINK             (0) // Forward MAVLink data
#define NETWORK_USE_DEBUG               (0) // Debug - Simple Telnet in ASCII
#define NETWORK_USE_ADSB                (0)
#define NETWORK_USE_LOGO                (0)
#define NETWORK_USE_CAM_TRACKING        (0) // Camera Tracking, also set CAM_USE_EXTERNAL_TARGET_DATA=1
#define NETWORK_USE_GPSTEST             (0) // GPS spoof testing
#define NETWORK_USE_PWMREPORT           (0) // PWM pin states
#define NETWORK_USE_XPLANE              (0) // Talk directly to Xplane without a plug. Weeee!!!!!
#define NETWORK_USE_TELEMETRY_EXTRA     (0) // Same data as what SERIAL_UDB_EXTRA generates in telemetry.c
#define NETWORK_USE_GROUND_STATION      (0) // Reduced binary telemetry data for ground stations
 */

