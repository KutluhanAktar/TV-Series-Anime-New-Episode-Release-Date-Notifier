
         ////////////////////////////////////////////////////  
        //        TV Series / Anime New Episode           //
       //             Release Date Notifier              //
      //           -------------------------            //
     //              Arduino Nano 33 IoT               //           
    //               by Kutluhan Aktar                // 
   //                                                //
  ////////////////////////////////////////////////////

// Get informed when new episodes of your favorite shows are on release with their opening songs via Nano 33 IoT and Raspberry Pi. 
//
// I developed a corroborating web application in PHP for this project, named TV Series / Anime Release Date Tracker. 
// You can either use a Raspberry Pi as the server, explained in the project tutorial, or TheAmplituhedron with the real-time database interface if you are a member. 
//
// For more information:
// https://www.theamplituhedron.com/projects/TV-Series-Anime-New-Episode-Release-Date-Notifier/
// 
// You can use the mentioned web application in free version on TheAmplituhedron as the host server if you are a subscriber:
// https://www.theamplituhedron.com/dashboard/TV-Series-Anime-Episode-Tracker/
//
// Connections
// Arduino Nano 33 IoT:           
//                                Nokia 5110 Screen
// D2  --------------------------- SCK (Clk)
// D3  --------------------------- MOSI (Din) 
// D4  --------------------------- DC 
// D5  --------------------------- RST
// D6  --------------------------- CS (CE)
//                                RGB
// D9  --------------------------- R
// D10 --------------------------- G
// D11 --------------------------- B
//                                LEFT_BUTTON
// A0 --------------------------- S
//                                OK_BUTTON
// A1 --------------------------- S
//                                RIGHT_BUTTON
// A2 --------------------------- S
//                                EXIT_BUTTON
// A3 --------------------------- S
//                               DS3231 (Optional for Nano and Not Required for Nano 33 IoT)
// A4 --------------------------- SDA  
// A5 --------------------------- SCL



// Include required libraries:
#include <SPI.h>
#include <WiFiNINA.h>
#include <LCD5110_Basic.h>


char ssid[] = "[_SSID_]";        // your network SSID (name)
char pass[] = "[_PASSWORD_]";    // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;                // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS;

// Note: Uncomment equivalent connection settings provided under related lines for using the web application hosted on TheAmplituhedron if you are a subscriber.

// Enter the IPAddress of your Raspberry Pi.
IPAddress server(192, 168, 1, 22);
/* 
// name address for TheAmplituhedron. Change it with your server if you are using a different host server than TheAmplituhedron.
char server[] = "www.theamplituhedron.com";
*/

// Define the pathway of the application in Raspberry Pi.
String application = "/TV-Series-Anime-Episode-Tracker/query.php";
/*
// Define your hedron if you are using TheAmplituhedron as the host server for this project:
String HEDRON = "[_HEDRON_]";
// Define the pathway of the web application. If you are using TheAmplituhedron as the host server for this project as I did, just enter your hedron. Otherwise, enter the pathway on your server.
String application = "/dashboard/TV-Series-Anime-Episode-Tracker/" + HEDRON;
*/

// Initialize the Ethernet client library
WiFiClient client;
/* WiFiSSLClient client; */

// Define screen settings.
LCD5110 myGLCD(2,3,4,5,6);

extern uint8_t SmallFont[];
extern uint8_t MediumNumbers[];
// Define the graphics for related screen modes.
extern uint8_t tv[];
extern uint8_t music[];

// Define the required MP3 Player Commands.
// You can inspect all given commands from the project page: 
// Select storage device to TF card
static int8_t select_SD_card[] = {0x7e, 0x03, 0X35, 0x01, 0xef}; // 7E 03 35 01 EF
// Play the song with the directory: /01/001xxx.mp3
static int8_t play_song_1[] = {0x7e, 0x04, 0x41, 0x00, 0x01, 0xef}; // 7E 04 41 00 01 EF
// Play the song with the directory: /01/002xxx.mp3
static int8_t play_song_2[] = {0x7e, 0x04, 0x41, 0x00, 0x02, 0xef}; // 7E 04 41 00 02 EF
// Play the song with the directory: /01/003xxx.mp3
static int8_t play_song_3[] = {0x7e, 0x04, 0x41, 0x00, 0x03, 0xef}; // 7E 04 41 00 03 EF
// Play the song with the directory: /01/004xxx.mp3
static int8_t play_song_4[] = {0x7e, 0x04, 0x41, 0x00, 0x04, 0xef}; // 7E 04 41 00 04 EF
// Play the song with the directory: /01/005xxx.mp3
static int8_t play_song_5[] = {0x7e, 0x04, 0x41, 0x00, 0x05, 0xef}; // 7E 04 41 00 05 EF
// Play the song.
static int8_t play[] = {0x7e, 0x02, 0x01, 0xef}; // 7E 02 01 EF
// Pause the song.
static int8_t pause[] = {0x7e, 0x02, 0x02, 0xef}; // 7E 02 02 EF
// Next song.
static int8_t next_song[] = {0x7e, 0x02, 0x03, 0xef}; // 7E 02 03 EF
// Previous song.
static int8_t previous_song[] = {0x7e, 0x02, 0x04, 0xef}; // 7E 02 04 EF

