#define KEY_BASE              0xFF200050    
#define SW_BASE               0xFF200040    // slide switches
#define GPIO_BASE             0xFF200060           // GPIO
#define MPCORE_PRIV_TIMER     0xFFFEC600    // A9 private timer, 200 MHz clock

#define uint32_t unsigned int

// Structure for the GPIO
typedef struct _GPIO {
    unsigned int data;
    unsigned int direction;
    unsigned int mask;
    unsigned int edge;
} GPIO;

typedef struct _a9_timer {
    int load;       
    int count;
    int control;    
    int status;     
} a9_timer;

// Pointer to the GPIO base
volatile GPIO* const gpio_ptr = (GPIO*) GPIO_BASE;

// Pointer to the private timer
volatile a9_timer* const timer = (a9_timer*) MPCORE_PRIV_TIMER;

void main() {
    // Set up GPIO port 0 as input for the MAX6576 temperature sensor
    int gpio_direction_mask = 0xFFFFFFFE;               // set GPIO port 0 to 0 (input)
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
    while(1) {
        wait();
        
        int current_temp_sensor_state = gpio_ptr->data;     // 0 or 1
        int prev_temp_sensor_state = 0;

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