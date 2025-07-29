//--------------------------
//This file count the rolling encoder from 0 to 60 to indicate the behavior for minutes and seconds.
#include <BfButton.h>
#include <EnableInterrupt.h> // Use PCINT to allow all GPIO pins for interrupts


// Rotary Encoder 1 (hEncoder)
#define hCLK 18  
#define hDT  19 

// Rotary Encoder 2 (mEncoder)
#define mCLK 2  
#define mDT  3  

// Rotary Encoder 3 (sEncoder)
#define sCLK 11
#define btnPin 10
#define sDT  12

bool isCountDown = false;


BfButton btn(BfButton::STANDALONE_DIGITAL, btnPin, true, LOW);

volatile int scounter = 0;
volatile int mcounter = 0;
volatile int hcounter = 0;

long hour = 0, minute = 1, second = 0;
long countdown_time = (hour*3600) + (minute * 60) + second;
unsigned long lastMillis = 0;

// Variables to store the previous states of each encoder
volatile uint8_t h_old_AB = 3; // 2-bit state for hEncoder
volatile uint8_t m_old_AB = 3; // 2-bit state for mEncoder
volatile uint8_t s_old_AB = 3; // 2-bit state for sEncoder

// Encoder state lookup table
const int8_t enc_states[] = {0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0};


//------servo part------------------
#include <Servo.h>

Servo hourServo;
Servo minuteServo;
Servo secondServo;

#define hourpin 6
#define minutepin 7
#define secondpin 8

//-------The latch servo-------------------

#define latchpin 5
Servo latchServo;
bool isLocked = false;

//--------------------------


/**
the heart component size: 16 x 16 pixel
5 hearts takes total width of 16 x 5 = 80
heart canvas: x: [0, 80], y [0,16]
emergency canvas: the x range of [80, 164], and y of [0, 16]
**/
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeMonoBoldOblique12pt7b.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library. 
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS  0x3C///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

//the heart component total used of canvas size
#define heartwidth 80
#define heartheight 16
GFXcanvas1 heartCanvas(heartwidth, heartheight);

#define clockwidth 128
#define clockheight 48
GFXcanvas1 clockCanvas(clockwidth, clockheight);

#define emergencywidth 84
#define emergencyheight 16
int heartCounter = 5;
GFXcanvas1 EmergencyCanvas(emergencywidth, emergencyheight);
bool isEmer;



//the heart component
static const unsigned char PROGMEM heart[] = {
  0b00001100, 0b00110000, 
  0b00111100, 0b00111000, 
  0b01111110, 0b01111100, 
  0b01111111, 0b11111100, 
  0b11111111, 0b11111110, 
  0b11111111, 0b11111110, 
  0b11111111, 0b11111110, 
  0b11111111, 0b11111110, 
  0b01111111, 0b11111100, 
  0b00111111, 0b11111000, 
  0b00011111, 0b11110000, 
  0b00001111, 0b11100000, 
  0b00000111, 0b11000000, 
  0b00000011, 0b10000000, 
  0b00000001, 0b00000000, 
  0b00000000, 0b00000000


  };

void setup() {
  Serial.begin(9600);

  //------rotary encoder----------------
  
  pinMode(btnPin, INPUT_PULLUP); 
  // Set encoder pins as inputs with pull-ups
  pinMode(hCLK, INPUT_PULLUP);
  pinMode(hDT, INPUT_PULLUP);
  pinMode(mCLK, INPUT_PULLUP);
  pinMode(mDT, INPUT_PULLUP);
  pinMode(sCLK, INPUT_PULLUP);
  pinMode(sDT, INPUT_PULLUP);
  
  // Attach Pin Change Interrupts for hEncoder (Encoder 1)
  enableInterrupt(hCLK, read_hEncoder, CHANGE);
  enableInterrupt(hDT, read_hEncoder, CHANGE);

  // Attach Pin Change Interrupts for mEncoder (Encoder 2)
  enableInterrupt(mCLK, read_mEncoder, CHANGE);
  enableInterrupt(mDT, read_mEncoder, CHANGE);

  // Attach Pin Change Interrupts for sEncoder (Encoder 3)
  enableInterrupt(sCLK, read_sEncoder, CHANGE);
  enableInterrupt(sDT, read_sEncoder, CHANGE);
 


  //Button settings
  btn.onPress(pressHandler)
  .onDoublePress(pressHandler) // default timeout
  .onPressFor(pressHandler, 1000); // custom timeout for 1 second


//------------servo setup--------
  hourServo.attach(hourpin);
  minuteServo.attach(minutepin);
  secondServo.attach(secondpin);
  latchServo.attach(latchpin);

  hourServo.write(90);
  minuteServo.write(90);
  secondServo.write(90);
  latchServo.write(0);


  //----------------------------------
  //-----OLED DISPLAY----------------

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  delay(1000); // Pause for 2 seconds

  // Clear the buffer
  display.clearDisplay();

  //show the emergency button for use
  //current emergency button is not selected
  emergencyText();

  switch(heartCounter){
    case 5:
    fiveHeart();
    break;
    case 4:
    fourHeart();
    break;
    case 3:
    threeHeart();
    break;
    case 2:
    twoHeart();
    break;
    case 1:
    oneHeart();
    break;
    default:
    fiveHeart();
    break;
  }


  displayClock(hcounter,mcounter,scounter);

  display.display();


}


