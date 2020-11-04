#include <PubSubClient.h>
#include <SimpleTimer.h>
#include <WifiUtil.h>
#include <SoftwareSerial.h>
#include <AnalogSensor.h>
#include <PWMServo.h>

char val = 0;
WifiUtil wifi(4,5);

char windowState = 0;

PWMServo servoMotor;
SoftwareSerial softSerial(4, 5); //RX, TX

const char ssid[] = "U+Net5353"; // 네트워크 ssid
const char password[] = "DDB9008923"; // 비밀번호
const char mqtt_server[] = "192.168.219.100";//->서버주소 = 내 pc주소

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
    
    if(strcmp("LED_ON", message)==0){
        digitalWrite(13,HIGH);
    }
    else if(strcmp("LED_OFF", message)==0){
        digitalWrite(13,LOW);
    }
    else if(strcmp("WINDOW_OPEN", message)==0){
        servoMotor.write(90);
    }
    else if(strcmp("WINDOW_CLOSE", message)==0){
        servoMotor.write(0);
    }
    // Serial.print(topic);
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
                        
        if(client.connect("IOT3_living")){//ID를 다르게 등록할경우 보통 장치의 시리얼번호를 사용
        //아닐경우 EEPROM에다가 번호등록가능 보통은 시리얼번호 사용
            Serial.println("connected");
            //subscriber로 등록
            client.subscribe("living/#"); //구독 신청
        }
        else{
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" try again in 5 seconds");
            delay(5000);
        }
    }
}
void publish(){
    int state = !digitalRead(13);
    char message[10];
    sprintf(message, "%d", state);

    //토픽 발행
    client.publish("living/LED/info", message);
}


SimpleTimer timer;


void test(){
    
}
void setup(){
    Serial.begin(9600);
    wifi.init(ssid,password);
    mqtt_init();
    servoMotor.attach(6);
    servoMotor.write(180); 
    pinMode(13,OUTPUT);

    timer.setInterval(2000,publish);
}
void loop(){
    if(!client.connected()){//재접속 검사
         reconnect();
    }
    client.loop();//메시지가 있는지 검사 수신이 있는가
    timer.run();
}