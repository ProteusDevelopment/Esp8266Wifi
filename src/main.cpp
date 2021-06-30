     
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
 
#ifndef STASSID
#define STASSID "kvanta"
#define STAPSK  "11110000b"
#endif
 
unsigned int localPort = 7777;      // local port to listen on
 
// buffers for receiving and sending data
char packetBuffer[UDP_TX_PACKET_MAX_SIZE + 1]; //buffer to hold incoming packet,
char  ReplyBuffer[] = "acknowledged\r\n";       // a string to send back
 
WiFiUDP Udp;
 
int phases[8][4] = {
  { LOW,  LOW,  LOW,  HIGH },
  { LOW,  LOW,  HIGH, HIGH },
  { LOW,  LOW,  HIGH, LOW },
  { LOW,  HIGH, HIGH, LOW },
  { LOW,  HIGH, LOW,  LOW },
  { HIGH, HIGH, LOW,  LOW },
  { HIGH, LOW,  LOW,  LOW },
  { HIGH, LOW,  LOW,  HIGH }
};
 
int dir = 1;
 
int get_speed(int number_of_steps, int speed_value)
{
  return 60L * 1000L * 1000L / number_of_steps / speed_value;
}
 
void stepper_step(int step_value)
{
  digitalWrite(12, phases[step_value][0]);
  digitalWrite(13, phases[step_value][1]);
  digitalWrite(14, phases[step_value][2]);
  digitalWrite(15, phases[step_value][3]);
}
 
void rotate_90(int dir)
{
  long long speed_value = get_speed(1024, 4);
  
  int step_value = 0;
  
  for (int i = 1024; i > 0; i--)
  {
    stepper_step(dir > 0 ? step_value : (8 - step_value) % 8);
    delay(speed_value / 10000);
    
    step_value = (step_value + 1) % 8;
  }
}
 
void setup() {
  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);
  pinMode(14, OUTPUT);
  pinMode(15, OUTPUT);
  
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(STASSID, STAPSK);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(500);
  }
  Serial.print("Connected! IP address: ");
  Serial.println(WiFi.localIP());
  Serial.printf("UDP server on port %d\n", localPort);
  Udp.begin(localPort);
}
 
void loop() {
  // if there's data available, read a packet
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    Serial.printf("Received packet of size %d from %s:%d\n    (to %s:%d, free heap = %d B)\n",
                  packetSize,
                  Udp.remoteIP().toString().c_str(), Udp.remotePort(),
                  Udp.destinationIP().toString().c_str(), Udp.localPort(),
                  ESP.getFreeHeap());
 
    // read the packet into packetBufffer
    int n = Udp.read(packetBuffer, UDP_TX_PACKET_MAX_SIZE);
    packetBuffer[n] = 0;
    Serial.println("Contents:");
    Serial.println(packetBuffer);
 
    // send a reply, to the IP address and port that sent us the packet we received
    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    Udp.write(ReplyBuffer);
    Udp.endPacket();
 
    rotate_90(dir);
    
    if (dir > 0)
      dir = -1;
    else
      dir = 1;
  }
  
}
 
/*
  test (shell/netcat):
  --------------------
      nc -u 192.168.esp.address 8888
*/
 