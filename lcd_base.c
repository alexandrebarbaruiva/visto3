/*
 * lcd_base.c
 *
 *  Created on: 30 de mai de 2023
 *      Author: user
 */
#include <msp430.h>
#include <lcd_base.h>

void lcd_run(void){
    gpio_config();
    i2c_config();

    if (pcf_teste(PCF_ADR)==FALSE){
        led_VM();           //Indicar que n�o houve ACK
        while(TRUE);        //Travar
    }
    else    led_VD();       //Houve ACK, tudo certo

    lcd_inic();     //Inicializar LCD
    pcf_write(8);   //Acender Back Light
}

void lcd_float(float f, int casas){

    int d;
    int casa = 0;

    d = (int) f;
    f -= d;

    lcd_char(((char) d) | 0x30);

    lcd_char(',');

    while(casa < casas){
        f *= 10;
        d = (int) f;
        f -= d;
        lcd_char(((char) d) | 0x30);
        casa += 1;
    }
}

void lcd_dec_only(float f){

    int d;
    int casa = 0;

    d = (int) f;
    f -= d;

    while(casa < 4){
        f *= 10;
        d = (int) f;
        f -= d;
        lcd_char(((char) d) | 0x30);
        casa += 1;
    }
}

void lcd_dec8(char n){
    char w;
    w = n/100;
    lcd_char(w | 0x30);
    n -= 100 * w;
    w = n/10;
    lcd_char(w|0x30);
    n -= 10 * w;
    lcd_char(n|0x30);
}

void lcd_dec16(int n){
    int w, i;
    int d = 10000;
    int value = 0;

    for(i = 0; i < 5; i++){
        w = n/d;
        value = w | 0x30;
        lcd_char(value);
        n -= d * w;
        d /= 10;
    }
}

void lcd_dec16_pos(int n, char position){
    lcd_cursor(position);
    lcd_dec16(n);
}

void lcd_cursor(char position){
    if((position >= 16) && (position < 32)){
        position = position - 16 + 0x40;
    }
    else if (position > 32){
        position = 0x4F;
    }
    position |= 0x80;
    char esquerda, direita;

    esquerda = position & 0xF0;
    direita  = (position & 0x0F) << 4;

    pcf_write(esquerda | 8);
    pcf_write(esquerda | 0xC);
    pcf_write(esquerda | 8);
    pcf_write(direita | 8);
    pcf_write(direita | 0xC);
    pcf_write(direita | 8);
}
void lcd_char(char dado){
    char esquerda, direita;

    esquerda = dado & 0xF0;
    direita  = (dado & 0x0F) << 4;

    pcf_write(esquerda | 9);
    pcf_write(esquerda | 0xD);
    pcf_write(esquerda | 9);
    pcf_write(direita | 9);
    pcf_write(direita | 0xD);
    pcf_write(direita | 9);
}

void lcd_str(char *dados){
    char i = 0;
    while( dados[i] != 0)
        lcd_char(dados[i++]);
}

void lcd_str_pos(char* dados, char position){
    lcd_cursor(position);
    lcd_str(dados);
}
// Incializar LCD modo 4 bits
void lcd_inic(void){

    // Preparar I2C para operar
    UCB0I2CSA = PCF_ADR;    //Endere�o Escravo
    UCB0CTL1 |= UCTR    |   //Mestre TX
            UCTXSTT;    //Gerar START
    while ( (UCB0IFG & UCTXIFG) == 0);          //Esperar TXIFG=1
    UCB0TXBUF = 0;                              //Sa�da PCF = 0;
    while ( (UCB0CTL1 & UCTXSTT) == UCTXSTT);   //Esperar STT=0
    if ( (UCB0IFG & UCNACKIFG) == UCNACKIFG)    //NACK?
        while(1);

    // Come�ar inicializa��o
    lcd_aux(0);     //RS=RW=0, BL=1
    delay(20000);
    lcd_aux(3);     //3
    delay(10000);
    lcd_aux(3);     //3
    delay(10000);
    lcd_aux(3);     //3
    delay(10000);
    lcd_aux(2);     //2

    // Entrou em modo 4 bits
    lcd_aux(2);     lcd_aux(8);     //0x28
    lcd_aux(0);     lcd_aux(8);     //0x08
    lcd_aux(0);     lcd_aux(1);     //0x01
    lcd_aux(0);     lcd_aux(6);     //0x06
    lcd_aux(0);     lcd_aux(0xF);   //0x0F

    while ( (UCB0IFG & UCTXIFG) == 0)   ;          //Esperar TXIFG=1
    UCB0CTL1 |= UCTXSTP;                           //Gerar STOP
    while ( (UCB0CTL1 & UCTXSTP) == UCTXSTP)   ;   //Esperar STOP
    delay(50);
}

