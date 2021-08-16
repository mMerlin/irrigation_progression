/**
 * Explore esp32 mcpwm library for motor speed control
 */
#include <driver/mcpwm.h>

const mcpwm_config_t PUMP_CONFIG = {
  100, 1.0, 100.0, MCPWM_DUTY_MODE_0, MCPWM_UP_COUNTER
};
// PUMP_CONFIG.frequency = 100;
// PUMP_CONFIG.cmpr_a = 99.0;
// PUMP_CONFIG.cmpr_b = 100.0;
// PUMP_CONFIG.duty_mode = MCPWM_DUTY_MODE_0;
// PUMP_CONFIG.counter_mode = MCPWM_UP_COUNTER;
const mcpwm_unit_t PUMP_UNIT = MCPWM_UNIT_0;
const mcpwm_timer_t PUMP_TIMER = MCPWM_TIMER_0;
const mcpwm_io_signals_t PUMP_IO_1 = MCPWM0A;
//const mcpwm_io_signals_t PUMP_IO_2 = MCPWM0B;
const int PUMP_GPIO_1 = 19;
//const int PUMP_GPIO_2 = 14;

void setup() {
  Serial.begin(115200);
  mcpwm_gpio_init(PUMP_UNIT, PUMP_IO_1, PUMP_GPIO_1);
//  mcpwm_gpio_init(PUMP_UNIT, PUMP_IO_2, PUMP_GPIO_2);
  mcpwm_init(PUMP_UNIT, PUMP_TIMER, &PUMP_CONFIG);
  mcpwm_set_frequency(PUMP_UNIT, PUMP_TIMER, PUMP_CONFIG.frequency);
  mcpwm_set_duty(PUMP_UNIT, PUMP_TIMER, MCPWM_OPR_A, 10.0);

  // mcpwm_capture_disable(MCPWM_UNIT_0, ... );
  mcpwm_deadtime_disable(PUMP_UNIT, PUMP_TIMER);
  mcpwm_sync_disable(MCPWM_UNIT_0, PUMP_TIMER);

  mcpwm_start(PUMP_UNIT, PUMP_TIMER);

//   mcpwm_set_frequency();
//   mcpwm_deadtime_enable();
//   mcpwm_gpio_init();
//   mcpwm_sync_enable();
//   mcpwm_set_pin();
//   mcpwm_capture_enable();
//   mcpwm_capture_signal_get_value();
//   mcpwm_capture_signal_get_edge();
//   mcpwm_fault_set_oneshot_mode();
//   mcpwm_fault_set_cyc_mode();
//   mcpwm_isr_register();
  delay(500);
  Serial.println("Start motor test cycle");
}

void loop() {
  // put your main code here, to run repeatedly:
  pump1On();
  pump_state("on");
  delay(300);
  pumpOff();
  pump_state("off");
  delay(300);

//  pump2On();
//  delay(1000);
//  pumpOff();
//  delay(500);
}

void pump_state(String state)
{
  Serial.print("GPIO ");
  Serial.print(PUMP_GPIO_1);
  Serial.print(" is now ");
  Serial.println(state);
}

void pump1On()
{
  mcpwm_set_signal_low(PUMP_UNIT, PUMP_TIMER, MCPWM_OPR_A);
  mcpwm_set_duty(PUMP_UNIT, PUMP_TIMER, MCPWM_OPR_B, 10.0);
  // mcpwm_set_signal_high(PUMP_UNIT, PUMP_TIMER, MCPWM_OPR_A);
  mcpwm_set_duty_type(PUMP_UNIT, PUMP_TIMER, MCPWM_OPR_B, MCPWM_DUTY_MODE_0);
}

//void pump2On()
//{
//  mcpwm_set_signal_low(PUMP_UNIT, PUMP_TIMER, MCPWM_OPR_B);
//  mcpwm_set_duty(PUMP_UNIT, PUMP_TIMER, MCPWM_OPR_A, 30.0);
//  // mcpwm_set_signal_high(PUMP_UNIT, PUMP_TIMER, MCPWM_OPR_B);
//  mcpwm_set_duty_type(PUMP_UNIT, PUMP_TIMER, MCPWM_OPR_A, MCPWM_DUTY_MODE_0);
//}

void pumpOff()
{
  mcpwm_set_signal_low(PUMP_UNIT, PUMP_TIMER, MCPWM_OPR_A);
  mcpwm_set_signal_low(PUMP_UNIT, PUMP_TIMER, MCPWM_OPR_B);
}
