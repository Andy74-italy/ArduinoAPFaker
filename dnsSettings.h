#include <WiFiUdp.h>

IPAddress gwip(172,128, 128, 1);
IPAddress apip(172,128, 128, 100);

#define UDP_PACKET_SIZE 128           // MAX UDP packaet size = 512
#define DNSHEADER_SIZE 12             // DNS Header
#define DNSANSWER_SIZE 16             // DNS Answer = standard set with Packet Compression
#define DNSMAXREQUESTS 16             // trigger first DNS requests, to redirect to own web-page
byte packetBuffer[ UDP_PACKET_SIZE];  // buffer to hold incoming and outgoing packets
byte dnsReplyHeader[DNSHEADER_SIZE] = { 
  0x00,0x00,   // ID, to be filled in #offset 0
  0x81,0x80,   // answer header Codes
  0x00,0x01,   //QDCOUNT = 1 question
  0x00,0x01,   //ANCOUNT = 1 answer
  0x00,0x00,   //NSCOUNT / ignore
  0x00,0x00    //ARCOUNT / ignore
  };
byte dnsReplyAnswer[DNSANSWER_SIZE] = {   
  0xc0,0x0c,  // pointer to pos 12 : NAME Labels
  0x00,0x01,  // TYPE
  0x00,0x01,  // CLASS
  0x00,0x00,  // TTL
  0x00,0x3c,  // TLL 1 hour
  0x00,0x04,   // RDLENGTH = 4
  0x00,0x00,  // IP adress octets to be filled #offset 12
  0x00,0x00   // IP adress octeds to be filled
  } ;
byte dnsReply[UDP_PACKET_SIZE];       // buffer to hold the send DNS repluy
IPAddress dnsclientIp;
unsigned int dnsclientPort;
unsigned int udpPort = 53;            // local port to listen for UDP packets
WiFiUDP Udp;                          // A UDP instance to let us send and receive packets over UDP
int dnsreqCount=0;

void udpScan()
{
int t=0;  // generic loop counter
int r,p;  // reply and packet counters
unsigned int packetSize=0;
unsigned int replySize=0;
  packetSize = Udp.parsePacket();
  if ( (packetSize!=0) && (packetSize<UDP_PACKET_SIZE) )  //only packets with small size
    {
    // We've received a packet, read the data from it
    Udp.read(packetBuffer, packetSize); // read the packet into the buffer
    dnsclientIp = Udp.remoteIP();
    dnsclientPort = Udp.remotePort();
    if ( (dnsclientIp != apip) && (dnsreqCount<=DNSMAXREQUESTS) ) // only non-local IP and only the first few DNSMAXREQUESTS x times
    {
    // DEBUG : Serial Print received Packet
    Serial.print("DNS-packets (");Serial.print(packetSize);
    Serial.print(") from ");Serial.print(dnsclientIp);
    Serial.print(" port ");Serial.println(dnsclientPort);
      for (t=0;t<packetSize;++t){
      Serial.print(packetBuffer[t],HEX);Serial.print(":");
      }
      Serial.println(" ");
      for (t=0;t<packetSize;++t){
      Serial.print( (char) packetBuffer[t]);//Serial.print("");
      }
    Serial.println("");
    
    //Copy Packet ID and IP into DNS header and DNS answer
    dnsReplyHeader[0] = packetBuffer[0];dnsReplyHeader[1] = packetBuffer[1]; // Copy ID of Packet offset 0 in Header
    dnsReplyAnswer[12] = apip[0];dnsReplyAnswer[13] = apip[1];dnsReplyAnswer[14] = apip[2];dnsReplyAnswer[15] = apip[3]; // copy AP Ip adress offset 12 in Answer
    r=0; // set reply buffer counter
    p=12; // set packetbuffer counter @ QUESTION QNAME section
    // copy Header into reply
    for (t=0;t<DNSHEADER_SIZE;++t) dnsReply[r++]=dnsReplyHeader[t];
    // copy Qusetion into reply:  Name labels till octet=0x00
    while (packetBuffer[p]!=0) dnsReply[r++]=packetBuffer[p++];
    // copy end of question plus Qtype and Qclass 5 octets
    for(t=0;t<5;++t)  dnsReply[r++]=packetBuffer[p++];
    //copy Answer into reply
    for (t=0;t<DNSANSWER_SIZE;++t) dnsReply[r++]=dnsReplyAnswer[t];
    replySize=r;
    
    // DEBUG : Serial print DSN reply
        Serial.print("DNS-Reply (");Serial.print(replySize);
        Serial.print(") from ");Serial.print(apip);
        Serial.print(" port ");Serial.println(udpPort);
        for (t=0;t<replySize;++t){
        Serial.print(dnsReply[t],HEX);Serial.print(":");
        }
        Serial.println(" ");
        for (t=0;t<replySize;++t){
        Serial.print( (char) dnsReply[t]);//Serial.print("");
        }
    Serial.println("");  
  // Send DSN UDP packet
  Udp.beginPacket(dnsclientIp, dnsclientPort); //reply DNSquestion
  Udp.write(dnsReply, replySize);
  Udp.endPacket();
  dnsreqCount++;
    } // end loop correct IP
 } // end loop received packet
  
}
