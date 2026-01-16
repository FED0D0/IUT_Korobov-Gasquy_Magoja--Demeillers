#ifndef UART_Protocol_H
#define	UART_Protocol_H
#define SET_ROBOT_STATE 0x0051
#define SET_ROBOT_MANUAL_CONTROL 0x0052

extern unsigned char autoControlActivated;

unsigned char UartCalculateChecksum(int msgFunction,
int msgPayloadLength, unsigned char* msgPayload);

void UartEncodeAndSendMessage(int msgFunction,
int msgPayloadLength, unsigned char* msgPayload);
void UartDecodeMessage(unsigned char c);
void UartProcessDecodedMessage(int function,
int payloadLength, unsigned char* payload);
void SetRobotState(unsigned char state);
void SetRobotAutoControlState(unsigned char payload);
void SendDeplacementStep(uint8_t step, uint32_t timeMs);
#endif	/* UART_H */