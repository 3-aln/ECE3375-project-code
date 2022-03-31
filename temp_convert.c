#define KEY_BASE              0xFF200050    
#define SW_BASE               0xFF200040    // slide switches
#define GPIO_BASE      0xFF200060           // GPIO
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

}

// Wait for 0.01 s.
void wait() {
    while (timer->status == 0)
        ;   // Wait for timer to expire
    timer->status = 1;  // reset timeout flag.
}
