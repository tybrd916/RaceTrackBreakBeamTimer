// todo:
// hook up button wires to trigger
// add three leds for feedback
// add switch for power?



#include <ESP8266WiFi.h>
//#include "./ESP8266WiFi.h"
#include "./DNSServer.h"                  // Patched lib
#include <ESP8266WebServer.h>

#define LANE1PIN 12
#define LANE2PIN 13
#define LANE3PIN 14
#define TRACKLENGTHINCHES 200

#define STARTBUTTONPIN 5
#define MAXRACETIMESECONDS 20000  //maximum time a race may take in seconds before timing out

const byte        DNS_PORT = 53;          // Capture DNS requests on port 53
IPAddress         apIP(10, 10, 10, 10);   // Private network for server
DNSServer         dnsServer;              // Create the DNS object
ESP8266WebServer  webServer(80);          // HTTP server

int mode = 0; // 0 Ready 1 Started 2 Ended 3 Error


int sensorState = 0, lastState=0, StartButtonState = 0;  

int NUMRACERS = 1;
// Store off times
unsigned long Start[3] = {0,0,0};
unsigned long End[3] = {0,0,0};
unsigned long Elapsed = 0;
String Position[] = {"?","?","?"};
const int Scale = 64; // 1/64 is hotwheel scale
int CurrentPosition = 1;

int Lane1Last = 0;
int Lane2Last = 0;
int Lane3Last = 0;
 
void setup() {
 
 pinMode (STARTBUTTONPIN, INPUT); // button
 pinMode ( LANE1PIN, INPUT_PULLUP );
 pinMode ( LANE2PIN, INPUT_PULLUP );
 pinMode ( LANE3PIN, INPUT_PULLUP );
 pinMode (STARTBUTTONPIN, INPUT_PULLUP);

   
  // Wifi Config
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP("Carrs on the run");
  
  Serial.begin(9600);
  Serial.println("Initializing..");

  // if DNSServer is started with "*" for domain name, it will reply with
  // provided IP to all DNS request
  dnsServer.start(DNS_PORT, "*", apIP);

 // When a webrequest comes in, and we don't explicitly handle it, this fires.
  webServer.onNotFound([]() 
  {
    webServer.send(200, "text/html", GetHTML());
    //Serial.println("Serving Main Page");
  });
  
webServer.begin();

}
 
////////////////////////////////////////  Begin webpages //////////////////////////////////////////////////////
 String GetHTML()
 {
  // note: Arduino concats strings if they're on seperate lines. I don't understand this, but..I used it to try to make this readable.
  String responseHTML = ""
                      "<html><Title>Welcome to Carrs on the run!</Title><body style='font-family: Arial;font-size:18px' bgcolor='#00577B' text='#008E97'>"
                      "<head>"
                      "<script>ESPN_refresh=window.setTimeout(function(){window.location.href=window.location.href},20);</script>"
                      "</head>"
                      "<Center>"
                      "<H1 style='color:#F58220'>Carrs on the run</h1>"
                      "<h4 style='color:white'<i>Powered by Gravity<i></h4>"
                      "<br><br></center>"
                      "<!--img src='https://img.brickowl.com/files/image_cache/larger/lego-racing-car-set-30150-25.jpg'/><br-->"
                      "<table style='width:100%;border:1px solid Orange;font-size:22px;background-color:#F58220'>"
                      " <tr><th style='text-align: left;color:#00577B'>Lane</th><th style='text-align: left;color:#00577B''>Place</th><th style='text-align: left;color:#00577B''>Time</th><th style='text-align: left;color:#00577B''>Avg. Speed</th> </tr>";
                      responseHTML += " <tr style='color:white'><td>1</td> <td>" + GetPosition(0) + "</td><td>"    + GetTime(0) + " seconds</td><td>" + GetSpeed(0) + " mph</td></tr>";
                      //responseHTML += " <tr style='color:white'><td>2</td> <td>" + GetPosition(1) + "</td><td>"    + GetTime(1) + " seconds</td><td>" + GetSpeed(1) + " mph</td></tr>";
                      //responseHTML += " <tr style='color:white'><td>3</td> <td>" + GetPosition(2) + "</td><td>"    + GetTime(2) + " seconds</td><td>" + GetSpeed(2) + " mph</td></tr>";
                      responseHTML += "</table><br><br><br><br>";
                      responseHTML += String("Debug: Button: ") +  String(StartButtonState) + "<br>";
                      responseHTML += "Sensors: " + GetSensorState() + "<br>";
                      responseHTML += "<h1 style='color:#F58220'>" + getRaceStateHTML() + "</h1>";
                      responseHTML += "</body></html>";

                      
        return responseHTML;
 }
 String GetSensorState()
 {
   return String("Lane 1: " + String(digitalRead(LANE1PIN)) + " Lane 2: " + String(digitalRead(LANE2PIN)) + " Lane 3: " + String(digitalRead(LANE3PIN)));
 }
 String GetTime(int track)
 {
    //Serial.println("Getting Time");
    if (Position[track] == "?")
    {
       return String(Elapsed);
    }
    else
    {
       if (Position[track] != "DNF") 
       {
         return String(End[track] / 1000.000);
       }
       else 
       {
        return String("-");
       }
    }
 }
 String GetPosition(int track)
 {
   //Serial.println("Getting position");
   return Position[track]; 
 }
