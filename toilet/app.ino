#include <PubSubClient.h>
#include <SimpleTimer.h>
#include <WifiUtil.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>

#define PIRpin 7
#define vibpin 8
#define FLOWSENSORPIN 9   
WifiUtil wifi(4,5);

char waterState = 0;
int vibState = 0;
int PIRState = 0;
int I = 0;
int time1 = 0;
int time2 = 0;
SimpleTimer timer;
SoftwareSerial softSerial(4, 5); //RX, TX



const char ssid[] = "Campus7_Room3_2.4"; // 네트워크 ssid
const char password[] = "12345678"; // 비밀번호
// const char mqtt_server[] = "192.168.0.109";//->서버주소 = 내 pc주소
const char mqtt_server[] = "192.168.0.138";
//MQTT용 WiFi 클라이언트 객체초기화
WiFiEspClient espClient;
PubSubClient client(espClient);

                  // 핀 번호 설정
volatile uint16_t pulses = 0;               // 데이터 유형 설정
volatile uint8_t lastflowpinstate;
volatile uint32_t lastflowratetimer = 0;
volatile float flowrate;
volatile uint16_t pulses2 = 0;
SIGNAL(TIMER0_COMPA_vect) {
  uint8_t x = digitalRead(FLOWSENSORPIN);   // 유량측정센서 값을 디지털로 읽음
  if (x == lastflowpinstate) {
    lastflowratetimer++;
    return;
  }
  if (x == HIGH) {                          // x값이 들어오면 pulse값을 1 증가시킴
    pulses++;
  }

  lastflowpinstate = x;
  flowrate = 1000.0;
  flowrate /= lastflowratetimer;
  lastflowratetimer = 0;
}
void useInterrupt(boolean v) {             // bool값에 따른 출력 설정
  if (v) {
    OCR0A = 0xAF;
    TIMSK0 |= _BV(OCIE0A);
  } else {
    TIMSK0 &= ~_BV(OCIE0A);
  }
}

void callback(char* topic, byte* payload, unsigned int length){
    /*payload[length] = NULL;
    char *message = payload;*/
    char message[64];
    memcpy(message, payload, length);
    message[length] = NULL;
    //strcmp c언어떄 문자열배열을 비교할때 사용
    //0이 리턴되면 두 문자열이 같다는 뜻
    
    //Serial.print(topic);
    //Serial.print(" : ");
    //Serial.println(message);
    
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
                        
        if(client.connect("IoT3_toliet")){//ID를 다르게 등록할경우 보통 장치의 시리얼번호를 사용
        //아닐경우 EEPROM에다가 번호등록가능 보통은 시리얼번호 사용
            Serial.println("connected");
            //subscriber로 등록
            //client.subscribe("toliet/+/info",1); //구독 신청
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
void publish_water(){
    char msg[15];
    StaticJsonBuffer<15> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();

    root["wat_s"] = waterState;

    //Serial.print("Json data : ");
    
    root.printTo(msg);
    //Serial.println(msg);
    //토픽 발행
    client.publish("iot3/toilet/water", msg);
}
void publish_PIR(){
    char msg[15];
    int A=0;
    StaticJsonBuffer<15> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    A = (PIRState!=0);
    root["pir_s"] = A;

    //Serial.print("Json data : ");
    
    root.printTo(msg);
    //Serial.println(msg);
    //토픽 발행
    client.publish("iot3/toilet/pir", msg);
}
void publish_vib(){
    char msg[15];
    int A=0;
    StaticJsonBuffer<15> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();
    A = (vibState!=0);
    root["vib_s"] = A;

    //Serial.print("Json data : ");
    
    root.printTo(msg);
    //Serial.println(msg);
    //토픽 발행
    client.publish("iot3/toilet/vib", msg);
}

void publish(){
  I++;
  if(I==1){
    publish_PIR();
  }
  else if(I==2){
    publish_vib();
  }
  else if(I==3){
    publish_water();
    I=0;
  }
}

void check(){
  
  if(time1){time1 -=1;}
  else vibState=0;

  if(time2){time2 -=1;}
  else {PIRState=0;}

  if (pulses2 != pulses){
      pulses2 = pulses;
      waterState = 1;
    }else{
      pulses = 0;
      pulses2 = 0;
      waterState = 0;
    }
}

void setup(){
    Serial.begin(9600);
    wifi.init(ssid,password);
    mqtt_init();
    pinMode(13,OUTPUT);
    pinMode(vibpin,INPUT);
    pinMode(PIRpin,INPUT);
    pinMode(FLOWSENSORPIN,INPUT);
    digitalWrite(FLOWSENSORPIN,1);
    useInterrupt(true);
    timer.setInterval(3000,publish);
    timer.setInterval(1951,check);
}


void loop(){
    
    if(digitalRead(vibpin)){vibState=1;time1=10;}
    if(digitalRead(PIRpin)){PIRState=1; time2=10;}
    if(!client.connected()){//재접속 검사
         reconnect();
    }
    //client.loop();//메시지가 있는지 검사 수신이 있는가
    
    timer.run();
}
