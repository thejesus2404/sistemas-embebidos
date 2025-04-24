#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#define SEGMENTS_AMOUNT 7

const int display_pins[SEGMENTS_AMOUNT] = {
    32, // a
    33, // b
    25, // c
    26, // d
    27, // e
    23, // f
    22, // g
};
const int LS_digit_control_pin = 13;
const int MS_digit_control_pin = 12;
const int add_pin = 16;
const int subtract_pin = 17;

volatile int current_number = 0;

const uint8_t digits[10] = {
    0b1111110, // 0
    0b0110000, // 1
    0b1101101, // 2
    0b1111001, // 3
    0b0110011, // 4
    0b1011011, // 5
    0b1011111, // 6
    0b1110000, // 7
    0b1111111, // 8
    0b1111011  // 9
};

void set_pins()
{
  for (int i = 0; i < SEGMENTS_AMOUNT; i++)
  {
    gpio_reset_pin(display_pins[i]);
    gpio_set_direction(display_pins[i], GPIO_MODE_OUTPUT);
    gpio_set_level(display_pins[i], 1);
  }
  gpio_reset_pin(LS_digit_control_pin);
  gpio_set_direction(LS_digit_control_pin, GPIO_MODE_OUTPUT);
  gpio_set_level(LS_digit_control_pin, 0);
  gpio_reset_pin(MS_digit_control_pin);
  gpio_set_direction(MS_digit_control_pin, GPIO_MODE_OUTPUT);
  gpio_set_level(MS_digit_control_pin, 0);

  gpio_reset_pin(add_pin);
  gpio_set_direction(add_pin, GPIO_MODE_INPUT);
  gpio_set_pull_mode(add_pin, GPIO_PULLDOWN_ONLY);
  gpio_set_intr_type(add_pin, GPIO_INTR_POSEDGE); // Detect rising edge

  gpio_reset_pin(subtract_pin);
  gpio_set_direction(subtract_pin, GPIO_MODE_INPUT);
  gpio_set_pull_mode(subtract_pin, GPIO_PULLDOWN_ONLY);
  gpio_set_intr_type(subtract_pin, GPIO_INTR_POSEDGE); // Detect rising edge
}

void display_digit(int num, gpio_num_t display_pin)
{
  uint8_t pattern = digits[num]; // Get pattern

  for (int i = 0; i < SEGMENTS_AMOUNT; i++)
  {
    int state = (pattern >> (6 - i)) & 1;
    gpio_set_level(display_pins[i], state);
  }

  gpio_set_level(display_pin, 1); // Enable display
  vTaskDelay(pdMS_TO_TICKS(15));  // Small delay (10ms)
  gpio_set_level(display_pin, 0); // Disable display
}

void clear_display()
{
  for (int i = 0; i < SEGMENTS_AMOUNT; i++)
  {
    gpio_set_level(display_pins[i], 0);
  }
}

void IRAM_ATTR add_button_isr_handler(void *arg)
{
  if (current_number < 99)
  {
    current_number++; // Increase number (max 9)
  }
  else
  {
    current_number = 0;
  }
}

void IRAM_ATTR sub_button_isr_handler(void *arg)
{
  if (current_number > 0)
  {
    current_number--; // Decrease number (min 0)
  }
  else
  {
    current_number = 99;
  }
}

void setup_interrupts()
{
  gpio_install_isr_service(0); // Initialize ISR service

  gpio_isr_handler_add(add_pin, add_button_isr_handler, NULL);
  gpio_isr_handler_add(subtract_pin, sub_button_isr_handler, NULL);
}

void app_main()
{
  set_pins();
  setup_interrupts();

  while (1)
  {
    int ms_digit = current_number / 10;
    int ls_digit = current_number % 10;

    display_digit(ms_digit, MS_digit_control_pin);
    display_digit(ls_digit, LS_digit_control_pin);
  }
}