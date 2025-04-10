#include "setup.h"

#include <stdio.h>
#include <string.h>
#include "driver/gpio.h"
#include "driver/i2c.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "driver/rmt_encoder.h"
#include "driver/rmt_tx.h"

#include "u8g2.h"
#include "u8g2_esp32_hal.h"

#define PIN_SDA GPIO_NUM_35
#define PIN_SCL GPIO_NUM_36
#define PIN_RST GPIO_NUM_NC

#define BUTTON_UP GPIO_NUM_1
#define BUTTON_DOWN GPIO_NUM_6
#define BUTTON_LEFT GPIO_NUM_3
#define BUTTON_RIGHT GPIO_NUM_5
#define BUTTON_SELECT GPIO_NUM_18
#define BUTTON_BACK GPIO_NUM_16

#define DEBOUNCE_TIME_MS (500)

#define WLED_GPIO GPIO_NUM_47
#define WLED_RMT_CHANNEL RMT_CHANNEL_0
#define WLED_RESOLUTION_HZ (10000000)
#define WLED_ON_DURATION_MS (100)
#define NUM_LEDS (1)

#define BUTTON_QUEUE_LENGTH 5
#define BUTTON_QUEUE_ITEM_SIZE sizeof(uint8_t)

static const char* TAG = "main";

QueueHandle_t buttonQueue = NULL;
volatile int counter = 0;
volatile int64_t last_interrupt_time = 0;

rmt_channel_handle_t rmt_led_chan = NULL;
rmt_encoder_handle_t rmt_led_encoder = NULL;
bool wled_is_on = false;
int64_t wled_turn_off_time = 0;

u8g2_t u8g2;
uint8_t received_signal;

static void IRAM_ATTR button_isr_handler(void* arg) {
  int64_t now = esp_timer_get_time();
  if ((now - last_interrupt_time) > (DEBOUNCE_TIME_MS * 1000)) {
    last_interrupt_time = now;

    uint8_t press_signal = 1;
    BaseType_t higherPriorityTaskWoken = pdFALSE;

    xQueueSendFromISR(buttonQueue, &press_signal, &higherPriorityTaskWoken);

    if (higherPriorityTaskWoken) {
      portYIELD_FROM_ISR();
    }
  }
}

static void init_rmt_ws2812b(void) {
  ESP_LOGI(TAG, "Initialize RMT TX Channel for WS2812B on GPIO %d", WLED_GPIO);
  rmt_tx_channel_config_t tx_chan_config = {
      .gpio_num = WLED_GPIO,
      .clk_src = RMT_CLK_SRC_DEFAULT,
      .resolution_hz = WLED_RESOLUTION_HZ,
      .mem_block_symbols = 64,
      .trans_queue_depth = 4,
      .intr_priority = 0,
  };
  ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_chan_config, &rmt_led_chan));

  ESP_LOGI(TAG, "Install RMT Bytes Encoder");
  rmt_bytes_encoder_config_t bytes_encoder_config = {
      .bit0 = {.duration0 = 4, .level0 = 1, .duration1 = 8, .level1 = 0},
      .bit1 = {.duration0 = 8, .level0 = 1, .duration1 = 4, .level1 = 0},
      .flags = {.msb_first = 1}};
  ESP_ERROR_CHECK(
      rmt_new_bytes_encoder(&bytes_encoder_config, &rmt_led_encoder));

  ESP_LOGI(TAG, "Activate RMT TX Kanal");
  ESP_ERROR_CHECK(rmt_enable(rmt_led_chan));
}

static void set_wled_color(uint8_t r, uint8_t g, uint8_t b) {
  if (!rmt_led_chan || !rmt_led_encoder) {
    ESP_LOGE(TAG, "RMT Channel or Encoder not initialized!");
    return;
  }

  size_t buffer_size = 3 * NUM_LEDS;
  uint8_t led_data[buffer_size];
  for (int i = 0; i < NUM_LEDS; i++) {
    led_data[i * 3 + 0] = g;
    led_data[i * 3 + 1] = r;
    led_data[i * 3 + 2] = b;
  }
  rmt_transmit_config_t tx_config = {
      .loop_count = 0,
  };
  esp_err_t ret = rmt_transmit(rmt_led_chan, rmt_led_encoder, led_data,
                               sizeof(led_data), &tx_config);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG, "RMT Transmit failed: %s", esp_err_to_name(ret));
  }
  ESP_ERROR_CHECK(rmt_tx_wait_all_done(rmt_led_chan, pdMS_TO_TICKS(100)));
}

