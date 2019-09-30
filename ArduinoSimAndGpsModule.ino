#include <SoftwareSerial.h>
#include <TinyGPS++.h>
#include <EEPROM.h>


SoftwareSerial gsm(10,11);
SoftwareSerial gps(5,6);
String buffer;
TinyGPSPlus tinygps;
String gpsdata = "";
char Buff[80];  
unsigned char BuffIndex;
char bike_status = '1';
int bike_status_address = 10;
void setup(){
  EEPROM.write(bike_status_address,bike_status);
  Serial.begin(9600);
  gps.begin(9600);
  gsm.begin(38400);
  Serial.println("Starting");
  //initGsm();
  pinMode(12,OUTPUT);
  pinMode(13,OUTPUT);
  digitalWrite(13,HIGH);
  memset(Buff, '\0', 80);
}

void loop(){
  gps.listen();
  delay(2000);
  getGpsData();
  if(gpsdata != "") {
    gsm.listen();
    delay(2000);
    initGsm();
    delay(2000);
    connectHttp();
    //delay(2000);
  }
  else {
    gsm.listen();
  }
  
  //Serial.println("Testing");
  String serial_data = "";
  int i = 0;
  delay(100);
  gsm.println("AT+CMGF=1");    
  delay(3000);  
  gsm.println("AT+CMGL=\"REC UNREAD\"");
  delay(100);
  if(gsm.available() > 0) {
    serial_data = gsm.readString();
  }
  Serial.println(serial_data);
  serial_data.trim();
  serial_data.replace("AT+CMGF=1","");
  serial_data.replace("OK","");
  serial_data.replace("AT+CMGL=\"REC UNREAD\"","");
  serial_data.replace("\"REC UNREAD\"","");
  serial_data.replace("+CMGL:","");
  serial_data.replace(",","");
  serial_data.replace("\n","");
  serial_data.replace("\r","");
  serial_data.replace("/","");
  serial_data.replace("+91","");
  serial_data.replace(":","");
  serial_data.replace("+","");
  Serial.println(serial_data);
  serial_data.toCharArray(Buff,80);
  while(i< 80) {
    //Serial.write(Buff[i]);
    if(Buff[i] == 'O' && Buff[i+1] == 'N') {
      digitalWrite(12,HIGH);
      digitalWrite(13,LOW);
      Serial.println("Led On");
      if(EEPROM.read(bike_status_address) == '0') {
        EEPROM.write(bike_status_address,'1');
      }
    }
    if(Buff[i] == 'O' && Buff[i+1] == 'F' && Buff[i+2] == 'F') {
      digitalWrite(12,LOW);
       Serial.println("Led Off");
      if(EEPROM.read(bike_status_address) == '1') {
        EEPROM.write(bike_status_address,'0');
      }
    }
    i++;
  }
  memset(Buff, '\0', 80);
  serial_data = "";
  //gsm.println("AT+CMGF=0");    
  delay(5000);
}

void initGsm(){
  gsm.println("AT+CREG?");
  gsm.println("AT+CGATT=1");
  delay(200);
  gsm.println("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"");
  delay(200);
  buffer = _readSerial();
  Serial.println(buffer);
  gsm.println("AT+SAPBR=3,1,\"APN\",\"www\"");
  delay(200);
  buffer = _readSerial();
  Serial.println(buffer);
  gsm.println("AT+SAPBR=1,1");
  delay(300);
  buffer = _readSerial();
  Serial.println(buffer);
}

void getGpsData(){
  gpsdata = "";
  while (gps.available() > 0){
    //delay(100);
    tinygps.encode(gps.read());
    if (tinygps.location.isUpdated()){
      gpsdata = "Latitude=" + String(tinygps.location.lat(),8) + ",Longitude=" + String(tinygps.location.lng(),8);
      //gsm.print("Latitude="); 
      //gsm.print(tinygps.location.lat(),8);
      //gsm.print(  "Longitude="); 
      //gsm.println(tinygps.location.lng(),8);
      Serial.println(gpsdata);
      break;
    }
  }
}

void connectHttp(){
  //if(gpsdata != NULL) {
    gsm.println("AT+HTTPINIT");
    delay(300);
    buffer = _readSerial();
    Serial.println(buffer);
    gsm.println("AT+HTTPPARA=\"CID\",1");
    delay(100);
    buffer = _readSerial();
    Serial.println(buffer);
    char bike_status_char = EEPROM.read(bike_status_address);
    //gsm.println("AT+HTTPPARA=\"URL\",\"tracking.evotechies.com/api/add-gps-location/1/" + gpsdata + "\"");
    gsm.println("AT+HTTPPARA=\"URL\",\"tracking.evotechies.com/api/add-gps-location?user_id=1&gps_data=" + gpsdata + "&bike_status="+bike_status_char+"\"");
    //gsm.println("AT+HTTPPARA=\"URL\",\"tracking.evotechies.com/api/add-gps-location/user_id=1/gps_data=/bike_status=1");
    gpsdata = "";
    delay(100);
    buffer = _readSerial();
    Serial.println(buffer);
    gsm.println("AT+HTTPSSL=0");
    delay(300);
    buffer = _readSerial();
    Serial.println(buffer);
    gsm.println("AT+HTTPACTION=0");
    delay(3000);
    //buffer = _readSerial();
    //Serial.println(buffer);
    printSerialData();
    gsm.println("AT+HTTPREAD=0,20");
    delay(3000);
    //buffer = _readSerial();
    //Serial.println(buffer);
    printSerialData();
    gsm.println("AT+HTTPTERM");
    buffer = _readSerial();
    Serial.println(buffer);
  //}
}

String _readSerial() {
  int _timeout = 0;
  while  (!gsm.available() && _timeout < 12000  )
  {
    delay(13);
    _timeout++;
  }
  if(gsm.available()){
    return gsm.readString();
  }
}
void printSerialData()
{
while(gsm.available()!=0)
Serial.write(gsm.read());
}

