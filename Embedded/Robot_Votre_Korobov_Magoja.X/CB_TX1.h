#ifndef CB_TX1_H
#define CB_TX1_H

void SendMessage(unsigned char *message, int length);
unsigned char CB_TX1_Add(unsigned char value);
unsigned char CB_TX1_Get(void);
int CB_TX1_GetDataSize(void);
int CB_TX1_GetRemainingSize(void);
unsigned char CB_TX1_IsBufferFull(void);
unsigned char CB_TX1_IsTransmitting(void);
void SendOne(void);

#endif