// Returns the scale speed. this forumla may need tweaking. 
// For now it'll be fun for the kids :)
String GetSpeed(int track)
{

   if(mode != 2)
     return "";

     if (Position[track] != "DNF")
     {
      
       if(String(End[track]) != "0")
       {
          //Serial.println("Calculating for "  + String(track));
          // note: scale is defined at the top of this page. 
          //Serial.println("Calculating speed!");
          //Serial.println("End[track] / 1000.00 is: " + String(End[track] / 1000.00));
          if((End[track] / 1000.00) == 0)
            return String("Error...impossible speed detected!");
            
          float speed = (TRACKLENGTHINCHES / (End[track] / 1000.00) / 17.6) * Scale;
          //Serial.println("Speed is: " + String(speed));
          return String(speed); 
       }
     }
     else
     {
        Serial.println("DNF Detected"); 
        return "DNF";
     }

}

String getRaceStateHTML()
{
  //Serial.println("GetRaceStateHTML");
  //Serial.println("race state is: " + String(mode));
  String result = "";
   switch(mode)
   {
       case 0:
       {
         result= "Hit button to start!";
         break;
       }
       case 1:
       {
           result= "Racing!";
           break;
       }
       case 2:
       {
           result= "Finished! Hit button to reset!";
           break;
       } 
       default:
       {
          break; 
       }
   }
   //Serial.println("Returning: " + result);
   return result;
}

