#include <xc.h>
#include "UART_Protocol.h"
unsigned char UartCalculateChecksum(int msgFunction,
int msgPayloadLength, unsigned char* msgPayload)
{
 char checksum = 0;
 checksum ^= 0xFE;
 checksum ^= (unsigned char)(msgFunction >> 8);
 checksum ^= (unsigned char)(msgFunction >> 0);
 checksum ^= (unsigned char)(msgPayloadLength >> 8);
 checksum ^= (unsigned char)(msgPayloadLength >> 0);
 for (int i = 0; i < msgPayloadLength; i++)
 {
     checksum ^= msgPayload[i];
 }

 return checksum;
}

void UartEncodeAndSendMessage(int msgFunction,
int msgPayloadLength, unsigned char* msgPayload)
{
int totalLength = 1 + 2 + 2 + msgPayloadLength + 1;
    unsigned char frame[totalLength];
    int index = 0;

    frame[index++] = 0xFE;

    frame[index++] = (msgFunction >> 8) ;
    frame[index++] = msgFunction ;

    frame[index++] = (msgPayloadLength >> 8) ;
    frame[index++] = msgPayloadLength ;

    for (int i = 0; i < msgPayloadLength; i++)
        frame[index++] = msgPayload[i];

    frame[index++] = UartCalculateChecksum(msgFunction, msgPayloadLength, msgPayload);
    SendMessage(frame, totalLength);
}
//int msgDecodedFunction = 0;
//int msgDecodedPayloadLength = 0;
//unsigned char msgDecodedPayload[128];
//int msgDecodedPayloadIndex = 0;
//void UartDecodeMessage(unsigned char c)
//{
////Fonction prenant en entree un octet et servant a reconstituer les trames
//...
//}
//void UartProcessDecodedMessage(int function,
//int payloadLength, unsigned char* payload)
//{
////Fonction appelee apres le decodage pour executer l?action
////correspondant au message recu
//...
//}
////*************************************************************************/
////Fonctions correspondant aux messages
////*************************************************************************/
