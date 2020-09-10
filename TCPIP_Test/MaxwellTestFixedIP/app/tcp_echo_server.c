
#include <xc.h>
#include <time.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "tcp_echo_server.h"
//#include "adc.h"
#include "tcpv4.h"


//Implement an echo server over TCP
void TCP_EchoServer(void)
{
    // Create the socket for the TCP Server
    static tcpTCB_t port7TCB;
   

    // Create the TX and RX Server's buffers
    static uint8_t rxdataPort7[BUFFER_SIZE];
    static uint8_t txdataPort7[BUFFER_SIZE];  

    uint16_t rxLen, txLen, i;
    socketState_t socket_state;

    // Check the status of the Socket
    socket_state = TCP_SocketPoll(&port7TCB);
//    -	NOT_A_SOCKET? ? the socket was not initialized
//    - SOCKET_CLOSED? ? the socket is initialized but is closed
//    - SOCKET_CONNECTED? ? the socket is connected and the data can be exchanged with remote machine

//    printf("socket_state is: %d\r\n", socket_state); //DB
    switch(socket_state)
    {
        case NOT_A_SOCKET:
            printf("Initializing the Socket\r\n");
//           Inserting and Initializing the socket
//            printf("socket_state is: NOT_A_SOCKET\r\n"); //DB
            TCP_SocketInit(&port7TCB);   
            break;
        case SOCKET_CLOSED:

            printf("socket_state is: SOCKET_CLOSED\r\n"); //DB
//            Configure the local port
            TCP_Bind(&port7TCB,7); 

            //  Add receive buffer 
            TCP_InsertRxBuffer(&port7TCB, rxdataPort7,sizeof(rxdataPort7));

            // Start the TCP server: Listen on port
            TCP_Listen(&port7TCB);
            
//            printf("Bind and Listen added to the Socket\r\n");

           
            break;
        case SOCKET_CONNECTED:
            // check if the buffer was sent, if yes we can send another buffer
//            printf("socket_state is: SOCKET_CONNECTED\r\n"); //DB
            if(TCP_SendDone(&port7TCB))
            {
                // check to see  if there are any received data
                
                rxLen = TCP_GetRxLength(&port7TCB);
                 printf("rxLen - %d \r\n",rxLen);
                if(rxLen > 0)
                {
//                     printf("rxLen - %d \r\n",rxLen);
                    rxLen = TCP_GetReceivedData(&port7TCB);

                    //simulate some buffer processing
                    for(i = 0; i < rxLen; i++)
                    {
                        txdataPort7[i] = rxdataPort7[i];
                    }
                    

                    // reuse the RX buffer
                    TCP_InsertRxBuffer(&port7TCB, rxdataPort7, strlen(rxdataPort7));
                    txLen = rxLen;
//                    printf("sending TCP Packet\r\n");
                    // Send data back to the Source
                    TCP_Send(&port7TCB,txdataPort7,txLen);
                   
                    
                }
            }
//            else
//            {
//                
//            }
            break;   
        case SOCKET_CLOSING:
            printf("Socket closing in server\r\n");
            TCP_SocketRemove(&port7TCB);
            port7TCB.socketState = NOT_A_SOCKET;            
            break; 
        default:
            // we should not end up here
            break;
    }
}

void   TCP_ClientInit()
{
#ifdef TESTING
    remoteSocketClient.addr.s_addr = MAKE_IPV4_ADDRESS(10,0,0,3);   
#else
    remoteSocketClient.addr.s_addr = MAKE_IPV4_ADDRESS(192,168,0,69);   
#endif
     remoteSocketClient.port = 7;
}

void TCP_EchoClient(void)
{
    // create the socket for the TCP Client
    static tcpTCB_t port60TCB;

    //// create the TX and RX Client's buffers
    static uint8_t rxdataPort60[BUFFER_SIZE];
    static uint8_t txdataPort60[BUFFER_SIZE];
    
    static time_t t_client, t_clientOld;
    uint16_t rx_len;
    socketState_t socketState;
    rx_len = 0;

    socketState = TCP_SocketPoll(&port60TCB);    

 time(&t_client);
    switch(socketState)
    {
        case NOT_A_SOCKET:            
            // Inserting and initializing the socket
            printf("Socket Init in client\r\n");
            TCP_SocketInit(&port60TCB);
            break;
        case SOCKET_CLOSED: 
            if(t_client >= t_clientOld)
            {
                // try to connect once at 2 seconds
                t_clientOld = t_client + 2;
                TCP_InsertRxBuffer(&port60TCB, rxdataPort60, sizeof(rxdataPort60));
                TCP_Connect(&port60TCB, &remoteSocketClient);
            }
            break;
        case SOCKET_IN_PROGRESS:
            port60TCB.socketState = SOCKET_CLOSED;
            break;
        case SOCKET_CONNECTED:       
                rx_len = TCP_GetReceivedData(&port60TCB);
                // handle the incoming data
                if(rx_len > 0)
                {    

                    // reuse the RX buffer
                    TCP_InsertRxBuffer(&port60TCB, rxdataPort60, sizeof(rxdataPort60));
                }
                 
                    sprintf(txdataPort60,"abcdefghijklmnopqrstuvwxyz");
                    //send data back to the source
                    TCP_Send(&port60TCB, txdataPort60, strlen(txdataPort60));

            break;
        case SOCKET_CLOSING:
            printf("Socket closing in client\r\n");
            TCP_SocketRemove(&port60TCB);
            port60TCB.socketState = NOT_A_SOCKET;            
            break;            
        default:
            // we should not end up here
            break;
    }
}
