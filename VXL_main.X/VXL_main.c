#include <xc.h> 
#include <stdio.h>
#include <stdlib.h>

#define _XTAL_FREQ 1000000

#pragma config FOSC = HS        // Oscillator Selection bits (High Speed oscillator)
#pragma config WDTE = OFF        // Watchdog Timer Enable bit (WDT enabled)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config BOREN = OFF       // Brown-out Reset Enable bit (BOR enabled)
#pragma config LVP = OFF        // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3/PGM pin has PGM function; low-voltage programming enabled)
#pragma config CPD = OFF        // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)
#pragma config WRT = OFF        // Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control)
#pragma config CP = OFF         // Flash Program Memory Code Protection bit (Code protection off)
int tocdo=0,temp;
void PWM_Initalize()
{
    TRISC2 = 0;
    TRISD0 = 1;
    TRISD1 = 1;
    TRISD2 = 1;
    TRISD3 = 1;
    TRISD4 = 0;
    TRISD5 = 0;
    TRISD6 = 0;
    TRISD7 = 0;
    //Bat che do PWM
    CCP1CONbits.CCP1M3 = 1;
    CCP1CONbits.CCP1M2 = 1;
    CCP1CONbits.CCP1M1 = 1;
    CCP1CONbits.CCP1M0= 1;
    //Bat timer 2
    T2CONbits.TMR2ON = 1;
    //Chon prescale la 16
    T2CONbits.T2CKPS0 = 1;
    T2CONbits.T2CKPS1 = 1;
    
}
unsigned int PWM_period = 0;
unsigned int PWM_duty = 0;

void PWM_DutyCycle(unsigned int duty)
{
    /*
     * 16f877A hien thi xung PWM duty cycle voi do phan giai la 10 bit = 1023 muc
     * Trong do 8bit cao la cua thanh ghi CCPR1L va 2 bit thap luu vao bit 4, bit 5 cua thanh ghi CCP1CON
     * Chu ki PWM la PR2 --- Khi TIMER2 tang bang gia tri PR2 thi 1 Chu ki reset
    */
    PR2 = (_XTAL_FREQ/(500*4*16)) - 1; // Gia su 1 chu ki = 1/500 giay
    // PWM_period la PR2 nhung don vi la giay
    PWM_period = (PR2+1)*4*16/_XTAL_FREQ;
    PWM_duty = 4*(PR2+1)*duty/100;
    //Luu PWM duty cycle vao cac thanh ghi
    CCP1CONbits.CCP1X = PWM_duty & 2;
    CCP1CONbits.CCP1Y = PWM_duty & 1;
    CCPR1L = PWM_duty >> 2; // 8 bit 
}

void ADCinit()
{
    TRISA = 0xFF;
    //TRISB = 0x00;
    TRISB = 0x00;
    // CHON TAN SO LAY MAU TIN HIEU TUONG TU = Fosc/16    
    ADCON0bits.ADCS0 = 0;
    ADCON0bits.ADCS1 = 1;
    ADCON1bits.ADCS2 = 1;
    //CHON PIN ANALOG DAU` VAO` LA AN0
    ADCON0bits.CHS2 = 0;
    ADCON0bits.CHS1 = 0;
    ADCON0bits.CHS0 = 0;
    //KET QUA DAU RA BIEN DIEN = 10/16 bit BEN PHAI
    ADCON1bits.ADFM = 1;
    //Port Configuration Control bits
    ADCON1bits.PCFG0 = 0;
    ADCON1bits.PCFG1 = 0;
    ADCON1bits.PCFG2 = 0;
    ADCON1bits.PCFG3 = 0;
    
    ADCON0bits.GO_DONE = 1;
    ADCON0bits.ADON = 1;
}
void hienthiled()
{
    int nhietdo = 0; 
            int bcdnhietdo= 0;
            int shift = 0;
            ADCON0bits.GO_DONE = 1;
            nhietdo = ADRESL*500.0/1023;
            temp=nhietdo;
            while (nhietdo > 0) {
            bcdnhietdo |= (nhietdo % 10) << (shift++ << 2);
             nhietdo /= 10;
            }
            PORTB = bcdnhietdo;
}
hienthitocdo(int a)
{
    switch(a)
    {
        case 0: 
            PORTDbits.RD4 = 0;
            PORTDbits.RD5 = 0;
            PORTDbits.RD6 = 0;
            PORTDbits.RD7 = 0;
            break;
        case 1: 
            PORTDbits.RD4 = 1;
            PORTDbits.RD5 = 0;
            PORTDbits.RD6 = 0;
            PORTDbits.RD7 = 0;
            break;
        case 2: 
            PORTDbits.RD4 = 0;
            PORTDbits.RD5 = 1;
            PORTDbits.RD6 = 0;
            PORTDbits.RD7 = 0;
            break;
        case 3: 
            PORTDbits.RD4 = 1;
            PORTDbits.RD5 = 1;
            PORTDbits.RD6 = 0;
            PORTDbits.RD7 = 0;
        break;
    }
}
void UARTinit(const long int baudrate)
{
	unsigned int x;
	x = (_XTAL_FREQ - baudrate*64)/(baudrate*64);
	if(x>255)
	{
		x = (_XTAL_FREQ - baudrate*16)/(baudrate*16);
		BRGH = 1;
	}
	if(x<256)
	{
        //khai bao thanh ghi TXSTA
        SPBRG = x;
        SYNC = 0;
        SPEN = 1;
        // khai bao thanh ghi RCSTA
        CREN = 1;
        TXEN = 1;
        //khai bao cong vao
        TRISC7 = 1;
        TRISC6 = 1;     
    }
}
char UARTtransmit()
{
  return TRMT;
}

// kiem tra nhan vao
char UARTreceive()
{
   return RCIF;
}
void UART_Write(char data)
{
  while(!TRMT);
  TXREG = data;
}
// in mot chuoi
void UART_Write_Text(char a[])
{
  int i;
  for(i=0;a[i]!='\0';i++)
	  UART_Write(a[i]);
}
void UART_Write_number(int a)
{
    switch(a)
    {
        case 0: UART_Write('0'); break;
        case 1: UART_Write('1'); break;
        case 2: UART_Write('2'); break;
        case 3: UART_Write('3'); break;
        case 4: UART_Write('4'); break;
        case 5: UART_Write('5'); break;
        case 6: UART_Write('6'); break;
        case 7: UART_Write('7'); break;
        case 8: UART_Write('8'); break;
        case 9: UART_Write('9'); break;
    }
}
void main()
{   
    UARTinit(1200);
    ADCinit();
    PWM_Initalize();
    while(1)
    {
        int temp1 = temp;
        if(ADCON0bits.GO_DONE == 0)
        {
            hienthiled();           
        }
        if(temp<25)
        {
            tocdo = 0;
            hienthitocdo(0);
            PWM_DutyCycle(0);
        }
        else if(25<temp && temp <30)
        {
            tocdo = 1;
            hienthitocdo(1);
            PWM_DutyCycle(25);
        }
        else if(30<temp && temp <35)
        {
            tocdo = 2;
            hienthitocdo(2);
            PWM_DutyCycle(50);
        }
        else
        {
            tocdo = 3;
            hienthitocdo(3);
            PWM_DutyCycle(100);
        }
        if(temp1 != temp)
        {
            UART_Write_Text("Nhiet do: ");
            int chuc=temp/10;
            int donvi=temp%10;
            UART_Write_number(chuc);
            UART_Write_number(donvi);
            UART_Write(0x0D);
            UART_Write_Text("Toc do: ");
            UART_Write_number(tocdo);
            UART_Write(0x0D);
        }
    }
}