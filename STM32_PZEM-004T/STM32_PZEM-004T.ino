
#define PZEM_m 1 // Кол-во подключенный устройств

#define  mys     // Включаем MySensors
/*
Устройства PZEM-004T подключаем к Serial2
Вместо VDD RX оптопары, подключаем у индиидуальному управляющему пину
Это необходимо для одновременного подключения нескольких устройств к одному Serial порту
*/

#ifdef mys      
#define MY_NODE_ID 23
// Enable and select radio type attached
#define MY_RS485
//#define MY_RADIO_RF24
//#define MY_RADIO_NRF5_ESB
//#define MY_RADIO_RFM69
//#define MY_RADIO_RFM95    

#define NODE_NAME           "STM32 power node 1"
#define NODE_NAME_VER       "1.55 OTA"


#define MY_RS485
#define MY_RS485_HWSERIAL   (USART_RS485)
#define USART_RS485         Serial1    
#define MY_RS485_BAUD_RATE  115200

#define DEBUG_m   // режим отладки

#ifdef DEBUG_m
 #define MY_SERIALDEVICE     Serial3
 #define MY_BAUD_RATE        115200
 #define MY_DEBUG
 #define MY_DEBUGDEVICE MY_SERIALDEVICE
 #define SERIAL_USB
#endif // DEBUG_m


#include <MySensors.h>
MyMessage msg(0, V_VOLTAGE);

void before()
{

}
#endif // mys 


#include <PZEM004T.h>


// Структура сенсоров PZEM-004T
  struct PZEM_S {
  public:
    IPAddress ip;       // Псевдо IP адрес для устройства PZEM-004T
    uint8_t childId_error;  // Id в системе MySensors для PZEM-004T!
    uint8_t childId_v;  // Id в системе MySensors для сенсора напряжения
    uint8_t childId_c;  // Id в системе MySensors для сенсора тока
    uint8_t childId_p;  // Id в системе MySensors для сенсора мощности
    uint8_t childId_e;  // Id в системе MySensors для сенсора счётчика (потреблёной электроэнергии)
    float voltage;      // Текущее значение напряжения
    float current;      // Текущее значение тока
    float power;        // Текущее значение мощности
    float energy;       // Текущее значение потреблённой электроэнергии
    float voltage_old;      // Предыдущее значение напряжения
    float current_old;      // Предыдущее значение тока
    float power_old;        // Предыдущее значение мощности
    float energy_old;       // Предыдущее значение потреблённой электроэнергии    
    uint32_t voltage_lastTime;  // Время последней отсылки сообщений
    uint16_t voltage_timeout;   // Таймаут (задаёт переодичность опроса сенсоров
    uint32_t current_lastTime;  // Время последней отсылки сообщений
    uint16_t current_timeout;   // Таймаут (задаёт переодичность опроса сенсоров    
    uint32_t power_lastTime;  // Время последней отсылки сообщений
    uint16_t power_timeout;   // Таймаут (задаёт переодичность опроса сенсоров
    uint32_t energy_lastTime;  // Время последней отсылки сообщений
    uint16_t energy_timeout;   // Таймаут (задаёт переодичность опроса сенсоров        
    uint16_t errorTime; // Твймаут ответа устройства PZEM-004T (если превышен, значит устройство не доступно)
    uint8_t lastResult;    // флаг, чтобы не спамить каждый раз при изменении состояния устройства 
    uint8_t pin;        // нога МК подключенная к оптопаре, требуется для управления несколькими устройствами
    char description[16];  // Описание сенсора в Mysensors
  } pzem_str[PZEM_m] = { // Инициализируем структуру
    #if MY_NODE_ID == 23
        {{192,168,1,1},90,91,92,93,94,0,0,0,0,0,0,0,0,0,1000,0,1000,0,1000,0,1000,2100,true,PA0, 
        {'E', 'n', 'e', 'r', 'g', 'y', ' ', 'm', 'e', 't', 'e', 'r', ' ', '0', '1', '\n'}}
        
    #endif     
  };

PZEM004T* pzem;



void setup() {

#ifdef DEBUG_m  
  MY_SERIALDEVICE.begin(MY_BAUD_RATE);
  while(!MY_SERIALDEVICE) { }
  MY_SERIALDEVICE.println("Started Node. Please wait...");
#endif // DEBUG_m


while(!Serial2) { }
pzem = new PZEM004T(&Serial2);
for (uint8_t i=0; i<PZEM_m; i++) {
   pinMode(pzem_str[i].pin, OUTPUT);
   digitalWrite(pzem_str[i].pin, HIGH);
   pzem->setAddress(pzem_str[i].ip);
   digitalWrite(pzem_str[i].pin, LOW);
}

  delay(1000);
  
}

#ifdef mys
void presentation()
{
  // Send the sketch version information to the gateway and Controller
  delay(5);
  sendSketchInfo(NODE_NAME, NODE_NAME_VER);
  delay(5);
  
for (uint8_t i=0; i<PZEM_m; i++) {
    present(pzem_str[i].childId_error, S_CUSTOM);    
    delay(5); 
    present(pzem_str[i].childId_v, S_MULTIMETER);
    delay(5);
    present(pzem_str[i].childId_c, S_MULTIMETER);    
    delay(5);
    present(pzem_str[i].childId_p, S_POWER);
    delay(5);
    present(pzem_str[i].childId_e, S_POWER);    
    delay(5);    
}

}
#endif // mys



