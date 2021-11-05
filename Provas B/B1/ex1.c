#include <msp430.h> 

//funções
unsigned int lfsr();

//variáveis
unsigned int number = 0xACE1;

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    while (1)
    {

        lfsr();

    }

}

unsigned int lfsr(){

    int bit16, bit14, bit13, bit11;
    int xor1, xor2, xor3;

    bit16 = (number >> 0) & 1;
    bit14 = (number >> 2) & 1;

    xor1 = bit16^bit14;

    bit13 = (number >> 3) & 1;

    xor2 = bit13^xor1;

    bit11 = (number >> 5) & 1;

    xor3 = bit11^xor2;

    if(xor3 == 1){
        number = ((number >> 1) & ~BITF) | (xor3 << 15);
    }
    else if(xor3 == 0){
        number = number >> 1;
    }

    return number;

}





