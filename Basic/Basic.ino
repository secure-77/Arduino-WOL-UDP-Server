#include <ESP8266WiFi.h>
#include <WiFiUDP.h>
#include <sendemail.h>
#include <ESP8266Ping.h>

// all settings are defined in the "config.h" 
//#include "config.h"

// renamed config file with own settings
#include "server-config.h"

// The ESP-12 has a blue LED on GPIO2
#define LED 2

// UDP Server
WiFiUDP udpServer;

// random pin
long randNumber;

// eMail text buffers, maybe you need to adjust the buffer size
char mail_subject[25];
char mail_text[140];

// Buffer for incoming UDP messages
char udp_buffer[WIFICLIENT_MAX_PACKET_SIZE + 1];


/** Runs once at startup */
void setup()
{
  // LED off
  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH);

  // Initialize the serial port
  Serial.begin(115200);

  // Give the serial monitor of the Arduino IDE time to start
  delay(500);

  // Use an external AP
  WiFi.mode(WIFI_STA);
  WiFi.begin(SSID, PASSWORD);
  WiFi.config(ip, gateway, subnet);

  // Wait until Wifi is connectet
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println(" connected");
  
  // LED On
  digitalWrite(LED, LOW);

  // Start the UDP server
  udpServer.begin(PORT);
  delay(500);

  // send Mail with start Code
  generatePin();
  generateMailText();
  Serial.println(mail_text);
  sendMail();

}

// send mail method
void sendMail() {
  SendEmail e(smtp_server, smtp_port, smtp_login, smtp_password, smtp_timeout, smtp_ssl);
  e.send(mail_from, mail_to, mail_subject, mail_text);
  Serial.println("E-Mail send");
}

// build email text and subject method
void generateMailText() {
  snprintf(mail_subject,25, "WOL start code %ld", randNumber);
  snprintf(mail_text,140, "UPD Server is up and running\nnc %s %u -u \nPlease use the following start code, to wake up the VPN Server: %ld",dynDNS, PORT, randNumber);
}

// generate pin method
void generatePin() {
  randomSeed(RANDOM_REG32);
  randNumber = random(1000, 10000);
}


/** Main loop, executed repeatedly */
void loop()
{
  process_incoming_udp();
  check_ap_connection();
}


/** Receive UDP messages and send an echo back */
void process_incoming_udp()
{
  if (udpServer.parsePacket())
  {
    // Fetch received message
    int len = udpServer.read(udp_buffer, sizeof(udp_buffer) - 1);
    udp_buffer[len] = 0;

    // Display the message for debugging
    Serial.print(F("Received from "));
    Serial.print(udpServer.remoteIP());
    Serial.print(":");
    Serial.print(udpServer.remotePort());
    Serial.print(": ");
    Serial.println(udp_buffer);
    char codeArray[3];
    sprintf (codeArray, "%ld", randNumber);


    // check imput for correct start code -> execute wol script
    if (strstr(udp_buffer, codeArray))
    {
      udpServer.beginPacket(udpServer.remoteIP(), udpServer.remotePort());
      udpServer.println(F("Start code correct!"));
      udpServer.endPacket();
      Serial.println(F("Start code correct!"));

      // x retries
      for (int i = 0; i < retryCount; i++) {

        // check if server is online
        udpServer.beginPacket(udpServer.remoteIP(), udpServer.remotePort());
        udpServer.println(F("checking server status..."));
        udpServer.endPacket();
        Serial.print(F("checking server status..."));

        // check if server is online
        if (checkServerStatus()) {

          // server is onine, cancel loop
          udpServer.beginPacket(udpServer.remoteIP(), udpServer.remotePort());
          udpServer.println(F("Server is up and running!"));
          udpServer.endPacket();
          Serial.print(F("Server is up and running!"));
          i = retryCount;

        } else {

          // server is offline
          udpServer.beginPacket(udpServer.remoteIP(), udpServer.remotePort());
          udpServer.println(F("Server is down or sleeping, sending WOL package..."));
          udpServer.endPacket();
          Serial.println(F("Server is down or sleeping, sending WOL package..."));

          // send WOL package and wait
          WOL(mac);
          delay(senddelay * 1000);

          // on last try
          if (i == retryCount - 1) {
            // last check
            if (checkServerStatus()) {
              udpServer.beginPacket(udpServer.remoteIP(), udpServer.remotePort());
              udpServer.println(F("Server is up and running!"));
              udpServer.endPacket();
              Serial.print(F("Server is up and running!"));
            } else {
              udpServer.beginPacket(udpServer.remoteIP(), udpServer.remotePort());
              udpServer.println(F("failed to wake up server, check power mode"));
              udpServer.endPacket();
              Serial.print(F("failed to wake up server, check power mode"));
            }
          }
        }
      }
    } else {
      // wrong start code entered
      code_tries = code_tries - 1;
      udpServer.beginPacket(udpServer.remoteIP(), udpServer.remotePort());
      udpServer.println(F("Wrong start code!"));
      udpServer.endPacket();
      Serial.print(F("Wrong start code!"));
      // to many wrong codes entered
      if (code_tries == 0) {
        udpServer.beginPacket(udpServer.remoteIP(), udpServer.remotePort());
        udpServer.println(F("max retry count reached, new code generated!"));
        udpServer.endPacket();
        Serial.print(F("max retry count reached, new code generated!"));
        generatePin();
        generateMailText();
        Serial.println(mail_text);
        sendMail();
      }

    }
  }
}

// check server status method
bool checkServerStatus() {

  bool ret = Ping.ping(serverIP, 5);
  int avg_time_ms = Ping.averageTime();

  if (ret) {
    Serial.print(F("Ping ok: "));
    Serial.print(avg_time_ms);
    Serial.println(" ms");
    return true;
  } else {
    Serial.println(F("Ping failed"));
    return false;
  }

}

// send wol method
void WOL(byte mac[6]) {

  digitalWrite(LED, HIGH);
  //the first 6 bytes of a WoL package are "0xFF"
  byte preamble[] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF
  };
  char i;

  //send a UDP package to the broadcast address of the actual configured network
  //to UDP port 7
  udpServer.beginPacket(bc, 7);

  //send preamble
  udpServer.write(preamble, sizeof preamble);

  //now send the MAC address 16 times as recommended by WoL protocol
  for (i = 0; i < 16; i++)
    udpServer.write(mac, 6);

  //finally end the package and send out
  udpServer.endPacket();
  delay(500);
  digitalWrite(LED, LOW);

}

// *optional* check ap connection method
void check_ap_connection()
{
    static wl_status_t preStatus = WL_DISCONNECTED;
    
    wl_status_t newStatus = WiFi.status();
    if (newStatus != preStatus)
    {
        if (newStatus == WL_CONNECTED)
        {
            digitalWrite(LED, LOW);
            
            // Display the own IP address and port
            Serial.print(F("AP connection established, listening on "));
            Serial.print(WiFi.localIP());
            Serial.print(":");
            Serial.println(PORT);
        }
        else
        {
            digitalWrite(LED, HIGH);
            Serial.println(F("AP conection lost"));
        }
        preStatus = newStatus;
    }
}
