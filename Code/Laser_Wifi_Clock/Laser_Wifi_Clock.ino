#include "Laser7WifiClock.h"
#include "OneButton.h" // Library Manager

#include <ArduinoJson.h>

#include <TimeLib.h>

#include "FS.h"
#include "math.h";

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager

/**** NTP Client Stuff ***/
#include <WiFiUdp.h>
unsigned int localPort = 2390;      // local port to listen for UDP packets
IPAddress timeServerIP; // time.nist.gov NTP server address
String ntpServerName = "time.nist.gov";
const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets

WiFiUDP udp;

uint8_t ntpSuccess = 0;
uint8_t ntpFailed = 0;
uint8_t ntpLoop = 0;
uint8_t ipResets = 0;

bool didSecondSwitch = false;
bool didMinuteSwitch = false;
int lastHour = 0;
int currentUpdateCheck = 0;

bool gotTimeFromServer = false;

const int SAVE_VERSION = 1;

time_t prevDisplay = 0;

bool flashConnect = false;
bool flashConnectState = false;

float valLDR = 0;

bool isReady = false;
bool isReboot = false;

unsigned long previousMillis = 0;
unsigned long previousTimeCheck = 0;

int visualState = 0;
int rainbow = 0;
int showingIP = 0;

String settings_filename = "settings.json";

Laser7WifiClock disp( 4, 15, 2 );

#define BUTTON 2
OneButton button (BUTTON, true);

ESP8266WebServer server(80);
WiFiManager wifiManager;

struct Settings {
  int ver = 0;
  int visualstate = 0;
  int timeformat = 24;
  bool useLDR = true;
  float gmt = -1000;
  int brightness = 80;
  int updateSchedule = 1;
  uint32_t defcolour_d = 0;
  uint32_t defcolour_c = 0;
  int daylightstart_d = 0;
  int daylightstart_m = 0;
  int daylightstart_t = 0;
  int daylightend_d = 0;
  int daylightend_m = 0;
  int daylightend_t = 0;
  String ntpserver;
};

Settings settings;

void setup()
{
  flashConnect = false;
  
  pinMode( BUTTON, INPUT );
  button.attachClick(ButtonClick);
  button.attachDoubleClick(ButtonDoubleClick);
  button.attachLongPressStop(ButtonLongPress);

  previousMillis = millis();
  isReady = false;
  Serial.begin(115200);
  disp.Begin( 50 ); // set brightness to 33%
  delay(100);

  valLDR = analogRead(A0);

  disp.DisplayTextColor( "-   ", disp.Color(0,255,0) );

  LoadSettings();

  delay(500);

  WiFi.hostname("LaserWifiClock");

  bootWifi();
}

void configModeCallback (WiFiManager *myWiFiManager)
{
  Serial.println("Entered config mode");

  ClearAll();
  disp.DisplayTextColor( "conf", disp.Color(0,155,0) );

  Serial.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  Serial.println(myWiFiManager->getConfigPortalSSID());
  Serial.println("Need new Wifi credentials!");
}


void bootWifi()
{
  disp.DisplayTextColor( "--  ", disp.Color(0,0,255) );
    
  Serial.println("Connecting via WifiManager...");
  WiFi.setSleepMode(WIFI_NONE_SLEEP);

  isReboot = false;

  wifiManager.setTimeout(180);
  wifiManager.setAPCallback(configModeCallback);

  if(!wifiManager.autoConnect("LaserWiFiClock"))
  {
    Serial.println("failed to connect and hit timeout");

    ClearAll();
    disp.DisplayTextColor( "err", disp.Color(255,0,0) );

    isReady = false;
    
    isReboot = true;

    delay(2000);
    ClearAll();
    disp.DisplayTextColor( "3", disp.Color(155,0,0) );
    delay(1000);
    disp.DisplayTextColor( "2", disp.Color(155,0,0) );
    delay(1000);
    disp.DisplayTextColor( "1", disp.Color(155,0,0) );
    delay(1000);
    ClearAll();

    ESP.restart();
  }
  else
  {
    disp.DisplayTextColor( "--- ", disp.Color(0,255,255) );
    
    delay(1000);
    bool haveTime = false;

    while (!gotTimeFromServer )
    {
      gotTimeFromServer = GetTimeFromServer();
      if ( !gotTimeFromServer )
      {
        disp.DisplayTextColor( "----", disp.Color(255,0,255) );
        delay(1000);
        disp.DisplayTextColor( "--- ", disp.Color(0,255,255) );
        delay(1000);
      }
    }

    StartWebServer();
    
    disp.DisplayTextColor( "----", disp.Color(0,255,0) );

    Serial.print("Wifi Strength: ");
    Serial.print( getWifiQuality() );
    Serial.println("%");

    delay(500);
    isReady = true;
  }
}


