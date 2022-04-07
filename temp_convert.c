#define KEY_BASE              0xFF200050    
#define SW_BASE               0xFF200040    // slide switches
#define GPIO_BASE             0xFF200060           // GPIO
#define ADC_BASE              0xFF204000	 // ADC
#define MPCORE_PRIV_TIMER     0xFFFEC600    // A9 private timer, 200 MHz clock

#define uint32_t unsigned int

// Structure for the GPIO
typedef struct _GPIO {
    unsigned int data;
    unsigned int direction;
    unsigned int mask;
    unsigned int edge;
} GPIO;

// Structure for ADC
typedef struct _ADC {
    unsigned int ch0;
    unsigned int ch1;
    unsigned int ch2;
    unsigned int ch3;
    unsigned int ch4;
    unsigned int ch5;
    unsigned int ch6;
    unsigned int ch7;
} ADC;

typedef struct _a9_timer {
    int load;       
    int count;
    int control;    
    int status;     
} a9_timer;

// Pointer to the ADC
volatile ADC* const adc_ptr = (ADC*) ADC_BASE;

// Pointer to the GPIO base
volatile GPIO* const gpio_ptr = (GPIO*) GPIO_BASE;

// Pointer to the private timer
volatile a9_timer* const timer = (a9_timer*) MPCORE_PRIV_TIMER;

// Pointer to the slide switch bank (switch between “real sensor mode” and “demo mode”)
volatile uint32_t* const switch_ptr = (uint32_t*) SW_BASE;

void main() {
    // Set up GPIO port 0 as input for the MAX6576 temperature sensor
    int gpio_direction_mask = 0xFFFFFBFF;               // set GPIO port 10 to 0 (input)
    int gpio_direction_state;
    gpio_direction_state = gpio_ptr->direction;
    gpio_direction_state &= gpio_direction_mask;        // Apply mask to set pins 0 to 9 as output
    gpio_ptr->direction = gpio_direction_state;

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

    while(1) {
        wait();
        
        int current_temp_sensor_state = gpio_ptr->data;     // 0 or 1
        int prev_temp_sensor_state = 0;

        // Record the state of the demo switch.
        demo_switch = (*switch_ptr & (1<<9));     // use switch 9
        if (demo_switch) {
            // Read ADC channel 1
            adc_demo_data = adc_ptr->ch1;       // value between 0 and 4095

            if (adc_demo_data & status_bit_mask) {
			// Update the simulated value for counting the number of 0.1 microsecond intervals
			    count_period_demo = count_period_demo / 100;     // value between 0 and 99 (temp in C)
                count_period_demo = count_period_demo + 273.15;  // Temp in K
                count_period_demo = count_period_demo * 10;      // Number of microseconds
                count_period_demo = count_period_demo * 10;      // Number of 0.1 microsecond intervals
		    }

            int period = count_period * 0.1;    // in microseconds
            temperature = period_to_temperature(period);

        } else {
            // “Real temperature sensor”
            if (gpio_ptr->data == 1) {
                count_period += 1;
            }

            if (prev_temp_sensor_state == 1 && current_temp_sensor_state == 0 ) {
                int period = count_period * 0.1; // in microseconds
                temperature = period_to_temperature(period);
                count_period = 0;
            }
            
            prev_temp_sensor_state = current_temp_sensor_state;
        }
    }
}

// Wait for 0.1 us.
void wait() {
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
