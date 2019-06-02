#include <Wire.h>
#include <WiFi.h>
#include <JY901.h>

//------ Configuration -----------
const char* ssid = "icenter02";
const char* password =  "";
const uint16_t port = 8090;
const char * host = "192.168.1.108";
#define PIN_PRESS_UP 2
#define PIN_DIG_0 3
#define PIN_DIG_1 4
#define PIN_DIG_2 7
#define PIN_ANG_V A3
#define STRING_LEFT_OR_RIGHT "L "


bool DEBUG = true;

#define MOVE_THRESHOLD 0.6
//
WiFiClient client;
String str_send = STRING_LEFT_OR_RIGHT;
int cnt = 0 ;
double pressure[8];
double press_old[8];


void setup(void) 
{
    Serial.begin(115200);
    Serial.println("Sensor initializing...");
    //-------- pressure resisdence initialize ---------------
    pinMode(PIN_PRESS_UP,OUTPUT);

    for(int i = 0;i<8;i++){
       press_old[i] = 0;
    }
    //--------- IMU initialize -------------
    JY901.StartIIC();

    
    Serial.println("Network initializing...");
    //--------- Socket initialize -------------        
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.println(".");
    }
    Serial.print("WiFi connected with IP: ");
    Serial.println(WiFi.localIP());

    while(!getPress()){
      Serial.println("Can't get press data...");
      delay(1000);
    }
      str_send += "PRE ";
      for(int i = 0;i <8;i++){    
          str_send += i;
          str_send += " ";    
          str_send += pressure[i];
          str_send += " ";
      }
    sendMsg(str_send);  
    str_send = ""; 
}
bool sendMsg(String str){
    int i = 0;
    // Note that a client is considered connected 
    // if the connection has been closed 
    // but there is still unread data.
    while (client.available()) {
        char c = client.read();
        Serial.print(c);
    }
    if(!client.connected())
        Serial.println("");
        while (!client.connect(host, port)) {
            Serial.print(i);
            Serial.println(" Connection to host failed");
            delay(1000);
            i++;
            if (i==5)return false;
        }
    //Serial.print("Send: ");
    //Serial.println(str_send);
    client.print(str_send.c_str());
    //client.stop();
    return true;
}
#define PRESS_THRESHOLD 2
bool getPress(){    
    digitalWrite(PIN_PRESS_UP,HIGH);
    delay(1);//
    int16_t adc[8];
    for(int i =0;i<8;i++){
        //
        int sg = i%2;
        int m = (i-sg)/2;
        if(sg>0)digitalWrite(PIN_DIG_0,HIGH);
        else digitalWrite(PIN_DIG_0,LOW);
        sg = m%2;
        m = (m-sg)/2;
        if(sg>0)digitalWrite(PIN_DIG_1,HIGH);
        else digitalWrite(PIN_DIG_1,LOW);        
        sg = m%2;
        m = (m-sg)/2;
        if(sg>0)digitalWrite(PIN_DIG_2,HIGH);
        else digitalWrite(PIN_DIG_2,LOW);
        //
        float value = analogRead(PIN_ANG_V);
        if(value==0)return false; //TODO: why?
        adc[i] = value;
    }
    for(int i = 0;i<8;i++){
        press_old[i] = pressure[i];
        pressure[i] = adc[i];
    }
    Serial.print(pressure[0]+pressure[1]+pressure[5]);
    Serial.print(" ");
    Serial.print(pressure[6]+pressure[7]);
    Serial.print(" ");
    Serial.println(pressure[2]+pressure[3]+pressure[4]);
    digitalWrite(PIN_PRESS_UP,LOW);
    return true;    
}

float angles[3];
void getAngle(){
    //JY901.GetAcc();
    JY901.GetAngle();
    angles[0] = (float)JY901.stcAngle.Angle[0]/32768*16; //stcAcc.a
    angles[1] = (float)JY901.stcAngle.Angle[1]/32768*16;
    angles[2] = (float)JY901.stcAngle.Angle[2]/32768*16;
    if(DEBUG) {
        Serial.print("ANG: ");
        Serial.print(angles[0]);
        Serial.print(" ");
        Serial.print(angles[1]);
        Serial.print(" ");
        Serial.println(angles[2]);
    }      
}
  
void loop() 
{
    bool have_msg = false;
    //timmer
    //int t_b = millis();
    //Serial.println(millis()-t_b);
    if(getPress()){
        for(int i = 0;i<8;i++){
        // only update changed data
            if(abs(press_old[i]-pressure[i])>PRESS_THRESHOLD){  
                if (!have_msg) str_send += "PRE ";
                have_msg = true;
                //Serial.print(pressure[i]);
                //Serial.print(" ");
                str_send += i;
                str_send += " ";
                str_send += pressure[i];
                str_send += " ";     
            }
        }
        /* let PC to deal with data
        for(int i = 0;i <8;i++){        
            str_send += pressure[i];
            str_send += " ";
        }*/
    }   

    getAngle();
    str_send += "ANG ";
    have_msg = true;
    for(int i = 0;i<3;i++){
        str_send += angles[i];
        str_send += " ";
    }

    if (have_msg){
      sendMsg(str_send);
    }
    str_send = STRING_LEFT_OR_RIGHT;
    //delay(1);
}                         

