#include "driver/gpio.h"
#include "d_event.h"
#include "doomdef.h"

// Match these GPIOs to your KiCad schematic for SW1-SW8
#define GPIO_SW1_UP      4
#define GPIO_SW2_RIGHT   5
#define GPIO_SW3_DOWN    6
#define GPIO_SW4_LEFT    7
#define GPIO_SW5_FIRE    15
#define GPIO_SW6_USE     16
#define GPIO_SW7_STRAFE  17
#define GPIO_SW8_WEAPON  18

typedef struct {
    int gpio;
    int doom_key;
} btn_map_t;

static const btn_map_t button_map[8] = {
    {GPIO_SW1_UP,     KEY_UPARROW},
    {GPIO_SW2_RIGHT,  KEY_RIGHTARROW},
    {GPIO_SW3_DOWN,   KEY_DOWNARROW},
    {GPIO_SW4_LEFT,   KEY_LEFTARROW},
    {GPIO_SW5_FIRE,   KEY_RCTRL},
    {GPIO_SW6_USE,    ' '},
    {GPIO_SW7_STRAFE, KEY_RALT},
    {GPIO_SW8_WEAPON, '2'} // Cycles/selects weapon
};

void I_InitInput(void) {
    for (int i = 0; i < 8; i++) {
        gpio_reset_pin(button_map[i].gpio);
        gpio_set_direction(button_map[i].gpio, GPIO_MODE_INPUT);
        gpio_set_pull_mode(button_map[i].gpio, GPIO_PULLUP_ONLY);
    }
}

void I_StartTic(void) {
    event_t ev;
    for (int i = 0; i < 8; i++) {
        int pressed = (gpio_get_level(button_map[i].gpio) == 0);
        ev.type = pressed ? ev_keydown : ev_keyup;
        ev.data1 = button_map[i].doom_key;
        D_PostEvent(&ev);
    }
}
