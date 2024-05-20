

#include <ESP8266WiFi.h>

#include <espMqttClientAsync.h>


#include <Servo.h>

const int adc = A0;  // This creates a constant integer in memory that stores the analog pin
int value = 0;  // This creates an integer location in memory to store the analog value

Servo  myservo;  //создаем  объект для управляющего сервопривода
int pos = 0;    //положение сервопривода
int kc_pos = 0;
int wc_pos = 0;
int on_off_pos = 0;

#define WIFI_SSID "RT-WiFi-D34D"////"Armor 11T 5G"   "HUAWEI P20"
#define WIFI_PASSWORD "pKesnPpg"  // "90227473"

#define MQTT_HOST "m5.wqtt.ru"//IPAddress(192, 168, 1, 10)
#define MQTT_PORT 9316//1883


#define PIN_KC_OPEN D1
#define PIN_KC_CLOSE D2
#define PIN_WC_OPEN D3
#define PIN_WC_CLOSE D4

#define PIN_VENT_REG D7
#define PIN_VENT_ON_OFF D8

WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;
espMqttClientAsync mqttClient;
bool reconnectMqtt = false;
uint32_t lastReconnect = 0;

void set_vent_speed(int new_pos){

  if (new_pos == 1) { 
    myservo.write(0);
  }

  
  if (new_pos == 2) { 
    myservo.write(90);
  }

  
  if (new_pos == 3) { 
    myservo.write(180);
  }

}

void wc_open_close(int new_pos){
  if (new_pos > 0) { 
    digitalWrite(PIN_WC_OPEN, LOW);
    digitalWrite(PIN_WC_CLOSE, HIGH);
  }
    if (new_pos == 0) { 
    digitalWrite(PIN_WC_OPEN, HIGH);
    digitalWrite(PIN_WC_CLOSE, LOW) ;
  }
}

void kc_open_close(int new_pos){
  if (new_pos > 0) { 
    digitalWrite(PIN_KC_OPEN, LOW);
    digitalWrite(PIN_KC_CLOSE, HIGH);
  }
    if (new_pos == 0) { 
    digitalWrite(PIN_KC_OPEN, HIGH);
    digitalWrite(PIN_KC_CLOSE, LOW);
  }
}

void on_off_vent(int new_pos){

  if (new_pos > 0) { 
    digitalWrite(PIN_VENT_ON_OFF, LOW);
  }
    if (new_pos == 0) { 
    digitalWrite(PIN_VENT_ON_OFF, HIGH);
  }

}

void onMqttMessage(const espMqttClientTypes::MessageProperties& properties, const char* topic, const uint8_t* payload, size_t len, size_t index, size_t total) {
 // (void) payload;
  Serial.println("Publish received.");
  Serial.print("  topic: ");
  Serial.println(topic);
  Serial.print("  qos: ");
  Serial.println(properties.qos);
  Serial.print("  dup: ");
  Serial.println(properties.dup);
  Serial.print("  retain: ");
  Serial.println(properties.retain);
  Serial.print("  len: ");
  Serial.println(len);
  Serial.print("  index: ");
  Serial.println(index);
  Serial.print("  total: ");
  Serial.println(total);
 Serial.println("topic:");
  Serial.println(topic);


//Переключение скоростей вентиляции
  if( strcmp(topic,"frunza81/vent/reg")==0){ //and properties.retain == 0){
    Serial.println("vent_speed_will_be:");
    int new_pos = atoi((const  char*)payload);
    Serial.print(String(new_pos).c_str());
    set_vent_speed(new_pos);
    mqttClient.publish("frunza81/vent/pos", 0, true,  String(new_pos).c_str() );
  }

  if( strcmp(topic,"frunza81/vent/pos")==0){
    Serial.println("vent_speed_seted_to: ");
    int seted_pos = atoi((char*)payload);
    Serial.print(String(seted_pos).c_str());
    pos = seted_pos;
  }

  //Открытие-закрытие клапана кухни
if( strcmp(topic,"frunza81/vent/kc_reg")==0){
    Serial.println("kc_valve_will_be:");
    int new_pos = atoi((const  char*)payload);
    Serial.print(String(new_pos).c_str());
    kc_open_close(new_pos);
    mqttClient.publish("frunza81/vent/kc_pos", 0, true,  String(new_pos).c_str() );
}

if( strcmp(topic,"frunza81/vent/kc_pos")==0){
    Serial.println("kc_valve_seted_to: ");
    int seted_pos = atoi((char*)payload);
    Serial.print(String(seted_pos).c_str());
    kc_pos = seted_pos;
  }


  //Открытие-закрытие клапана кухни
if( strcmp(topic,"frunza81/vent/wc_reg")==0){
    Serial.println("wc_valve_will_be:");
    int new_pos = atoi((const  char*)payload);
    Serial.print(String(new_pos).c_str());
    wc_open_close(new_pos);
    mqttClient.publish("frunza81/vent/wc_pos", 0, true,  String(new_pos).c_str() );
}

if( strcmp(topic,"frunza81/vent/wc_pos")==0){
    Serial.println("wc_valve_seted_to: ");
    int seted_pos = atoi((char*)payload);
    Serial.print(String(seted_pos).c_str());
    wc_pos = seted_pos;
  }

    //Вкыл-выкл вентмашина
if( strcmp(topic,"frunza81/vent/on_off_reg")==0){
    Serial.println("on_off_will_be:");
    int new_pos = atoi((const  char*)payload);
    Serial.print(String(new_pos).c_str());
    on_off_vent(new_pos);
    mqttClient.publish("frunza81/vent/on_off_pos", 0, true,  String(new_pos).c_str() );
}

if( strcmp(topic,"frunza81/vent/on_off_pos")==0){
    Serial.println("on_off_seted_to: ");
    int seted_pos = atoi((char*)payload);
    Serial.print(String(seted_pos).c_str());
    on_off_pos = seted_pos;
  }

  
}