// Define menu options and modes using volatile booleans.
volatile boolean TV = false;
volatile boolean Music = false;
volatile boolean Sleep = false;
volatile boolean Activated = false;

// Define the control buttons.
#define B_Exit A3
#define B_Right A2
#define B_OK A1
#define B_Left A0

// Define RGB LED pins.
#define redPin 9
#define greenPin 10
#define bluePin 11

// Define data holders:
int Right, OK, Left, Exit;
int selected = 0;

void setup() {
  // Buttons:
  pinMode(B_Exit, INPUT);
  pinMode(B_Right, INPUT);
  pinMode(B_OK, INPUT);
  pinMode(B_Left, INPUT);
  // RGB:
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);
  adjustColor(0, 0, 0); // Black
  
  // Initiate screen.
  myGLCD.InitLCD();
  myGLCD.setFont(SmallFont);

  Serial.begin(9600);

  // Initiate serial communication for the Serial MP3 Player Module.
  Serial1.begin(9600);

  // Select the SD Card.
  send_command_to_MP3_player(select_SD_card, 5);
  
  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) { Serial.println("Communication with WiFi module failed!"); myGLCD.print("Connection Failed!", 0, 8); while (true); }
  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    myGLCD.print("Waiting...", 0, 8);
    myGLCD.print("Attempting to", 0, 16);
    myGLCD.print("connect to", 0, 24);
    myGLCD.print("WiFi !!!", 0, 32);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);
    // wait 10 seconds for connection:
    delay(10000);
  }

  // Verify connection on both the serial monitor and Nokia 5110 Screen.
  Serial.println("Connected to wifi");
  myGLCD.clrScr();
  myGLCD.print("Connected to", 0, 8);
  myGLCD.print("WiFi!!!", 0, 16);
  delay(2000);
  myGLCD.clrScr();
}