void lockLatch(){
  latchServo.write(45);
}

void unlockLatch(){
  latchServo.write(0);
}

void displayClock(int h, int m, int s){
  clearClock();
  display.setFont(&FreeMonoBoldOblique12pt7b);
  display.setCursor(4, 40);
  display.setTextSize(1);
  if(h<10){
    display.print("0");
    display.print(h);
  } else {
    display.print(h);
  }
  display.print(":");
  if(m<10){
    display.print("0");
    display.print(m);
  } else {
    display.print(m);
  }
  display.print(":");
  if(s<10){
    display.print("0");
    display.print(s);
  } else {
    display.print(s);
  }
}

void loop() {
  //read the btn
  btn.read();

  if (isCountDown && !isLocked) {
    lockLatch();
    isLocked = true;
    Serial.println("Latch Locked!");
  }

  if(isCountDown){
    startCountDown();
  }

  //-------rotary number
  static int lastHCounter = -1;
  static int lastMCounter = -1;
  static int lastSCounter = -1;

   // Only update clock if the counter changes
  if (hcounter != lastHCounter || mcounter != lastMCounter || scounter != lastSCounter) {
    displayClock(hcounter,mcounter,scounter);
    lastHCounter = hcounter;
    lastMCounter = mcounter;
    lastSCounter = scounter;
  }
  //---------heart amount
  switch(heartCounter){
    case 5:
    fiveHeart();
    break;
    case 4:
    fourHeart();
    break;
    case 3:
    threeHeart();
    break;
    case 2:
    twoHeart();
    break;
    case 1:
    oneHeart();
    break;
    default:
    fiveHeart();
    break;
  }
  display.display();
}


void setServoSpeeds(long countdownhour, long countdownminute, long countdownsecond){
  if (!isCountDown) {
  hourServo.write(90); 
  minuteServo.write(90); 
  secondServo.write(90); 
  return; // Exit the function to avoid recalculating speeds
}
  int hourSpeed = 90 - (90* (float)countdownhour/23.0);
  int minuteSpeed = 90 - (90* (float) countdownminute / 59);
  int secondSpeed = 90 - (90* (float) countdownsecond / 59);

  // if (hour > 0) {
  //  hourServo.write(hourSpeed);
  //   } else {
  //     hourServo.write(90); // Stop hour servo
  //   }

  //   if (minute > 0) {
  //     minuteServo.write(minuteSpeed);
  //   } else {
  //     minuteServo.write(90); // Stop minute servo
  //   }

  //   if (second > 0) {
  //     secondServo.write(secondSpeed);
  //   } else {
  //     secondServo.write(90); // Stop second servo
  //   }

    hourServo.write(hourSpeed);
    minuteServo.write(minuteSpeed);
    secondServo.write(secondSpeed);
  }



void startCountDown() {

  if(countdown_time == 0){
    // Display the updated clock on the OLED
  displayClock(0, 0, 0);
  setServoSpeeds(0, 0, 0);
  return;
  }
  // Check if 1 second has passed since the last update
  if (millis() - lastMillis >= 1000) { // 1000 ms = 1 second
    countdown_time--; // Decrement countdown time
    lastMillis = millis(); // Reset lastMillis for the next second
  }

  // If the countdown reaches 0, stop it
  if (countdown_time <= 0) {
    isCountDown = false;
    isLocked = false;
    unlockLatch();
    countdown_time = 0; // Just to be safe
  }

  // Calculate the hours, minutes, and seconds from total countdown time
  long countdown_hour = countdown_time / 3600;
  long countdown_minute = (countdown_time / 60) % 60;
  long countdown_sec = countdown_time % 60;

  // Display the updated clock on the OLED
  displayClock(countdown_hour, countdown_minute, countdown_sec);
  setServoSpeeds(countdown_hour, countdown_minute, countdown_sec);
}


