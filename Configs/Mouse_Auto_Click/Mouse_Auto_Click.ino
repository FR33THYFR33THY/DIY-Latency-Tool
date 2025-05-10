#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif
// optimize for faster analog read

#include "U8g2lib.h" // u8g2 for screens
 
U8G2_SH1106_128X64_NONAME_F_HW_I2C OLED_1(U8G2_R0, U8X8_PIN_NONE);
U8G2_SH1106_128X64_NONAME_F_HW_I2C OLED_2(U8G2_R0, U8X8_PIN_NONE);
// config for screens

const byte photodiode = A3, hold = 9, reset = 8, mouse=A2, keyboard=A1;
unsigned long startTime, endTime;
float somme=0, moyenne=0, responseTime=0, maxi=0, mini=0, current=0;
int i=0;
// config for buttons, pins, measuring, timers, and stats

void setup()
{
  char buffer[10];
  OLED_1.setI2CAddress(0x3C * 2);
  OLED_1.begin();
  OLED_1.setFont(u8g2_font_crox4tb_tf);
  OLED_1.setCursor(30, 20); OLED_1.print("");
  OLED_1.setCursor(40, 50); OLED_1.print("Mouse");
  OLED_1.sendBuffer();

  OLED_2.setI2CAddress(0x3D * 2);
  OLED_2.begin();
  OLED_2.setFont(u8g2_font_crox4tb_tf);
  OLED_2.setCursor(30, 20); OLED_2.print("");
  OLED_2.setCursor(40, 50); OLED_2.print("Auto");
  OLED_2.sendBuffer();
  // config for screens, serial and text

  pinMode(hold, INPUT_PULLUP);
  pinMode(reset, INPUT_PULLUP);
  pinMode(photodiode, INPUT);
  pinMode(mouse, INPUT);
  pinMode(keyboard, INPUT);
  // config for pin mode, buttons and sensors

  sbi(ADCSRA,ADPS2) ;
  cbi(ADCSRA,ADPS1) ;
  cbi(ADCSRA,ADPS0) ;
  // optimize for faster analog read

  pinMode(7, OUTPUT); // config for pin 7, external transistor mouse click
  digitalWrite(7, LOW); // transistor stop mouse left click
}
void loop()
{
  while (digitalRead(reset) == LOW) // reset button
  {
    somme = maxi = mini = responseTime = moyenne = i = current = 0; // clear screen
    OLED_1.clearBuffer();
    OLED_1.setCursor(30, 20); OLED_1.print("");
    OLED_1.setCursor(40, 50); OLED_1.print("Mouse");
    OLED_1.sendBuffer();

    OLED_2.clearBuffer();
    OLED_2.setCursor(30, 20); OLED_2.print("");
    OLED_2.setCursor(40, 50); OLED_2.print("Auto");
    OLED_2.sendBuffer();
    // screen text
   }

 while (digitalRead(hold) == LOW && digitalRead(reset) == HIGH) // measurement button
  {
     float photodiode_state = analogRead(photodiode); // read screen brightness
     float photodiode_low = (photodiode_state*0.97); // lower threshold for screen brightness
     float photodiode_high = (photodiode_state*1.03); // upper threshold for screen brightness

	 digitalWrite(7, HIGH);  // transistor send mouse left click

     startTime = micros(); delayMicroseconds(1); // record start time and add 1µs delay to avoid errors

     while((photodiode_low < photodiode_state) && (photodiode_state < photodiode_high) && digitalRead(hold) == LOW ) // monitor brightness and check hold button to avoid getting stuck
     {
       photodiode_state = analogRead(photodiode); // read photodiode sensor
     }
     digitalWrite(7, LOW); // transistor stop mouse left click

     endTime = micros();
     responseTime = (endTime - startTime)/1000.0; // calculate elapsed time in milliseconds

     maths(); // calculate and store min, max, average, and last values
     drawOLED_1(); // display results on screen 1
     drawOLED_2(); // display results on screen 2

     delay(random(600, 800));; // delay between measurments
  }
}
void maths(void) // calculate and store statistical values
  {
    if (2 <= responseTime && responseTime <= 50) // delete error values ​​less than 2ms and greater than 50ms
    {
      current=responseTime; // store current response time
      if (maxi == 0 && mini == 0)
        {maxi = mini = responseTime;} // initialize max and min values with the first response time

      if (responseTime >= maxi && maxi != 0)
        {maxi = responseTime;} // store max value

      if (responseTime <= mini && mini != 0)
        { mini = responseTime;} // store min value

      i++; // test counter
      somme = (somme + responseTime); // add current response time to sum for average calculation
      moyenne = (somme / i); // calculate average
    }
  }
void drawOLED_1(void) // display min and max values on screen 1
{
  OLED_1.clearBuffer();
  OLED_1.setCursor(0, 15); OLED_1.print("");
  OLED_1.setCursor(0, 35); OLED_1.print((String)"min: "+mini+" ms");
  OLED_1.setCursor(0, 55); OLED_1.print((String)"max: "+maxi+" ms");
  OLED_1.sendBuffer();
}
void drawOLED_2(void) // display current value, average, and number of tests on screen 2
{
  OLED_2.clearBuffer();
  OLED_2.setCursor(0, 15); OLED_2.print((String)"cur: "+current+" ms");
  OLED_2.setCursor(0, 35); OLED_2.print((String)"avg: "+moyenne+" ms");
  OLED_2.setCursor(0, 55); OLED_2.print((String)"tot:  "+i+" val");
  OLED_2.sendBuffer();
}