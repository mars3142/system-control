#include <stdio.h>
#include "driver/i2c_master.h"
#include "esp_err.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_sh1106.h"
#include "esp_log.h"
#include "esp_lvgl_port.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lvgl.h"

static const char* TAG = "example";

#define I2C_BUS_PORT 0
#define EXAMPLE_PIN_NUM_SDA 35
#define EXAMPLE_PIN_NUM_SCL 36

// The pixel number in horizontal and vertical
#define EXAMPLE_LCD_H_RES 128
#define EXAMPLE_LCD_V_RES 64

void example_lvgl_demo_ui(lv_disp_t* disp) {
  lv_obj_t* scr = lv_disp_get_scr_act(disp);
  lv_obj_t* label = lv_label_create(scr);
  lv_label_set_long_mode(label, LV_LABEL_LONG_SCROLL_CIRCULAR);
  lv_label_set_text(label, "Hello Espressif, Hello LVGL.");
  /* Size of the screen (if you use rotation 90 or 270, please set
   * disp->driver->ver_res) */
  lv_obj_set_width(label, disp->driver->hor_res);
  lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
}

void app_main(void) {
  ESP_LOGI(TAG, "Initialize I2C bus");
  i2c_master_bus_config_t bus_config = {
      .i2c_port = I2C_BUS_PORT,
      .sda_io_num = EXAMPLE_PIN_NUM_SDA,
      .scl_io_num = EXAMPLE_PIN_NUM_SCL,
      .clk_source = I2C_CLK_SRC_DEFAULT,
      .glitch_ignore_cnt = 7,
      .intr_priority = 0,
      .trans_queue_depth = 0,
      .flags =
          {
              .enable_internal_pullup = true,
              .allow_pd = false,
          },
  };
  i2c_master_bus_handle_t i2c_bus_handle = NULL;
  ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &i2c_bus_handle));

  ESP_LOGI(TAG, "Install panel IO");
  esp_lcd_panel_io_handle_t io_handle = NULL;
  esp_lcd_panel_io_i2c_config_t io_config = ESP_SH1106_DEFAULT_IO_CONFIG;
  ESP_ERROR_CHECK(
      esp_lcd_new_panel_io_i2c(i2c_bus_handle, &io_config, &io_handle));

  esp_lcd_panel_dev_config_t panel_config = {
      .reset_gpio_num = -1,
      .rgb_ele_order = LCD_RGB_ELEMENT_ORDER_RGB,
      .data_endian = LCD_RGB_DATA_ENDIAN_LITTLE,
      .bits_per_pixel = SH1106_PIXELS_PER_BYTE / 8,
      .flags =
          {
              .reset_active_high = false,
          },
      .vendor_config = NULL,
  };

  esp_lcd_panel_handle_t panel_handle = NULL;
  ESP_ERROR_CHECK(
      esp_lcd_new_panel_sh1106(io_handle, &panel_config, &panel_handle));

  ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));

  ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));

  ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));

  ESP_LOGI(TAG, "Initialize LVGL");
  const lvgl_port_cfg_t lvgl_cfg = ESP_LVGL_PORT_INIT_CONFIG();
  lvgl_port_init(&lvgl_cfg);

  const lvgl_port_display_cfg_t disp_cfg = {
      .io_handle = io_handle,
      .panel_handle = panel_handle,
      .buffer_size = EXAMPLE_LCD_H_RES * EXAMPLE_LCD_V_RES,
      .double_buffer = true,
      .hres = EXAMPLE_LCD_H_RES,
      .vres = EXAMPLE_LCD_V_RES,
      .monochrome = true,
      .rotation = {
          .swap_xy = true,
          .mirror_x = false,
          .mirror_y = false,
      }};
  lv_disp_t* disp = lvgl_port_add_disp(&disp_cfg);

  ESP_LOGI(TAG, "Display LVGL Scroll Text");
  // Lock the mutex due to the LVGL APIs are not thread-safe
  if (lvgl_port_lock(0)) {
    example_lvgl_demo_ui(disp);
    // Release the mutex
    lvgl_port_unlock();
  }
}