void loop() {

    read_buttons();
  
    change_menu_options();

    interface();

    if(TV == true){
      do{
        myGLCD.invertText(true);
        myGLCD.print("A.Init Tracker", 0, 16);
        myGLCD.invertText(false);
        delay(100);
        if(OK == HIGH){
          myGLCD.clrScr();
          Activated = true;
          while(Activated == true){
            // Connect to the web application named TV Series / Anime Release Date Tracker. Change '80' with '443' if you are using TheAmplituhedron as the host.
            if (client.connect(server, 80)) {
              Serial.println("connected to server"); // if you get a connection, report back via serial:
              myGLCD.print("Connected to", 0, 8);
              myGLCD.print("the server!!!", 0, 16);
              // Make an HTTP request:
              client.println("GET " + application + " HTTP/1.1");
              //client.println("Host: www.theamplituhedron.com");
              client.println("Host: 192.168.1.22");
              client.println("Connection: close");
              client.println();
            }else{
              myGLCD.print("Connection", 0, 8);
              myGLCD.print("Error!!!", 0, 16);
            }
            delay(2000); // Wait 2 seconds after connection...
            // If there are incoming bytes available, get the response from the web application.
            String response = "";
            while (client.available()) { char c = client.read(); response += c; }
            if(response != "" && response.endsWith("%")){
              // Split the response string by a pre-defined delimiter in a simple way. '%'(percentage) is defined as the delimiter in this project.
              int delimiter, delimiter_1, delimiter_2, delimiter_3, delimiter_4;
              delimiter = response.indexOf("%");
              delimiter_1 = response.indexOf("%", delimiter + 1);
              delimiter_2 = response.indexOf("%", delimiter_1 +1);
              delimiter_3 = response.indexOf("%", delimiter_2 +1);
              delimiter_4 = response.indexOf("%", delimiter_3 +1);
              // Glean information as substrings.
              String Series_Name = response.substring(delimiter + 1, delimiter_1);
              String Season = response.substring(delimiter_1 + 1, delimiter_2);
              String Episode = response.substring(delimiter_2 + 1, delimiter_3);
              String Episode_Name = response.substring(delimiter_3 + 1, delimiter_4);
              // Print information.
              myGLCD.clrScr();
              myGLCD.print(Series_Name, 0, 0);
              myGLCD.print(Season + " x " + Episode, 0, 16);
              myGLCD.print(Episode_Name, 0, 32);
              // Play the opening song of the released series / anime until ceased:
              if(Series_Name == "One Piece") { send_command_to_MP3_player(play_song_1, 6); adjustColor(255,0,0); }
              if(Series_Name == "My Hero Academia") { send_command_to_MP3_player(play_song_2, 6); adjustColor(0,255,0); }
              if(Series_Name == "Westworld") { send_command_to_MP3_player(play_song_3, 6); adjustColor(0,0,255); }
              if(Series_Name == "The Simpsons") { send_command_to_MP3_player(play_song_4, 6); adjustColor(255,255,0); }
              if(Series_Name == "The Late Late Show") { send_command_to_MP3_player(play_song_5, 6); adjustColor(80,0,80); }
              // Wait until the Exit button pressed...
              volatile boolean song = true;
              while(song == true){
                read_buttons();
                if(Exit == HIGH){ song = false; send_command_to_MP3_player(pause, 4); adjustColor(0,0,0); }
              }
              myGLCD.clrScr();
            }else{
              // Print information.
              myGLCD.clrScr();
              myGLCD.print("No Released", 0, 0);
              myGLCD.print("Episode", 0, 16);
              myGLCD.print("Detected :(", 0, 32);
              delay(5000); // Wait 5 seconds to display information...
              myGLCD.clrScr();
            }
            // Draw TV icon while counting to the new request...
            myGLCD.drawBitmap(8, 0, tv, 60, 48);
            delay(10 * 1000); // Wait until next request...
            myGLCD.clrScr();
            // Exit.
            read_buttons();
            if(Exit == HIGH){ Activated = false; myGLCD.clrScr(); }
          }
        }
      }while(TV == false);
    }

    if(Music == true){
      do{
        myGLCD.invertText(true);
        myGLCD.print("B.MP3 Player", 0, 24);
        myGLCD.invertText(false);
        delay(100);
        if(OK == HIGH){
          myGLCD.clrScr();
          Activated = true;
          while(Activated == true){
            read_buttons();
            // Draw music player icon.
            myGLCD.drawBitmap(8, 0, music, 60, 48);
            // MP3 Player:
            if(Right == true) send_command_to_MP3_player(next_song, 4);
            if(Left == true) send_command_to_MP3_player(previous_song, 4);
            if(OK == true) send_command_to_MP3_player(pause, 4);
            // Exit.
            if(Exit == HIGH){ Activated = false; myGLCD.clrScr(); send_command_to_MP3_player(pause, 4); }
          }
        }
      }while(Music == false);
    }

    if(Sleep == true){
      do{
        myGLCD.invertText(true);
        myGLCD.print("C.Sleep", 0, 32);
        myGLCD.invertText(false);
        delay(100);
        if(OK == HIGH){
          // Activate the sleep mode in 10 seconds.
          myGLCD.clrScr();
          myGLCD.print("Entering", CENTER, 0);
          myGLCD.print("Sleep Mode", CENTER, 8);
          myGLCD.print("in", CENTER, 16);
          myGLCD.print("Seconds", CENTER, 40);
          // Print remaining seconds.
          myGLCD.setFont(MediumNumbers);
          for (int s=10; s>=0; s--){ myGLCD.printNumI(s, CENTER, 24, 2, '0'); delay(1000); }
          myGLCD.enableSleep();
          Activated = true;
          while(Activated == true){
            // Color Pattern:
            adjustColor(255,0,0);
            delay(500);
            adjustColor(0,255,0);
            delay(500);
            adjustColor(0,0,255);
            delay(500);
            adjustColor(255,255,0);
            delay(500);
            adjustColor(80,0,80);
            delay(500);
            // Exit.
            read_buttons();
            if(Exit == HIGH){ Activated = false; myGLCD.clrScr(); myGLCD.disableSleep(); myGLCD.setFont(SmallFont); adjustColor(0,0,0); }
          }
        }
      }while(Sleep == false);
    }

}

void read_buttons(){
  // Read the control buttons:
  Right = digitalRead(B_Right);
  OK = digitalRead(B_OK);
  Left = digitalRead(B_Left);
  Exit = digitalRead(B_Exit);
}

void send_command_to_MP3_player(int8_t command[], int len){
  Serial.print("\nMP3 Command => ");
  for(int i=0;i<len;i++){ Serial1.write(command[i]); Serial.print(command[i], HEX); }
  delay(1000);
}

void interface(){
   // Define options.
   myGLCD.print("Menu Options :", 0, 0);
   myGLCD.print("A.Init Tracker", 0, 16);
   myGLCD.print("B.MP3 Player", 0, 24);
   myGLCD.print("C.Sleep", 0, 32);
}

void change_menu_options(){
  // Increase or decrease the option number using Right and Left buttons.
  if(Right == true) selected++;
  if(Left == true) selected--;
  if(selected < 0) selected = 3;
  if(selected > 3) selected = 1;
  delay(100);
  // Depending on the selected option number, change boolean status.
  switch(selected){
    case 1:
      TV = true;
      Music = false;
      Sleep = false;
    break;
    case 2:     
      TV = false;
      Music = true;
      Sleep = false;
    break;
    case 3:
      TV = false;
      Music = false;
      Sleep = true;
    break;
  }
}

void adjustColor(int red, int green, int blue){
 red = 255 - red;
 green = 255 - green;
 blue = 255 - blue;
 analogWrite(redPin, red);
 analogWrite(greenPin, green);
 analogWrite(bluePin, blue);
}
