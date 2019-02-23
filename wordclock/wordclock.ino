/**
 *  @file    wordclock.ino
 *  @author  telegnom (ccc@telegnom.org)
 *  @date    2019-02-23
 *  @version 1.1
 *
 *  @brief firmware for the c3e wordclock
 *
 *  requires the time.h and timezone.h libraries
 *  https://github.com/PaulStoffregen/Time/
 *  https://github.com/JChristensen/Timezone/
 *
 */


#include <FastLED.h>
#include <TimeLib.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <Timezone.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>


#define NUM_LEDS 43
#define DATA_PIN 5

CRGB leds[NUM_LEDS];

// NTP Servers:
static const char ntpServerName[] = "pool.ntp.org";

// defining words

#define M_ES 0
#define M_IST 1
#define M_FUENF 2
#define M_ZEHN 3
#define M_ZWANZIG 4
#define M_VIERTEL 5
#define M_VOR 6
#define M_NACH 8
#define M_HALB 7
#define M_UHR 21
#define H_EINS 11
#define H_ZWEI 12
#define H_DREI 13
#define H_VIER 14
#define H_FUENF 10
#define H_SECHS 15
#define H_SIEBEN 17
#define H_ACHT 16
#define H_NEUN 9
#define H_ZEHN 19
#define H_ELF 20
#define H_ZWOELF 18

TimeChangeRule myDST = {"CEST", Last, Sun, Mar, 2, 120};    //Daylight time = UTC - 4 hours
TimeChangeRule mySTD = {"CET", Last, Sun, Oct, 2, 60};     //Standard time = UTC - 5 hours
Timezone myTZ(myDST, mySTD);

int clockWords[22][10] = {
  {0,-1,-1,-1,-1,-1,-1,-1,-1,-1}, // es 0
  {1,-1,-1,-1,-1,-1,-1,-1,-1,-1},  // ist 1
  {2,3,-1,-1,-1,-1,-1,-1,-1,-1},  // fuenf 2
  {7,8,-1,-1,-1,-1,-1,-1,-1,-1},  // zehn 3
  {4,5,6,-1,-1,-1,-1,-1,-1,-1}, //  zwanzig 4
  {9,10,11,-1,-1,-1,-1,-1,-1,-1},  // viertel 5
  {12,-1,-1,-1,-1,-1,-1,-1,-1,-1},  // vor 6
  {13,14,-1,-1,-1,-1,-1,-1,-1,-1},  // nach 8
  {15,16,-1,-1,-1,-1,-1,-1,-1,-1},  // halb 7
  {17,18,-1,-1,-1,-1,-1,-1,-1,-1},  // neun 9 
  {19,20,-1,-1,-1,-1,-1,-1,-1,-1},  // fuenf 10
  {21,22,-1,-1,-1,-1,-1,-1,-1,-1},  // ein 11
  {23,24,-1,-1,-1,-1,-1,-1,-1,-1},  // zwei 12
  {25,26,-1,-1,-1,-1,-1,-1,-1,-1},  // drei 13
  {27,28,-1,-1,-1,-1,-1,-1,-1,-1},  // vier 14
  {29,30,-1,-1,-1,-1,-1,-1,-1,-1},  // sechs 15
  {31,32,-1,-1,-1,-1,-1,-1,-1,-1},  // acht 16
  {33,34,35,-1,-1,-1,-1,-1,-1,-1},  // sieben 17
  {36,37,38,-1,-1,-1,-1,-1,-1,-1}, // zwölf 18
  {39,40,-1,-1,-1,-1,-1,-1,-1,-1},  // zehn 19
  {41,-1,-1,-1,-1,-1,-1,-1,-1,-1},  // elf 20
  {42,-1,-1,-1,-1,-1,-1,-1,-1},  // uhr 21
};

WiFiUDP Udp;
unsigned int localPort = 8888;  // local port to listen for UDP packets
time_t getNtpTime();
time_t loctime;
uint8_t color = 0;
int lastmin = 100;
void printDigits(int digits);
void sendNTPpacket(IPAddress &address);

