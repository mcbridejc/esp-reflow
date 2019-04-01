#include "button.h"
#include "driver/gpio.h"
    


static int assigned_event_bits = 0;
static EventGroupHandle_t event_group;

static void _isr_handler(void *arg)
{
    button_t *button = (button_t*)arg;
    BaseType_t result, higher_prio_task_woken;
    result = xEventGroupSetBitsFromISR(event_group, (1<<button->event_bit), &higher_prio_task_woken);
    /* Was the message posted successfully? */
    if( result != pdFAIL && higher_prio_task_woken)
    {
        /* If xHigherPriorityTaskWoken is now set to pdTRUE then a context
        switch should be requested. */
        portYIELD_FROM_ISR();
    }
}

int button_init(button_t *button, int gpio)
{
    if (assigned_event_bits == 0) {
        // Module level init happens on first button init
        event_group = xEventGroupCreate();
        ESP_ERROR_CHECK(gpio_install_isr_service(ESP_INTR_FLAG_LEVEL1));
    }
    button->gpio = gpio;
    button->event_bit = assigned_event_bits++;

    gpio_config_t cfg;
    cfg.pin_bit_mask = (1ULL<<gpio);
    cfg.pull_up_en = true;
    cfg.mode = GPIO_MODE_INPUT;
    cfg.intr_type = GPIO_INTR_NEGEDGE;
    ESP_ERROR_CHECK(gpio_config(&cfg));
    ESP_ERROR_CHECK(gpio_isr_handler_add(gpio, _isr_handler, button));
    return button->event_bit;
}

EventGroupHandle_t button_event_group()
{
    return event_group;
}


