/*
 brushed dc motor control example

 https://github.com/espressif/esp-idf/blob/a20df743f1c51e6d65b021ed2ffd3081a2feec64/
 examples/peripherals/mcpwm/mcpwm_brushed_dc_control/main/mcpwm_brushed_dc_control_example.c

 This example will show you how to use MCPWM module to control brushed dc motor.
 This code is tested with L298 motor driver.
 User may need to make changes according to the motor driver they use.
*/
#include "esp_attr.h"
#include "driver/mcpwm.h"
//#include "soc/mcpwm_periph.h"

struct my_mcpwm_id_t {
  mcpwm_unit_t unit;
  mcpwm_io_signals_t io;
  mcpwm_timer_t timer;
  mcpwm_operator_t opr;
  int gpio;
};
const my_mcpwm_id_t pump_1 = { MCPWM_UNIT_0, MCPWM0B, MCPWM_TIMER_0, MCPWM_OPR_B, 15};
const my_mcpwm_id_t pump_2 = { MCPWM_UNIT_0, MCPWM0A, MCPWM_TIMER_0, MCPWM_OPR_A, 16};
const unsigned long STEP_LENGTH = 2000; // milliseconds

static void mcpwm_example_gpio_initialize(void)
{
  Serial.println("initializing mcpwm gpio...");
  mcpwm_gpio_init(pump_1.unit, pump_1.io, pump_1.gpio);
//  mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, GPIO_PWM0A_OUT);
  mcpwm_gpio_init(pump_2.unit, pump_2.io, pump_2.gpio);
//  mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0B, GPIO_PWM0B_OUT);
}

///**
// * @brief motor moves in forward direction, with duty cycle = duty %
// */
//static void brushed_motor_forward(mcpwm_unit_t mcpwm_num, mcpwm_timer_t timer_num , float duty_cycle)
//{
//  mcpwm_set_signal_low(mcpwm_num, timer_num, MCPWM_OPR_B);
//  mcpwm_set_duty(mcpwm_num, timer_num, MCPWM_OPR_A, duty_cycle);
//  mcpwm_set_duty_type(mcpwm_num, timer_num, MCPWM_OPR_A, MCPWM_DUTY_MODE_0); //call this each time, if operator was previously in low/high state
//}
//
///**
// * @brief motor moves in backward direction, with duty cycle = duty %
// */
//static void brushed_motor_backward(mcpwm_unit_t mcpwm_num, mcpwm_timer_t timer_num , float duty_cycle)
//{
//  mcpwm_set_signal_low(mcpwm_num, timer_num, MCPWM_OPR_A);
//  mcpwm_set_duty(mcpwm_num, timer_num, MCPWM_OPR_B, duty_cycle);
//  mcpwm_set_duty_type(mcpwm_num, timer_num, MCPWM_OPR_B, MCPWM_DUTY_MODE_0);  //call this each time, if operator was previously in low/high state
//}
//
///**
// * @brief motor stop
// */
//static void brushed_motor_stop(mcpwm_unit_t mcpwm_num, mcpwm_timer_t timer_num)
//{
//  mcpwm_set_signal_low(mcpwm_num, timer_num, MCPWM_OPR_A);
//  mcpwm_set_signal_low(mcpwm_num, timer_num, MCPWM_OPR_B);
//}

/**
 * @brief Configure MCPWM module for brushed dc motor
 */
//static void mcpwm_example_brushed_motor_control(void *arg)
static void mcpwm_example_brushed_motor_control()
{
  //1. mcpwm gpio initialization
  mcpwm_example_gpio_initialize();

  //2. initial mcpwm configuration
  Serial.println("Configuring Initial Parameters of mcpwm...");
  mcpwm_config_t pwm_config;
  pwm_config.frequency = 1000;    //frequency = 500Hz,
  pwm_config.cmpr_a = 0;    //duty cycle of PWMxA = 0
  pwm_config.cmpr_b = 0;    //duty cycle of PWMxb = 0
  pwm_config.counter_mode = MCPWM_UP_COUNTER;
  pwm_config.duty_mode = MCPWM_DUTY_MODE_0;
  mcpwm_init(pump_1.unit, pump_1.timer, &pwm_config);    //Configure PWM0A & PWM0B with above settings
//  mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);    //Configure PWM0A & PWM0B with above settings
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("Testing brushed motor...");
  mcpwm_example_brushed_motor_control();
  pump_off(pump_1);
  pump_off(pump_2);
}

void loop() {
  pump_on(pump_1);
//  brushed_motor_forward(MCPWM_UNIT_0, MCPWM_TIMER_0, 50.0);
  delay(STEP_LENGTH);
  pump_off(pump_1);

  pump_on(pump_2);
//  brushed_motor_backward(MCPWM_UNIT_0, MCPWM_TIMER_0, 30.0);
  delay(STEP_LENGTH);
//  pump_off(pump_1);
  pump_off(pump_2);
//  brushed_motor_stop(MCPWM_UNIT_0, MCPWM_TIMER_0);

  delay(STEP_LENGTH);
}

void pump_on(my_mcpwm_id_t any_pump)
{
  // *all* args to be taken from pump configuration struct
  mcpwm_set_duty(any_pump.unit, any_pump.timer, any_pump.opr, 50.0);
  mcpwm_set_duty_type(any_pump.unit, any_pump.timer, any_pump.opr, MCPWM_DUTY_MODE_0); //call this each time, if operator was previously in low/high state

//  mcpwm_set_signal_low(mcpwm_num, timer_num, MCPWM_OPR_B);
//  mcpwm_set_duty(mcpwm_num, timer_num, MCPWM_OPR_A, duty_cycle);
//  mcpwm_set_duty_type(mcpwm_num, timer_num, MCPWM_OPR_A, MCPWM_DUTY_MODE_0); //call this each time, if operator was previously in low/high state
}

void pump_off(my_mcpwm_id_t any_pump)
{
  mcpwm_set_signal_low(any_pump.unit, any_pump.timer, any_pump.opr);
}
