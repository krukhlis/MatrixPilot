#ifndef _MYIPUART1_C_
#define _MYIPUART1_C_

#include "defines.h"
#if (USE_NETWORK == 1) && (NETWORK_USE_UART1 == 1)

#include "TCPIP_Stack/TCPIP.h"
#include "MyIpData.h"
#include "MyIpUART1.h"


//////////////////////////////////////
// Local Functions

//////////////////////////
// Module Variables


void MyIpOnConnect_UART1(const BYTE s)
{
    /*
    // Print any one-time connection annoucement text
    StringToSocket(s, "\r\nYou've connected to UART1 on "); // 31 chars
    StringToSocket(s, ID_LEAD_PILOT); // 15ish chars
    StringToSocket(s, "'s aircraft. More info at "); // 26 chars
    StringToSocket(s, ID_DIY_DRONES_URL); // 45ish chars
    StringToSocket(s, "\r\n"); // 2 chars
    MyIpData[s].sendPacket = TRUE; // send right away
    */
}


void MyIpInit_UART1(const BYTE s)
{
}

void MyIpService_UART1(const BYTE s)
{
    // Nothing to do here, it's all done in ISRs
}

BOOL MyIpThreadSafeSendPacketCheck_UART1(const BYTE s, const BOOL doClearFlag)
{
#if (HILSIM == 1)
    (void)s; // mute the unused var compiler warning
    (void)doClearFlag; // mute the unused var compiler warning

    return TRUE;
#else
    BYTE isrState;
    BOOL sendpacket;

    isrState = _U1TXIE;
    _U1TXIE = 0; // inhibit the UART1 ISR from changing this on us during a read
    sendpacket = MyIpData[s].sendPacket;
    if (doClearFlag)
    {
        MyIpData[s].sendPacket = FALSE;
    }
    _U1TXIE = isrState; // resume ISR
    return sendpacket;
#endif
}


int MyIpThreadSafeReadBufferHead_UART1(const BYTE s)
{
    BYTE isrState;
    int head;

    isrState = _U1TXIE;
    _U1TXIE = 0; // inhibit the UART1 ISR from loading more data for a moment (protect _head reads)
    head = MyIpData[s].buffer_head;
    _U1TXIE = isrState; // resume ISR

    return head;
}

void MyIpProcessRxData_UART1(const BYTE s)
{
    BYTE isrState, rxData;
    BOOL successfulRead;

    do
    {
        if (eTCP == MyIpData[s].type)
        {
            successfulRead = TCPGet(MyIpData[s].socket, &rxData);
        }
        else //if (eUDP == MyIpData[s].type)
        {
            successfulRead = UDPGet(&rxData);
        }

        if (successfulRead)
        {
            isrState = _U1RXIE;
            _U1RXIE = 0;
            udb_gps_callback_received_byte(rxData);
            _U1RXIE = isrState;
        }
    } while (successfulRead);
}

	
#endif // #if (USE_NETWORK == 1)
#endif // _MYIPUART1_C_
