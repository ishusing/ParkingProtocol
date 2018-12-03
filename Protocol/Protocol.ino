#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiUDP.h>
#include <EEPROM.h> 
#include <ESP8266HTTPClient.h>

int localUdpPort = 8888; // the destination port
MDNSResponder mdns;
int pin_OP = 4;
int key_addr = 514;
WiFiUDP Udp;
char incomingPacket[255];  // buffer for incoming packets
char  replyPacekt[] = "Hi there! Got the message :-)";  // a reply string to send back
//Packet Buffer 
const int packetSize = 20;
byte packetBuffer[packetSize]; 
int key =0;
// Replace with your network credentials
String startString = "swag";
char* ssid = "swag";
const char* password = "ishusing";
int currentLevel =10000;
ESP8266WebServer server(80);

String webPage = "";

 extern "C" {
#include <user_interface.h>
}

int client_status( )
{

unsigned char number_client;
struct station_info *stat_info;

struct ip_addr *IPaddress;
IPAddress address;
int i=1;

number_client= wifi_softap_get_station_num();
yield();
delay(100);
stat_info = wifi_softap_get_station_info();

Serial.print("Total connected_client are = ");
Serial.println(number_client);
 
delay(100);
return number_client;
} 
int decrypt(String action,String salt){
  Serial.println(action.toInt());
  Serial.println(key);
  return action.toInt()-salt.toInt()-key;
}
int toggle(String action,String salt){
  Serial.println(salt.toInt());
  int act = decrypt(action,salt);
  if(act == 1){
    Serial.println("One");
    digitalWrite(pin_OP,LOW);
  }
  else{
    Serial.println("Two"); 
    digitalWrite(pin_OP,HIGH); 
  }
    return act;
}
String sendHTTP(String type){
  HTTPClient http;
  //GET AP number
  
  http.begin("http://"+WiFi.gatewayIP().toString()+"/getChild");
  int httpCode = http.GET();
   Serial.println("PArent IP is: "+WiFi.gatewayIP().toString());
    if (httpCode > 0) { //Check the returning code
 
      String payload = http.getString();   //Get the request response payload
      Serial.println(payload);                     //Print the response payload
 
    }
 
    http.end();   //Close connection
}
void startAP(String ssid){
  String payload = sendHTTP("getChild");
int childNumber = payload.toInt();
   boolean conn = WiFi.softAP((startString+currentLevel+"_"+childNumber).c_str(), "ishusing");
   Serial.println(conn);
    Serial.println("AP Started" + conn);


    
}
void startServer(){
    webPage += "<h1>ESP8266 Web Server</h1><p>Socket #1 <a href=\"socket1On\"><button>ON</button></a>&nbsp;<a href=\"socket1Off\"><button>OFF</button></a></p>";
  webPage += "<p>Socket #2 <a href=\"socket2On\"><button>ON</button></a>&nbsp;<a href=\"socket2Off\"><button>OFF</button></a></p>";
    
  if (mdns.begin("esp8266", WiFi.localIP())) {
    Serial.println("MDNS responder started");
  }
 server.on("/setKey", [](){
  String qsid =server.arg("key"); 
//  key = (server.arg("key"));
  Serial.println("Key received "+ qsid);
    for (int i = 0; i < qsid.length(); ++i)
      {
        EEPROM.write(i, qsid[i]);
        Serial.print("Wrote: ");
        Serial.println(qsid[i]);
      }
      Serial.println(qsid);
      Serial.println("");
//  EEPROM.write(0, 'c'); //write the first half
EEPROM.commit();
    server.send(200, "text/html", qsid);
  });
   server.on("/getKey", [](){
  char str = EEPROM.read(0);
  Serial.println(str);
String epass= "";
  for (int i = 0; i < 1; ++i)
  {
    epass += char(EEPROM.read(i));
  }
  Serial.print("PASS: ");
  Serial.println(epass);
  key = epass.toInt();
    server.send(200, "text/html", epass);
  });
   server.on("/status", [](){
    int status = client_status();
    Serial.println(status);
    Serial.println("Was the status");
    server.send(200, "text/html", webPage);
  });
   server.on("/start", [](){
    String id = server.arg("id");
    String ssid = "espwifi_"+id;
     startAP(ssid);
//    Serial.println("Was the status");
    server.send(200, "text/html", "");
  });
   server.on("/stop", [](){
  
      boolean conn =WiFi.softAPdisconnect (true);
   Serial.println(conn);
    Serial.println("AP stopped" + conn);
//    Serial.println("Was the status");
    server.send(200, "text/html", webPage);
  });
  
  server.on("/", [](){
    server.send(200, "text/html", webPage);
  });
  server.on("/socket1On", [](){
    String action = server.arg("action");
    String salt= server.arg("salt");
    Serial.println("SOCKET ON "+action +" "+salt);
    int ret = toggle(action,salt);
    server.send(200, "text/html", "Return Received was "+ret);
  
    delay(100);
  });
 pinMode(pin_OP,OUTPUT);
 digitalWrite(pin_OP,LOW);
  server.begin();
  Serial.println("HTTP server started");
}
boolean isHigherLevel(String id){
 int indexOflevel = id.indexOf(startString) + startString.length();
 int indexOfUnderScore = id.indexOf("_");
 Serial.println("HIGGERRRRRRRRRRRRRRR "+id.substring(indexOflevel,indexOfUnderScore));
 
 int parentLevel = (id.substring(indexOflevel,indexOfUnderScore).toInt());
 if(parentLevel < currentLevel){
  return true;
 }
 return false;
}
   String getWifiNetworks(){
   int n = WiFi.scanNetworks();
   String nets[n]; 
   String ret = "";
   int min = -1000;
  Serial.println("scan done");
  if (n == 0) {
    Serial.println("no networks found");
  } else {
    Serial.print(n);
    Serial.println(" networks found");
    for (int i = 0; i < n; ++i) {
      // Print SSID and RSSI for each network found
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      nets[i] = WiFi.SSID(i);
      if(nets[i].startsWith(startString) && WiFi.RSSI(i)> min && isHigherLevel(nets[i])){
        Serial.println("ISHUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU");
        String id = nets[i];
         int indexOflevel = id.indexOf(startString) + startString.length();
 int indexOfUnderScore = id.indexOf("_");
 Serial.println("HIGGERRRRRRRRRRRRRRR "+id.substring(indexOflevel,indexOfUnderScore));
 
 int parentLevel = (id.substring(indexOflevel,indexOfUnderScore).toInt());
 currentLevel = parentLevel  +1;
        ret = nets[i];
      }
      Serial.print(" (");
      Serial.println("NET: "+ nets[i]);
      Serial.print(WiFi.RSSI(i));
      Serial.print(")");
      Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");
      delay(10);
    }
  }
  Serial.println("Closest ssid is "+ ret);
  if(ret.equals("")){
    delay(2000);
    getWifiNetworks();
  }
  return ret;
}
void setup(void){
  
  delay(1000);
  

  Serial.begin(115200);
     String ret= getWifiNetworks();
     Serial.println(ret+"  Returneeeeeeeeeeeeeeeeeeed Network");
    EEPROM.begin(512);
String epass= "";
  for (int i = 0; i < 1; ++i)
  {
    epass += char(EEPROM.read(i));
  }
  Serial.print("PASS: ");
  Serial.println(epass);
  key = epass.toInt(); 
  char s[ret.length()+1];
  ret.toCharArray(s, ret.length()+1);
  Serial.print("Converted array ");
  Serial.println(s);
//  for(int i=0;i<ret.length();i++){
//    s[i] = ret.
//  }
  WiFi.begin(s, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ret);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
    Serial.println(WiFi.localIP());
    startAP("swag_1") ;
    Udp.begin(localUdpPort);
  Serial.printf("Now listening at IP %s, UDP port %d\n", WiFi.localIP().toString().c_str(), localUdpPort);
  startServer();  
 
}

void loop(void){
  server.handleClient();
  

  int packetSize = Udp.parsePacket();
  if (packetSize)
  {
    // receive incoming UDP packets
    Serial.printf("Received %d bytes from %s, port %d\n", packetSize, Udp.remoteIP().toString().c_str(), Udp.remotePort());
    int len = Udp.read(incomingPacket, 255);
    if (len > 0)
    {
      incomingPacket[len] = 0;
    }
    Serial.printf("UDP packet contents: %s\n", incomingPacket);

    // send back a reply, to the IP address and port we got the packet from
    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    Udp.write(replyPacekt);
    Udp.endPacket();
  }
} 