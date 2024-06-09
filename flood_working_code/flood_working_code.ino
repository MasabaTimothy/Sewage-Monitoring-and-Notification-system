#include <SoftwareSerial.h>

SoftwareSerial gsm(11, 12); // GSM module connected to Arduino pins 11 and 12
SoftwareSerial esp(6, 5);   // ESP module connected to Arduino pins 6 and 5 (RX, TX)

String uri_short = "/v1/post/hardware.php?data=1";
String uri;

int node1;
int node2;
int node3 = 3;
int node4 = 4;
int node5 = 5;

void setup() {
  pinMode(13, OUTPUT);
  Serial.begin(9600); // Start serial communication with Arduino IDE
  gsm.begin(9600);    // Start communication with GSM module
  esp.begin(9600);    // Start communication with ESP module

  delay(2000); // Wait for GSM module to initialize

  connectWifi();
}

void loop() {
  uri = uri_short + "&node1=" + node1 + "&node2=" + node2 + "&node3=" + node3 + "&node4=" + node4 + "&node5=" + node5;
  Serial.println(uri);
  httppost(); 
  s1();
  s2();
}

void sendSMS(String number, String message) {
  gsm.println("AT+CMGF=1"); // Set SMS mode to text mode
  delay(100);
  gsm.println("AT+CMGS=\"" + number + "\""); // Specify the recipient number
  delay(100);
  gsm.println(message); // The SMS message
  delay(100);
  gsm.println((char)26); // End of message character
  delay(1000);
}

void s1() {
  int x = analogRead(A1);
  int y = analogRead(A2);
  int z = analogRead(A3);

  Serial.print("x: "); Serial.println(x); 
  Serial.print("y: "); Serial.println(y); 
  Serial.print("z: "); Serial.println(z); 

  if (x == 0 && y == 0 && z == 0) {
    node1 = 0;
  } else if (x > 0 && y == 0 && z == 0) {
    node1 = 25;
  } else if (x > 0 && y > 0 && z == 0) {
    sendSMS("+256774100905", "Location 1: Sewage level Half");
    node1 = 50;
  } else if (x > 0 && y > 0 && z > 0) {
    sendSMS("+256774100905", "Location 1: Sewage level is critically full");
    node1 = 100;
  }
}

void s2() {
  int a = analogRead(A4);
  int b = analogRead(A5);
  int c = analogRead(A7);

  Serial.print("a: "); Serial.println(a); 
  Serial.print("b: "); Serial.println(b); 
  Serial.print("c: "); Serial.println(c); 

  if (a == 0 && b == 0 && c == 0) {
    node2 = 0;
  } else if (a > 0 && b == 0 && c == 0) {
    node2 = 25;
  } else if (a > 0 && b > 0 && c == 0) {
    sendSMS("+256774100905", "Location 2: Sewage level Half");
    node2 = 50;
  } else if (a > 0 && b > 0 && c > 0) {
    sendSMS("+256774100905", "Location 2: Sewage level is critically full");
    node2 = 100;
  }
}

void connectWifi() {
  String ssid = "Spot";
  String password = "password";

  Serial.println("Trying to connect");
  esp.println("AT+CWMODE=3");
  delay(100);
  String cmd = "AT+CWJAP=\"" + ssid + "\",\"" + password + "\"";
  esp.println(cmd);
  delay(4000);

  if (esp.find("OK")) { 
    Serial.println("Connected!");
  } else {
    Serial.println("Cannot connect to wifi"); 
  }
}

void httppost() {
  String data;
  String server = "192.168.43.125";  // IP address of the server

  Serial.println("Starting Conn");
  esp.println("AT+CIPSTART=\"TCP\",\"" + server + "\",80"); // Start a TCP connection

  if (esp.find("OK")) {
    Serial.println("TCP connection ready");
  } 
  delay(1000);

  String postRequest =
    "POST " + uri +
    " HTTP/1.0\r\n" +
    "Host: " + server + "\r\n" + 
    "Accept: */*\r\n" +
    "Content-Length: " + data.length() + "\r\n" +
    "Content-Type: application/x-www-form-urlencoded\r\n" +
    "\r\n" + data;

  String sendCmd = "AT+CIPSEND="; // Determine the number of characters to be sent

  esp.print(sendCmd);
  esp.println(postRequest.length());
  delay(500);

  if (esp.find(">")) {
    Serial.println("Sending...");
    esp.print(postRequest);

    if (esp.find("SEND OK")) {
      Serial.println("Packet sent");

      while (esp.available()) {
        String tmpResp = esp.readString();
        Serial.println("tmpResp=");
        Serial.println(tmpResp);
        if (tmpResp.indexOf("off") > -1) {
          digitalWrite(13, 0);
          Serial.println(".................................................0");
        } else {
          digitalWrite(13, 1);
          Serial.println("................................................1"); 
        }
      }
      esp.println("AT+CIPCLOSE"); // Close the connection
    }
  }
}