void bootWifiQuiet()
{
  Serial.println("Stopping webserver...");
  server.stop();

  Serial.println("QUIET: Connecting via WifiManager...");
  
  WiFiManager wifiManager;
  wifiManager.setTimeout(180);
  wifiManager.setAPCallback(configModeCallback);
  
  if(!wifiManager.autoConnect("LaserWIFIClock"))
  {
    Serial.println("failed to connect and hit timeout");
    isReady = false;
    ClearAll();
    //disp.DisplayTextColor( "err", disp.Color(255,0,0) );
    flashConnect = true;
    delay(500);
  }
  else
  {
    delay(100);
    Serial.println("Starting UDP");
    udp.begin(localPort);
    Serial.print("Local port: ");
    Serial.println(udp.localPort());

    gotTimeFromServer = false;

    while (!gotTimeFromServer )
    {
      gotTimeFromServer = GetTimeFromServer();
      if ( !gotTimeFromServer )
      {
        delay(2000);
      }
    }

    StartWebServer();

    ipResets++;

    Serial.print("Wifi Strength: ");
    Serial.print( getWifiQuality() );
    Serial.println("%");
    
    isReady = true;
  }
}

void ResetSettings()
{
  settings.ver = SAVE_VERSION;
  settings.ntpserver = String( ntpServerName );
  settings.visualstate = 0;
  settings.timeformat = 24;
  settings.useLDR = true;
  settings.gmt = -1000;
  settings.updateSchedule = 1;
  settings.brightness = 80;
  settings.defcolour_d = Color(0,0,255);
  settings.defcolour_c = Color(255,0,0);
  settings.daylightstart_d = 0;
  settings.daylightstart_m = 0;
  settings.daylightstart_t = 0;
  settings.daylightend_d = 0;
  settings.daylightend_m = 0;
  settings.daylightend_t = 0;
}

void LoadSettings()
{
  // always use this to "mount" the filesystem
  bool result = SPIFFS.begin();

  if ( result )
  {
    delay(100);
  
    DynamicJsonBuffer settingsBuffer;
  
    // this opens the file "f.txt" in read-mode
    File f = SPIFFS.open(settings_filename , "r");
    
    if (!f || f.size() < 1 ) 
    {
      Serial.println("LOAD: File doesn't exist yet. Creating it");
  
      // open the file in write mode
      File f = SPIFFS.open(settings_filename, "w");
      if (!f)
      {
        Serial.println("LOAD: File creation failed");
        return;
      }

      flashConnect = true;
      SaveSettings();
    }
    else
    {
      delay(100);
  
//      Serial.print("Size? ");
//      Serial.println( f.size() );
//  
//      Serial.print("Left? ");
//      Serial.println( f.available() );
      
      if ( f.available() == 0 )
      {
          Serial.println("LOAD: Counldn't grab data, so stomping..");
          ResetSettings();
          SaveSettings();

          flashConnect = true;
       }
       else
       {
        // we could open the file
        while(f.available() )
        {
          //Lets read line by line from the file
          String line = f.readStringUntil('\n');
    
          // check we have data and then copy over?
          if ( line.startsWith("{\"ver") )
          {
            Serial.println( "LOAD: Found JSON Settings" );
            Serial.println( "LOAD: " + line );
            
            // extract JSON
            JsonObject& jdata = settingsBuffer.parseObject(line);
            
            settings.ver = jdata["ver"];
    
            if ( settings.ver == SAVE_VERSION )
            {
              settings.timeformat = jdata["timeformat"]; 
              settings.visualstate = jdata["visualstate"];
              settings.brightness = jdata["brightness"]; 
              settings.gmt = jdata["gmt"]; 
              settings.useLDR = jdata["useLDR"];
              settings.updateSchedule = jdata["updateSchedule"];
              settings.defcolour_d = jdata["defcolour_d"]; 
              settings.defcolour_c = jdata["defcolour_c"]; 
              settings.daylightstart_d = jdata["daylightstart_d"];
              settings.daylightstart_m = jdata["daylightstart_m"]; 
              settings.daylightstart_t = jdata["daylightstart_t"]; 
              settings.daylightend_d = jdata["daylightend_d"];
              settings.daylightend_m = jdata["daylightend_m"];
              settings.daylightend_t = jdata["daylightend_t"];
              settings.ntpserver = jdata["ntpserver"].as<String>();
            }
            else
            {
              Serial.print("LOAD: Old save version found : ");
              Serial.print( settings.ver );
              Serial.print(", Current: ");
              Serial.println( SAVE_VERSION );
              Serial.println( "LOAD: Stompoing data, sorry!" );
              
              ResetSettings();
              SaveSettings();
            }
          }
          else
          {
            Serial.println("LOAD: No save version found, stomping");
            ResetSettings();
            SaveSettings();
          }
        }
      }
       
      if ( settings.gmt > -1000 )
        flashConnect = false;
    }
  f.close();
  }
  else
  {
    Serial.println("SAVE: Failed mounting SPIFFS");
  }
}


