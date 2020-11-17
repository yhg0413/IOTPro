#include <PubSubClient.h>
#include <WifiUtil.h>
#include <AltSoftSerial.h>
#include <ArduinoJson.h>
#include <MiniCom.h>
#include <DHT.h>

#define DHTPIN 3
#define DHTTYPE DHT11
#define DUST_PIN 9

int I=1;
unsigned long lowpulseoccupancy = 0;
float ratio = 0;
float humi = 0;
float temp = 0;
int dustDensity =0;
int dust_level = 0;
WifiUtil wifi(4,5);

AltSoftSerial softSerial(4, 5); //RX, TX

const char ssid[] = "awon"; // 네트워크 ssid
const char password[] = "whddkwhddk"; // 비밀번호
// const char mqtt_server[] = "192.168.0.109";//->서버주소 = 내 pc주소
const char mqtt_server[] = "172.20.10.8";

//MQTT용 WiFi 클라이언트 객체초기화
WiFiEspClient espClient;
PubSubClient client(espClient);
DHT dht(DHTPIN, DHTTYPE);
MiniCom com;

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
    // else if(strcmp("LED_OFF", message)==0){
    //     digitalWrite(13,LOW);
    // }
//    Serial.print(topic);
//    Serial.print(" : ");
    Serial.println(message);
}

void mqtt_init(){
    client.setServer(mqtt_server, 1883);
    //subscriber인경우 메시지 수순시 호출할 콜백 함수 등록
    client.setCallback(callback);
}

void reconnect(){
    while(!client.connected()){
//        Serial.print("Attempting MQTT connection...");
                        
        if(client.connect("IoT3_living")){//ID를 다르게 등록할경우 보통 장치의 시리얼번호를 사용
        //아닐경우 EEPROM에다가 번호등록가능 보통은 시리얼번호 사용
            Serial.println("connected");
            //subscriber로 등록
            //client.subscribe("living/#"); //구독 신청
        }
        else{
//            Serial.print("failed, rc=");
//            Serial.print(client.state());
            Serial.print(" try again in 5 seconds");
            delay(5000);
        }
    }
}

void publish_DHT(){
    char msg[25];
    StaticJsonBuffer<25> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    root["hu"]= humi;
    root["te"] = temp;

    root.printTo(msg);
    //토픽 발행
    client.publish("iot3/living/dht", msg);
}

void LCD(){
  com.print(0,"humi",humi,"temp",temp);
  com.print(1,"d_D",dustDensity,"d_l",dust_level);
}
void publish_dust(){
    char msg[25];
    StaticJsonBuffer<25> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    root["dd"] = dustDensity;
    root["dl"] = dust_level;

    root.printTo(msg);
    //토픽 발행
    client.publish("iot3/living/dust",msg);
}

// void publish2(){
//   String value = "\"h\": " + String(humi) ;
//   String value2 = ", \"t\": " + String(temp) ;
 
//  // Add both value together to send as one string. 
//   value = value + value2;
  
  
//   // This sends off your payload. 
//   String payload = "{" + value +"}";
//   payload.toCharArray(data, (payload.length() + 1));
//   client.publish("inner/DHT/info", data);
// }
void setup() {
  com.init();
  dht.begin();
  wifi.init(ssid,password);
  mqtt_init();
  pinMode(DUST_PIN, INPUT);
  pinMode(13,OUTPUT);
  
  com.setInterval(2000,publish);
  
}
void loop() {
  mainwork();
  if(!client.connected()){//재접속 검사
         reconnect();
  }
  com.run();
  
}

void mainwork(){
  work();
  dust_check();
  LCD();
}

void publish(){
  if(I==1){publish_DHT();I=2;}
  else if(I==2){publish_dust();I=1;}
    //else if(I==3){LCD();I==1;}
}

void work(){
  float h = 0;
  float t = 0;
  h = dht.readHumidity(); // 변수 h에 습도 값을 저장
  t = dht.readTemperature(); // 변수 t에 온도 값을 저장
  if (isnan(h)==0){ humi = h; }
  if (isnan(t)==0){temp = t; }
}

void dust_check() {
  unsigned long duration = pulseIn(DUST_PIN, LOW);
  if(duration){
    lowpulseoccupancy += duration;
    char DustCalculate_RUN = false;
    char DustCalculate_Done = true;
    // Integer percentage 0=>100
    ratio = lowpulseoccupancy / (3000 * 10.0);
    float concentration = 1.1 * pow(ratio, 3) - 3.8 * pow(ratio, 2) +520 * ratio + 0.62; // using spec sheet curve
    if(concentration * 100 / 13000 != 0){
      dustDensity = concentration * 100 / 13000;
    }
  }
  lowpulseoccupancy = 0;
  if(dustDensity > 150) dust_level = 3;
  else if(dustDensity > 80) dust_level = 2;
  else if(dustDensity > 30) dust_level = 1;
  else dust_level = 0;
}
