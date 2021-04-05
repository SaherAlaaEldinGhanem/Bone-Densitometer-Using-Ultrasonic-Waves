#define _XTAL_FREQ 16000000

#define RS RD2
#define EN RD3
#define D4 RD4
#define D5 RD5
#define D6 RD6
#define D7 RD7

#include <xc.h>

#pragma config FOSC = HS        // Oscillator Selection bits (HS oscillator)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled)
#pragma config PWRTE = ON       // Power-up Timer Enable bit (PWRT enabled)
#pragma config BOREN = ON       // Brown-out Reset Enable bit (BOR enabled)
#pragma config LVP = OFF        // Low-Voltage (Single-Supply) In-Circuit Serial Programming Enable bit (RB3 is digital I/O, HV on MCLR must be used for programming)
#pragma config CPD = OFF        // Data EEPROM Memory Code Protection bit (Data EEPROM code protection off)
#pragma config WRT = OFF        // Flash Program Memory Write Enable bits (Write protection off; all program memory may be written to by EECON control)
#pragma config CP = OFF         // Flash Program Memory Code Protection bit (Code protection off)

void ADC_Initialize()
{
  ADCON0 = 0b01000001; //ADC ON and Fosc/16 is selected
  ADCON1 = 0b11000000; // Internal reference voltage is selected
}

unsigned int ADC_Read(unsigned char channel)
{
  ADCON0 &= 0x11000101; //Clearing the Channel Selection Bits
  ADCON0 |= channel<<3; //Setting the required Bits
  __delay_ms(2); //Acquisition time to charge hold capacitor
  GO_nDONE = 1; //Initializes A/D Conversion
  while(GO_nDONE); //Wait for A/D Conversion to complete
  return ((ADRESH<<8)+ADRESL); //Returns Result
}

//LCD Functions Developed by Circuit Digest.
void Lcd_SetBit(char data_bit) //Based on the Hex value Set the Bits of the Data Lines
{
    if(data_bit& 1) 
        D4 = 1;
    else
        D4 = 0;

    if(data_bit& 2)
        D5 = 1;
    else
        D5 = 0;

    if(data_bit& 4)
        D6 = 1;
    else
        D6 = 0;

    if(data_bit& 8) 
        D7 = 1;
    else
        D7 = 0;
}

void Lcd_Cmd(char a)
{
    RS = 0;           
    Lcd_SetBit(a); //Incoming Hex value
    EN  = 1;         
        __delay_ms(4);
        EN  = 0;         
}

Lcd_Clear()
{
    Lcd_Cmd(0); //Clear the LCD
    Lcd_Cmd(1); //Move the curser to first position
}

void Lcd_Set_Cursor(char a, char b)
{
    char temp,z,y;
    if(a== 1)
    {
      temp = 0x80 + b - 1; //80H is used to move the curser
        z = temp>>4; //Lower 8-bits
        y = temp & 0x0F; //Upper 8-bits
        Lcd_Cmd(z); //Set Row
        Lcd_Cmd(y); //Set Column
    }
    else if(a== 2)
    {
        temp = 0xC0 + b - 1;
        z = temp>>4; //Lower 8-bits
        y = temp & 0x0F; //Upper 8-bits
        Lcd_Cmd(z); //Set Row
        Lcd_Cmd(y); //Set Column
    }
}

void Lcd_Start()
{
  Lcd_SetBit(0x00);
  for(int i=1065244; i<=0; i--)  NOP();  
  Lcd_Cmd(0x03);
    __delay_ms(5);
  Lcd_Cmd(0x03);
    __delay_ms(11);
  Lcd_Cmd(0x03); 
  Lcd_Cmd(0x02); //02H is used for Return home -> Clears the RAM and initializes the LCD
  Lcd_Cmd(0x02); //02H is used for Return home -> Clears the RAM and initializes the LCD
  Lcd_Cmd(0x08); //Select Row 1
  Lcd_Cmd(0x00); //Clear Row 1 Display
  Lcd_Cmd(0x0C); //Select Row 2
  Lcd_Cmd(0x00); //Clear Row 2 Display
  Lcd_Cmd(0x06);
}

void Lcd_Print_Char(char data)  //Send 8-bits through 4-bit mode
{
   char Lower_Nibble,Upper_Nibble;
   Lower_Nibble = data&0x0F;
   Upper_Nibble = data&0xF0;
   RS = 1;             // => RS = 1
   Lcd_SetBit(Upper_Nibble>>4);             //Send upper half by shifting by 4
   EN = 1;
   for(int i=2130483; i<=0; i--)  NOP(); 
   EN = 0;
   Lcd_SetBit(Lower_Nibble); //Send Lower half
   EN = 1;
   for(int i=2130483; i<=0; i--)  NOP();
   EN = 0;
}

void Lcd_Print_String(char *a)
{
    int i;
    for(i=0;a[i]!='\0';i++)
       Lcd_Print_Char(a[i]);  //Split the string using pointers and call the Char function 
}


/*
void Transmit()
{
   for(long int i=0;i<20000;++i)
   {
        PORTBbits.RB6 = 1;
        __delay_ms(36);
        PORTBbits.RB2 = 1;
        __delay_ms(30);
   }
}

*/

