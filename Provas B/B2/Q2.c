#include <msp430.h>

#define LED2_ON              (P4OUT |= BIT7)
#define LED2_OFF             (P4OUT &= ~BIT7)
#define LED2_TOGGLE          (P4OUT ^= BIT7)

#define FLLN(x) ((x)-1)
void clockInit();
void pmmVCore (unsigned int level);

void init_ADC();
void init_timer(unsigned int frequencia);
void start_timer();

void funcao_dado_consolidado();
void refresh();

void configLED2();
void configTIMER_VERDE();

int adc12_conversion_ready;
char data_buffer[32];

int g=0, i=0, dado_consolidado=0, freq;

const long int timer_value = 1000;

/**
 * main.c
 */
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    clockInit();

    //Codigo do aluno vem aqui.

        configLED2();

        init_timer(32);
        init_ADC();

        configTIMER_VERDE();

        __enable_interrupt();

        adc12_conversion_ready = 0;

        start_timer();

    while(1){
             adc12_conversion_ready = 0;
        }

    return 0;
}

#pragma vector = ADC12_VECTOR
__interrupt void ADC12_interrupt(void)
{
    data_buffer[g] = ADC12MEM0;
    g++;
    if(g==32){
        adc12_conversion_ready = 1;
        funcao_dado_consolidado();
        g=0;
        dado_consolidado = 0;
    }
    ADC12CTL0 |= ADC12ENC;

}

void init_ADC(){
    //Configuro o P6.0 para o pino A0 do ADC.
    P6SEL |= BIT0;

    //Desliga o módulo
    ADC12CTL0 &= ~ADC12ENC;

    ADC12CTL0 = ADC12SHT0_3 |                         //Usando 32 ciclos para o tsample
                            ADC12ON;                                  //Liga o ADC

    ADC12CTL1 = ADC12CSTARTADD_0   |           //Start address: 0
                            ADC12SHS_1 |                           //Conversão via TimerA0.1
                            ADC12SHP |                               //Sample and Hold Pulse mode: input
                            ADC12DIV_0 |                            //Divide o clock por 1
                            ADC12SSEL_0 |                         //Escolhe o clock MODCLK: 4.8 MHz
                            ADC12CONSEQ_1;                    //Modo: single channel / REPEAT conversion

    ADC12CTL2 = ADC12TCOFF |                           //Desliga o sensor de temperatura
                            ADC12RES_0;                            //8 bits resolution


    //Configurações dos canais
    ADC12MCTL0 = ADC12SREF_0 |                     //Vcc/Vss = 3.3V/0V
                               ADC12EOS |                           //END OF SEQUENCE (não importa no single channel)
                               ADC12INCH_0 ;                      //Amostra o pino A0

    ADC12IE = ADC12IE0;                                      //Liga a interrupção do canal 1.

    //Liga o ADC.
    ADC12CTL0 |= ADC12ENC;
}

void init_timer(unsigned int frequencia)
{
    TA0CTL = TASSEL__ACLK |                                 //Usa o ACLK: 32768
                     MC__STOP;                              //Timer parado

    TA0CCTL1 = OUTMOD_6;

    TA0CCR0 = 16384/(2*frequencia);
    TA0CCR1 = 8192/(2*frequencia);
}

void start_timer()
{
    TA0CTL |= (MC__UP | TACLR); //Start timer.
}

void configTIMER_VERDE(){

    P4SEL |= BIT7; //Disponibilizar P4.7
    P4DIR |= BIT7; //P4.7 como saída
    PMAPKEYID = 0X02D52; //Liberar mapeamento de P4
    P4MAP7 = PM_TB0CCR1A; //P4.7 = TB0.1

    TB0CTL = TASSEL__SMCLK | TBIDEX_5 | ID__2 | MC__UP | TACLR; //Conta até o limiar

    TB0CCTL1 = CM_0 | OUTMOD_6;

    TB0CCR0 = timer_value - 1; //Tempo de Reset
    TB0CCR1 = timer_value/2 - 1; //Tempo de Set

}


//LED verde, P4.7
void configLED2()
{
    P4SEL &= ~BIT7;
    P4DIR |= BIT7;
    LED2_OFF;
}

void funcao_dado_consolidado(){
    int x;
     for(i=0; i<32; i++){
        dado_consolidado += data_buffer[i];
     }
     dado_consolidado /= 32;
     x = timer_value*dado_consolidado;
     x /= (2*255);
     TB0CCR1 = x;
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
