#include <PubSubClient.h>
#include <SimpleTimer.h>
#include <WifiUtil.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>

#define PIDpin 7
#define bivepin 8
#define FLOWSENSORPIN 9   
WifiUtil wifi(4,5);

char waterState = 0;
int BiveState = 0;
int PIDState = 0;
    
SimpleTimer timer;
SoftwareSerial softSerial(4, 5); //RX, TX



const char ssid[] = "Campus7_Room3_2.4"; // 네트워크 ssid
const char password[] = "12345678"; // 비밀번호
const char mqtt_server[] = "192.168.0.109";//->서버주소 = 내 pc주소

//MQTT용 WiFi 클라이언트 객체초기화
WiFiEspClient espClient;
PubSubClient client(espClient);


void setup(){
    pinMode(9, OUTPUT);
    
}

void loop(){
  analogWrite(9,180);
}
