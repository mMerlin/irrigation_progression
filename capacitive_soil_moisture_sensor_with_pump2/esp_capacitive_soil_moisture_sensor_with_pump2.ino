/* Really useful site: https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/adc.html
 *
 *  Values to be input through config file
 *
 *  airValue - get this from ESP32_Capacitive_Soil_Moisture_Calibration
 *  waterValue - get this from ESP32_Capacitive_Soil_Moisture_Calibration
 *  triggerPercent
 *  pumpPin
 *  sensorPin
 *  onTime (ms)
 *  soakTime (ms)
 *
 *  freq
 *  pumpChannel
 *  resolution
 *  dutyCycle
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

int soilMoistureValue;
int soilMoisturePercent;
int triggerPercent = 30;    //pump is triggered to water below this value

int onTime = 2500;          // the number of milliseconds for the motor to turn on for
int soakTime = 5000;        // the minimum delay time to allow water to soak in before next watering session

void setup() {
  Serial.begin(115200);     // open serial port, set the baud rate

  adc_power_acquire();
  adc_gpio_init(ADC_UNIT_1, (adc_channel_t)sensorPin);
  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(sensorPin, ADC_ATTEN_DB);
  //for (int i = 0; i <= ADC_CHANNEL_MAX; i++) {
  //  gpio_num_t g;
  //  adc1_pad_get_io_num((adc1_channel_t)i, &g);
  //  Serial.print("ADC channel");
  //  Serial.print(i);
  //  Serial.print(", ");
  //  Serial.println((int)g);
  //}

  // Pump motor control configuration
  mcpwm_gpio_init(PUMP_UNIT, PUMP_IO, PUMP_GPIO);
  mcpwm_init(PUMP_UNIT, PUMP_TIMER, &PUMP_CONFIG);
  mcpwm_set_frequency(PUMP_UNIT, PUMP_TIMER, PUMP_CONFIG.frequency);
  // mcpwm_set_duty(PUMP_UNIT, PUMP_TIMER, MCPWM_OPR_A, 80.0);

  // mcpwm_capture_disable(MCPWM_UNIT_0, ... );
  mcpwm_deadtime_disable(PUMP_UNIT, PUMP_TIMER);
  mcpwm_sync_disable(MCPWM_UNIT_0, PUMP_TIMER);

  mcpwm_start(PUMP_UNIT, PUMP_TIMER);
}

void loop() {
  soilMoistureValue = 0;
  for (int i = 0; i<16; i++) {
    int adc_value = adc1_get_raw(sensorPin);
    soilMoistureValue += adc_value;
//    soilMoistureValue = soilMoistureValue + adc1_get_raw(sensorPin);
    delay(15);
    Serial.print("Measurement = ");
    Serial.println(adc_value);
  }
//  Serial.println(" ");
  soilMoistureValue /= 16;
  Serial.print("soilMoistureValue = ");
  Serial.println(soilMoistureValue);
  soilMoisturePercent = map(soilMoistureValue, airValue, waterValue, 0, 100);
  if(soilMoisturePercent < 0) { soilMoisturePercent = 0; }
  if(soilMoisturePercent > 100) { soilMoisturePercent = 100; }
  Serial.print("soilMoisturePercent = ");
  Serial.print(soilMoisturePercent);
  Serial.println(" %");
  if (soilMoisturePercent < triggerPercent) {
    Serial.println("Turning pump on");
    pumpOn();
    delay (onTime);                         // water the plant
    Serial.println("Turning pump off");
    pumpOff();
    delay (soakTime);                       // let the water soak in
  }
}

void pumpOn()
{
  mcpwm_set_duty(PUMP_UNIT, PUMP_TIMER, MCPWM_OPR_A, 40.0);
}

void pumpOff()
{
  mcpwm_set_duty(PUMP_UNIT, PUMP_TIMER, MCPWM_OPR_A, 0.0);
}