int main()
{
    
    // Variables Initialize
    int OldMIn;
    int OldMax;
    float OldValue;
    float NewValue;
    
    int min = 0;
    int max = 0;
    int err = 0;

    ADC_Initialize();


    //Set PORTA as Input
	TRISA = 0xFF;
    // Set PORTB as Output
	TRISB = 0x00;
    // Set PORTC as Input
	TRISC = 0xFF;
    // Set PORTD as Output
    TRISD = 0x00;
    
    
    Lcd_Start();
    Lcd_Clear();
    Lcd_Set_Cursor(1,3);
    Lcd_Print_String("Cal.Min-Max");
    Lcd_Set_Cursor(2,5);
    Lcd_Print_String("Then Start");

    while(1)
    {
        //Turn off sound peizo
        PORTBbits.RB0 = 0;

        //Set Minumum value
        if(PORTCbits.RC0 == 1){
            Lcd_Clear();
            Lcd_Print_String("Set Minumum Value");
             __delay_ms(1000);
            Lcd_Clear();
            Lcd_Print_String("Wait For 20s");
            Lcd_Set_Cursor(2,1);
            Lcd_Print_String("Then press Max");
            //Turn On Transmiter
            PORTBbits.RB2 = 1;
            OldMIn=(ADC_Read(4));
            //Transmit();
            
            /*
            for(long int i=0;i<20000;++i)
            {
                PORTBbits.RB6 = 1;
                __delay_ms(36);
                PORTBbits.RB2 = 1;
                __delay_ms(30);
            }
            
            
            */
            min = 1;
            }
        
        
        //Set Maximum Value
        if(PORTCbits.RC1 == 1){
            Lcd_Clear();
            Lcd_Print_String("Set Maximum Value");
             __delay_ms(1000);
            Lcd_Clear();
            Lcd_Print_String("Wait For 20s");
            Lcd_Set_Cursor(2,1);
            Lcd_Print_String("Then press Start");

        // Turn on Transmiter
            PORTBbits.RB2 = 1;
            OldMax=(ADC_Read(4));
            //Transmit();
            
            /*
            for(long int i=0;i<20000;++i)
            {
                 PORTBbits.RB6 = 1;
                 __delay_ms(36);
                 PORTBbits.RB2 = 1;
                 __delay_ms(30);
            }
            
            */
            max = 1;
            }
        
        //Press start button
        if(PORTCbits.RC2 == 1){
            if(min == 1 && max == 1){
                //Turn on Transmiter
                PORTBbits.RB2 = 1;
                Lcd_Clear();
                Lcd_Print_String("Progress...");
                Lcd_Set_Cursor(2,1);
                Lcd_Print_String("Wait For 20s");
                __delay_ms(1000);
                //Transmit();
                
                /*
                for(long int i=0;i<20000;++i)
                {
                    PORTBbits.RB6 = 1;
                    __delay_ms(36);
                    PORTBbits.RB2 = 1;
                    __delay_ms(30);
                }
                
                */
                OldValue=(ADC_Read(4));
                //Mathematical formula to Set Min and Max -4 and 4
                //NewValue = (((OldValue - OldMin) * (NewMax - NewMin))/(OldMax-OldMin))+NewMin
                NewValue = (((OldValue-OldMIn)*(-4-(4)))/(OldMax-OldMIn))+(4);
                //Decide a patient status
                if(NewValue > 1 && NewValue <= 4 ){
                    Lcd_Clear();
                    Lcd_Set_Cursor(1,3);
                    Lcd_Print_String("The Result Is");
                    Lcd_Set_Cursor(2,5);
                    Lcd_Print_String("High");
                    err=0;
                    } else if(NewValue > -1 && NewValue < 1 ){
                        Lcd_Clear();
                        Lcd_Set_Cursor(1,3);
                        Lcd_Print_String("The Result Is");
                        Lcd_Set_Cursor(2,5);
                        Lcd_Print_String("Normal");
                        err=0;
                        }else if(NewValue > -2 && NewValue < -1 ){
                            Lcd_Clear();
                            Lcd_Set_Cursor(1,3);
                            Lcd_Print_String("The Result Is");
                            Lcd_Set_Cursor(2,5);
                            Lcd_Print_String("Low");
                            err=0;
                            }else if(NewValue >= -4 && NewValue < -2 ){
                                Lcd_Clear();
                                Lcd_Set_Cursor(1,3);
                                Lcd_Print_String("The Result Is");
                                Lcd_Set_Cursor(2,5);
                                Lcd_Print_String("V.Low");
                                err=0;
                                }else{
                                    Lcd_Clear();
                                    Lcd_Set_Cursor(1,3);
                                    Lcd_Print_String("Error");
                                    Lcd_Set_Cursor(2,3);
                                    Lcd_Print_String("Recalibrate");
                                    err=1;
                                    }
                // Turn on The Piezo Sound
                PORTBbits.RB0 = 1;
                
                
                
                //Delay to unbounce
                __delay_ms(1000);
                
            }}
     
    }
    return 0;
}
