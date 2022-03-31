#define KEY_BASE              0xFF200050    // 4 push-button switches (operation of stopwatch)
#define SW_BASE               0xFF200040    // slide switches (bit 0 toggles lap/current time)
#define MPCORE_PRIV_TIMER     0xFFFEC600    // A9 private timer, 200 MHz clock

#define uint32_t unsigned int

// Pointer to the GPIO base
volatile GPIO* const gpio_ptr = (GPIO*) GPIO_BASE;

// Pointer to the private timer
volatile a9_timer* const timer = (a9_timer*) MPCORE_PRIV_TIMER;

// Wait for 0.01 s.
void wait() {
    while (timer->status == 0)
        ;   // Wait for timer to expire
    timer->status = 1;  // reset timeout flag.
}