void clear(bool show=true)
{
  for(int i = 0; i<NUM_LEDS;i++) {
    leds[i] = 0x000000;
  }
  if (show) {
    FastLED.show();
  }
}

void noWifi()
{
  for(int i=0; i<NUM_LEDS; i++)
  {
    leds[i] = CHSV((255/(NUM_LEDS-1))*i, 255, 128);
    FastLED.show();
  }
}

void printDigits(uint8_t digits, bool colon = true)
{
  // utility for digital clock display: prints preceding colon and leading 0
  if (colon)
  {
    Serial.print(":");
  }
  if (digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

void digitalClockDisplay()
{
  // digital clock display of the time
  printDigits((uint8_t) hour(loctime));
  printDigits((uint8_t) minute(loctime));
  printDigits((uint8_t) second(loctime));
  Serial.print(" ");
  Serial.print(day(loctime));
  Serial.print(".");
  Serial.print(month(loctime));
  Serial.print(".");
  Serial.print(year(loctime));
  Serial.println();
}

void setup()
{
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  clear();
  for (int initcount=0; initcount <= NUM_LEDS; initcount++) {
    clear();
    leds[initcount] = CHSV((255/NUM_LEDS)*initcount, 255, 128);
    FastLED.show();
    delay(100);
  }
  clear();
  Serial.begin(115200);
  WiFiManager wifiManager;
  while (!Serial) ; // Needed for Leonardo only
  delay(250);
  Serial.println("WordClock by telegnom");
  Serial.print("Setup Wifi ... ");
  wifiManager.autoConnect("WordClock");
  Serial.println('[ ok ]');
  while (WiFi.status() != WL_CONNECTED) {
    noWifi();
    delay(500);
  }

  Serial.print("IP number assigned by DHCP is ");
  Serial.println(WiFi.localIP());
  Serial.println("Starting UDP");
  Udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(Udp.localPort());
  Serial.println("waiting for sync");
  setSyncProvider(getNtpTime);
  setSyncInterval(600);
}

int prevDisplay = 100; // when the digital clock was displayed

void loop()
{
  // time_t utc = now();
  loctime = myTZ.toLocal(now());
  if (timeStatus() != timeNotSet) {
  //    if (prevDisplay != second(localtime))
    if (prevDisplay != second(loctime)) { //update the display only if time has changed
    prevDisplay = second(loctime);
      if (second()%10 == 0)
      {
        digitalClockDisplay();
        if (lastmin != minute(loctime))
        {
          setClock();
        }
        prevDisplay = second(loctime);
      }
  //  }
    }
  }
}

void set_word(int word, time_t current_time = loctime) {
  int wordlen = sizeof(clockWords[word])/sizeof(clockWords[word][0]);
  if (lastmin != minute(loctime))
  {
    color += 40;
  }
  for (int i=0; i<wordlen; i++)
  {
    if (clockWords[word][i] >= 0)
    {
      leds[clockWords[word][i]] = CHSV(color, 255, 128);
      Serial.print(clockWords[word][i]);
      Serial.print("  ");
    }
  }
  Serial.println();
}

void setClock() {
  Serial.println("Updating display...");
  time_t current_time = loctime;
  clear(false);
  set_word(M_ES);  // switch es on
  set_word(M_IST);  // switch ist on
  getMinuteWord();  //get word for "minute"
  getHourWord();  // get word for hour
  FastLED.show(); //writing to led stipe
  lastmin = minute(loctime);
  Serial.println("... display updated.");
}

int getHour()
{
  int h = int(hour(loctime));
  int m = int(minute(loctime));
  /*
    get current hour if minute < 20
      Es ist viertel nach zehn
    get next hour if minute >=20
      Es ist zehn vor halb elf
    adjust hour to correct word around midnight:
      23:25 --> Es ist fuenf vor halb zwoelf
  */
  Serial.println(m);
  if (m >= 20)
  {
    if (h < 23)
    {
      return h+1;
    } else
    {
      return 0;
    }
  } else
  {
    return h;
  }
}

void getHourWord()
{
  switch(getHour())
  {
    case 12:
    case 0:
      set_word(H_ZWOELF);
      Serial.println("zwoelf");
      break;
    case 1:
    case 13:
      // special case to get sigular @ "ein uhr"
      if (int(minute(loctime)) < 5)
      {
        set_word(H_EINS);
        Serial.println("ein");
      } else
      {
        set_word(H_EINS);
        Serial.println("eins");
      }
      break;
    case 2:
    case 14:
      set_word(H_ZWEI);
      Serial.println("zwei");
      break;
    case 3:
    case 15:
      set_word(H_DREI);
      Serial.println("drei");
      break;
    case 4:
    case 16:
      set_word(H_VIER);
      Serial.println("vier");
      break;
    case 5:
    case 17:
      set_word(H_FUENF);
      Serial.println("fuenf");
      break;
    case 6:
    case 18:
      set_word(H_SECHS);
      Serial.println("sechs");
      break;
    case 7:
    case 19:
      set_word(H_SIEBEN);
      Serial.println("sieben");
      break;
    case 8:
    case 20:
      set_word(H_ACHT);
      Serial.println("acht");
      break;
    case 9:
    case 21:
      set_word(H_NEUN);
      Serial.println("neun");
      break;
    case 10:
    case 22:
      set_word(H_ZEHN);
      Serial.println("zehn");
      break;
    case 11:
    case 23:
      set_word(H_ELF);
      Serial.println("elf");
      break;
  }
}

void getMinuteWord()
{
  /*
  get word for current "minute"
  */
  switch(int(minute(loctime)))
  {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
      set_word(M_UHR);
      Serial.println("uhr");
      break;
    case 5:
    case 6:
    case 7:
    case 8:
    case 9:
      set_word(M_FUENF);
      set_word(M_NACH);
      Serial.println("fuenf nach");
      break;
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
      set_word(M_ZEHN);
      set_word(M_NACH);
      Serial.println("zehn nach");
      break;
    case 15:
    case 16:
    case 17:
    case 18:
    case 19:
      set_word(M_VIERTEL);
      set_word(M_NACH);
      Serial.println("viertel nach");
      break;
    case 20:
    case 21:
    case 22:
    case 23:
    case 24:
      set_word(M_ZWANZIG);
      set_word(M_NACH);
      Serial.println("zehn vor halb");
      break;
    case 25:
    case 26:
    case 27:
    case 28:
    case 29:
      set_word(M_FUENF);
      set_word(M_VOR);
      set_word(M_HALB);
      Serial.println("fuenf vor halb");
      break;
    case 30:
    case 31:
    case 32:
    case 33:
    case 34:
      set_word(M_HALB);
      Serial.println("halb");
      break;
    case 35:
    case 36:
    case 37:
    case 38:
    case 39:
      set_word(M_FUENF);
      set_word(M_NACH);
      set_word(M_HALB);
      Serial.println("fuenf nach halb");
      break;
    case 40:
    case 41:
    case 42:
    case 43:
    case 44:
      set_word(M_ZWANZIG);
      set_word(M_VOR);
      Serial.println("zehn nach halb");
      break;
    case 45:
    case 46:
    case 47:
    case 48:
    case 49:
      set_word(M_VIERTEL);
      set_word(M_VOR);
      Serial.println("viertel vor");
      break;
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
      set_word(M_ZEHN);
      set_word(M_VOR);
      Serial.println("zehn vor");
      break;
    case 55:
    case 56:
    case 57:
    case 58:
    case 59:
    case 60:
      set_word(M_FUENF);
      set_word(M_VOR);
      Serial.println("fuenf vor");
      break;
  }
}

/*-------- NTP code ----------*/

const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets

time_t getNtpTime()
{
  IPAddress ntpServerIP; // NTP server's ip address

  while (Udp.parsePacket() > 0) ; // discard any previously received packets
  Serial.println("Transmit NTP Request");
  // get a random server from the pool
  WiFi.hostByName(ntpServerName, ntpServerIP);
  Serial.print(ntpServerName);
  Serial.print(": ");
  Serial.println(ntpServerIP);
  sendNTPpacket(ntpServerIP);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Serial.println("Receive NTP Response");
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL;
    }
  }
  Serial.println("No NTP Response :-(");
  return 0; // return 0 if unable to get the time
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}
