
//LED for mode check up
//LED0: Turn on when the machine is turned on.
//LED1: Lights up when the Celsius mode is on. LED is off when Fahrenheit mode is on.
//LED2: Safety switch: Turns on when the current temperature is over 45 degress of celsius
volatile int *LED_PTR = (int *)0xFF200000;

int current_LED = 0b0;
int current_LED_temp = 0b0;

int LED_Bit[] = {0b0, 0b1, 0b11, 0b111, 0b1111, 0b11111, 0b111111, 0b1111111, 0b11111111, 0b111111111, 0b1111111111};

int safety_on = 0;

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

int start_on = 0;

//Operating state = 1 when the start button was pressed
//Operating state = 0 when the stop button was pressed
int Operating_state = 0;



//GPIO Address
# define GPIO_A_BASE 0xFF200060

//ADC Address
# define ADC_BASE 0xFF204000



//For simulator, 16
//int bit_mask_15 = 0b10000000000000000;

//For real hardware
int bit_mask_15 = 0b1000000000000000;


//////////////////////////////////////////////////////////
#define MPCORE_PRIV_TIMER     0xFFFEC600    // A9 private timer, 200 MHz clock
#define uint32_t unsigned int

typedef struct _a9_timer {
    int load;       
    int count;
    int control;    
    int status;     
} a9_timer;

// Pointer to the private timer
volatile a9_timer* const timer = (a9_timer*) MPCORE_PRIV_TIMER;
//////////////////////////////////////////////////////////







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

//Variable initialization
int adc_data = 0;
int adc_data_divided = 0;
int adc_int = 0;
int adc_data_temp = 0;


//Maximum range of the returning data from the ADC (12 bits)
//Let 4095 be 81 degrees of celsius and 0 be 0 degree of celsius
//We need to read in resolution of 1 degree of celsius
int max_adc = 0b111111111111;
//Use 50 for the interval for every 1 degree of celsius
//30.0 degrees should give adc_data value of 1500~1549
int ADC_Interval = 50;

