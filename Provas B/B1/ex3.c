#include <msp430.h> 

#define LED1_ON             (P1OUT |= BIT0)
#define LED1_OFF            (P1OUT &= ~BIT0)
#define LED1_TOGGLE         (P1OUT ^= BIT0)

#define LED2_ON             (P4OUT |= BIT7)
#define LED2_OFF            (P4OUT &= ~BIT7)
#define LED2_TOGGLE         (P4OUT ^= BIT7)

#define CHAVE1_PRESSIONADA  ((P2IN & BIT1) == 0)
#define CHAVE2_PRESSIONADA  ((P1IN & BIT1) == 0)


void configLED1();
void configLED2();
void configCHAVE1_INT();
void configCHAVE2_INT();
void configCHAVE2();
void debounce();

int sw1_flag;
int sw2_flag;

/**
 * main.c
 */
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    configLED1();
    configLED2();
    configCHAVE1_INT();
    configCHAVE2_INT();


    _enable_interrupt();

    sw1_flag = 0;

    sw2_flag = 0;

    while (1)
    {

        if (sw1_flag != 0)
        {
            debounce();    //enrolo.
            P2IE |= BIT1;  //Ligo a interrupção.
            sw1_flag = 0;
        }
        if (sw2_flag != 0)
        {
            debounce();    //enrolo.
            P1IE |= BIT1;  //Ligo a interrupção.
            sw2_flag = 0;
        }

        if((!CHAVE2_PRESSIONADA) && (!CHAVE1_PRESSIONADA)){
            LED1_OFF;
            LED2_OFF;
        }

    }



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

//Chave da esquerda, P2.1 - CHAVE 1 - LED VERMELHO
void configCHAVE1_INT()
{
    P2SEL &= ~BIT1;
    P2DIR &= ~BIT1;
    P2REN |= BIT1;
    P2OUT |= BIT1;

    P2IES |= BIT1;
    P2IE |= BIT1;

    do {
        P2IFG = 0;
    } while (P2IFG != 0);
}

//Chave da direita, P1.1 - CHAVE 2 - LED VERDE
void configCHAVE2_INT()
{
    P1SEL &= ~BIT1;
    P1DIR &= ~BIT1;
    P1REN |= BIT1;
    P1OUT |= BIT1;

    P1IES |= BIT1;
    P1IE |= BIT1;

    do {
        P1IFG = 0;
    } while (P1IFG != 0);
}

void debounce()
{
    volatile unsigned int x = 50000;
    while (x--);
}

#pragma vector = PORT2_VECTOR
__interrupt void qualquer_coisa1()
{
    switch (_even_in_range(P2IV, 0x10))
    {
    case P2IV_NONE:
        break;
    case P2IV_P2IFG0:
            break;
    case P2IV_P2IFG1:
        //Momento que apertou a chave S1.
        if ((P2IES & BIT1) != 0) //se o flanco for de descida
        {
            sw1_flag = 1;
            P2IES &= ~BIT1; // inverte o IES (troca o flanco)
            P2IE &= ~BIT1; // desliga a interrupção
            //O que eu vou fazer quando aperta o botão.
            if(CHAVE1_PRESSIONADA){
                if(!CHAVE2_PRESSIONADA){
                    LED1_ON;
                }
            }
        }
        else{
            //se o flanco for de subida
            sw1_flag = 2;
            P2IES |= BIT1; //troca do flanco
            P2IE &= ~BIT1; //desliga a interrupção
        }
                break;
    case P2IV_P2IFG2:
                break;
    case P2IV_P2IFG3:
                break;
    case P2IV_P2IFG4:
                break;
    case P2IV_P2IFG5:
                break;
    case P2IV_P2IFG6:
                break;
    case P2IV_P2IFG7:
                break;
    default:
        break;
    }
}

#pragma vector = PORT1_VECTOR
__interrupt void qualquer_coisa2()
{
    switch (_even_in_range(P1IV, 0x10))
    {
    case P1IV_NONE:
        break;
    case P1IV_P1IFG0:
            break;
    case P1IV_P1IFG1:
        //Momento que apertou a chave S1.
        if ((P1IES & BIT1) != 0) //se o flanco for de descida
        {
            sw2_flag = 1;
            P1IES &= ~BIT1; // inverte o IES (troca o flanco)
            P1IE &= ~BIT1; // desliga a interrupção
            //O que eu vou fazer quando aperta o botão.
            if(CHAVE2_PRESSIONADA){
                 if(!CHAVE1_PRESSIONADA){
                     LED2_ON;
                }
            }
        }
        else{
            //se o flanco for de subida
            sw2_flag = 2;
            P1IES |= BIT1; //troca do flanco
            P1IE &= ~BIT1; //desliga a interrupção
        }
                break;
    case P1IV_P1IFG2:
                break;
    case P1IV_P1IFG3:
                break;
    case P1IV_P1IFG4:
                break;
    case P1IV_P1IFG5:
                break;
    case P1IV_P1IFG6:
                break;
    case P1IV_P1IFG7:
                break;
    default:
        break;
    }
}

 