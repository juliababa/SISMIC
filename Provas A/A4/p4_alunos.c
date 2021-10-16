#include <msp430.h> 

//O objetivo é encontrar essas duas variáveis: o endereço do SLAVE e a SENHA.
#define ADDRESS       0x05
#define DATA               0x0F

// - UCB1 é configurado como SLAVE RECEIVER
//Hardware setup:
// - P3.0 - P4.1 - SDA
// - P3.1 - P4.2 - SCL
// - O código NÃO LIGA os resistores de pull-up internos do RECEIVER (você pode ligar os resistores do master)
//O receiver usa o LED VERDE quando ele recebe o byte correto.

void initialize_I2C_UCB1_Slave();
unsigned char byteReceived;


void delay_us(unsigned int time_us);



/**
 * main.c
 */
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    initialize_I2C_UCB1_Slave();


    while(1)
    {

        //Após encontrar o endereço e o byte correto, o código deve travar aqui embaixo.
        while (1);
    }

    return 0;
}

/*
 * Delay microsseconds.
 */
void delay_us(unsigned int time_us)
{
    //Configure timer A0 and starts it.
    TA0CCR0 = time_us;
    TA0CTL = TASSEL__SMCLK | ID__1 | MC_1 | TACLR;

    //Locks, waiting for the timer.
    while((TA0CTL & TAIFG) == 0);

    //Stops the timer
    TA0CTL = MC_0 | TACLR;
}

/*
 *  P4.1 - SDA
 *  P4.2 - SCL
 */
void initialize_I2C_UCB1_Slave()
{
    //Configura o LED2.
    P4SEL &= ~BIT7;
    P4DIR |= BIT7;
    P4OUT &= ~BIT7;


    //Desliga o módulo
    UCB1CTL1 |= UCSWRST;

    //Configura os pinos
    P4SEL |= BIT1;     //Configuro os pinos para "from module"
    P4SEL |= BIT2;
    P4REN &= ~BIT1; //Desliga os resistores externos.
    P4REN &= ~BIT2;


    UCB1CTL0 = UCMODE_3 |    //I2C Mode
                           UCSYNC;         //Synchronous Mode

    UCB1CTL1 = UCSSEL__ACLK |    //Clock Source: ACLK
                           UCSWRST ;             //Mantém o módulo desligado

    //Escrevo meu endereço
    UCB1I2COA = ADDRESS;

    //Prepara minhas variáveis.
    byteReceived = 0;

    //Liga o módulo.
    UCB1CTL1 &= ~UCSWRST;

    //Liga a interrupção de RX.
    UCB1IE = UCRXIE;

    _enable_interrupt();
}



#pragma vector = USCI_B1_VECTOR;
__interrupt void i2c_isr()
{
    switch (__even_in_range(UCB1IV,12)) {
    case USCI_NONE:
        break;
    case USCI_I2C_UCALIFG:
        break;
    case USCI_I2C_UCNACKIFG:
        break;
    case USCI_I2C_UCSTTIFG:
        break;
    case USCI_I2C_UCSTPIFG:
        break;
    case USCI_I2C_UCRXIFG:
        byteReceived = (unsigned char) UCB1RXBUF;
        volatile char d = DATA;
        if (byteReceived == d)
        {
            P4OUT |= BIT7;
        } else {
            P4OUT &= ~BIT7;
        }
        break;
    case USCI_I2C_UCTXIFG:
        break;

    default:
        break;
    }
}
