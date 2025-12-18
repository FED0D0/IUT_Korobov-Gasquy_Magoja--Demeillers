#ifndef UART_Protocol_H
#define	UART_Protocol_H

unsigned char UartCalculateChecksum(int msgFunction,
int msgPayloadLength, unsigned char* msgPayload);
void UartEncodeAndSendMessage(int msgFunction,
int msgPayloadLength, unsigned char* msgPayload);
void UartDecodeMessage(unsigned char c);
void UartProcessDecodedMessage(int function,
int payloadLength, unsigned char* payload);


#endif	/* UART_H */