void loop() {
  static uint32_t currentMillis = millis();

  if (reconnectMqtt && currentMillis - lastReconnect > 5000) {
    connectToMqtt();
  }

}

void setup() {
pinMode(PIN_VENT_ON_OFF, OUTPUT);
digitalWrite(PIN_VENT_ON_OFF, HIGH);

pinMode(PIN_VENT_REG, OUTPUT);
myservo.attach(PIN_VENT_REG);

pinMode(PIN_KC_OPEN, OUTPUT);
pinMode(PIN_KC_CLOSE, OUTPUT);
pinMode(PIN_WC_OPEN, OUTPUT);
pinMode(PIN_WC_CLOSE, OUTPUT);

digitalWrite(PIN_KC_OPEN, HIGH);
digitalWrite(PIN_KC_CLOSE, HIGH);
digitalWrite(PIN_WC_OPEN, HIGH);
digitalWrite(PIN_WC_CLOSE, HIGH);

  Serial.begin(921600);
  Serial.println();
  Serial.println();


  WiFi.setAutoConnect(false);
  WiFi.setAutoReconnect(true);
  wifiConnectHandler = WiFi.onStationModeGotIP(onWiFiConnect);
  wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWiFiDisconnect);

  mqttClient.onConnect(onMqttConnect);
  mqttClient.onDisconnect(onMqttDisconnect);
  mqttClient.onSubscribe(onMqttSubscribe);
  mqttClient.onUnsubscribe(onMqttUnsubscribe);
  mqttClient.onMessage(onMqttMessage);
  mqttClient.onPublish(onMqttPublish);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);

  connectToWiFi();
 
}







void connectToWiFi() {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void connectToMqtt() {
  mqttClient.setCredentials("u_C5CSOM", "jdgMKkq5");
  Serial.println("Connecting to MQTT...");
  if (!mqttClient.connect()) {
    reconnectMqtt = true;
    lastReconnect = millis();
    Serial.println("Connecting failed.");
  } else {
    reconnectMqtt = false;
  }
}

void onWiFiConnect(const WiFiEventStationModeGotIP& event) {
  (void) event;
  Serial.println("Connected to Wi-Fi.");
  connectToMqtt();
}

void onWiFiDisconnect(const WiFiEventStationModeDisconnected& event) {
  (void) event;
  Serial.println("Disconnected from Wi-Fi.");
}

void onMqttConnect(bool sessionPresent) {
  Serial.println("Connected to MQTT.");
  Serial.print("Session present: ");
  Serial.println(sessionPresent);

  uint16_t packetIdSub = mqttClient.subscribe("frunza81/vent/reg", 2);
  Serial.print("Subscribing at QoS 2, packetId: ");
  Serial.println(packetIdSub);
  uint16_t packetIdSub2 = mqttClient.subscribe("frunza81/vent/pos", 2);
  Serial.print("Subscribing at QoS 2, packetId: ");
  Serial.println(packetIdSub2);

  uint16_t packetIdSub3 = mqttClient.subscribe("frunza81/vent/wc_reg", 2);
  Serial.print("Subscribing at QoS 2, packetId: ");
  Serial.println(packetIdSub3);
  int16_t packetIdSub4 = mqttClient.subscribe("frunza81/vent/wc_pos", 2);
  Serial.print("Subscribing at QoS 2, packetId: ");
  Serial.println(packetIdSub4);

  uint16_t packetIdSub5 = mqttClient.subscribe("frunza81/vent/kc_reg", 2);
  Serial.print("Subscribing at QoS 2, packetId: ");
  Serial.println(packetIdSub5);
  uint16_t packetIdSub6 = mqttClient.subscribe("frunza81/vent/kc_pos", 2);
  Serial.print("Subscribing at QoS 2, packetId: ");
  Serial.println(packetIdSub6);

    uint16_t packetIdSub7 = mqttClient.subscribe("frunza81/vent/on_off_reg", 2);
  Serial.print("Subscribing at QoS 2, packetId: ");
  Serial.println(packetIdSub7);
  uint16_t packetIdSub8 = mqttClient.subscribe("frunza81/vent/on_off_pos", 2);
  Serial.print("Subscribing at QoS 2, packetId: ");
  Serial.println(packetIdSub8);


}

void onMqttDisconnect(espMqttClientTypes::DisconnectReason reason) {
  Serial.printf("Disconnected from MQTT: %u.\n", static_cast<uint8_t>(reason));

  if (WiFi.isConnected()) {
    reconnectMqtt = true;
    lastReconnect = millis();
  }
}

void onMqttSubscribe(uint16_t packetId, const espMqttClientTypes::SubscribeReturncode* codes, size_t len) {
  Serial.println("Subscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
  for (size_t i = 0; i < len; ++i) {
    Serial.print("  qos: ");
    Serial.println(static_cast<uint8_t>(codes[i]));
  }
}

void onMqttUnsubscribe(uint16_t packetId) {
  Serial.println("Unsubscribe acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}


void onMqttPublish(uint16_t packetId) {
  Serial.println("Publish acknowledged.");
  Serial.print("  packetId: ");
  Serial.println(packetId);
}



