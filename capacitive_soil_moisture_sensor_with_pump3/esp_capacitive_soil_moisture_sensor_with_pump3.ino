/* Automatic Plant Watering

  Capacitive moisture sensor combined with windshield washer pump to supply
  more water when the detected moisture falls below a configured limit.

  Really useful site: https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/adc.html
 */
#include <driver/adc.h>
#include <driver/mcpwm.h>

// Pump motor control configuration data
const mcpwm_config_t PUMP_CONFIG = {
  100, 99.0, 100.0, MCPWM_DUTY_MODE_0, MCPWM_UP_COUNTER
};
// PUMP_CONFIG.frequency = 100;
// PUMP_CONFIG.cmpr_a = 99.0;
// PUMP_CONFIG.cmpr_b = 100.0;
// PUMP_CONFIG.duty_mode = MCPWM_DUTY_MODE_0;
// PUMP_CONFIG.counter_mode = MCPWM_UP_COUNTER;
const mcpwm_unit_t PUMP_UNIT = MCPWM_UNIT_0;
const mcpwm_timer_t PUMP_TIMER = MCPWM_TIMER_0;
const mcpwm_io_signals_t PUMP_IO = MCPWM0A;
const int PUMP_GPIO = 16;

// Capacitive moisture sensor configuration data
const adc1_channel_t sensorPin = ADC1_CHANNEL_6; // gpio 34 for reading moisture sensor
const adc_atten_t ADC_ATTEN_DB = ADC_ATTEN_DB_11; // 3.9V maximum for sensor readings
const int airValue = 2000;    //you need to replace this value with Value_1
const int waterValue = 1210;  //you need to replace this value with Value_2
int TRIGGER_PERCENT = 30;     //pump is triggered to water below this value

int WATERING_TIME = 2500;     // the number of milliseconds for the motor to turn on for
int SOAK_TIME = 5000;         // the minimum delay time to allow water to soak in before next watering session
const float PUMP_ON_SPEED = 40.0; // percentage of maximum
const float PUMP_OFF_SPEED = 0.0;

void setup() {
  Serial.begin(115200);     // open serial port, set the baud rate
  SoilSensorInit();
  PumpInit();
}

void loop() {
  if (getMoisturePercent() < TRIGGER_PERCENT) {
    waterPlants();
  }
}

void  SoilSensorInit()
{
  adc_power_acquire();
  adc_gpio_init(ADC_UNIT_1, (adc_channel_t)sensorPin);
  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(sensorPin, ADC_ATTEN_DB);
}

void PumpInit()
{
  // Pump motor control configuration
  mcpwm_gpio_init(PUMP_UNIT, PUMP_IO, PUMP_GPIO);
  mcpwm_init(PUMP_UNIT, PUMP_TIMER, &PUMP_CONFIG);
  mcpwm_set_frequency(PUMP_UNIT, PUMP_TIMER, PUMP_CONFIG.frequency);

  mcpwm_deadtime_disable(PUMP_UNIT, PUMP_TIMER);
  mcpwm_sync_disable(PUMP_UNIT, PUMP_TIMER);
  mcpwm_start(PUMP_UNIT, PUMP_TIMER);
}

int getMoisturePercent()
{
  int soilMoistureValue;
  int soilMoisturePercent;
  soilMoistureValue = 0;
  for (int i = 0; i<16; i++) {
    int adc_value = adc1_get_raw(sensorPin);
    soilMoistureValue += adc_value;
    delay(15);
    Serial.print("Measurement = ");
    Serial.println(adc_value);
  }
  soilMoistureValue /= 16;
  Serial.print("soilMoistureValue = ");
  Serial.println(soilMoistureValue);
  soilMoisturePercent = map(soilMoistureValue, airValue, waterValue, 0, 100);
  if(soilMoisturePercent < 0) { soilMoisturePercent = 0; }
  if(soilMoisturePercent > 100) { soilMoisturePercent = 100; }
  Serial.print("soilMoisturePercent = ");
  Serial.print(soilMoisturePercent);
  Serial.println(" %");
  return soilMoisturePercent;
}

void waterPlants()
{
  Serial.println("Turning pump on");
  pumpOn();
  delay (WATERING_TIME);                   // water the plant
  Serial.println("Turning pump off");
  pumpOff();
  delay (SOAK_TIME);                       // let the water soak in
}

void pumpOn()
{
  mcpwm_set_duty(PUMP_UNIT, PUMP_TIMER, MCPWM_OPR_A, PUMP_ON_SPEED);
}

void pumpOff()
{
  mcpwm_set_duty(PUMP_UNIT, PUMP_TIMER, MCPWM_OPR_A, PUMP_OFF_SPEED);
}
