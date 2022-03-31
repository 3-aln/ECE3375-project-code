
//LED for mode check up
//LED0: Start button is pressed
//LED1: Stay warm mode is pressed
//LED2: Stop is pressed
//LED3: Lights up when the Celsius mode is on.
volatile int *LED_PTR = (int *)0xFF200000;


//Right 4 Seven-segments display
volatile int *hex_ptr0 = (int *)0xFF200020;
//Left 2 Seven-segments display
volatile int *hex_ptr1 = (int *)0xFF200030;

//Switch address
//Switch 0 = Temperature set up mode or Current temperature mode
//Switch 1 = Celsius or Farenheit mode
volatile int *SW_BASE_ptr = (int *)0xFF200040;




//Button address
//Button 0: Start
//Button 1: Stay Warm!
//Button 2: Stop
volatile int *KEY_BASE_ptr = (int *)0xFF200050;





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






//Function to display the right four sides
void DisplayHexSH(int value)
{    
    *(hex_ptr0) = value;
}

//Function to display the left two sides
void DisplayHexM(int value)
{
    *(hex_ptr1) = value;
}

int ReadSwitches(void)
{
    return *(SW_BASE_ptr) &= 0x1;
}

int ReadButtons(void)
{
    return *(KEY_BASE_ptr) &= 0xF;
}




//Main code
int main(void){

    //Swtich 1 up for the Temperature set up mode










}