bool IsDaylightSaving()
{
  // if start day is 0 ignore daylight savings settings
  if ( settings.daylightstart_d == 0 || settings.daylightstart_m == 0 )
      return false;
      
  // if start and end months are the same, never set daylight saving to true
  if ( settings.daylightstart_m == settings.daylightend_m )
      return false;

  // if start is later than end  oct -> april
  if ( settings.daylightstart_m > settings.daylightend_m )
  {
    if ( month() < settings.daylightstart_m && month() > settings.daylightend_m )
      return false;
  }
  else
  {
    if ( month() < settings.daylightstart_m || month() > settings.daylightend_m )
      return false;
  }

  if ( month() == settings.daylightstart_m && day() < settings.daylightstart_d )
    return false;

  if ( month() == settings.daylightstart_m && day() == settings.daylightstart_d && hour() < settings.daylightstart_t )
    return false;

  if ( month() == settings.daylightend_m && day() > settings.daylightend_d )
    return false;

  if ( month() == settings.daylightend_m && day() == settings.daylightend_d && hour() > settings.daylightend_t )
    return false;
  
    return true;
 
}

bool Is24Hour() 
{
  return ( settings.timeformat == 24 );
}

void SaveSettings()
{
  // always use this to "mount" the filesystem
  bool result = SPIFFS.begin();

  if ( result )
  {
    // this opens the file "f.txt" in write mode
    File f = SPIFFS.open(settings_filename , "w");
    
    if (!f) 
    {
      Serial.println("SAVE: File creation failed");
      return;
    }
  
    DynamicJsonBuffer settingsBuffer;
    JsonObject &jdata = settingsBuffer.createObject();
  
     jdata["ver"] = SAVE_VERSION;
     jdata["timeformat"] = settings.timeformat; 
     jdata["visualstate"] = settings.visualstate;
     jdata["brightness"] = settings.brightness; 
     jdata["useLDR"] = settings.useLDR;
     jdata["gmt"] = settings.gmt; 
     jdata["updateSchedule"] = settings.updateSchedule;
     jdata["defcolour_d"] = settings.defcolour_d;
     jdata["defcolour_c"] = settings.defcolour_c; 
     jdata["daylightstart_d"] = settings.daylightstart_d; 
     jdata["daylightstart_m"] = settings.daylightstart_m; 
     jdata["daylightstart_t"] = settings.daylightstart_t; 
     jdata["daylightend_d"] = settings.daylightend_d; 
     jdata["daylightend_m"] = settings.daylightend_m;
     jdata["daylightend_t"] = settings.daylightend_t;
     jdata["ntpserver"] = settings.ntpserver;
  
    String output;
    jdata.printTo(output);
  
    Serial.print ("JSON: ");
    Serial.println( output );
  
    f.println( output );
        
    Serial.println( "Settings saved");
  
    f.close();
  }
  else
  {
    Serial.println("SAVE: Failed mounting SPIFFS");
  }
}


