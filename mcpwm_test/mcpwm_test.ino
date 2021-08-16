/**
 * Explore esp32 mcpwm library for motor speed control

 https://github.com/espressif/esp-idf/blob/a20df743f1c51e6d65b021ed2ffd3081a2feec64/
 examples/peripherals/mcpwm/mcpwm_basic_config/main/mcpwm_basic_config_example.c
 https://github.com/espressif/esp-idf/blob/a20df743f1c51e6d65b021ed2ffd3081a2feec64/
 examples/peripherals/mcpwm/mcpwm_brushed_dc_control/main/mcpwm_brushed_dc_control_example.c
 */
/* brushed dc motor control example
 */
#include <driver/mcpwm.h>

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("Start motor test cycle");
  mcpwm_config();
}

void loop() {
}

// values to use with the gpio pin initialization
#define GPIO_PWM0A_OUT 19   //Set GPIO 19 as PWM0A
#define GPIO_PWM0B_OUT 18   //Set GPIO 18 as PWM0B
#define GPIO_PWM1A_OUT 17   //Set GPIO 17 as PWM1A
#define GPIO_PWM1B_OUT 16   //Set GPIO 16 as PWM1B
#define GPIO_PWM2A_OUT 15   //Set GPIO 15 as PWM2A
#define GPIO_PWM2B_OUT 14   //Set GPIO 14 as PWM2B
// used as gpio_num_t, invalid conversion from int
//#define GPIO_SYNC0_IN   2   //Set GPIO 02 as SYNC0
//#define GPIO_SYNC1_IN   4   //Set GPIO 04 as SYNC1
//#define GPIO_SYNC2_IN   5   //Set GPIO 05 as SYNC2
//#define GPIO_FAULT0_IN 32   //Set GPIO 32 as FAULT0
//#define GPIO_FAULT1_IN 33   //Set GPIO 33 as FAULT1
//#define GPIO_FAULT2_IN 34   //Set GPIO 34 as FAULT2
/* initialiaze (for now) using individual pin init calls
 * until learn what is (minimally) needed for the pump control
 */
void mcpwm_gpio_init()
{
  Serial.println("initializing mcpwm gpio...");
  mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, GPIO_PWM0A_OUT);
  mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0B, GPIO_PWM0B_OUT);
  mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM1A, GPIO_PWM1A_OUT);
  mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM1B, GPIO_PWM1B_OUT);
  mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM2A, GPIO_PWM2A_OUT);
  mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM2B, GPIO_PWM2B_OUT);
//  mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM_CAP_0, GPIO_CAP0_IN);
//  mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM_CAP_1, GPIO_CAP1_IN);
//  mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM_CAP_2, GPIO_CAP2_IN);
//  mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM_SYNC_0, GPIO_SYNC0_IN);
//  mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM_SYNC_1, GPIO_SYNC1_IN);
//  mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM_SYNC_2, GPIO_SYNC2_IN);
//  mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM_FAULT_0, GPIO_FAULT0_IN);
//  mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM_FAULT_1, GPIO_FAULT1_IN);
//  mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM_FAULT_2, GPIO_FAULT2_IN);

//  gpio_pulldown_en(GPIO_SYNC0_IN);   //Enable pull down on SYNC0  signal
//  gpio_pulldown_en(GPIO_SYNC1_IN);   //Enable pull down on SYNC1  signal
//  gpio_pulldown_en(GPIO_SYNC2_IN);   //Enable pull down on SYNC2  signal
//  gpio_pulldown_en(GPIO_FAULT0_IN);  //Enable pull down on FAULT0 signal
//  gpio_pulldown_en(GPIO_FAULT1_IN);  //Enable pull down on FAULT1 signal
//  gpio_pulldown_en(GPIO_FAULT2_IN);  //Enable pull down on FAULT2 signal
}

void mcpwm_config()
{
  //1. mcpwm gpio initialization
  mcpwm_gpio_init();

  //2. initialize mcpwm configuration
  Serial.println("Configuring Initial Parameters or mcpwm...");
    mcpwm_config_t pwm_config;
    pwm_config.frequency = 1000;    //frequency = 1000Hz
    pwm_config.cmpr_a = 60.0;       //duty cycle of PWMxA = 60.0%
    pwm_config.cmpr_b = 50.0;       //duty cycle of PWMxb = 50.0%
    pwm_config.counter_mode = MCPWM_UP_COUNTER;
    pwm_config.duty_mode = MCPWM_DUTY_MODE_0;
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);   //Configure PWM0A & PWM0B with above settings

    pwm_config.frequency = 500;     //frequency = 500Hz
    pwm_config.cmpr_a = 45.9;       //duty cycle of PWMxA = 45.9%
    pwm_config.cmpr_b = 7.0;        //duty cycle of PWMxb = 07.0%
    pwm_config.counter_mode = MCPWM_UP_COUNTER;
    pwm_config.duty_mode = MCPWM_DUTY_MODE_0;
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_1, &pwm_config);   //Configure PWM1A & PWM1B with above settings

    pwm_config.frequency = 400;     //frequency = 400Hz
    pwm_config.cmpr_a = 23.2;       //duty cycle of PWMxA = 23.2%
    pwm_config.cmpr_b = 97.0;       //duty cycle of PWMxb = 97.0%
    pwm_config.counter_mode = MCPWM_UP_DOWN_COUNTER; //frequency is half when up down count mode is set i.e. SYMMETRIC PWM
    pwm_config.duty_mode = MCPWM_DUTY_MODE_1;
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_2, &pwm_config);   //Configure PWM2A & PWM2B with above settings
}
