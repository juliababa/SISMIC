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

void initialize_I2C_UCB0_MasterTransmitter();
void master_I2C_UCB0_TransmitOneByte(unsigned char address, unsigned char data);
unsigned char byteTransmitted;
unsigned char addresTransmitted;

void initialize_I2C_UCB1_Slave();
unsigned char byteReceived;


void delay_us(unsigned int time_us);

unsigned char endereço_slave;
unsigned char senha_data;

/**
 * main.c
 */
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    initialize_I2C_UCB0_MasterTransmitter();

    initialize_I2C_UCB1_Slave();


    byteTransmitted = 0x01;
    addresTransmitted =0x00;

    while(1)
    {

        while (1){
            master_I2C_UCB0_TransmitOneByte(addresTransmitted, byteTransmitted);

            delay_us(30000);

            if(byteReceived == byteTransmitted){
                endereço_slave = addresTransmitted;
                break;
            }
            addresTransmitted++;
        }

        while (1)
        {
            master_I2C_UCB0_TransmitOneByte(endereço_slave, byteTransmitted);

            volatile int n;

            delay_us(30000);

            if((P4OUT & BIT7) != 0){
                senha_data = byteTransmitted;
                break;
            }

            byteTransmitted++;
        }

        while(1);
    }

    return 0;
}


void initialize_I2C_UCB0_MasterTransmitter()
{
    __disable_interrupt();

    //Desliga o módulo/reseta a interface
    UCB0CTL1 |= UCSWRST;

    //Configura os pinos
    P3SEL |= BIT0;     //Configuro os pinos para "from module"
    P3SEL |= BIT1;
    P3REN &= ~BIT0;     //Resistores externos.
    P3REN &= ~BIT1;


    UCB0CTL0 = UCMST |  //Master Mode
                          UCMODE_3 |    //I2C Mode
                          UCSYNC;         //Synchronous Mode

    UCB0CTL1 = UCSSEL__ACLK |    //Clock Source: ACLK
                          UCTR |                      //Transmitter
                          UCSWRST ;             //Mantém o módulo desligado

    //Divisor de clock para o BAUDRate
    UCB0BR0 = 2;
    UCB0BR1 = 0;

    //Liga o módulo. / zera ob ti de RST paar deixar a interface funcionar
    UCB0CTL1 &= ~UCSWRST;

    UCB0IE = 0;

}

void master_I2C_UCB0_TransmitOneByte(unsigned char address, unsigned char data)
{
    //Coloco o slave address
    UCB0I2CSA = address;

    //Espero a linha estar desocupada.
    while (UCB0STAT & UCBBUSY);

    //Peço um START
    UCB0CTL1 |= UCTXSTT;

    //Espero até o buffer de transmissão estar disponível
    while ((UCB0IFG & UCTXIFG) == 0);

    //Escrevo o dado
    UCB0TXBUF = data;

    //Aguardo o acknowledge
    while (UCB0CTL1 & UCTXSTT);

    //Verifico se é um ACK ou um NACK
    if (UCB0IFG & UCTXNACK != 0)
    {
        //Peço uma condição de parada
        UCB0CTL1 |= UCTXSTP;
    } else
    {
        //Peço uma condição de parada
        UCB0CTL1 |= UCTXSTP;
    }

    return;
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
