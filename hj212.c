#include <stdlib.h>
#include <string.h>

#include "main.h"
#include "hj212.h"

int hj212_valid(const char *packet, char mn[])
{
    char crc[6] = {'0', 'x', 0, 0, 0, 0};
    unsigned int crc16 = 0;
    unsigned int crc16_calc = 0;
    int len = 0;
    char *p_mn = NULL;
    char *p_colon = NULL;

    if (NULL == packet)
        return -1;
    if (strncmp(packet, "##", 2) != 0)
        return -1;

    len = strlen(packet);
    if (len >= 2) {
        if (strncmp(packet + len - 2, "\r\n", 2) != 0)
            return -1;
    } else
        return -1;
    if (len <= 12)
        return -1;
    
    memcpy(&crc[2], packet + len - 6, 4);
    crc16 = (unsigned int)strtoul(crc, NULL, 16);
    crc16_calc = CRC16_Checkout(packet + 6, len - 12);
    if (crc16 != crc16_calc)
        return -1;

    p_mn = strstr(packet, "MN=");
    if (p_mn == NULL)
        return -1;

    p_colon = index(p_mn + 3, ';');
    if (p_colon == NULL)
        return -1;
    
    if ((p_colon - p_mn - 3 > 0) &&
        (p_colon - p_mn - 3) <= MN_SIZE)
        memcpy(mn, p_mn + 3, p_colon - p_mn - 3);
    else
        return -1;

    return 0;
}