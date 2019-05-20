#include <Wire.h>
#include <DFRobot_ADS1115.h>
#include <WiFi.h>
#include <JY901.h>
#include<pt.h>
#include<PT_timer.h>

//------ Configuration -----------
const char* ssid = "icenter02";
const char* password =  "";
const uint16_t port = 8090;
const char * host = "192.168.1.124";
#define PIN_PRESS_UP A0

bool DEBUG = true;

#define MOVE_THRESHOLD 0.6
//
WiFiClient client;
String str_send = "";
int cnt = 0 ;
DFRobot_ADS1115 ads0;
DFRobot_ADS1115 ads1;
double pressure[8];
double press_old[8];
char move_dir = 'n';


void setup(void) 
{
    Serial.begin(115200);
    Serial.println("Sensor initializing...");
    //-------- ADC initialize ---------------
    pinMode(PIN_PRESS_UP,OUTPUT);
    ads0.setAddr_ADS1115(ADS1115_IIC_ADDRESS0);   // 0x48
    ads0.setGain(eGAIN_TWOTHIRDS);   // 2/3x gain
    ads0.setMode(eMODE_SINGLE);       // single-shot mode
    ads0.setRate(eRATE_128);          // 128SPS (default)
    ads0.setOSMode(eOSMODE_SINGLE);   // Set to start a single-conversion
    ads0.init();
    
    ads1.setAddr_ADS1115(ADS1115_IIC_ADDRESS1);   // 0x48
    ads1.setGain(eGAIN_TWOTHIRDS);   // 2/3x gain
    ads1.setMode(eMODE_SINGLE);       // single-shot mode
    ads1.setRate(eRATE_128);          // 128SPS (default)
    ads1.setOSMode(eOSMODE_SINGLE);   // Set to start a single-conversion
    ads1.init();

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

    delay(1000);
    while(!getPress()){
      Serial.println("Can't get press data...");
      delay(2000);
    }
      str_send = "PRE ";
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
    while (!client.connect(host, port)) {
        Serial.print(i);
        Serial.println(" Connection to host failed");
        delay(1000);
        i++;
        if (i==5)return false;
    }
    Serial.print("Send: ");
    Serial.println(str_send);
    client.print(str_send.c_str());
    client.stop();
    return true;
}
#define PRESS_THRESHOLD 200
bool getPress(){
    if(!(ads0.checkADS1115()&&ads1.checkADS1115())){
        if (DEBUG) Serial.println("ERROR: ADS");
        return false;
    }
    digitalWrite(PIN_PRESS_UP,HIGH);
    delay(10);//
    int16_t adc[8];
    for(int i = 0;i<4;i++){
        adc[i] = ads0.readVoltage(i);
        adc[i+4] = ads1.readVoltage(i);
    }    
    for(int i = 0;i<8;i++){
        //TODO the relationship ...
        press_old[i] = pressure[i];
        pressure[i] = adc[i];
    }
    digitalWrite(PIN_PRESS_UP,LOW);
    return true;    
}
float ax,ay,az;
static struct pt pt1;
PT_timer servotimer1;
static int old_time1,need2update; //counter为定时计数器，state为每个灯的状态
static int pt1_move_watcher(struct pt *pt) //线程1，控制灯1
{  
  PT_BEGIN(pt);  //线程开始
  while(1) //每个线程都不会死
  {  
    servotimer1.setTimer(200);//设定跪10秒，这里单位1为一微妙
    PT_WAIT_UNTIL(pt,(servotimer1.Expired()));//当时间溢出，这里是10秒就结束
    //PT_WAIT_UNTIL(pt, millis()-old_time1>200000); //如果时间满了0.2秒，则继续执行，否则记录运行点，退出线程1
    //old_time1=millis(); //计数器置零
    JY901.GetAcc();
    ax = (float)JY901.stcAcc.a[0]/32768*16;
    ay = (float)JY901.stcAcc.a[1]/32768*16;
    //az = (float)JY901.stcAcc.a[2]/32768*16;
    if(ax<MOVE_THRESHOLD&&ay<MOVE_THRESHOLD){//&&az<MOVE_THRESHOLD){
        change_move('n');
    }
    else{
        if(abs(ax)>=abs(ay)){
            if (ax>0)change_move('r');
            else change_move('l');
        }
        else{
            if (ay>0)change_move('u');
            else change_move('d');
        }
    }
  } 
  PT_END(pt); //线程结束
} 
void getMove(){
    //JY901.GetAcc();
    JY901.GetAngle();
    ax = (float)JY901.stcAngle.Angle[0]/32768*16; //stcAcc.a
    ay = (float)JY901.stcAngle.Angle[1]/32768*16;
    az = (float)JY901.stcAngle.Angle[2]/32768*16;
    if(DEBUG) {
      Serial.print("ACC: ");
      Serial.print(ax);
      Serial.print(" ");
      Serial.print(ay);
      Serial.print(" ");
      Serial.println(az);
      }if(abs(ax)<MOVE_THRESHOLD&&abs(ay)<MOVE_THRESHOLD){//&&az<MOVE_THRESHOLD){
        change_move('n');
    }
    else{
        if(abs(ax)>=abs(ay)){
            if (ax>0)change_move('l');
            else change_move('r');
        }
        else{
            if (ay>0)change_move('u');
            else change_move('d');
        }
    }
  }
void change_move(char c){
  if(DEBUG){
  Serial.print("move: ");
  Serial.print(c);
  Serial.print(" old: ");
  Serial.println(move_dir);
  }
    need2update = !(move_dir==c);
    move_dir = c;
}
void loop() 
{
    //pt1_move_watcher(&pt1);
    /*
    getMove();
    if(need2update){ 
        str_send += "DIR "   ;
        str_send += move_dir;
        str_send += " ";
    }*/
    if(getPress()){
      bool flag = false;
      for(int i = 0;i<8;i++){
        if(abs(press_old[i]-pressure[i])>PRESS_THRESHOLD){          
            str_send += "PRE ";
            flag = true;
            break;
            
          }
      }
      if(flag){
      for(int i = 0;i<8;i++){
        Serial.print(pressure[i]);
        Serial.print(" ");
        if(abs(press_old[i]-pressure[i])>PRESS_THRESHOLD){
          str_send += i;
          str_send += " ";
          str_send += pressure[i];
          str_send += " ";
          }
      }
      Serial.println("");
      /*
      for(int i = 0;i <8;i++){        
          str_send += pressure[i];
          str_send += " ";
      }*/
    }  
    }  
    if (str_send!="")    sendMsg(str_send);
    str_send = "";
    delay(10);
}                         
