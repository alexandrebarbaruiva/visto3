/**** lcd_base.h ****/
#ifndef LCD_BASE_H_
#define LCD_BASE_H_

// LCD_Base
// Rotinas b�sicas para usar o LCD

// P3.0 ==> SDA
// P3.1 ==> SCL

#include <msp430.h>

#define TRUE    1
#define FALSE   0

// Defini��o do endere�o do PCF_8574
#define PCF_ADR1 0x3F
#define PCF_ADR2 0x27
#define PCF_ADR  PCF_ADR2

#define BR_100K    11  //SMCLK/100K = 11
#define BR_50K     21  //SMCLK/50K  = 21
#define BR_10K    105  //SMCLK/10K  = 105

void lcd_run(void);
void lcd_inic(void);
void lcd_aux(char dado);
int pcf_read(void);
void pcf_write(char dado);
int pcf_teste(char adr);
void led_vd(void);
void led_VD(void);
void led_vm(void);
void led_VM(void);
void i2c_config(void);
void gpio_config(void);
void delay(long limite);

// funcoes para printar
void lcd_char(char dado);
void lcd_str(char *dados);
void lcd_str_pos(char *dados, char position);
void lcd_cursor(char position);
void lcd_dec_only(float f);
void lcd_dec8(char n);
void lcd_dec16(int n);
void lcd_dec16_pos (int n, char position);
void lcd_float(float n, int casas);


#endif /* LCD_BASE_H_ */

