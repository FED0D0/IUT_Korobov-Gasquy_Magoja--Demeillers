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

void SendDeplacementStep(uint8_t step, uint32_t timeMs)
{
    uint8_t payload[5];

    payload[0] = step;
    payload[1] = (timeMs >> 24) & 0xFF;
    payload[2] = (timeMs >> 16) & 0xFF;
    payload[3] = (timeMs >> 8) & 0xFF;
    payload[4] = timeMs & 0xFF;

    UartEncodeAndSendMessage(0x0050, 5, payload);
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



typedef enum {
    WAITING,
    FUNCTION_MSB,
    FUNCTION_LSB,
    PAYLOAD_LENGTH_MSB,
    PAYLOAD_LENGTH_LSB,
    PAYLOAD,
    CHECKSUM
} StateReception;
static StateReception rcvState = WAITING;
static int msgDecodedFunction = 0;
static int msgDecodedPayloadLength = 0;
static unsigned char msgDecodedPayload[128];
static int msgDecodedPayloadIndex = 0;
static unsigned char receivedChecksum = 0;

void UartDecodeMessage(unsigned char c)
{
    switch (rcvState)
    {
        case WAITING:
            // On attend le début du message
            if (c == 0xFE) {
                rcvState = FUNCTION_MSB; // On passe à la lecture de la fonction
            }
            break;

        case FUNCTION_MSB:
            msgDecodedFunction = c << 8; // Premier octet de la fonction
            rcvState = FUNCTION_LSB;     // On passe au second octet
            break;

        case FUNCTION_LSB:
            msgDecodedFunction += c;     // On complète la fonction
            rcvState = PAYLOAD_LENGTH_MSB; // On passe à la longueur
            break;

        case PAYLOAD_LENGTH_MSB:
            msgDecodedPayloadLength = c << 8; // Premier octet de la longueur
            rcvState = PAYLOAD_LENGTH_LSB;   // On lit le second octet
            break;

        case PAYLOAD_LENGTH_LSB:
            msgDecodedPayloadLength += c;    // On complète la longueur
            msgDecodedPayloadIndex = 0;      // On commence à stocker le payload
            rcvState = PAYLOAD;
            break;

        case PAYLOAD:
            if (msgDecodedPayloadIndex < sizeof(msgDecodedPayload)) {
                msgDecodedPayload[msgDecodedPayloadIndex++] = c; // On stocke l'octet
                if (msgDecodedPayloadIndex >= msgDecodedPayloadLength) {
                    rcvState = CHECKSUM; // On a tout lu, on attend la checksum
                }
            } else {
                // Si le message est trop long, on abandonne et on recommence
                rcvState = WAITING;
            }
            break;

        case CHECKSUM:
            receivedChecksum = c; // On lit la checksum
            // Vérifie si le message est correct
            if (UartCalculateChecksum(msgDecodedFunction, msgDecodedPayloadLength, msgDecodedPayload) == receivedChecksum) {
                // Message valide -> on le traite
                UartProcessDecodedMessage(msgDecodedFunction, msgDecodedPayloadLength, msgDecodedPayload);
            }
            // Toujours recommencer pour le prochain message
            rcvState = WAITING;
            break;

        default:
            rcvState = WAITING;
            break;
    }
}

//void UartProcessDecodedMessage(int function, int payloadLength, unsigned char* payload)
//{
//    switch (function)
//    {
//        case 0x0080: // Texte reçu
//            // Ici on peut afficher ou stocker le texte
//            break;
//
//        case 0x0020: // Commande LED
//            // Exemple : allumer ou éteindre les LEDs
//            break;
//
//        case 0x0030: // Capteurs IR
//            // Exemple : mettre à jour les valeurs IR
//            break;
//
//        case 0x0040: // Commande moteurs
//            // Exemple : mettre à jour la vitesse des moteurs
//            // payload[0] = vitesse gauche
//            // payload[1] = vitesse droite
//            break;
//
//        case 0x0050: // Déplacement
//            // Exemple : afficher le déplacement en cours
//            break;
//
//        default:
//            // Message inconnu
//            break;
//    }
//}
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
