#include <Arduino.h>
#include <SoftwareSerial.h>
#include <avr/wdt.h>
#define SSID "Localization"
#define KEY "L0calSystem"
 
 
SoftwareSerial wiflyUart(9, 10); // create a serial connection to the WiFi shield TX and RX pins.
String macs; 
int ledPin = 7;
int WDcount;

int respreceive(uint8_t *buf, int timeout) //function that reads bytes from UART
{  
    int read_bytes = 0;
    int ret;
    unsigned long end_millis;

    while (read_bytes < 1) {
        end_millis = millis() + timeout;
        do {
            ret = wiflyUart.read();
            if (ret >= 0) {
                break;
            }
        } while (millis() < end_millis);

        if (ret < 0) {
            return read_bytes;
        }
        buf[read_bytes] = (char)ret;
        read_bytes++;
    }

    return read_bytes;
}

String cmd_manager(String cmd) //function that receives string
{
  char character;
  String ret="";
  wiflyUart.flush();
  wiflyUart.print(cmd);
 
  while (respreceive((uint8_t *)&character,1000) > 0) 
         ret += character;
          
  return ret;
  ret ="";
    
}

void ok_enforcer(String cmd) // function that makes sure that AOK is received from RN-171
{
  String okstring="";
  wdt_enable(WDTO_8S);
  while (okstring.indexOf("AOK")<0) 
    {
     okstring = cmd_manager(cmd);
     delay(100);
    }
 wdt_disable();
 okstring = "";
}

void cmd_enforcer() // ensures RN-171 in command mode
{
  String okstring="";
  okstring = cmd_manager("show net\r");
  wdt_enable(WDTO_8S);
  
  while (okstring.indexOf("4.00")<0)
    {
     wiflyUart.print("\r\n");
     okstring = cmd_manager("$$$");
     wiflyUart.print("\r\n");
     wiflyUart.print("\r");
    }   
  
  wdt_disable();
  okstring ="";
}

void port() // setting the RN-171 IP
{
    ok_enforcer("set ip proto 18\r");
    delay(50);
    ok_enforcer("set ip address 0\r");
    delay(50);
    ok_enforcer("set ip remote 80\r");
    delay(50);
    ok_enforcer("set com remote 0\r");
    delay(50);
    ok_enforcer("set com idle 10\r");
    delay(50);
    wiflyUart.print("save\r");
    
}

void doscan() // scanning function that reads nearby wifi access points and then sends data to the server
{
  digitalWrite(ledPin, HIGH);
  char character;
  int var=0;
  int count =0;
  String ch="";
  String rssi="";
  String resp="";
  resp.reserve(700);
  ch.reserve(5);
  rssi.reserve(5);
  
  wiflyUart.flush();
  wiflyUart.print("scan\r");
  while (respreceive((uint8_t *)&character,4000) > 0 && var!=13) {  
    resp += character;
    if (character=='\n')
     var++;}
    
  var=0;
     
  for (int i=0; resp[i]!=',';i++)
    var++;
     
    
  resp.remove(0,var-2);
  var=0;
  
  for (int i=0; i<resp.length(); i++){ 
    if (resp[i]=='\n')
       var++;
    if (var==10) {
       resp.remove(i+1);
       break;}
    } 
    
  for (int i=0; i<resp.length(); i++)
      { if (resp[i]==','){
            count++; 
            
            // constant channel length , cause not exceeding 23
            if((count%8)==1)
              ch += resp.substring(i+1,i+3);
         
            // not constant RSSI
            if((count%8)==2)
               var=i;
            
            if ((count%8)==3)
              rssi += resp.substring(var,i);
                 
            //constant mac address always
            if ((count%8)==7)
             { 
               macs += resp.substring(i+1,i+19);
               macs += ch;
               macs += rssi;
               macs += '_';
               ch = "";
               rssi = "";
             }
           }
        } 
    var=0;
    count=0;
    macs+=" \n\n";
    resp="";
    
    //sending formatted data to server
    wiflyUart.print("open localization.kaust.edu.sa 80\r");
    delay (1000);
    wiflyUart.print(macs);
    Serial.print(macs);
    macs ="GET /Test/wtrack.php?ID=smart&W=";
    wiflyUart.flush(); //important needed to escape v1
    wiflyUart.print("$$$");//important needed to escape v1
    digitalWrite(ledPin, LOW);}

void wifi()// checking and joining wifi network
{
  wiflyUart.flush();// flushing buffer
  String okstring="";
  okstring = cmd_manager("show net\r");
  delay(50);
  
  //connectincting to wifi if not connected yet
  if (okstring.indexOf(SSID)<0)
   {
    wiflyUart.print("set w p ");
    wiflyUart.print(KEY);
    wiflyUart.print("\r");
    wiflyUart.print("join ");
    wiflyUart.print(SSID);
    wiflyUart.print("\r");
    delay(500);
    cmd_enforcer(); }
  
  okstring= "";
  okstring = cmd_manager("show net\r");
  Serial.println("Here");
  while (okstring.indexOf("Auth=Fail")>0) {
    wiflyUart.print("set w p ");
    wiflyUart.print(KEY);
    wiflyUart.print("\r");
    wiflyUart.print("join ");
    wiflyUart.print(SSID);
    wiflyUart.print("\r");
    delay(500);
    cmd_enforcer(); }
      }

void authorization(){// checking is association with wifi is ok  
  wiflyUart.flush();// flushing buffer
  String okstring="";
  okstring = cmd_manager("show net\r");
  if (okstring.indexOf("Assoc=FAIL")>0)
      {
        wiflyUart.print("reboot\r");
        delay(100);
        cmd_enforcer();
        wifi();
       }
 
  okstring = "";}

  
  void setup() 
  {
  wdt_disable();
  
  pinMode(ledPin, OUTPUT);  //ledblinking
  int k;
  for (k = 1; k <= 3; k = k + 1) {
        digitalWrite(ledPin, HIGH);
        delay(250L);
        digitalWrite(ledPin, LOW);
        delay(250L);}
 
  wiflyUart.begin(9600); // start the serial connection to the RN-171 module
  macs ="GET /Test/wtrack.php?ID=smart&W=";
  macs.reserve(300);
  cmd_enforcer();
  wifi();
  delay(300);
  authorization();
  delay(200);
  port();
  delay(1000);  
  WDcount = 0;
  wiflyUart.flush(); }
 
  void loop() 
  { 
    wifi();
    delay(200);
    doscan();
    delay(200);
    cmd_enforcer(); 


    if (WDcount==1){  // restarting the device 
    wiflyUart.print("reboot\r");
    wdt_enable(WDTO_1S);
    delay(2000);
    }
    delay (1000);
    authorization();
    WDcount++; }
