#include <msp430.h>
#include "string.h"

#define FLLN(x) ((x)-1)
void clockInit();
void pmmVCore (unsigned int level);

#define CHAVE2_PRESSIONADA   ((P1IN & BIT1) == 0)
#define CHAVE1_PRESSIONADA   ((P2IN & BIT1) == 0)

void configCHAVE2();
void configCHAVE1();
void debounce();

void initializeUART_UCA0();

int tx_ready;

/**
 * main.c
 */
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    clockInit();

    //Codigo do aluno vem aqui.

        configCHAVE2();
        configCHAVE1();

        initializeUART_UCA0();

        __enable_interrupt();

        tx_ready = 1;

    while(1){
            if(CHAVE1_PRESSIONADA){
                debounce();
                while(tx_ready == 0);
                tx_ready = 0;
                UCA0TXBUF = 0xE7;
                while(tx_ready == 0);
                tx_ready = 0;
                UCA0TXBUF = 0x49;
                while(tx_ready == 0);
                tx_ready = 0;
                UCA0TXBUF = 0x07;

            }
            if(CHAVE2_PRESSIONADA){
                debounce();
                while(tx_ready == 0);
                tx_ready = 0;
                UCA0TXBUF = 0xB2;
                while(tx_ready == 0);
                tx_ready = 0;
                UCA0TXBUF = 0x38;
                while(tx_ready == 0);
                tx_ready = 0;
                UCA0TXBUF = 0xFA;

            }
    }

}

#pragma vector = USCI_A0_VECTOR
__interrupt void UARTA0_INTERRUPT(void){
    switch(_even_in_range(UCA0IV,4)){
        case 0: break;
        case 2:  //Reception Interrupt
            break;
        case 4:  //Transmission Interrupt
            tx_ready = 1;
            break;
        default: break;
    }
}
/*************************************************
 * INITIALIZATION FUNCTIONS
 *************************************************/
/*
 * P3.3 - TX
 * P4.2 - RX
 *
 * Baud Rate
 */

void initializeUART_UCA0()
{
    P3DIR |= BIT3;
    P3SEL |= BIT3;

    P3REN &= ~BIT3;

    //Desliga o módulo
    UCA0CTL1 |= UCSWRST;

    UCA0CTL0 = UCPEN |    //Parity enable: 1=ON, 0=OFF
               //UCPAR |    //Parity: 0:ODD, 1:EVEN
               //UCMSB |    //LSB First: 0, MSB First:1
               //UC7BIT |   //8bit Data: 0, 7bit Data:1
               UCSPB |    //StopBit: 0:1 Stop Bit, 1: 2 Stop Bits
               UCMODE_0 | //USCI Mode: 00:UART, 01:Idle-LineMultiprocessor, 10:AddressLine Multiprocessor, 11: UART with automatic baud rate
               //UCSYNC    //0:Assynchronous Mode, 1:Synchronous Mode
               0;

    UCA0CTL1 = UCSSEL__SMCLK | //00:UCAxCLK, 01:ACLK, 10:SMCLK, 11:SMCLK
               //UCRXEIE     | //Erroneous Character IE
               //UCBRKIE     | //Break Character IE
               //UCDORM      | //0:NotDormant, 1:Dormant
               //UCTXADDR    | //Next frame: 0:data, 1:address
               //UCTXBRK     | //TransmitBreak
               UCSWRST;        //Mantém reset.

    //BaudRate: 9600
    //BRCLK  = 1 MHz
    //UCBRx  = 104
    //UCBRSx = 1
    //UCBRFx = 0
    //UCOS16 = 0

    //UCA0MCTL = //UCBRF_0 |    //Modulation Stage. Ignored when UCOS16 = 0
    //           UCBRS_1 |    //Second Modulation stage
               //UCOS16 |     //Oversampling Mode. 0:disabled, 1:enabled.
    //           0;

    UCA0BRW = 4;
    UCA0MCTL = UCBRF_5 | UCOS16;


    //Liga o módulo
    UCA0CTL1 &= ~UCSWRST;

    UCA0IE =   UCTXIE ; //Interrupt on transmission
    //           UCRXIE |   //Interrupt on Reception
      //         0;


}

void configCHAVE2()
{
    P1SEL &= ~BIT1;
    P1DIR &= ~BIT1;
    P1REN |= BIT1;
    P1OUT |= BIT1;
}

//Chave da esquerda, P2.1
void configCHAVE1()
{
    P2SEL &= ~BIT1;
    P2DIR &= ~BIT1;
    P2REN |= BIT1;
    P2OUT |= BIT1;
}

void pmmVCore (unsigned int level)
{
#if defined (__MSP430F5529__)
    PMMCTL0_H = 0xA5;                       // Open PMM registers for write access

    SVSMHCTL =                              // Set SVS/SVM high side new level
            SVSHE            +
            SVSHRVL0 * level +
            SVMHE            +
            SVSMHRRL0 * level;

    SVSMLCTL =                              // Set SVM low side to new level
            SVSLE            +
//          SVSLRVL0 * level +              // but not SVS, not yet..
            SVMLE            +
            SVSMLRRL0 * level;

    while ((PMMIFG & SVSMLDLYIFG) == 0);    // Wait till SVM is settled

    PMMIFG &= ~(SVMLVLRIFG + SVMLIFG);      // Clear already set flags

    PMMCTL0_L = PMMCOREV0 * level;          // Set VCore to new level

    if ((PMMIFG & SVMLIFG))                 // Wait till new level reached
        while ((PMMIFG & SVMLVLRIFG) == 0);

    SVSMLCTL =                              // Set SVS/SVM low side to new level
            SVSLE            +
            SVSLRVL0 * level +
            SVMLE            +
            SVSMLRRL0 * level;

    PMMCTL0_H = 0x00;                       // Lock PMM registers for write access
#endif
}

void clockInit()
{
    pmmVCore(1);
    pmmVCore(2);
    pmmVCore(3);

    P5SEL |= BIT2 | BIT3 | BIT4 | BIT5;
    UCSCTL0 = 0x00;
    UCSCTL1 = DCORSEL_5;
    UCSCTL2 = FLLD__1 | FLLN(25);
    UCSCTL3 = SELREF__XT2CLK | FLLREFDIV__4;
    UCSCTL6 = XT2DRIVE_2 | XT1DRIVE_2 | XCAP_3;
    UCSCTL7 = 0;                            // Clear XT2,XT1,DCO fault flags

    do {                                    // Check if all clocks are oscillating
      UCSCTL7 &= ~(   XT2OFFG |             // Try to clear XT2,XT1,DCO fault flags,
                    XT1LFOFFG |             // system fault flags and check if
                       DCOFFG );            // oscillators are still faulty
      SFRIFG1 &= ~OFIFG;                    //
    } while (SFRIFG1 & OFIFG);              // Exit only when everything is ok

    UCSCTL5 = DIVPA_1 | DIVA_0 | DIVM_0; //Divide ACLK por 2.

    UCSCTL4 = SELA__XT1CLK    |             // ACLK  = XT1   =>      16.384 Hz
              SELS__XT2CLK    |             // SMCLK = XT2   =>   4.000.000 Hz
              SELM__DCOCLK;                 // MCLK  = DCO   =>  25.000.000 Hz

}

void debounce()
{
    volatile unsigned int x = 50000;
    while (x--);
}
