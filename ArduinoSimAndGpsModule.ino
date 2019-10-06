#include <SoftwareSerial.h>
#include <TinyGPS++.h>
#include <EEPROM.h>


SoftwareSerial gsm(10,11);
SoftwareSerial gps(5,6);
String buffer,bike_data;
TinyGPSPlus tinygps;
String gpsdata = "";
char Buff[80],bike_data_array[100];  
unsigned char BuffIndex;
char bike_status = '1';
int bike_status_address = 10;
int gps_init_counter = 0;
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
  initGsm();
  delay(2000);

}

void loop(){
  gps.listen();
  delay(2000);
  getGpsData();
  if(gpsdata == "") {
     getSim800lGpsData();
  }
    gsm.listen();
    delay(2000);
    if(gps_init_counter > 5) {
      initGsm();
      delay(2000);
      gps_init_counter = 0;
      Serial.println("Reset Counter");
    }
    //initGsm();
    //delay(2000);
   
    
    connectHttp();
    //delay(2000);
  //}
  //else {
    //gsm.listen();
  //}
      
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

void getSim800lGpsData() {
  gsm.println("AT+CIPGSMLOC=1,1");
  delay(2000);
  buffer = _readSerial();
  String location_raw = buffer;
  location_raw.trim();
  location_raw.replace("AT+CIPGSMLOC=1,1","");
  location_raw.replace("+CIPGSMLOC:","");
  location_raw.replace(" ","");
  location_raw.replace(" ","");
  location_raw.replace(" ","");
  location_raw.trim();
  char location_raw_bytes[44];
  location_raw.toCharArray(location_raw_bytes,44);
  String loc_longitude="Longitude=",loc_latitude="Latitude=";
  int i =0,flag=0;
  for(i = 2; i <= 44; i++) {
    if(flag == 0) {
      if(location_raw_bytes[i] != ',') {
        loc_longitude += location_raw_bytes[i];
      }
      else {
        flag = 1;
      }
    }
    else {
      if(location_raw_bytes[i] != ',') {
        loc_latitude += location_raw_bytes[i];
      }
      else {
        break;
      }
    }
  }
  gpsdata = loc_latitude + "," + loc_longitude;
  Serial.println("My GPS DATA: " + gpsdata + "\n");
   
  Serial.println(buffer);
  delay(200);
}

void connectHttp(){
  //if(gpsdata != NULL) {
    gsm.println("AT+HTTPINIT");
    delay(300);
    buffer = _readSerial();
    //Serial.println(buffer);
    gsm.println("AT+HTTPPARA=\"CID\",1");
    delay(100);
    buffer = _readSerial();
    //Serial.println(buffer);
    char bike_status_char = EEPROM.read(bike_status_address);
    //gpsdata = "Latitude=8.0,Longitude=88.22";
    //gsm.println("AT+HTTPPARA=\"URL\",\"tracking.evotechies.com/api/add-gps-location/1/" + gpsdata + "\"");
    gsm.println("AT+HTTPPARA=\"URL\",\"tracking.evotechies.com/api/add-gps-location?user_id=1&gps_data=" + gpsdata + "&bike_status="+bike_status_char+"\"");
    //gsm.println("AT+HTTPPARA=\"URL\",\"tracking.evotechies.com/api/add-gps-location/user_id=1/gps_data=/bike_status=1");
    gpsdata = "";
    delay(100);
    buffer = _readSerial();
    //Serial.println(buffer);
    gsm.println("AT+HTTPSSL=0");
    delay(300);
    buffer = _readSerial();
    //Serial.println(buffer);
    gsm.println("AT+HTTPACTION=0");
    delay(3000);
    //buffer = _readSerial();
    //Serial.println(buffer);
    //printSerialData();
    gsm.println("AT+HTTPREAD=0,20");
    delay(3000);
    //buffer = _readSerial();
    //Serial.println(buffer);
    printSerialData();
    gsm.println("AT+HTTPTERM");
    delay(2000);
    buffer = _readSerial();
    //Serial.println(buffer);
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
while(gsm.available()!=0) {

  //Serial.write(gsm.read());
  bike_data = gsm.readString();
  bike_data.toCharArray(bike_data_array,50);
  char bike_status_array[20];
  String show_data_str = "";
  String show_data_str2 = "";
  int l = 0;
  int k = 0;
  Serial.println("Original Data : " + bike_data+"\n");
  show_data_str = bike_data;

  show_data_str.trim();
  show_data_str.replace("AT+HTTPACTION=0","");
  show_data_str.replace("OK","");
  show_data_str.replace("+HTTPACTION: 0,200,20","");
  show_data_str.replace("AT+HTTPREAD=0,20","");
  show_data_str.replace("+HTTPREAD: 20","");
  show_data_str.replace(",","");
  show_data_str.replace("\n","");
  show_data_str.replace("\r","");
  show_data_str.replace("/","");
  show_data_str.replace("OK","");

  Serial.println("My data : " + show_data_str+" \n");
  
  show_data_str2 = show_data_str;
  show_data_str.trim();
  show_data_str2.replace("{","");
  show_data_str2.replace("}","");
  show_data_str2.replace(":","");
  show_data_str2.replace("\"","");
  show_data_str2.replace("\"","");
  show_data_str2.replace("\"","");
  show_data_str2.replace("\"","");
  show_data_str2.replace("1","");
  show_data_str2.replace("0","");
  show_data_str2.replace("\n","");
  show_data_str2.replace("\r","");
  show_data_str2.replace("/","");
  show_data_str2.trim();
  show_data_str.toCharArray(bike_status_array,20);

  Serial.println("String 2 : " + show_data_str2+" \n");
  if(show_data_str2 == "bike_status") {
    if(bike_status_array[17] == '1') {
      digitalWrite(12,HIGH);
      digitalWrite(13,LOW);
      Serial.println("Led On");
      EEPROM.write(bike_status_address,'1');
    }
    else {
      digitalWrite(12,LOW);
      digitalWrite(13,HIGH);
      Serial.println("Led Off");
      EEPROM.write(bike_status_address,'0');
    }
  }
  else if(show_data_str == "+HTTPACTION: 06010") {
    gps_init_counter++;
    Serial.println("ERROR");
  }
}
}

