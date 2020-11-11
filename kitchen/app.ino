#include <PubSubClient.h>
#include <SimpleTimer.h>
#include <WifiUtil.h>
#include <AltSoftSerial.h>
#include <ArduinoJson.h>

#define gasPin A0
#define firePin A1
#define buzPin 6
WifiUtil wifi(4,5);

char windowState = 0;

SimpleTimer timer;
AltSoftSerial softSerial(4, 5); //RX, TX
const char ssid[] = "Campus7_Room3_2.4"; // 네트워크 ssid
const char password[] = "12345678"; // 비밀번호
const char mqtt_server[] = "192.168.0.109";//->서버주소 = 내 pc주소

WiFiEspClient espClient;
PubSubClient client(espClient);

int GasValue; // gas라는 정수의 값을 설정합니다.

int FireValue = 0;

void callback(char* topic, byte* payload, unsigned int length){
    /*payload[length] = NULL;
    char *message = payload;*/
    char message[64];
    memcpy(message, payload, length);
    message[length] = NULL;
    //strcmp c언어떄 문자열배열을 비교할때 사용
    //0이 리턴되면 두 문자열이 같다는 뜻
    
    if(strcmp("LED_ON", message)==0){
        digitalWrite(13,HIGH);
    }
    else if(strcmp("LED_OFF", message)==0){
        digitalWrite(13,LOW);
       
    }

    Serial.print(topic);
    Serial.print(" : ");
    Serial.println(message);
    
}

void mqtt_init(){
    client.setServer(mqtt_server, 1883);
    //subscriber인경우 메시지 수순시 호출할 콜백 함수 등록
    client.setCallback(callback);
}

//MQTT 서버에 접속될 때까지 재접속 시도
void reconnect(){
    while(!client.connected()){
        Serial.print("Attempting MQTT connection...");
                        
        if(client.connect("IoT3_kitchen")){//ID를 다르게 등록할경우 보통 장치의 시리얼번호를 사용
        //아닐경우 EEPROM에다가 번호등록가능 보통은 시리얼번호 사용
            Serial.println("connected");
            //subscriber로 등록
        //    client.subscribe("inner/+/info",1); //구독 신청
        }
        else{
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            delay(5000);
        }
    }
}
void gasCheck(){
    GasValue = analogRead(gasPin); //gasvalue는 gaspin의 값을 읽어옵니다.
    if (GasValue >= 500) //500보다 크거나 같을시에
    {
        digitalWrite(buzPin, HIGH); //LED의 빛이 나옵니다.
    }
    else
    {
        digitalWrite(buzPin, LOW); // 작을시에는 꺼집니다.
    }
    //Serial.print("GasValue=");
    //Serial.println(GasValue);
}
void fireCheck(){
    FireValue = analogRead(firePin);
    if(FireValue<=1000){
        digitalWrite(buzPin,HIGH);
    }
    else{
        digitalWrite(buzPin, LOW);
    }
   // Serial.print("analog Value : ");
    //Serial.println(FireValue);
}

void publish_Gas(){
    char msg[12];
    StaticJsonBuffer<12> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    root["gas"]= (GasValue >= 500);

    root.printTo(msg);
    //토픽 발행
    client.publish("iot3/kitchen/Gas/", msg);
}
void publish_Fire(){
    char msg[12];
    StaticJsonBuffer<12> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    root["fire"] = (FireValue<=1000);

    root.printTo(msg);
    //토픽 발행
    client.publish("iot3/kitchen/Fire/",msg);
}
void publish(){
    publish_Fire();
    publish_Gas();
}
void setup()
{
    Serial.begin(9600); //serial포트를 시작하고
    wifi.init(ssid,password);
    mqtt_init();
    pinMode(buzPin, OUTPUT); //핀의 LED를 빛을 내주는 OUTPUT의 단자로 활용합니다.
    timer.setInterval(2000,publish);
}

void loop(){
    gasCheck();
    fireCheck();
    if(!client.connected()){//재접속 검사
         reconnect();
    }
    timer.run();
}