void setup(void) {
  buttonQueue = xQueueCreate(BUTTON_QUEUE_LENGTH, BUTTON_QUEUE_ITEM_SIZE);
  if (buttonQueue == NULL) {
    ESP_LOGE(TAG, "Error while Queue creation!");
    return;
  }
  ESP_LOGI(TAG, "Button Queue created.");

  gpio_config_t gpio_button_up;
  gpio_button_up.intr_type = GPIO_INTR_NEGEDGE;
  gpio_button_up.pin_bit_mask = (1ULL << BUTTON_UP);
  gpio_button_up.mode = GPIO_MODE_INPUT;
  gpio_button_up.pull_up_en = GPIO_PULLUP_ENABLE;
  gpio_button_up.pull_down_en = GPIO_PULLDOWN_DISABLE;
  gpio_config(&gpio_button_up);

  esp_err_t isr_service_err = gpio_install_isr_service(ESP_INTR_FLAG_IRAM);
  if (isr_service_err != ESP_OK && isr_service_err != ESP_ERR_INVALID_STATE) {
    ESP_LOGE(TAG, "Error in gpio_install_isr_service: %s",
             esp_err_to_name(isr_service_err));
  }

  esp_err_t add_isr_err =
      gpio_isr_handler_add(BUTTON_UP, button_isr_handler, (void*)BUTTON_UP);
  if (add_isr_err != ESP_OK) {
    ESP_LOGE(TAG, "Error in gpio_isr_handler_add: %s",
             esp_err_to_name(add_isr_err));
  }

  ESP_LOGI(TAG, "Button interrupt configured for GPIO %d", BUTTON_UP);

  u8g2_esp32_hal_t u8g2_esp32_hal = U8G2_ESP32_HAL_DEFAULT;
  u8g2_esp32_hal.bus.i2c.sda = PIN_SDA;
  u8g2_esp32_hal.bus.i2c.scl = PIN_SCL;
  u8g2_esp32_hal.reset = PIN_RST;
  u8g2_esp32_hal_init(u8g2_esp32_hal);

  u8g2_Setup_sh1106_i2c_128x64_noname_f(&u8g2, U8G2_R0, u8g2_esp32_i2c_byte_cb,
                                        u8g2_esp32_gpio_and_delay_cb);
  u8x8_SetI2CAddress(&u8g2.u8x8, 0x3C * 2);

  ESP_LOGI(TAG, "u8g2_InitDisplay");
  u8g2_InitDisplay(&u8g2);

  ESP_LOGI(TAG, "u8g2_SetPowerSave");
  u8g2_SetPowerSave(&u8g2, 0);

  init_rmt_ws2812b();
  set_wled_color(0, 0, 0);

  ESP_LOGI(TAG, "Start of main loop. Waiting for button press...");
}

void loop(void) {
  u8g2_ClearBuffer(&u8g2);
  u8g2_SetFont(&u8g2, u8g2_font_ncenB10_tr);
  u8g2_DrawStr(&u8g2, 5, 20, "Ready!");
  char count_str[12];
  snprintf(count_str, sizeof(count_str), "Counter: %d", counter);
  u8g2_DrawStr(&u8g2, 5, 45, count_str);
  u8g2_SendBuffer(&u8g2);

  if (xQueueReceive(buttonQueue, &received_signal, pdMS_TO_TICKS(10)) ==
      pdTRUE) {
    ESP_LOGI(TAG, "Button event from Queue received!");

    counter++;

    u8g2_ClearBuffer(&u8g2);
    u8g2_DrawStr(&u8g2, 5, 20, "Pressed!");
    u8g2_SetFont(&u8g2, u8g2_font_ncenB10_tr);
    snprintf(count_str, sizeof(count_str), "Counter: %d", counter);
    u8g2_DrawStr(&u8g2, 5, 45, count_str);
    u8g2_SendBuffer(&u8g2);
    ESP_LOGD(TAG, "Display refreshed with counter: %d", counter);

    ESP_LOGD(TAG, "Switch WLED ON");
    set_wled_color(255, 0, 255);
    wled_is_on = true;
    wled_turn_off_time = esp_timer_get_time() + (WLED_ON_DURATION_MS * 1000);
  }

  if (wled_is_on && esp_timer_get_time() >= wled_turn_off_time) {
    ESP_LOGD(TAG, "Switch WLED OFF");
    set_wled_color(0, 0, 0);
    wled_is_on = false;
  }
}