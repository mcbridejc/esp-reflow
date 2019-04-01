#include <errno.h>
#include <time.h>


#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_system.h"

#include "display.h"
#include "mdns.h"
#include "profile.h"
#include "wifi.h"
#include "SSD1306.h"


extern "C" {

static const profile_point_t DEFAULT_PROFILE[] = {
	{150, 60, 0}, // Preheat
	{180, 120, 1}, // Soak
	{245, 15, 0}, // Reflow
	{150, 25, 1}, // Cooldown
	{0, 0, 0}, // Terminator
};

void app_main()
{
    esp_err_t ret;
    const esp_app_desc_t *app_desc;
    app_desc = esp_ota_get_app_description();

	SSD1306 ssd1306(GPIO_NUM_4, GPIO_NUM_15, GPIO_NUM_16);
	ssd1306.init();
	ssd1306.flipScreenVertically();
	Display display(&ssd1306);
    WIFI_Initialize();

	uint16_t tempReading = 0;
	uint16_t tempTarget = 100;
	uint8_t output = 0;
	uint8_t outputAsc = 1;
    while(1) {
		tempReading = (tempReading + 1) % 500;
		tempTarget = (tempTarget + 1) % 500;

		if(output >= 100) {
			outputAsc = 0;
		}
		else if(output <= 0) {
			outputAsc = 1;
		}
		if(outputAsc) { 
			output += 2;
		} else {
			output -= 2;
		}
		display.setTempReading(tempReading);
		display.setTempTarget(tempTarget);
		display.setOutput(output);
		display.setStatus("Meaningless");
		display.setProfile(DEFAULT_PROFILE, 1, 60);
		display.setTimer((uint32_t)(esp_timer_get_time() / 1000000));
		display.update();
        vTaskDelay(500 / portTICK_RATE_MS);

    }
}

}
