#include <Wire.h>
#include <WiFi.h>
#include <JY901.h>

//------ Configuration -----------
const char* ssid = "ljftest";//"icenter02";
const char* password =  "testpass";
const uint16_t port = 13579;
const char * host = "192.168.43.194";//"192.168.1.191";

#define PIN_PRESS_UP D7
#define PIN_DIG_0 D2
#define PIN_DIG_1 D3
#define PIN_DIG_2 D4
#define PIN_ANG_V A0
#define STRING_LEFT_OR_RIGHT "L "

bool DEBUG = true;

#define MOVE_THRESHOLD 0.6

WiFiClient client;
String str_send = STRING_LEFT_OR_RIGHT;
double pressure[8];
double press_old[8];

void setup(void) 
{
    Serial.begin(115200);
    Serial.println("Sensor initializing...");
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, HIGH);
    //-------- pressure resisdence initialize ---------------
    pinMode(PIN_PRESS_UP,OUTPUT);
    pinMode(PIN_DIG_0,OUTPUT);
    pinMode(PIN_DIG_1,OUTPUT);
    pinMode(PIN_DIG_2,OUTPUT);
    Serial.println("Debug 1");
    pinMode(PIN_ANG_V,INPUT);

    for(int i = 0;i<8;i++){
       press_old[i] = 0;
    }
    Serial.println("Debug 2");
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
    
    for(int i=0;i<10;i++){      
      digitalWrite(LED_BUILTIN, LOW);
      delay(100);
      digitalWrite(LED_BUILTIN, HIGH);
      delay(100);
      }

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
    
    for(int i=0;i<3;i++){      
      digitalWrite(LED_BUILTIN, LOW);
      delay(600);
      digitalWrite(LED_BUILTIN, HIGH);
      delay(600);
      }
      
    str_send = STRING_LEFT_OR_RIGHT; 
    digitalWrite(LED_BUILTIN,LOW);
}
bool sendMsg(String str){  
    // Note that a client is considered connected 
    // if the connection has been closed 
    // but there is still unread data.
/*    while (client.available()) {
        char c = client.read();
        Serial.print(c);
    }*/
    //if(!client.connected()){
      digitalWrite(LED_BUILTIN, LOW);
       // Serial.println("");          
      int i = 0;
      while (!client.connect(host, port)) {
          Serial.print(i+1);
          Serial.println("th Connection to host failed");
          delay(100);
          i++;
          if (i==5)return false;
      }
   // }
   // Serial.print("Send: ");
   // Serial.println(str_send);
    client.print(str_send.c_str());
    client.stop();  
      digitalWrite(LED_BUILTIN, HIGH);  
    return true;
}
#define PRESS_THRESHOLD 2
bool getPress(){    
    digitalWrite(PIN_PRESS_UP,HIGH);
    delay(1);//
    int16_t adc[8];
    float value = 0;
    for(int i =0;i<8;i++){
        //
        int sg = i%2;
        int m = (i-sg)/2;
        if(sg>0){digitalWrite(PIN_DIG_0,HIGH);}
        else digitalWrite(PIN_DIG_0,LOW);
        sg = m%2;
        m = (m-sg)/2;
        if(sg>0){digitalWrite(PIN_DIG_1,HIGH);}
        else digitalWrite(PIN_DIG_1,LOW);

        sg = m%2;
        m = (m-sg)/2;
        if(sg>0){digitalWrite(PIN_DIG_2,HIGH);}
        else digitalWrite(PIN_DIG_2,LOW);
        
        delay(10);
        value = analogRead(PIN_ANG_V);  
        adc[i] = value;
    }
        if(value==0){
          return false;
         }//TODO: resistance or chip has problems
    for(int i = 0;i<8;i++){
        press_old[i] = pressure[i];
        pressure[i] = adc[i];
        Serial.print(pressure[i]);
        Serial.print(" ");
    }
    Serial.println("");
    /*
    Serial.print(pressure[0]+pressure[1]+pressure[5]);
    Serial.print(" ");
    Serial.print(pressure[6]+pressure[7]);
    Serial.print(" ");
    Serial.println(pressure[2]+pressure[3]+pressure[4]);
    */
    digitalWrite(PIN_PRESS_UP,LOW);
    return true;    
}

float angles[3];
bool getAngle(){
    //JY901.GetAcc();
    // Attention! if IIC get something wrong, 
    // it will keep waiting for data
    // That will make program stop at here.
    // So I modified the lib
    bool sta = JY901.GetAngle();
    angles[0] = (float)JY901.stcAngle.Angle[0]/32768*16; //stcAcc.a
    angles[1] = (float)JY901.stcAngle.Angle[1]/32768*16;
    angles[2] = (float)JY901.stcAngle.Angle[2]/32768*16;
    if(0) {
        Serial.print("ANG: ");
        Serial.print(angles[0]);
        Serial.print(" ");
        Serial.print(angles[1]);
        Serial.print(" ");
        Serial.println(angles[2]);
    }      
    return sta;
}


void loop() 
{
    digitalWrite(LED_BUILTIN, HIGH);
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

    if(getAngle()){        
      str_send += "ANG ";
      have_msg = true;
      for(int i = 0;i<3;i++){
          str_send += angles[i];
          str_send += " ";
      }
    }

    if (have_msg){
      sendMsg(str_send);
    }
    str_send = STRING_LEFT_OR_RIGHT;
    digitalWrite(LED_BUILTIN, LOW);
    delay(1);
}                         

