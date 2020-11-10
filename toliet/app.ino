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
SoftwareSeria softSerial(4, 5); //RX, TX



const char ssid[] = "Campus7_Room3_2.4"; // 네트워크 ssid
const char password[] = "12345678"; // 비밀번호
const char mqtt_server[] = "192.168.0.109";//->서버주소 = 내 pc주소

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
    
    if(strcmp("LED_ON", message)==0){
        digitalWrite(13,HIGH);
    }
    else if(strcmp("LED_OFF", message)==0){
        digitalWrite(13,LOW);
       
    }
    else if(strcmp("WINDOW_OPEN", message)==0){
    }
    else if(strcmp("WINDOW_CLOSE", message)==0){
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
                        
        if(client.connect("IoT3_living")){//ID를 다르게 등록할경우 보통 장치의 시리얼번호를 사용
        //아닐경우 EEPROM에다가 번호등록가능 보통은 시리얼번호 사용
            Serial.println("connected");
            //subscriber로 등록
            //client.subscribe("toliet/+/info",1); //구독 신청
        }
        else{
            Serial.print("failed, rc=");
            Serial.print(client.state());
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
void publish(){
    char msg[50];
    StaticJsonBuffer<50> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();

    root["waterState"] = waterState;
    root["PIDState"] = PIDState;
    root["BiveState"] = BiveState;

    Serial.print("Json data : ");
    
    root.printTo(msg);
    Serial.println(msg);
    //토픽 발행
    client.publish("toliet/", msg);
}


void work(){
    int i = 0;
    int P = 0;
    int bive =0;
    BiveState = 0;
    for(i = 0; i< 100; i++){
      P = digitalRead(bivepin);
      bive += P;
      if(bive-1 == i){
        BiveState = 1;
      } 
    }
}
void check(){
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
    pinMode(bivepin,INPUT);
    pinMode(PIDpin,INPUT);
    pinMode(FLOWSENSORPIN,INPUT);
    digitalWrite(FLOWSENSORPIN,1);
    useInterrupt(true);
    timer.setInterval(2000,publish);
    timer.setInterval(5000,check);
}


void loop(){
    if(!client.connected()){//재접속 검사
         reconnect();
    }
    client.loop();//메시지가 있는지 검사 수신이 있는가
    PIDState=digitalRead(PIDpin);
    timer.run();
}
