//LED for simulator check
volatile int *LED_TEST_PTR = (int *)0xFF200000;

//GPIO Address
# define GPIO_A_BASE 0xFF200060

//ADC Address
# define ADC_BASE 0xFF204000

//Switch Address
volatile int *SW_BASE_ptr = (int *)0xFF200040;





//GPIO Structure
typedef struct _gpio
{
    int data;
    int control;
} gpio;

volatile gpio* const port = (gpio*) GPIO_A_BASE;

//ADC Structure
typedef struct _t_ADC
{
    int ch0;
    int ch1;
} t_ADC;

volatile t_ADC* const adc = (t_ADC*) ADC_BASE;






//Reads the first switch
int ReadSwitches(void)
{
    return *(SW_BASE_ptr) &= 0x1;
}


//Variable initialization
int adc_data = 0;
int adc_data_temp = 0;

//For simulator, 16
//int bit_mask_15 = 0b10000000000000000;

//For real hardware
int bit_mask_15 = 0b1000000000000000;

int number_of_LEDs = 0;
int bit_LED = 0;



//We need pins from 0 to 9 for the LEDs
//Sout = {0,1,2,3,4,5,6,7,8,9} => 0b1111111111 (1023)
int bit_mask_Output = 0b1111111111;

//(2^32)-1-511
int bit_mask_Input = 0b11111111111111111111111000000000;

int LED_Bit[] = {0b0, 0b1, 0b11, 0b111, 0b1111, 0b11111, 0b111111, 0b1111111, 0b11111111, 0b111111111, 0b1111111111};

//Maximum range of the returning data from the ADC (12 bits)
//4095
int max_adc = 0b111111111111;
//max_adc / 10
int LED_Interval = 409;





int main(void) {

    //Setting up the ports for output
    port -> control = bit_mask_Output;

    while (1){    


        //ADC mode 1 (Channel 0, Update)
        if (ReadSwitches() == 1){
            
            //Channel 0 for update
            adc->ch0 = 1;
            adc->ch1 = 0;

            //Read converted data
            adc_data = adc -> ch0;
            adc_data_temp = adc_data;

            if(adc_data_temp &= bit_mask_15){


                //Remove the status bit
                adc_data = adc_data - bit_mask_15;

            }

        }

        //ADC mode 2 (Channel 1, Auto-update)
        else{
      
            //Channel 1 for auto-update
            adc->ch0 = 0;
            adc->ch1 = 1;

            //Read converted data
            adc_data = adc -> ch1;
            adc_data_temp = adc_data;

            if(adc_data_temp &= bit_mask_15){

                //Remove the status bit
                adc_data = adc_data - bit_mask_15;

            }

        }

        number_of_LEDs = adc_data / LED_Interval;
        bit_LED = number_of_LEDs - (number_of_LEDs % 1);

        if(adc_data > 3890){
            bit_LED = 10;
        }

        port->data = LED_Bit[bit_LED];
        //*(LED_TEST_PTR) = LED_Bit[bit_LED];


    }




}