//Button press hanlding function
void pressHandler (BfButton *btn, BfButton::press_pattern_t pattern) {
  switch (pattern) {
    case BfButton::SINGLE_PRESS:
      Serial.println("Single push");
      hour = hcounter;
      minute = mcounter;
      second = scounter;
      countdown_time = (hour * 3600) + (minute * 60) + second;
      isCountDown = true;
      isLocked = true;
      lockLatch();
      lastMillis = millis();
      break;
      
    case BfButton::DOUBLE_PRESS:
      Serial.println("Double push");
      break;
      
    case BfButton::LONG_PRESS:
      Serial.println("Long push");
      selectedEmergencytext();
      heartCounter--;
      isCountDown = false;
      countdown_time = 0;
      startCountDown();
      unlockLatch();
      isLocked = false;
      delay(1000);
      isEmer = false;
      emergencyText();
      if(heartCounter<=0){
        Serial.println("No more chance to open the box during countdown");
      }
      break;
  }
}

void read_hEncoder() {
  h_old_AB <<= 2;
  if (digitalRead(hCLK)) h_old_AB |= 0x02;
  if (digitalRead(hDT)) h_old_AB |= 0x01;

  int8_t result = enc_states[h_old_AB & 0x0F];
  hcounter += result;

  // Set boundaries for hcounter (0 to 23)
  if (hcounter > 23) hcounter = 23;
  if (hcounter < 0) hcounter = 0;
}

void read_mEncoder() {
  m_old_AB <<= 2;
  if (digitalRead(mCLK)) m_old_AB |= 0x02;
  if (digitalRead(mDT)) m_old_AB |= 0x01;

  int8_t result = enc_states[m_old_AB & 0x0F];
  mcounter += result;

  // Set boundaries for mcounter (0 to 59)
  if (mcounter > 59) mcounter = 59;
  if (mcounter < 0) mcounter = 0;

}

void read_sEncoder() {
  s_old_AB <<= 2;
  if (digitalRead(sCLK)) s_old_AB |= 0x02;
  if (digitalRead(sDT)) s_old_AB |= 0x01;

  int8_t result = enc_states[s_old_AB & 0x0F];
  scounter += result;

  // Set boundaries for scounter (0 to 59)
  if (scounter > 59) scounter = 59;
  if (scounter < 0) scounter = 0;

}


void fiveHeart() {
  clearHeart();

  int xposition = 0;
  for(int i = 0; i < 5; i++){
    display.drawBitmap(xposition,1,heart, 16, 16, 2);
    xposition +=16;
  }
  display.display();
}

//clear the section of heart canvas
void clearHeart(){
  display.drawBitmap(0,0,heartCanvas.getBuffer(),heartCanvas.width(), heartCanvas.height(), 0xFFFF, 0x0000);
}


//clear the section of heart canvas
void clearClock(){
  display.drawBitmap(0,16,clockCanvas.getBuffer(),clockCanvas.width(), clockCanvas.height(), 0xFFFF, 0x0000);
}

void clearEmergencyCanvas(){
  display.drawBitmap(50,0,EmergencyCanvas.getBuffer(),EmergencyCanvas.width(), EmergencyCanvas.height(), 0xFFFF, 0x0000);
}

void fourHeart() {
  clearHeart();

  int xposition = 0;
  for(int i = 0; i < 4; i++){
    display.drawBitmap(xposition,1,heart, 16, 16, 2);
    xposition +=16;
  }
  display.display();
}

void threeHeart() {
  clearHeart();

  int xposition = 0;
  for(int i = 0; i < 3; i++){
    display.drawBitmap(xposition,1,heart, 16, 16, 2);
    xposition +=16;
  }
  display.display();
}

void twoHeart() {
  clearHeart();

  int xposition = 0;
  for(int i = 0; i < 2; i++){
    display.drawBitmap(xposition,1,heart, 16, 16, 2);
    xposition +=16;
  }
  display.display();
}


void oneHeart() {
  clearHeart();

  int xposition = 0;
  for(int i = 0; i < 1; i++){
    display.drawBitmap(xposition,1,heart, 16, 16, 2);
    xposition +=16;
  }
  display.display();
}

void emergencyText(){
  clearEmergencyCanvas();
  isEmer = false; //the current state of emergency button is not selected
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(90,4); //---------> x: 10, y:o each letter takes 8 pixel total, and it has 16 height, in the middle with will be 4
  display.println("EMER");
  display.display();
}

void selectedEmergencytext() {
  isEmer = true; //the state of the button is selected
  display.setTextSize(1);
  display.setTextColor(BLACK,WHITE);
  display.setCursor(90,4); //---------> x: 10, y:o
  display.println("EMER");
  display.display();
}


String TimeLeft(unsigned long MsLeft){
  String Result;
  int H;
  int M;
  int S;
  M=(long)MsLeft/60000;
  Serial.print(MsLeft);
  Serial.print(";");
  Serial.println(M);
  if (M<10) Result=(String)"0"+ M + ":";else Result=(String)M+":";
  S=(long)((MsLeft-M*60000)/1000);
  if (S<10) Result=(String)Result + "0"+ S ;else Result=(String)Result +S;
  return Result;
  
}


