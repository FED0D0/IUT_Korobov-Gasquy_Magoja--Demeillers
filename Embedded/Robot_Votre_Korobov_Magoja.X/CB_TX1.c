#include <xc.h>
#include <stdio.h>
#include <stdlib.h>
#include "CB_TX1.h"
#define CBTX1_BUFFER_SIZE 128
volatile int cbTx1Head = 0;
volatile int cbTx1Tail = 0;
volatile unsigned char cbTx1Buffer[CBTX1_BUFFER_SIZE];
volatile unsigned char isTransmitting = 0;


/* ============================================================
   ---- BUFFER CIRCULAIRE : FONCTIONS PRINCIPALES ----
   ============================================================ */

int CB_TX1_GetDataSize(void) {
    if (cbTx1Head >= cbTx1Tail)
        return cbTx1Head - cbTx1Tail;
    else
        return CBTX1_BUFFER_SIZE - (cbTx1Tail - cbTx1Head);
}

int CB_TX1_GetRemainingSize(void) {
    return CBTX1_BUFFER_SIZE - CB_TX1_GetDataSize() - 1;
}

unsigned char CB_TX1_IsBufferFull(void) {
    int next = cbTx1Head + 1;
    if (next >= CBTX1_BUFFER_SIZE) next = 0;
    return (next == cbTx1Tail);
}

unsigned char CB_TX1_IsTransmitting(void) {
    return isTransmitting;
}


/* ============================================================
   ----  ÉCRITURE DANS LE BUFFER ----
   ============================================================ */

unsigned char CB_TX1_Add(unsigned char value) {
    int next = cbTx1Head + 1;
    if (next >= CBTX1_BUFFER_SIZE) next = 0;

    if (next == cbTx1Tail) {
        // BUFFER PLEIN
        return 0; 
    }

    cbTx1Buffer[cbTx1Head] = value;
    cbTx1Head = next;

    return 1;
}


/* ============================================================
   ----  LECTURE DU BUFFER ----
   ============================================================ */

unsigned char CB_TX1_Get(void) {
    unsigned char value = cbTx1Buffer[cbTx1Tail];

    cbTx1Tail++;
    if (cbTx1Tail >= CBTX1_BUFFER_SIZE)
        cbTx1Tail = 0;

    return value;
}


/* ============================================================
   ----  TRANSMISSION UART ----
   ============================================================ */

void SendOne(void) {
    // Attend que le registre TX soit libre
    while (U1STAbits.UTXBF);

    isTransmitting = 1;

    unsigned char c = CB_TX1_Get();
    U1TXREG = c;
}


/* ============================================================
   ----  ENVOI D?UN MESSAGE ENTIER ----
   ============================================================ */

void SendMessage(unsigned char *message, int length) {
    int i;

    if (CB_TX1_GetRemainingSize() < length)
        return;  // pas assez de place

    // Écriture dans le buffer
    for (i = 0; i < length; i++)
        CB_TX1_Add(message[i]);

    // Si on n'est pas en transmission -> lancer la première émission
    if (!CB_TX1_IsTransmitting())
        SendOne();
}


/* ============================================================
   ----  INTERRUPTIONS UART ----
   ============================================================ */

void __attribute__((interrupt, no_auto_psv)) _U1TXInterrupt(void) {
    IFS0bits.U1TXIF = 0;  // reset flag

    // S'il reste des données, envoyer la suivante
    if (cbTx1Tail != cbTx1Head)
        SendOne();
    else
        isTransmitting = 0;
}
