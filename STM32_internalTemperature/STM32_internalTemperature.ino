#if defined (ARDUINO_ARCH_STM32F1)  // Скетч только для STM32F1

#define  ENABLE_MYSENSORS     // Включаем MySensors
/*
В МК семейства STM32F103 имеется встроенный сенсор температуры. 
Особо точным его назвать сложно, но для общего контроля температуры - самое то.
*/

#define DEBUG_m   // Включаем режим отладки
#define MY_SERIALDEVICE     Serial1       // Указываем порт для отладки, TX3 - PB10, RX3 - PB11
#define MY_DEBUGDEVICE MY_SERIALDEVICE
#define MY_BAUD_RATE        115200        // Скорость порта

    struct INTTEMP_S {    // Структура для датчика температуры
    uint8_t childId;      // Номер сенсора в данной ноде
    float value;          // Последнее значение температуры
    uint32_t lasttime;    // Время последнего чтения сенсора температуры
    uint32_t timeout;     // Периодичность опроса
    } inttemp_str =
    {
          97, 0, 0, 1000
    };

#include <libmaple/adc.h>

#ifdef ENABLE_MYSENSORS      
  #define MY_NODE_ID 51
  // Выбираем и включаем протокол передачи данных в системе MySensors
  // Enable and select radio type attached
  #define MY_RS485  // Протокол на основе RS485 (проводной)
  //#define MY_RADIO_RF24
  //#define MY_RADIO_NRF5_ESB
  //#define MY_RADIO_RFM69
  //#define MY_RADIO_RFM95    

  #define NODE_NAME           "STM32 intTemp"  // название ноды
  #define NODE_NAME_VER       "1.60"           // текущая версия

  #define USART_RS485         Serial2    
  #define MY_RS485_HWSERIAL   (USART_RS485)   // Указываем UART порт для RS485
  #define MY_RS485_BAUD_RATE  115200          // Скорость порта

  #ifdef DEBUG_m
    #define SERIAL_USB                        // При включении данного дефайна нумерация портов UART начинается с Serial1, а не с Serial
  #endif // DEBUG_m

  #include <MySensors.h>                      // Подключаем библиотеку MySensors
  MyMessage msg_inttemp(inttemp_str.childId, V_TEMP); // Создаём сообщение для передачи данных температуры

  #endif // ENABLE_MYSENSORS 






  void setup_temperature_sensor() {
    adc_reg_map *regs = ADC1->regs;

  // 3. Set the TSVREFE bit in the ADC control register 2 (ADC_CR2) to wake up the
  //    temperature sensor from power down mode.  Do this first 'cause according to
  //    the Datasheet section 5.3.21 it takes from 4 to 10 uS to power up the sensor.

    //regs->CR2 |= ADC_CR2_TSVREFE ;
    regs->CR2 |= ADC_CR2_TSVREFE;
    
  // 2. Select a sample time of 17.1 μs
  // set channel 16 sample time to 239.5 cycles
  // 239.5 cycles of the ADC clock (72MHz/6=12MHz) is over 17.1us (about 20us), but no smaller
  // sample time exceeds 17.1us.

    regs->SMPR1 = (0b111 << (3*6));     // set channel 16, the temp. sensor
    }


void setup() {
setup_temperature_sensor();
#ifdef DEBUG_m  
#ifndef ENABLE_MYSENSORS
  MY_SERIALDEVICE.begin(MY_BAUD_RATE);
  while(!MY_SERIALDEVICE) { }
#endif //  ENABLE_MYSENSORS 
  MY_SERIALDEVICE.println("Started Node.");
#endif // DEBUG_m

}

#ifdef ENABLE_MYSENSORS
void presentation()
{
  // Send the sketch version information to the gateway and Controller
  wait(5);
  sendSketchInfo(NODE_NAME, NODE_NAME_VER);
  wait(5);
  present(inttemp_str.childId, S_TEMP, "Internal temp"); // Презентуем сенсор температуры
}
#endif // ENABLE_MYSENSORS

void loop() {

  uint32_t uptime = millis();

  // Выдерживаем переодичность опроса
  if (uptime > (inttemp_str.lasttime + inttemp_str.timeout)) {
    const uint16_t V25 = 1750;
    const float Avg_Slope = 4.3; //when avg_slope=4.3mV/C at ref 3.3V

    float adc_value = adc_read(ADC1, 16); 
    float temp = ((V25-adc_value)/Avg_Slope+25);

    if (inttemp_str.value != temp) {
      inttemp_str.value = temp;
#ifdef ENABLE_MYSENSORS
      send(msg_inttemp.set(inttemp_str.value, 1));
#else
      MY_SERIALDEVICE.print("Internal temp: ");
      MY_SERIALDEVICE.println(inttemp_str.value);
#endif // ENABLE_MYSENSORS

    }
    inttemp_str.lasttime = uptime;
  }
}

#else
    #error "This sketch use only with STM32F1!!!"
#endif 