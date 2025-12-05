#include <xc.h>
#include <stdio.h>
#include <stdlib.h>
#include "CB_RX1.h"
#define CBRX1_BUFFER_SIZE 128
#define CBRX1_BUFFER_SIZE 128

volatile int cbRx1Head = 0;
volatile int cbRx1Tail = 0;
volatile unsigned char cbRx1Buffer[CBRX1_BUFFER_SIZE];

/* ============================================================
   ----  BUFFER CIRCULAIRE : CALCULS ----
   ============================================================ */

int CB_RX1_GetDataSize(void)
{
    if (cbRx1Head >= cbRx1Tail)
        return cbRx1Head - cbRx1Tail;
    else
        return CBRX1_BUFFER_SIZE - (cbRx1Tail - cbRx1Head);
}

int CB_RX1_GetRemainingSize(void)
{
    return CBRX1_BUFFER_SIZE - CB_RX1_GetDataSize() - 1;
}

unsigned char CB_RX1_IsDataAvailable(void)
{
    return (cbRx1Head != cbRx1Tail);
}

unsigned char CB_RX1_IsBufferFull(void)
{
    int next = cbRx1Head + 1;
    if (next >= CBRX1_BUFFER_SIZE) next = 0;
    return (next == cbRx1Tail);
}

/* ============================================================
   ----  AJOUT DANS LE BUFFER ----
   ============================================================ */

void CB_RX1_Add(unsigned char value)
{
    int next = cbRx1Head + 1;
    if (next >= CBRX1_BUFFER_SIZE) next = 0;

    if (next == cbRx1Tail)
    {
        // Buffer plein ? on perd un caractère
        // OPTION : cbRx1Tail = (cbRx1Tail + 1) % CBRX1_BUFFER_SIZE; // overwrite
        return; // ignore (sécurisé)
    }

    cbRx1Buffer[cbRx1Head] = value;
    cbRx1Head = next;
}

/* ============================================================
   ----  LECTURE DU BUFFER ----
   ============================================================ */

unsigned char CB_RX1_Get(void)
{
    if (cbRx1Head == cbRx1Tail)
        return 0; // buffer vide, valeur par défaut

    unsigned char value = cbRx1Buffer[cbRx1Tail];

    cbRx1Tail++;
    if (cbRx1Tail >= CBRX1_BUFFER_SIZE)
        cbRx1Tail = 0;

    return value;
}

/* ============================================================
   ----  INTERRUPT UART ----
   ============================================================ */

void __attribute__((interrupt, no_auto_psv)) _U1RXInterrupt(void)
{
    IFS0bits.U1RXIF = 0; // clear RX flag

    // Check frame error
    if (U1STAbits.FERR)
        U1STAbits.FERR = 0;

    // Check overrun
    if (U1STAbits.OERR)
        U1STAbits.OERR = 0;

    // Lire tous les caractères disponibles
    while (U1STAbits.URXDA == 1)
    {
        unsigned char c = U1RXREG;
        CB_RX1_Add(c);
    }
}