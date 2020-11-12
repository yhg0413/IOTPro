#include <PWMServo.h>


PWMServo door;



void setup(){
    Serial.begin(9600);
    pinMode(11, INPUT);
    door.attach(9);
}



void loop(){
    if(digitalRead(11)){
        door.write(100);
    }else{
        door.write(180);
    }
}