void loop() {


for (uint8_t i=0; i<PZEM_m; i++) {
  uint32_t timenow = millis();
  //pzem->setAddress(pzem_str[i].ip);
  uint32_t time_start=millis();
  digitalWrite(pzem_str[i].pin, HIGH);
  delay(5); // Без задержки иногда подглюкивает и напряжение приходит с 0
  pzem_str[i].voltage = pzem->voltage(pzem_str[i].ip);
  pzem_str[i].current = pzem->current(pzem_str[i].ip);
  pzem_str[i].power   = pzem->power(pzem_str[i].ip);
  pzem_str[i].energy  = pzem->energy(pzem_str[i].ip);
  //uint32_t time_stop = millis() - time_start;
  if ((millis() - time_start) > pzem_str[i].errorTime) 
  {
    if (pzem_str[i].lastResult == true) {
      send(msg.setSensor(pzem_str[i].childId_error).setType(V_VAR1).set(pzem_str[i].lastResult, 0));  
      #ifdef DEBUG_m
      MY_SERIALDEVICE.print(pzem_str[i].lastResult); 
      MY_SERIALDEVICE.println("Error! PZEM-004t is not connected!!!"); // Устройство не подключено!
      #endif 
    }
    pzem->setAddress(pzem_str[i].ip);
    pzem_str[i].lastResult = false;
    delay(2000);
    //return false; // Устройство не доступно
  }
  else 
  {

  if (pzem_str[i].voltage <= 0.0) {
      pzem_str[i].voltage = 0.0;
    } 

  if((pzem_str[i].voltage > 0) && (pzem_str[i].lastResult==false)) {
    send(msg.setSensor(pzem_str[i].childId_error).setType(V_VAR1).set(pzem_str[i].lastResult, 0));
    #ifdef DEBUG_m
    MY_SERIALDEVICE.print(pzem_str[i].lastResult); 
    MY_SERIALDEVICE.println("PZEM-004t is connected!!!"); 
    #endif
    }
    
  if( pzem_str[i].voltage >= 0.0){ 
    #ifdef mys
    if (timenow > pzem_str[i].voltage_lastTime + pzem_str[i].voltage_timeout) {
      if (pzem_str[i].voltage != pzem_str[i].voltage_old) {
        send(msg.setSensor(pzem_str[i].childId_v).setType(V_VOLTAGE).set(pzem_str[i].voltage, 1));
        pzem_str[i].voltage_old = pzem_str[i].voltage;
        pzem_str[i].voltage_lastTime = timenow;
        #ifdef DEBUG_m
        MY_SERIALDEVICE.print(pzem_str[i].voltage);Serial.print("V; "); 
        #endif
      }
    }
    #endif // mys    
    }
  if(pzem_str[i].current >= 0.0){ 
    #ifdef mys    
    if (timenow > pzem_str[i].current_lastTime + pzem_str[i].current_timeout) {
      if (pzem_str[i].current != pzem_str[i].current_old) {
        send(msg.setSensor(pzem_str[i].childId_c).setType(V_CURRENT).set(pzem_str[i].current, 2));
        pzem_str[i].current_old = pzem_str[i].current;
        pzem_str[i].current_lastTime = timenow;
        #ifdef DEBUG_m
        MY_SERIALDEVICE.print(pzem_str[i].current);Serial.print("A; "); 
        #endif
      }
     }
    #endif // mys
    }
  if(pzem_str[i].power >= 0.0){ 
    #ifdef mys    
    if (timenow > pzem_str[i].power_lastTime + pzem_str[i].power_timeout) {
      if (pzem_str[i].power != pzem_str[i].power_old) {
        send(msg.setSensor(pzem_str[i].childId_p).setType(V_WATT).set(pzem_str[i].power, 1));
        pzem_str[i].power_old = pzem_str[i].power;
        pzem_str[i].power_lastTime = timenow;
        #ifdef DEBUG_m
        MY_SERIALDEVICE.print(pzem_str[i].power);Serial.print("W; "); 
        #endif
      }
     }
    #endif // mys    
    }
  if(pzem_str[i].energy >= 0.0){ 
    #ifdef mys    
    if (timenow > pzem_str[i].energy_lastTime + pzem_str[i].energy_timeout) {
      if (pzem_str[i].energy != pzem_str[i].energy_old) {   
        send(msg.setSensor(pzem_str[i].childId_e).setType(V_KWH).set(pzem_str[i].energy/1000, 3));
        pzem_str[i].energy_old = pzem_str[i].energy;
        pzem_str[i].energy_lastTime = timenow;
        #ifdef DEBUG_m
        MY_SERIALDEVICE.print(pzem_str[i].energy);Serial.print("Wh; "); 
        #endif
      }
     }
    #endif // mys    
    }
  #ifdef mys      
  #endif // mys
  pzem_str[i].lastResult = true;  
    
    //return true;  // Устройство ответило в заданный промежуток

  }
  digitalWrite(pzem_str[i].pin, LOW);
}
}
