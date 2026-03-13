#pragma once

#include "u_sys/config.h"

namespace ufo
{
    namespace utl
    {
        esp_err_t gpio_config(gpio_num_t pin, gpio_mode_t mode){

            // no need to reset
            gpio_config_t conf = {
                .pin_bit_mask = 1ULL << pin,
                .mode = mode,           //GPIO_MODE_INPUT | GPIO_MODE_OUTPUT
                .pull_up_en = GPIO_PULLUP_ENABLE,
                .pull_down_en = GPIO_PULLDOWN_DISABLE,
                .intr_type = GPIO_INTR_DISABLE
            };
            esp_err_t e = gpio_config(&conf);
            return e;
        }

        esp_err_t gpio_config(gpio_num_t pin, gpio_mode_t mode, bool pup, bool pdw){
            // no need to reset
            gpio_config_t conf = {
                .pin_bit_mask = 1ULL << pin,
                .mode = mode, // GPIO_MODE_INPUT | GPIO_MODE_OUTPUT
                .pull_up_en = pup ? GPIO_PULLUP_ENABLE : GPIO_PULLUP_DISABLE,
                .pull_down_en = pdw ? GPIO_PULLDOWN_ENABLE : GPIO_PULLDOWN_DISABLE,
                .intr_type = GPIO_INTR_DISABLE};
            esp_err_t e = gpio_config(&conf);
            return e;
        }

    } // namespace utl
    

} // namespace ufo