// Auxiliar inicializa��o do LCD (RS=RW=0)
// *** S� serve para a inicializa��o ***
void lcd_aux(char dado){
    while ( (UCB0IFG & UCTXIFG) == 0);              //Esperar TXIFG=1
    UCB0TXBUF = ((dado<<4)&0XF0) | BIT3;            //PCF7:4 = dado;
    delay(50);
    while ( (UCB0IFG & UCTXIFG) == 0);              //Esperar TXIFG=1
    UCB0TXBUF = ((dado<<4)&0XF0) | BIT3 | BIT2;     //E=1
    delay(50);
    while ( (UCB0IFG & UCTXIFG) == 0);              //Esperar TXIFG=1
    UCB0TXBUF = ((dado<<4)&0XF0) | BIT3;            //E=0;
}

// Ler a porta do PCF
int pcf_read(void){
    int dado;
    UCB0I2CSA = PCF_ADR;                //Endere�o Escravo
    UCB0CTL1 &= ~UCTR;                  //Mestre RX
    UCB0CTL1 |= UCTXSTT;                //Gerar START
    while ( (UCB0CTL1 & UCTXSTT) == UCTXSTT);
    UCB0CTL1 |= UCTXSTP;                //Gerar STOP + NACK
    while ( (UCB0CTL1 & UCTXSTP) == UCTXSTP)   ;   //Esperar STOP
    while ( (UCB0IFG & UCRXIFG) == 0);  //Esperar RX
    dado=UCB0RXBUF;
    return dado;
}

// Escrever dado na porta
void pcf_write(char dado){
    UCB0I2CSA = PCF_ADR;        //Endere�o Escravo
    UCB0CTL1 |= UCTR    |       //Mestre TX
            UCTXSTT;        //Gerar START
    while ( (UCB0IFG & UCTXIFG) == 0)   ;          //Esperar TXIFG=1
    UCB0TXBUF = dado;                              //Escrever dado
    while ( (UCB0CTL1 & UCTXSTT) == UCTXSTT)   ;   //Esperar STT=0
    if ( (UCB0IFG & UCNACKIFG) == UCNACKIFG)       //NACK?
        while(1);                          //Escravo gerou NACK
    UCB0CTL1 |= UCTXSTP;                        //Gerar STOP
    while ( (UCB0CTL1 & UCTXSTP) == UCTXSTP)   ;   //Esperar STOP
}

// Testar endere�o I2C
// TRUE se recebeu ACK
int pcf_teste(char adr){
    UCB0I2CSA = adr;                            //Endere�o do PCF
    UCB0CTL1 |= UCTR | UCTXSTT;                 //Gerar START, Mestre transmissor
    while ( (UCB0IFG & UCTXIFG) == 0);          //Esperar pelo START
    UCB0CTL1 |= UCTXSTP;                        //Gerar STOP
    while ( (UCB0CTL1 & UCTXSTP) == UCTXSTP);   //Esperar pelo STOP
    if ((UCB0IFG & UCNACKIFG) == 0)     return TRUE;
    else                                return FALSE;
}

// Configurar UCSB0 e Pinos I2C
// P3.0 = SDA e P3.1=SCL
void i2c_config(void){
    UCB0CTL1 |= UCSWRST;    // UCSI B0 em ressete
    UCB0CTL0 = UCSYNC |     //S�ncrono
            UCMODE_3 |   //Modo I2C
            UCMST;       //Mestre
    UCB0BRW = BR_100K;      //100 kbps
    P3SEL |=  BIT1 | BIT0;  // Use dedicated module
    UCB0CTL1 = UCSSEL_2;    //SMCLK e remove ressete
}

void led_vd(void)   {P4OUT &= ~BIT7;}   //Apagar verde
void led_VD(void)   {P4OUT |=  BIT7;}   //Acender verde
void led_vm(void)   {P1OUT &= ~BIT0;}   //Apagar vermelho
void led_VM(void)   {P1OUT |=  BIT0;}   //Acender vermelho

// Configurar leds
void gpio_config(void){
    P1DIR |=  BIT0;      //Led vermelho
    P1OUT &= ~BIT0;      //Vermelho Apagado
    P4DIR |=  BIT7;      //Led verde
    P4OUT &= ~BIT7;      //Verde Apagado
}

void delay(long limite){
    volatile long cont=0;
    while (cont++ < limite);
}

