#include <stdio.h>
#include <string.h>
#include "hub75.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "monitor_anime.h"
#include "esp_log.h"

void app_main(void)
{
    HUB75_CLEAR();	
    vTaskDelay(10/portTICK_PERIOD_MS);

    hub75_init();
    esp_err_t ret = hub75_timer_init();
    if (ret != ESP_OK) {
        ESP_LOGE("SPI", "Failed to initialize hub75 timer: %s", esp_err_to_name(ret));
        return;
    }
    char str[] = "Alisher";
    HUB75_CLEAR();
    Puts_STR_8(str, strlen(str),0,0, 1,1,1);
    HUB75_PAINT_STR_CPY();
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    char str2[] = "Fayz";
    HUB75_CLEAR();
    Puts_STR_8(str2, strlen(str2),0,8, 0,1,0);
    HUB75_PAINT_STR_CPY();
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    while(1){
        HUB75_CLEAR();
        BIG_LCD_WELCOME_anime();
        HUB75_PAINT_STR_CPY();
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}