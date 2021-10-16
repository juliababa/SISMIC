//Aluno(a): Júlia Yuri Garcia Baba
//Matrícula: 190057921
//TURMA A

#include <msp430.h>

#define LED1_ON               (P1OUT |= BIT2)
#define LED1_OFF              (P1OUT &= ~BIT2)
#define LED1_TOGGLE           (P1OUT ^= BIT2)

#define LED2_ON              (P4OUT |= BIT7)
#define LED2_OFF             (P4OUT &= ~BIT7)
#define LED2_TOGGLE          (P4OUT ^= BIT7)


void configLED1();
void configLED2();


void configTIMER_VERDE();
void configTIMER_VERMELHO();

float vector[4] = {0.1, 0.3, 0.5, 0.7, 0.9};

int i = 0, freq;

void main(void){
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    configLED1();
    configLED2();


    configTIMER_VERDE();
    configTIMER_VERMELHO();

    __enable_interrupt();

    while(1){

    }

}

//LED vermelho, P1.0
void configLED1()
{
    P1SEL &= ~BIT2;
    P1DIR |= BIT2;
    LED1_OFF;
}

//LED verde, P4.7
void configLED2()
{
    P4SEL &= ~BIT7;
    P4DIR |= BIT7;
    LED2_OFF;
}



void configTIMER_VERDE(){
    TA1CTL = TASSEL__ACLK | MC__UP | TACLR;

    TA1CCR0 = 32768/4;

    TA1CCTL1 = CM_0 | OUTMOD_6 | OUT | CCIE; //subida e descida

    TA1CCR1 = 32768/8;

}


void configTIMER_VERMELHO(){

    P1SEL |= BIT2;
    P1DIR |= BIT2;
    P1OUT &= ~BIT2;

    TA0CTL = TASSEL__ACLK | MC__UP | TACLR;

    TA0CCR0 = 499;

    TA0CCTL1 = CM_0 | OUTMOD_6 | OUT; //subida e descida

    TA0CCR1 =TA0CCR0/2;
}

#pragma vector = TIMER1_A1_VECTOR
__interrupt void LEDVERDE(){
    switch(TA1IV){
        case TA1IV_NONE:
            break;
        case TA1IV_TA1CCR1:
            LED2_TOGGLE;

            freq = 499*vector[i];
            TA0CCR1 = freq;

            if(i==4){
                i = -1;
            }
            i++;
            break;
        default:
            break;
    }
}





