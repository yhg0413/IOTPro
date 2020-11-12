#include <PubSubClient.h>
#include <SimpleTimer.h>
#include <WifiUtil.h>
#include <AltSoftSerial.h>
#include <PWMServo.h>
#include <ArduinoJson.h>


WifiUtil wifi(4,5);


SimpleTimer timer;
AltSoftSerial softSerial(4, 5); //RX, TX
int I = 1;
const char ssid[] = "Campus7_Room3_2.4"; // 네트워크 ssid
const char password[] = "12345678"; // 비밀번호
// const char mqtt_server[] = "192.168.0.109";//->서버주소 = 내 pc주소
const char mqtt_server[] = "192.168.0.138";

//MQTT용 WiFi 클라이언트 객체초기화
WiFiEspClient espClient;
PubSubClient client(espClient);

void callback(char* topic, byte* payload, unsigned int length){
    /*payload[length] = NULL;
    char *message = payload;*/
    char message[64];
    memcpy(message, payload, length);
    message[length] = NULL;
    //strcmp c언어떄 문자열배열을 비교할때 사용
    //0이 리턴되면 두 문자열이 같다는 뜻
    
    // if(strcmp("LED_ON", message)==0){
    //     digitalWrite(13,HIGH);
    // }

    //Serial.print(topic);
   // Serial.print(" : ");
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
                        
        if(client.connect("IoT3_inner")){//ID를 다르게 등록할경우 보통 장치의 시리얼번호를 사용
        //아닐경우 EEPROM에다가 번호등록가능 보통은 시리얼번호 사용
            Serial.println("connected");
            //subscriber로 등록
        //    client.subscribe("inner/+/info",1); //구독 신청
        }
        else{
            //Serial.print("failed, rc=");
            //Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            delay(5000);
        }
    }
}
// void publish_Light(){
//     int state = !digitalRead(13);
//     char message[10];
//     sprintf(message, "%d", state);

//     //토픽 발행
//     client.publish("IoT3/home/living/Light/info", message);
// }
// void publish_Window(){
//     char message[10];
//     sprintf(message, "%d", windowState);
//     client.publish("IoT3/home/living/Window/ifno", message);
// }
void publish_Arduino_Rain(){
    char msg[15];

    sprintf(msg, "%d", digitalRead(10));
    
    Serial.println(msg);
    //토픽 발행
    client.publish("iot3/inner/ArduRain/info", msg);
}
void publish_Rain(){
    char msg[15];
    StaticJsonBuffer<15> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();

    root["rs"] = digitalRead(10);

    //Serial.print("Json data : ");
    
    root.printTo(msg);
    //Serial.println(msg);
    //토픽 발행
    client.publish("iot3/inner/rain", msg);
}

void publish(){
    if(I==1)publish_Arduino_Rain();
    else if(I==2){publish_Rain();I=0;}
    I++;
}

void setup(){
    Serial.begin(9600);
    wifi.init(ssid,password);
    mqtt_init();
    pinMode(13,OUTPUT);
    pinMode(10,INPUT);
    timer.setInterval(1500,publish);
}


void loop(){
    if(!client.connected()){//재접속 검사
         reconnect();
    }
    timer.run();
}