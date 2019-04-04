#include <errno.h>
#include <time.h>


#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_system.h"

#include "Display.h"
#include "HttpServer.h"
#include "Max31855.h"
#include "mdns.h"
#include "PowerControl.h"
#include "profile.h"
#include "wifi.h"
#include "ReflowLog.h"
#include "SSD1306.h"
#include "Model.h"
#include "Control.h"

#define SPI_CLK_PIN 18
#define SPI_MISO_PIN 19
#define SPI_CS_PIN 5
#define PWR_PWM_PIN 17

extern "C" {

static const profile_point_t DEFAULT_PROFILE[] = {
    {150, 60, 0}, // Preheat
    {180, 120, 1}, // Soak
    {245, 15, 0}, // Reflow
    {150, 25, 1}, // Cooldown
    {0, 0, 0}, // Terminator
};

void stimulus_loop(Display &display) {
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
        display.update();
        vTaskDelay(500 / portTICK_RATE_MS);

    }
}

void sim_loop(Display &display) {
    Model model;
    Control control(&model, &model);
    ReflowLog log(500);
    control.setProfile(DEFAULT_PROFILE);
    
    while(1) {
        control.run();
        display.setTempReading(model.read());
        display.setTempTarget(control.targetTemp());
        display.setOutput(model.getOutput());
        display.setStatus("Simulating");
        display.setProfile(
            DEFAULT_PROFILE,
            control.profileStage(),
            control.profileElapsedTime());
        display.update();
        log.log(control.integrationValue(), model.read(), control.targetTemp(), model.getOutput());
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void main_loop(Display &display, Max31855 &sensor) {
    
    PowerControl output(PWR_PWM_PIN);
    Control control(&sensor, &output);
    ReflowLog log(500);
    HttpServer http(&control);
    http.init();
    control.setProfile(DEFAULT_PROFILE);

    while(1) {
        control.run();
        display.setTempReading(sensor.lastReading());
        display.setTempTarget(control.targetTemp());
        display.setOutput(control.output());
        switch(control.currentState()) {
        case Control::Idle:
            display.setStatus("Idle");
            break;
        case Control::RunningProfile:
            display.setStatus("Running");
            break;
        case Control::RunningTempHold:
            display.setStatus("TempHold");
            break;
        default:
            display.setStatus("unknown");
            break;
        }
        display.setProfile(
            DEFAULT_PROFILE,
            control.profileStage(),
            control.profileElapsedTime());
        display.update();
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

void app_main() {
    esp_err_t ret;
    const esp_app_desc_t *app_desc;
    app_desc = esp_ota_get_app_description();

    esp_log_level_set("spi_master", ESP_LOG_VERBOSE);
    esp_log_level_set("*", ESP_LOG_VERBOSE);

    SSD1306 ssd1306(GPIO_NUM_4, GPIO_NUM_15, GPIO_NUM_16);
    ssd1306.init();
    ssd1306.flipScreenVertically();
    Display display(&ssd1306);
    Max31855 sensor(HSPI_HOST, SPI_CLK_PIN, SPI_MISO_PIN, SPI_CS_PIN);
    sensor.connect();
    WIFI_Initialize();

    //stimulus_loop(display);
    //sim_loop(display);
    main_loop(display, sensor);
}

}
