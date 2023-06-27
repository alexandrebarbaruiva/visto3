// ER 11.4
// Acender leds de acordo com conversão
// 0x00 ==> 0x3F: led1 led2
// 0x3F ==> 0x7F: led1 LED2
// 0x80 ==> 0xBF: LED1 led2
// 0xC0 ==> 0xFF: LED1 LED2
// SW alterna entre VRx e VRy
// Usar TA0.1 para disparar a conversão em 100 Hz
#include <msp430.h>
#include "lcd_base.h"
#define FALSE 0
#define TRUE 1
#define VRX 0 //Indicar canal VRx
#define VRY 1 //Indicar canal VRy
#define SMIN 524
#define SMAX 2621
#define DBC 1000
#define FECHADA 0 //SW fechada
#define ABERTA 1 //SW aberta
#define DBC 1000 //Debounce

// Funções
void leds(char x);
int sw_mon(void);
void ADC_config(void);
void TA0_config(void);
void GPIO_config(void);
void move_servo(int dist);
void debounce(int valor);

volatile int media_x, media_y;
volatile float vx,vy;

int main(void){
    char canal=VRX;
    WDTCTL = WDTPW | WDTHOLD; // stop watchdog timer
    GPIO_config();
    TA0_config();
    ADC_config();
    __enable_interrupt();
    lcd_run();
    float min_val = 4.0;
    float max_val = 0.0;
    float tmp;
    while(TRUE){
        if (sw_mon() == TRUE){
            canal ^= 1; //Inverter
            min_val = 3.4;
            max_val = 0.0;
        }

        while( (ADC12IFG & ADC12IFG7) == 0);
        media_x = (ADC12MEM0 + ADC12MEM2 + ADC12MEM4 + ADC12MEM6)/4; // A1
        media_y = (ADC12MEM1 + ADC12MEM3 + ADC12MEM5 + ADC12MEM7)/4; // A2

        //        max_val

        vx = (3.3/4095) * media_x;
        vy = ((3.3/4095) * media_y - 3.3) * (-1);
        lcd_cursor(0);
        lcd_str("A");
        if (canal == VRX) {
            lcd_str("1=");
            lcd_float(vx, 3);
            lcd_str("V");
            lcd_cursor(12);
            lcd_dec_only(vx);

            if (vx < min_val) min_val = vx;
            if (vx > max_val) max_val = vx;
            move_servo((int) (vx * 10));
        }
        else {
            lcd_str("2=");
            lcd_float(vy, 3);
            lcd_str("V");

            lcd_cursor(12);
            lcd_dec_only(vy);

            if (vy < min_val) min_val = vy;
            if (vy > max_val) max_val = vy;
            move_servo((int) (vy * 10));
        }

        lcd_cursor(16);
        lcd_str("Mn=");
        lcd_float(min_val, 2);
        lcd_cursor(25);
        lcd_str("Mx=");
        lcd_float(max_val, 2);

    }
    return 0;
}
// Controlar os leds de acordo com x
void leds(char x){
    switch(x){
    case 0: led_vm(); led_vd(); break; //0
    case 1: led_vm(); led_VD(); break; //1
    case 2: led_VM(); led_vd(); break; //2
    case 3: led_VM(); led_VD(); break; //3
    }
}

//#pragma vector = 54
//#pragma vector = ADC12_VECTOR
//__interrupt void adc_int(void){
//    volatile unsigned int *pt;
//    unsigned int i,soma;
//    pt = &ADC12MEM0;
//    soma = 0;
//    for (i=0; i<8; i++) soma +=pt[i];
//    media_x = soma >>3;
//    soma = 0;
//    for (i=8; i<16; i++) soma +=pt[i];
//    media_y = soma >>3;
//}

// Monitorar SW (P6.3), retorna TRUE se foi acionada
int sw_mon(void){
    static int psw=ABERTA; //Guardar passado de Sw
    if ( (P6IN&BIT3) == 0){ //Qual estado atual de Sw?
        if (psw==ABERTA){ //Qual o passado de Sw?
            debounce(DBC);
            psw=FECHADA;
            return TRUE;
        }
    }
    else {
        if (psw==FECHADA){ //Qual o passado de Sw?
            debounce(DBC);
            psw=ABERTA;
            return FALSE;
        }
    }
    return FALSE;
}

void move_servo(int dist)
{
    if(dist <= 50){
        TA2CCR2 = ( (long)2097 * (50 - dist) ) / 50 + 524;
    }
}


void ADC_config(void){
    volatile unsigned char *pt;
    unsigned char i;
    ADC12CTL0 &= ~ADC12ENC; //Desabilitar para configurar
    ADC12CTL0 = ADC12ON; //Ligar ADC
    ADC12CTL1 = ADC12CONSEQ_3 | //Modo sequência de canais repetido
            ADC12SHS_1 | //Selecionar TA0.1
            ADC12CSTARTADD_0 | //Resultado a partir de ADC12MEM0
            ADC12SSEL_3; //ADC12CLK = SMCLK
    ADC12CTL2 = ADC12RES_2; //ADC12RES=2, Modo 12 bits

    ADC12MCTL0 = ADC12SREF_0 | ADC12INCH_1; // X
    ADC12MCTL1 = ADC12SREF_0 | ADC12INCH_2; // Y
    ADC12MCTL2 = ADC12SREF_0 | ADC12INCH_1; // X
    ADC12MCTL3 = ADC12SREF_0 | ADC12INCH_2;
    ADC12MCTL4 = ADC12SREF_0 | ADC12INCH_1; // X
    ADC12MCTL5 = ADC12SREF_0 | ADC12INCH_2;
    ADC12MCTL6 = ADC12SREF_0 | ADC12INCH_1; // X
    ADC12MCTL7 = ADC12SREF_0 | ADC12INCH_2 | ADC12EOS; //ADC12MCTL0 até ADC12MCTL7, EOS em ADC12MCTL7

    //    for (i=8; i<16; i++)
    //        pt[i]=ADC12SREF_0 | ADC12INCH_2; //ADC12MCTL8 até ADC12MCTL15

    P6SEL |= BIT2|BIT1; // Desligar digital de P6.2,1
    ADC12CTL0 |= ADC12ENC; //Habilitar ADC12
    // ADC12IE |= ADC12IE15; //Hab interrupção MEM2
}
void TA0_config(void){
    TA0CTL = TASSEL_1 | MC_1;
    TA0CCR0 = 1024;
    TA0CCTL1 = OUTMOD_6; //Out = modo 6
    TA0CCR0 = 1024; //200 Hz (100 Hz por canal)
    TA0CCR1 = TA0CCR0/2; //Carga 50%
}
void GPIO_config(void){
    P1DIR |= BIT0; P1OUT &= ~BIT0; //Led Vermelho
    P4DIR |= BIT7; P4OUT &= ~BIT7; //Led Verde

    P6DIR &= ~BIT3; //P6.3 = SW -> Joystick
    P6REN |= BIT3;
    P6OUT |= BIT3;

    // SERVO ===============================
    P2DIR |= BIT5;
    P2SEL |= BIT5
            TA2CTL = TASSEL_2 | MC__UP | TACLR; //
    TA2CCR0 = 20971;                    //
    TA2CCTL2 = OUTMOD_6;                //
    TA2CCR2 = SMIN;                     // SETA SERVO PARA 0
    // !SERVO ==============================
}
// Debounce
void debounce(int valor){
    volatile int x; //volatile evita optimizador
    for (x=0; x<valor; x++); //Apenas gasta tempo
}
