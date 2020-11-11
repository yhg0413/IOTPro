#include <PubSubClient.h>
#include <SimpleTimer.h>
#include <WifiUtil.h>
#include <AltSoftSerial.h>
#include <PWMServo.h>
#include <ArduinoJson.h>


WifiUtil wifi(4,5);

char innerwindowState = 0;
char livingwindowState = 0;
char doorState = 0;
int LEDState =0 ;

SimpleTimer timer;
PWMServo innerWindow;
PWMServo livingWindow;
PWMServo door;
AltSoftSerial softSerial(4, 5); //RX, TX
int I = 0;
int start = 0;
const char ssid[] = "Campus7_Room3_2.4"; // 네트워크 ssid
const char password[] = "12345678"; // 비밀번호
const char mqtt_server[] = "192.168.0.109";//->서버주소 = 내 pc주소

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
    
    // if(strcmp("WINDOW_OPEN", message)==0){
    //     if(strcmp("iot3/inner/Window/info/",topic)==0){innerWindow.write(180);I=1;innerwindowState = 1;}
    //     else if(strcmp("iot3/living/Window/info/",topic)==0){livingWindow.write(180);I=2;livingwindowState = 1;}
    //     else if(strcmp("iot3/door/info/",topic)==0){door.write(180);I=3;doorState = 1;}
    // }
    // else if(strcmp("WINDOW_CLOSE", message)==0){
    //     if(strcmp("iot3/inner/Window/info/",topic)==0){innerWindow.write(100);I=1;innerwindowState = 0;}
    //     else if(strcmp("iot3/living/Window/info/",topic)==0){livingWindow.write(100);I=2;livingwindowState = 0;}
    //     else if(strcmp("iot3/door/info/",topic)==0){door.write(100);I=3;doorState = 0;}   
    // }
    if(strcmp("All_State", message)==0){
        start = 0;
    }
    else if(strcmp("iot3/inner/ArduRain/info/", topic)==0){
        if(strcmp("1", message)==0){
            innerWindow.write(100);
            livingWindow.write(100);
            innerwindowState=0;
            livingwindowState = 0;
            start = 0;
        }
    }
    else if(strcmp("iot3/inner/Led/info/",topic)==0){
        analogWrite(6,atoi(message));
        I=4;
        LEDState = (atoi(message)!=0);
    }
    else if(strcmp("iot3/inner/Window/info/",topic)==0){
        innerWindow.write(map(atoi(message),0,255,100,180));
        I=1;
        innerwindowState = (atoi(message)!=0);
    }
    else if(strcmp("iot3/living/Window/info/",topic)==0){
        livingWindow.write(map(atoi(message),0,255,100,180));
        I=2;
        livingwindowState = (atoi(message)!=0);
    }
    else if(strcmp("iot3/door/info/",topic)==0){
        door.write(map(atoi(message),0,255,100,180));
        I=3;
        doorState = (atoi(message)!=0);
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
                        
        if(client.connect("IoT3_input_con")){//ID를 다르게 등록할경우 보통 장치의 시리얼번호를 사용
        //아닐경우 EEPROM에다가 번호등록가능 보통은 시리얼번호 사용
            Serial.println("connected");
            //subscriber로 등록
            client.subscribe("iot3/+/+/info/",1); //구독 신청
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
void publish_Window(){
    char msg[15];
    StaticJsonBuffer<15> jsonBuffer;
    JsonObject& root = jsonBuffer.createObject();

    if(I==1)root["WS"] = innerwindowState;
    else if(I==2)root["WS"] = livingwindowState;
    else if(I==3)root["WS"] = doorState;
    else if(I==4)root["LED"] = LEDState;
    Serial.print("Json data : ");
    
    root.printTo(msg);
    //Serial.println(msg);
    //토픽 발행
    if(I==1)client.publish("iot3/inner/Window/", msg);
    else if(I==2)client.publish("iot3/living/Window/", msg);
    else if(I==3)client.publish("iot3/door/", msg);
    else if(I==4)client.publish("iot3/inner/Led/", msg);
}


void setup(){
    Serial.begin(9600);
    wifi.init(ssid,password);
    mqtt_init();
    pinMode(6,OUTPUT);
    innerWindow.attach(9);
    livingWindow.attach(10);
    door.attach(11);
    innerWindow.write(100);
    livingWindow.write(100);
    door.write(100);
    //timer.setInterval(1500,add);
}


void loop(){
    
    if(!client.connected()){//재접속 검사
         reconnect();
    }
    client.loop();//메시지가 있는지 검사 수신이 있는가
    if(start==0){
        I++;
        publish_Window(); 
        if(I==4){
            start =1;
            I = 0;
        }
    }
    else{
        if(I==1){publish_Window(); I=0;}
        else if(I==2){publish_Window(); I=0;}
        else if(I==3){publish_Window(); I=0;}
        else if(I==4){publish_Window(); I=0;}
    }
    
}