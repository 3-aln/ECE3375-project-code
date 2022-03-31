
//LED for mode check up
//LED0: Start button is pressed
//LED1: Stay warm mode is pressed
//LED2: Stop is pressed
//LED3: Lights up when the Celsius mode is on. LED is off when Fahrenheit mode is on.
//LED4: Safety switch: Turns on when the current temperature is over 30 degress of celsius
volatile int *LED_PTR = (int *)0xFF200000;



//Right 4 Seven-segments display
volatile int *hex_ptr0 = (int *)0xFF200020;
//Left 2 Seven-segments display
volatile int *hex_ptr1 = (int *)0xFF200030;



//Switch address
//Switch 0 = Temperature set up mode or Current temperature mode (Up = set up, Down = display current temp)
//Switch 1 = Celsius or Fahrenheit mode (Up = Celsius, Down = Fahrenheit)
volatile int *SW_BASE_ptr = (int *)0xFF200040;




//Button address
//Button 0: Start
//Button 1: Stop

volatile int *KEY_BASE_ptr = (int *)0xFF200050;

//GPIO Address
# define GPIO_A_BASE 0xFF200060

//ADC Address
# define ADC_BASE 0xFF204000

//Operating state = 1 when the start button was pressed
//Operating state = 0 when the stop button was pressed
int Operating_state = 0;






//GPIO Structure
typedef struct _gpio
{
    int data;
    int control;
} gpio;

volatile gpio* const port = (gpio*) GPIO_A_BASE;

//ADC Structure
typedef struct ADC
{
    int ch0;
    int ch1;
} ADC;

volatile ADC* const adc = (ADC*) ADC_BASE;





//Seven segment array
//Numbers
char num_code[] = {0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F};
//0: degree symbol, 1: C, 2: F
//(0)Degree symbol: (0,1,5,6): 0b0011 0011 = 0x33
//(1)Celsius: (0,3,4,5): 0b0011 1001 = 0x39
//(2)Fahrenheit: (0,4,5,6): 0b0111 0001 = 0x71
char temp_mode_code[] = {0x33, 0x39, 0x71};

int temp_Alphabet = 0;
int temp_degree = 0x33;

//Seven-segments from the third from the right end.
int T0 = 0x3F;
int T1 = 0x3F;
int T2 = 0x3F;
int T3 = 0x3F;











//Function to display the right four sides
void DisplayHexR4(int value)
{    
    *(hex_ptr0) = value;
}

//Function to display the left two sides
void DisplayHexL2(int value)
{
    *(hex_ptr1) = value;
}

int ReadSwitches(void)
{
    return *(SW_BASE_ptr) &= 0xF;
}

int ReadButtons(void)
{
    return *(KEY_BASE_ptr) &= 0xF;
}


//Separate the 4 digit decimals into 4
//For example, 1234 to 12 and 34.
//This one extracts 12 (First two digits)
//Ready to use for the DisplayHex Functions!

//TAKES IN CELSIUS AS A DEFAULT!
int Thousand_Hundred_Separator(int value){

    int val = value;
    int Mode_Button = ReadSwitches() & 0b10;
    //Celsius
    if(Mode_Button == 0b10){
        val = value;
    }
    //Fahrenheit
    else{
        val = (value*(9.0/5))+32;
    }

    int Thousand = (val / 1000) % 10;
    int Hundred = (val / 100) % 10;
    int result = (num_code[Hundred])+(num_code[Thousand]*256);

    return result;

}

//Combined into the mode_Tenth_One but here to show the function as a separator
int Tenth_One_Separator(int value){

    int Tenth = (value / 10) % 10;
    int One = value % 10;
    int result = (num_code[One] * 65536)+(num_code[Tenth] * 16777216);

    return result;

}

//TAKES IN CELSIUS AS A DEFAULT!
int mode_Tenth_One(int value){

    int val = value;
    int Mode_Button = ReadSwitches() & 0b10;
    int Mode_Int = 0;

    //Celsius
    if(Mode_Button == 0b10){
        val = value;
        Mode_Int = (0x39)+(0x63*256);
    }
    //Fahrenheit
    else{
        val = (value*(9.0/5))+32;
        Mode_Int = (0x71)+(0x63*256);
    }
    int Tenth = (val / 10) % 10;
    int One = val % 10;
    int result = (num_code[One] * 65536)+(num_code[Tenth] * 16777216)+Mode_Int;

    return result;

}




//Main code
int main(void){

    int Test = 123;



    //Repeating while loop
    while(1){

        DisplayHexR4(mode_Tenth_One(Test));
        DisplayHexL2(Thousand_Hundred_Separator(Test));








    }





}








