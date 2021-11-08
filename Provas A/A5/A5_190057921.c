/***********************************************************************************************
      CÁLCULO DA VELOCIDADE MÁXIMA
resolução: tc > ln (2^(N+1)) * (Rin + 1.8k) * 25p + 800n
           tc > ln (2^(12+1)) * (10000 + 1800) * 25*(10^(-12)) + 800*(10^(-9))
           tc > 3.4582*(10^(-6))

           freq = 1 / 3.4582*(10^(-6))
           freq = 289.166,14983

           Ou seja, podem ser feitas, no máximo, 289.166 medições por segundo, aproximadamente.

***********************************************************************************************/

#include <msp430.h>
#include <stdint.h>

#define LED1_ON               (P1OUT |= BIT0)
#define LED1_OFF              (P1OUT &= ~BIT0)
#define LED1_TOGGLE           (P1OUT ^= BIT0)

#define LED2_ON              (P4OUT |= BIT7)
#define LED2_OFF             (P4OUT &= ~BIT7)
#define LED2_TOGGLE          (P4OUT ^= BIT7)

uint32_t SumData[2] = {0};
int adc12_conversion_ready;
int count = 0;

void configLED1();
void configLED2();

void init_ADC();
void init_timer(unsigned int frequencia);
void start_timer();

void acende_apaga_led();

int main(void){
    WDTCTL = WDTPW | WDTHOLD;

    configLED1();
    configLED2();

    init_timer(120);
    init_ADC();

    __enable_interrupt();

    adc12_conversion_ready = 0;

    start_timer();
    while (1)
    {
        acende_apaga_led();
        adc12_conversion_ready = 0;
    }

}

#pragma vector = ADC12_VECTOR
__interrupt void ADC12_interrupt(void)
{
    SumData[0] = ADC12MEM0;
    SumData[1] = ADC12MEM1;
    ADC12CTL0 |= ADC12ENC;
    adc12_conversion_ready = 1;
}

//LED vermelho, P1.0
void configLED1()
{
    P1SEL &= ~BIT0;
    P1DIR |= BIT0;
    LED1_OFF;
}

//LED verde, P4.7
void configLED2()
{
    P4SEL &= ~BIT7;
    P4DIR |= BIT7;
    LED2_OFF;
}

void init_ADC()
{
    //Configuro o P6.0 para o pino A0 do ADC.
    P6SEL |= BIT0;
    //Configuro o P6.1 para o pino A1 do ADC.
    P6SEL |= BIT1;

    //Desliga o módulo
    ADC12CTL0 &= ~ADC12ENC;

    ADC12CTL0 = ADC12SHT0_3 |                               //Usando 32 ciclos para o tsample
                ADC12ON;                                    //Liga o ADC

    ADC12CTL1 = ADC12CSTARTADD_0   |                        //Start address: 0
                ADC12SHS_1 |                                //Conversão via TimerA0.1
                ADC12SHP |                                  //Sample and Hold Pulse mode: input
                ADC12DIV_0 |                                //Divide o clock por 1
                ADC12SSEL_0 |                               //Escolhe o clock MODCLK: 4.8 MHz
                ADC12CONSEQ_1;                              //Modo: single channel / REPEAT conversion

    ADC12CTL2 = ADC12TCOFF |                                //Desliga o sensor de temperatura
                ADC12RES_2;                                 //12 bits resolution


    //Configurações dos canais
    ADC12MCTL0 = ADC12SREF_0 |                              //Vcc/Vss = 3.3V/0V
                               //ADC12EOS |                 //END OF SEQUENCE (não importa no single channel)
                               ADC12INCH_0 ;                //Amostra o pino A0

    ADC12MCTL1 = ADC12SREF_0 |                              //Vcc/Vss = 3.3V/0V
                 ADC12EOS |                                 //END OF SEQUENCE
                 ADC12INCH_1 ;                              //Amostra o pino A1


    ADC12IE = ADC12IE1;                                     //Liga a interrupção do canal 1.

    //Liga o ADC.
    ADC12CTL0 |= ADC12ENC;
}

void init_timer(unsigned int frequencia)
{
    TA0CTL = TASSEL__ACLK |                                 //Usa o ACLK: 32768
                     MC__STOP;                              //Timer parado

    TA0CCTL1 = OUTMOD_6;

    TA0CCR0 = 32768/(4*frequencia);
    TA0CCR1 = 16384/(4*frequencia);
}

void start_timer()
{
    TA0CTL |= (MC__UP | TACLR); //Start timer.
}

void acende_apaga_led(){
    if(SumData[0] > (SumData[1]+248)){
        LED1_ON;
        LED2_OFF;
    }
    if(SumData[1]> (SumData[0]+248)){
        LED1_OFF;
        LED2_ON;
    }
    if((SumData[1] -  SumData[0]) < 248){
        LED1_ON;
        LED2_ON;
    }
}