int set_up_temp_c = 0;
int current_temp_c = 0;




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
        current_LED_temp = current_LED & 0b10;

        if(current_LED_temp == 0b00){
                current_LED += 0b10;
        }

        val = value;
    }
    //Fahrenheit
    else{
        current_LED_temp = current_LED & 0b10;

        if(current_LED_temp == 0b10){
                current_LED -= 0b10;
        }

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


//@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@
//Use This for the display! Putting in the Celsius value will do things for you.
void DisplaySevenSegment(int value){
    DisplayHexR4(mode_Tenth_One(value));
    DisplayHexL2(Thousand_Hundred_Separator(value));
}


void ADC_READ(void){

    //Channel 0 for update
    adc->ch0 = 1;
    adc->ch1 = 0;

    //Read converted data
    adc_data = adc -> ch0;
    adc_data_temp = adc_data;

    if(adc_data_temp &= bit_mask_15){

        //Remove the status bit and store in adc_data
        adc_data = adc_data - bit_mask_15;

    }

    adc_data_divided = adc_data / ADC_Interval;
    adc_int = adc_data_divided - (adc_data_divided % 1);

    int switch_set = ReadSwitches() & 0b1;

    if(switch_set == 0b1){
        set_up_temp_c = adc_int;
        DisplaySevenSegment(adc_int);
        
    }
    


}


////////////////////////////////////////////////////////////
// Added functions

// Wait for 0.1 us.
void wait(void) {
    while (timer->status == 0)
        ;   // Wait for timer to expire
    timer->status = 1;  // reset timeout flag.
}

int period_to_temperature(int period) {
    // Period in microseconds
    int temperature = period / 10;
    temperature = temperature - 273.15;
    return temperature;
}
////////////////////////////////////////////////////////////





//Main code
int main(void){

    int Test = 50;
    current_temp_c = Test;



    /////////////////////////////////////////////////////////////////
    // Set up GPIO port 0 as input for the MAX6576 temperature sensor
    int gpio_direction_mask = 0xFFFFFBFF;               // set GPIO port 10 to 0 (input)
    int gpio_direction_state;
    gpio_direction_state = port->control;
    gpio_direction_state &= gpio_direction_mask;        // Apply mask to set pins 0 to 9 as output
    port->control = gpio_direction_state;

    // Set up timer to count 0.1 us intervals
    int timer_interval = 20; // 200 MHz * 0.1 us = 20 cycles
    timer->load = timer_interval;
    timer->control = 0b011; // Set timer to count down and repeat

    // Continually count the number of 0.1 us intervals that have passed.
    // Keep track of the state of the MAX6576 sensor pulse train. When a falling edge is detected,
    // convert the recorded period to the sensor temperature.
    int count_period = 0;
    int temperature;

    int demo_switch = 0;
    int adc_demo_data;
    int status_bit_mask = 1 << 15;
    int count_period_demo;

    /////////////////////////////////////////////////////////////////



    //Repeating while loop
    while(1){

        int switch_set = ReadSwitches() & 0b1;

        if(switch_set == 0b0){
            DisplaySevenSegment(Test);
        }
        ADC_READ();

        //Start or stop the machine
        if(ReadButtons()==0b01){
            Operating_state = 1;
            start_on = 1;

        }
        else if(ReadButtons()==0b10){
            Operating_state = 0;
            start_on = 0;
            
        }


        //Compares the current temperature and the set up temperature
        if(current_temp_c >= set_up_temp_c){
            
            Operating_state = 0;
        }
        else{

            if(start_on == 1){

                Operating_state = 1;

            }

        }


        //Operating state LED
        if(Operating_state == 1){

            current_LED_temp = current_LED & 0b1;

            if(current_LED_temp == 0b0){
                //Turns on
                current_LED += 0b1;
            }

        }
        else{

            current_LED_temp = current_LED & 0b1;

            if(current_LED_temp == 0b1){
                //Turns off
                current_LED -= 0b1;
            }
        }

        if(current_temp_c >= 45){
        current_LED_temp = current_LED & 0b100;
        safety_on = 1;

        if(current_LED_temp == 0b100){
            //Turns off
            current_LED -= 0b100;
        }
        else if(current_LED_temp == 0b000 && safety_on == 1){
            //Turns on
            current_LED += 0b100;

            }
        }
        else if(current_temp_c <= 45){

            safety_on = 0;

        }


        *(LED_PTR) = current_LED;


        /////////////////////////////////////////////////////////////////////

        wait();

        int current_temp_sensor_state = port->data;     // 0 or 1
        int prev_temp_sensor_state = 0;

        // Record the state of the demo switch.
        demo_switch = (*SW_BASE_ptr & (1<<9));     // use switch 9
        if (demo_switch) {
            // For “demo mode”
            // Read ADC channel 1
            adc_demo_data = adc->ch1;       // value between 0 and 4095

            if (adc_demo_data & status_bit_mask) {
			// Update the simulated value for counting the number of 0.1 microsecond intervals
			    count_period_demo = count_period_demo / 100;     // value between 0 and 99 (temp in C)
                count_period_demo = count_period_demo + 273.15;  // Temp in K
                count_period_demo = count_period_demo * 10;      // Number of microseconds
                count_period_demo = count_period_demo * 10;      // Number of 0.1 microsecond intervals
		    }

            int period = count_period * 0.1;    // in microseconds
            temperature = period_to_temperature(period);
            current_temp_c = temperature;

        } else {
            // “Real temperature sensor”
            if (port->data == 1) {
                count_period += 1;
            }

            if (prev_temp_sensor_state == 1 && current_temp_sensor_state == 0 ) {
                int period = count_period * 0.1; // in microseconds
                temperature = period_to_temperature(period);
                current_temp_c = temperature;
                count_period = 0;
            }
            
            prev_temp_sensor_state = current_temp_sensor_state;
        }

        /////////////////////////////////////////////////////////////////////



    }





}








