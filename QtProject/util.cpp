#include <string.h>

char nibbleToHex(int nibble)
{
    const int ascii_zero = 48;
    const int ascii_a = 65;

    if((nibble >= 0) && (nibble <= 9))
    {
        return (char) (nibble + ascii_zero);
    }
    if((nibble >= 10) && (nibble <= 15))
    {
        return (char) (nibble - 10 + ascii_a);
    }
    return '?';
}

char upperToHex(int byteVal)
{
    int i = (byteVal & 0xF0) >> 4;
    return nibbleToHex(i);
}

char lowerToHex(int byteVal)
{
    int i = (byteVal & 0x0F);
    return nibbleToHex(i);
}

char* toBinary (int v)
{
    static char binstr[17] ;
    int i ;

    binstr[16] = '\0' ;
    for (i = 0; i < 16; i++)
    {
        binstr[15 - i] = v & 1 ? '1' : '0';
        v = v / 2;
    }

    return binstr;
}
