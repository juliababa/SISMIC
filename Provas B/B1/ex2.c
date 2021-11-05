#include <msp430.h> 

#define LED1_ON               (P1OUT |= BIT0)
#define LED1_OFF              (P1OUT &= ~BIT0)
#define LED1_TOGGLE           (P1OUT ^= BIT0)

#define LED2_ON              (P4OUT |= BIT7)
#define LED2_OFF             (P4OUT &= ~BIT7)
#define LED2_TOGGLE          (P4OUT ^= BIT7)


#define CHAVE2_PRESSIONADA   ((P1IN & BIT1) == 0)
#define CHAVE1_PRESSIONADA   ((P2IN & BIT1) == 0)


void configLED1();
void configLED2();
void configCHAVE2();
void configCHAVE1();
void debounce();
void tempo();
void start();
void createSequence(int n);
void showSequence(int n);
void youwon(); //Sequência correta
void youlose(); //Sequência Incorreta
int game(int n);
unsigned int lfsr();

//variaveis
unsigned int sequence[10];

int n = 2;


int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

    int k;
    int i;

    configLED1();
    configLED2();
    configCHAVE2();
    configCHAVE1();

    while(1){
        start();
        createSequence(n);
        showSequence(n);

        k = game(n);

        if(k==0){
            youlose();
        }
        if(k==1){
            youwon();
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

//Chave da direita, P1.1
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

void tempo(){
 volatile unsigned int x = 50000;
 while (x--);
}

void start(){
    tempo();
    tempo();
    LED1_ON;
    LED2_ON;
    tempo();
    LED1_OFF;
    tempo();
    tempo();
    LED2_OFF;
    tempo();
    tempo();
    tempo();
}

unsigned int mynumber = 1;
unsigned int lfsr(){
     mynumber++;
     unsigned int x = (mynumber % 3);
     return (x % 2);
}

void createSequence(int n){
    int i;
    for(i = 0; i<n; i++){
        sequence[i] = lfsr();
    }
}

void showSequence(int n){
    int i;
    for(i = 0; i<n; i++){
        if(sequence[i]==0){
            LED1_ON;
            tempo();
            LED1_OFF;
            tempo();
            tempo();
        }
        if(sequence[i]==1){
            LED2_ON;
            tempo();
            LED2_OFF;
            tempo();
            tempo();
        }
    }
}

int game(int n){

    int i;
    int resultado;

    for(i=0; i<n;){
        if(CHAVE1_PRESSIONADA){
            while(CHAVE1_PRESSIONADA){
                LED2_OFF;
            }
            debounce();
            if(sequence[i]==0){
                resultado = 1;
            }
            else{
                resultado = 0;
                break;
            }
            i++;
        }

        if(CHAVE2_PRESSIONADA){
            while(CHAVE2_PRESSIONADA){
                LED2_OFF;
            }
            debounce();
            if(sequence[i]==1){
                resultado = 1;
            }
            else{
                resultado = 0;
                break;
            }
            i++;
        }
    }

    return resultado;
}

void youwon(){
    int i;
    for(i=0; i<10; i++){
        LED2_TOGGLE;
        tempo();
    }
    if(n==10){
        while(1){
            LED2_TOGGLE;
            tempo();
        }
    }
    n++;
}
void youlose(){
    int i;
    for(i=0; i<10; i++){
        LED1_TOGGLE;
        tempo();
    }
    n = 2;
}



void debounce()
{
    volatile unsigned int x = 50000;
    while (x--);
}