void StartWebServer()
{
  if (MDNS.begin("LaserWiFiClock"))
  {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);
  server.on("/f12", handleF12);
  server.on("/f24", handleF24);

  server.on("/inline", []() {
    server.send(200, "text/plain", "this works as well");
  });

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
}

void handleF12()
{
  settings.timeformat = 12;
  Serial.println("Time is now 12 hour");
  // Redirect to /
  server.sendHeader("Location", String("/"), true);
  server.send ( 302, "text/plain", "");
  // Save settings
  SaveSettings();
}

void handleF24()
{
  settings.timeformat = 24;
  Serial.println("Time is now 24 hour");
  // Redirect to /
  server.sendHeader("Location", String("/"), true);
  server.send ( 302, "text/plain", "");
  // Save settings
  SaveSettings();
}

String GetNTPServer()
{
  if ( settings.ntpserver.length() < 1 || settings.ntpserver == NULL )
    return  ntpServerName;
  else
    return settings.ntpserver;
  
}


// This is a catch-all for broad range settings
void handleNotFound()
{
  // Was it a GMT we got?
  if ( server.uri().substring(0,2) == "/G" )
  {
    Serial.print("Got GMT zone: ");
    Serial.println( server.uri().substring(2) );
    
    settings.gmt = server.uri().substring(2).toFloat();
    
    server.sendHeader("Location", String("/"), true);
    server.send ( 302, "text/plain", "");
    
    SaveSettings();

    GetTimeFromServer();
  }
  // Was it a Brightness we got?
  else if ( server.uri().substring(0,2) == "/B" )
  {
    Serial.print("Got Max brightness: ");
    Serial.println( server.uri().substring(2) );
    
    settings.brightness = server.uri().substring(2).toInt();
    
    server.sendHeader("Location", String("/"), true);
    server.send ( 302, "text/plain", "");
    
    SaveSettings();
  }
  // Was it a internet update time?
  else if ( server.uri().substring(0,2) == "/U" )
  {
    Serial.print("Got Update Time: ");
    Serial.println( server.uri().substring(2) );
    
    settings.updateSchedule = server.uri().substring(2).toInt();
    
    server.sendHeader("Location", String("/"), true);
    server.send ( 302, "text/plain", "");
    
    SaveSettings();
  }
  else if ( server.uri().substring(0,4) == "/ds_" )
  {
    Serial.print("Daylight Saving Starts: ");

    if ( server.uri().substring(4,5) == "d" )
    {
      Serial.println( "Day " + String(server.uri().substring(5)) );
      settings.daylightstart_d = server.uri().substring(5).toInt();
    }
    else if ( server.uri().substring(4,5) == "m" )
    {
      Serial.println( "Month " + String(server.uri().substring(5)) );
      settings.daylightstart_m = server.uri().substring(5).toInt();
    }
    else if ( server.uri().substring(4,5) == "t" )
    {
      Serial.println( "Hour " + String(server.uri().substring(5)) );
      settings.daylightstart_t = server.uri().substring(5).toInt();
    }
  
    server.sendHeader("Location", String("/"), true);
    server.send ( 302, "text/plain", "");
        
    SaveSettings();
  }
   else if ( server.uri().substring(0,5) == "/ldr_" )
  {
    Serial.print("LDR Control: ");
    if ( server.uri().substring(5,7) == "on" )
    {
      Serial.println( "ON");
      settings.useLDR = true;
    }
    else
    {
      Serial.println( "OFF");
      settings.useLDR = false;
    }

    server.sendHeader("Location", String("/"), true);
    server.send ( 302, "text/plain", "");
        
    SaveSettings();

  }
  else if ( server.uri().substring(0,4) == "/de_" )
  {
    Serial.print("Daylight Saving Ends: ");
    
    if ( server.uri().substring(4,5) == "d" )
    {
      Serial.println( "Day " + String(server.uri().substring(5)) );
      settings.daylightend_d = server.uri().substring(5).toInt();
    }
    else if ( server.uri().substring(4,5) == "m" )
    {
      Serial.println( "Month " + String(server.uri().substring(5)) );
      settings.daylightend_m = server.uri().substring(5).toInt();
    }
    else if ( server.uri().substring(4,5) == "t" )
    {
      Serial.println( "Hour " + String(server.uri().substring(5)) );
      settings.daylightend_t = server.uri().substring(5).toInt();
    }
  
    server.sendHeader("Location", String("/"), true);
    server.send ( 302, "text/plain", "");
        
    SaveSettings();
  }
  else if ( server.uri().substring(0,5) == "/ntp_" )
  {
    Serial.print("NTP Server: ");
    settings.ntpserver = string2constchar (server.uri().substring(5) );

    server.sendHeader("Location", String("/"), true);
    server.send ( 302, "text/plain", "");
        
    SaveSettings();
  }
  else if ( server.uri().substring(0,5) == "/def_" )
  {
//    Serial.print("Color: ");
//    Serial.print( server.uri() );
//    Serial.println( " " + server.uri().substring(5,6) + " " + server.uri().substring(6,7) );
//    
    int colVal = server.uri().substring(7).toInt();
    if ( server.uri().substring(5,6) == "D" )
    {
      int r = disp.Red( settings.defcolour_d );
      int g = disp.Green( settings.defcolour_d );
      int b = disp.Blue( settings.defcolour_d );
    
      if ( server.uri().substring(6,7) == "R" )
        r = colVal;
      else if ( server.uri().substring(6,7) == "G" )
        g = colVal;
      else if ( server.uri().substring(6,7) == "B" )
        b = colVal;

//       Serial.print( server.uri().substring(6,7) +" ");
//       Serial.println( colVal );

       settings.defcolour_d = disp.Color( r, g, b );
    }
    else if ( server.uri().substring(5,6) == "C" )
    {
      int r = disp.Red( settings.defcolour_c );
      int g = disp.Green( settings.defcolour_c );
      int b = disp.Blue( settings.defcolour_c );
    
      if ( server.uri().substring(6,7) == "R" )
        r = colVal;
      else if ( server.uri().substring(6,7) == "G" )
        g = colVal;
      else if ( server.uri().substring(6,7) == "B" )
        b = colVal;

       settings.defcolour_c = disp.Color( r, g, b );
    }
        
    server.sendHeader("Location", String("/"), true);
    server.send ( 302, "text/plain", "");
        
    SaveSettings();
  }
  
  else
  {
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += server.uri();
    message += "\nMethod: ";
    message += (server.method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += server.args();
    message += "\n";
    for (uint8_t i = 0; i < server.args(); i++) {
      message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
    }
    server.send(404, "text/plain", message);
    }
}

char* string2char(String command){
    if(command.length()!=0){
        char *p = const_cast<char*>(command.c_str());
        return p;
    }
}

const char* string2constchar(String command){
    if(command.length()!=0){
        const char *p = const_cast<const char*>(command.c_str());
        return p;
    }
}

void digitalClockDisplay()
{
  // digital clock display of the time
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.print(" ");
  Serial.print(day());
  Serial.print(".");
  Serial.print(month());
  Serial.print(".");
  Serial.print(year());
  Serial.println();
}

bool ShowIP()
{
  if ( showingIP == 0 )
    return false;
    
  IPAddress ip = WiFi.localIP();

  disp.SetPixel( 28, disp.Color(0,0,0) );
  disp.SetPixel( 29, disp.Color(0,0,0) );
  
  if ( millis() - previousMillis > 1200 )
  {
    if ( showingIP == 5 )
    {
      ClearAll();
      disp.DisplayTextColor( disp.SpaceToDisplay((String)ip[0]), disp.Color(0,0,255) );
      Serial.println( ip[0] );
      showingIP--;
    }
    else if ( showingIP == 4 )
    {
      ClearAll();
      disp.DisplayTextColor( disp.SpaceToDisplay((String)ip[1]), disp.Color(0,255,255) );
      Serial.println( ip[1] );
      showingIP--;
    }
    else if ( showingIP == 3 )
    {
      ClearAll();
      disp.DisplayTextColor( disp.SpaceToDisplay((String)ip[2]), disp.Color(255,0,255) );
      Serial.println( ip[2] );
      showingIP--;
    }
    else if ( showingIP == 2 )
    {
      ClearAll();
      disp.DisplayTextColor( disp.SpaceToDisplay((String)ip[3]), disp.Color(255,255, 0) );
      Serial.println( ip[3] );
      showingIP--;
    }
    else
    {
      //nextSwitch = millis();
      showingIP = 0;
    }
  
    previousMillis = millis();
  }

  return true;
}

void printDigits(int digits)
{
  // utility for digital clock display: prints preceding colon and leading 0
  Serial.print(":");
  if (digits < 10)
    Serial.print('0');
  Serial.print(digits);
}

void loop()
{
  // Check for button press
  button.tick();

  if ( settings.useLDR )
  {
    // Adjust brightness based on LDR readings
    valLDR = analogRead(A0);
    float bright = constrain(( ( valLDR / 555 )) * settings.brightness, 15, 80);
    disp.SetBrightness( bright );
  }
  else
  {
     float bright = map( settings.brightness, 33,100, 15, 80);
      disp.SetBrightness( bright );
  }

  if ( flashConnect || settings.gmt < -999 )
  {
    if ( ShowIP() )
    {
      // nothing here... showing of IP is in the ShowIP() method
    }
    else
    {
      if (  millis() - previousMillis > 500 )
      {
        if ( flashConnectState )
          disp.DisplayError( disp.Color(255,0,0) );
        else
          ClearAll();
  
        flashConnectState = !flashConnectState;
        previousMillis = millis();
      }
    }

    if ( settings.gmt < -999 )
    {
      // run the webserver
      server.handleClient();
      return;
    }
  }
    
  // If wifi is not connected with correct time from NPT, don't go any further
  if ( !isReady )
    return;

  if ( flashConnect )
    flashConnect  = false;

  // run the webserver
  server.handleClient();

  bool hourSwitched = false;
  didSecondSwitch = false;
  if (now() != prevDisplay)
  {
    prevDisplay = now();

    // Log the time and date to the serial console
    //digitalClockDisplay();
    
    didSecondSwitch = true;

    if ( lastHour != hour() )
    {
      lastHour = hour();
      hourSwitched = true;
    }

    // make sure we only do this once in the loop per second == 0
    if ( didSecondSwitch )
    {
      // every 60 seconds we check for IP
      if (second() < 1 )
      {
        Serial.print(">>>>>>>>>>>>>> Free Heap: ");
        Serial.println( ESP.getFreeHeap() );
        
        Serial.println(">>>>>>>>>>>>>> Check WiFi"); 
        if ( WiFi.status() != WL_CONNECTED )
        {
          Serial.print("**** WiFi Disconnected: ");
          Serial.println ( String((wl_status_t)WiFi.status()) ); 
          bootWifiQuiet();
          return;
        }
      }
    }

    // every hour we grab the correct time from NTP and adjust the clock
    if ( hourSwitched )
    {
      hourSwitched = false;
  
      if ( currentUpdateCheck >= settings.updateSchedule )
      {
        gotTimeFromServer = GetTimeFromServer();
      }
      else
      {
        currentUpdateCheck++;
      }
    }
  }

  // Show IP to the user if they selected it with a dbl press of the button
  if ( ShowIP() )
  {
    // nothing here... showing of IP is in the ShowIP() method  
  }
  else // show the time based on the visual display settings chosen
  {
    if ( settings.visualstate == 0 )
    {
      if ( didSecondSwitch )
        disp.DisplayTime( GetHour(), GetMins(), second(), settings.defcolour_d, settings.defcolour_c );
    }
    else if ( settings.visualstate == 1 )
    {
      uint32_t col = disp.Wheel(rainbow & 255);
      disp.DisplayTime( GetHour(), GetMins(), second(), col );
  
      if (  millis() - previousMillis > 33 )
      {
        previousMillis = millis();
        rainbow++;
      }
    }
    else if ( settings.visualstate == 2 )
    {
      if ( millis() %2 == 0 )
        disp.DisplayTimeCycleH( GetHour(), GetMins(), second(), rainbow, -1 );
  
      if (  millis() - previousMillis > 33 )
      {
        previousMillis = millis();
        rainbow++;
      }
    }
    else if ( settings.visualstate == 3 )
    {
      disp.DisplayTimeCycleV( GetHour(), GetMins(), second(), rainbow, 1 );
  
      if (  millis() - previousMillis > 33 )
      {
        previousMillis = millis();
        rainbow++;
      }
    }
    else if ( settings.visualstate == 4 )
    {
      if ( didSecondSwitch )
        disp.DisplayTimeRandom( GetHour(), GetMins(), second() );
    }
    else if ( settings.visualstate == 5 )
    {
      disp.DisplayTimeVerticalRainbow( GetHour(), GetMins(), second(), disp.Color( 255, 255, 0), disp.Color(0, 200, 0) );
    }
  }
}


// methods for managing Hour and Mins
int GetHour()
{
  int newHour = hour() + ( IsDaylightSaving() ? 1 : 0 );

  if ( !Is24Hour() )
  {
    if ( newHour > 12 )
      newHour -= 12;
  }
  else if ( newHour > 23 )
  {
    newHour -= 24;
  }

  return newHour;
}


int GetMins()
{
  int newMins = minute();
  return newMins;
}

// Clear all pixels on the display
void ClearAll()
{
    disp.ClearAll();
}

// Handle all of the button clicks
void ButtonClick()
{
  if ( showingIP == 0 )
  {
    settings.visualstate++;
    if ( settings.visualstate == 6)
      settings.visualstate = 0;

    SaveSettings();
    
    previousMillis = millis();
  }
}

void ButtonDoubleClick()
{
  if ( showingIP == 0 )
  {
    showingIP = 5;
    previousMillis = millis() -2000;
  }
}

void ButtonLongPress()
{
  WiFiManager wifiManager;
  wifiManager.resetSettings();
  delay(1000);

  Serial.println( "Re connect");
  bootWifi();
}


// NTP and Wifi Utils
// get time from server
bool GetTimeFromServer()
{
  Serial.println("Starting UDP");
  udp.begin(localPort);
  Serial.print("Local port: ");
  Serial.println(udp.localPort());

  if ( ntpLoop < 5 )
  {
    //get a random server from the pool
    WiFi.hostByName( string2constchar(GetNTPServer()), timeServerIP);
  }
  else
  {
    settings.ntpserver = ntpServerName;
    WiFi.hostByName( string2constchar(GetNTPServer()), timeServerIP);
  }

  sendNTPpacket(timeServerIP); // send an NTP packet to a time server
  // wait to see if a reply is available
  delay(1000);
  
  int cb = udp.parsePacket();
  if (!cb) {
    Serial.println("no packet yet");

    ntpFailed++;
    ntpLoop++;

    udp.stop();
    
    return false;
  }
  else {
    Serial.print("packet received, length=");
    Serial.println(cb);
    // We've received a packet, read the data from it
    udp.read(packetBuffer, NTP_PACKET_SIZE); // read the packet into the buffer

    //the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, esxtract the two words:

    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;
    Serial.print("Seconds since Jan 1 1900 = " );
    Serial.println(secsSince1900);

    // now convert NTP time into everyday time:
    Serial.print("Unix time = ");
    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
    const unsigned long seventyYears = 2208988800UL;
    // subtract seventy years:
    unsigned long epoch = secsSince1900 - seventyYears;
    // print Unix time:
    Serial.println(epoch);

    //time_t theTime = epoch + settings.gmt * SECS_PER_HOUR;
    setTime( epoch + settings.gmt * SECS_PER_HOUR );

    ntpSuccess++;
    ntpLoop = 0;  
    udp.stop();

    currentUpdateCheck = 0;

    lastHour = hour();

    previousTimeCheck = millis();
    
    return true;
  }
}


// send an NTP request to the time server at the given address
unsigned long sendNTPpacket(IPAddress& address)
{
  Serial.println("sending NTP packet...");
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  udp.beginPacket(address, 123); //NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}

// converts the dBm to a range between 0 and 100%
int8_t getWifiQuality()
{
  int32_t dbm = WiFi.RSSI();
  if(dbm <= -100) {
      return 0;
  } else if(dbm >= -50) {
      return 100;
  } else {
      return 2 * (dbm + 100);
  }
}

uint32_t Color(uint8_t r, uint8_t g, uint8_t b)
{
  return ((uint32_t)r << 16) | ((uint32_t)g <<  8) | b;
}

#include "webcode.h";


void handleRoot()
{
  server.send(200, "text/html", getPage() );
}