void ProcessRaceState()
{

   switch(mode)
   {
     case 0:
     {
       for(int i = 0; i < NUMRACERS; i++)
          {
            Start[i] = 0;
            End[i]   = 0;
            Elapsed  = 0;
            Position[i] = "?";
          }
       if(StartButtonState == 0)
       {
          // start button pressed..move to next state!
          mode = 1;
          Start[0] = millis();
          Start[1] = millis();
          Start[2] = millis();
          CurrentPosition = 1;
          Serial.println("Race started!");
       }
       break;
     }
     case 1:
     {
          for(int i = 0; i < NUMRACERS; i++)
          {
            long current = millis();
            Elapsed = current - Start[i];
            if(Position[i] == "?")
            {
              End[i] = Elapsed; 
            }
          }

         //Serial.println("Debug:");
         //Serial.println(digitalRead(LANE1PIN));
         //Serial.println(digitalRead(LANE2PIN));
         //Serial.println(digitalRead(LANE3PIN));

         if(digitalRead(LANE1PIN)== LOW && Position[0] == "?")
         {
            Serial.println("Racer 1 finished!");
            Position[0] = CurrentPosition;
            CurrentPosition++;
         }
         
         if(digitalRead(LANE2PIN) == LOW && Position[1] == "?")
         {
            Serial.println("Racer 2 finished!");
            Position[1] = CurrentPosition;
            CurrentPosition++;
         }

         if(digitalRead(LANE3PIN) == LOW && (Position[2] == "?"))
         {
            Serial.println("Racer 3 finished!");
            Position[2] = CurrentPosition;
            CurrentPosition++;
         }

         // if the timeout hits, DNF anyone that hasn't crossed the line (as calcualted by a postion assignment in the race)
         if(Elapsed > MAXRACETIMESECONDS)
         {
            for(int i = 0; i < NUMRACERS; i++)
            {
               if(Position[i] == "?")
               {
                  Serial.println("Racer : " + String(i) + " did not finish");
                  Position[i] = "DNF";
                  End[i] = 0.0; 
               }  
            } 
            Serial.println("Race finished! There was a timeout...DNF ");
            mode = 2; // finish line crossed, end race!
            return;
         } 

         if(CurrentPosition > NUMRACERS)
         {
            mode = 2;
            Serial.println("CurrentPosition is: " + String(CurrentPosition));
            Serial.println("All cars successfully finished!");
            
         }
         break;
         
     }
     case 2:
     {
        //Serial.println("Mode is 2...");
        if(StartButtonState == 0)
       {
          // start button pressed..reset race!
          // mode = 0;
          
          mode = 1; 
          
          for(int i = 0; i < NUMRACERS; i++)
          {
            Start[i] = 0;
            End[i]   = 0;
            Elapsed  = 0;
            Position[i] = "?";
          }
          Start[0] = millis();
          Start[1] = millis();
          Start[2] = millis();
          
          CurrentPosition = 1;
          // restart race 
          Serial.println("Reset button hit...resetting!");
          //delay(500); // debounce
       }
       break;
     } 
     default:
     {
        // should never be here.. 
     }
   }  
}



////////////////////////////////////////  End webpages //////////////////////////////////////////////////////
void PrintLastPins()
{
         Serial.println("Debug (Pins 1, 2, 3, button input, startbutton state");
         Serial.println(digitalRead(LANE1PIN));
         
         Serial.println(digitalRead(LANE2PIN));
         
         Serial.println(digitalRead(LANE3PIN));
         Serial.println(digitalRead(STARTBUTTONPIN));
         Serial.println(StartButtonState);
         Serial.println("Mode: " + String(mode));
         Serial.println("Button: " + String(StartButtonState));
         //delay(300);
}
void loop()

{
  StartButtonState = digitalRead(STARTBUTTONPIN);
  
  //PrintLastPins();
  ProcessRaceState();

  dnsServer.processNextRequest();
  webServer.handleClient();
}

//// COde boneyard ///
/*
   //pinMode(LANE1PIN, INPUT);     
  //pinMode(LANE2PIN, INPUT);
  //pinMode(LANE3PIN, INPUT); 
  //digitalWrite(LANE1PIN, HIGH); // turn on the pullup
  digitalWrite(LANE2PIN, HIGH); 
  digitalWrite(LANE3PIN, HIGH);


     // basic formula is Inches per second * scale / 17.6 (I think 17.6 is inches / sec for mph). Assuming 64 for scale (hot wheel cars I believe are 1/64th)
   // forumla source: http://cs.trains.com/mrr/f/88/t/134482.aspx
   // 10.5 foot track = 126 inches
   // distance / time / 17.6 = actual mph.  Note: Time is in seconds...so we take time/1000 to get it down to seconds.

     //Serial.println("Track is: " + String(track));
     //Serial.println("Scale is: " + String(Scale));
     //Serial.println("End[track] is: " + String(End[track]));
     //Serial.println("Entering if statement...");
     
     //Serial.println("Position[Track] is: " + String(Position[track]));

       // debug
  if(StartButtonState == 0)
  {
    //Serial.println("Button down!");  
  }

  */
