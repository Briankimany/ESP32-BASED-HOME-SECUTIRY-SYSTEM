// XClk.cpp – Fixed for ESP‑IDF v4+ / Arduino Core 2.x
#include "XClk.h"
#include "driver/ledc.h"

bool ClockEnable(int pin, int Hz)
{
    periph_module_enable(PERIPH_LEDC_MODULE);

    ledc_timer_config_t timer_conf = {
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_1_BIT,   // 1‑bit resolution (max 40 MHz output)
        .timer_num = LEDC_TIMER_0,
        .freq_hz = Hz,                         // e.g., 10 MHz or 20 MHz
        .clk_cfg = LEDC_AUTO_CLK               // let the driver choose the best clock
    };
    esp_err_t err = ledc_timer_config(&timer_conf);
    if (err != ESP_OK) {
        return false;
    }

    ledc_channel_config_t ch_conf = {
        .gpio_num = pin,
        .speed_mode = LEDC_HIGH_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_0,
        .duty = 1,                             // 50% duty for 1‑bit resolution
        .hpoint = 0,
        .flags = { .output_invert = 0 }
    };
    err = ledc_channel_config(&ch_conf);
    if (err != ESP_OK) {
        return false;
    }
    return true;
}

void ClockDisable()
{
    periph_module_disable(PERIPH_LEDC_MODULE);
}