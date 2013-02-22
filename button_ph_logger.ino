//String api_key = "rlhc";
char api_key[] = "rlhc";

// https://github.com/jcw/ethercard
#include <EtherCard.h>
// https://github.com/tigoe/Button
#include <Button.h>
// http://www.arduino.cc/playground/Code/TimedAction
#include <TimedAction.h>

#include <SoftwareSerial.h>

SoftwareSerial lcd( 48, 49 );

// button stuff
const int buttonLed1 = 42; // green
Button button1 = Button( 44, BUTTON_PULLDOWN );
boolean isDown1 = false;
const int buttonLed2 = 38; // red
Button button2 = Button( 40, BUTTON_PULLDOWN );
boolean isDown2 = false;
const int buttonLed3 = 30; // yellow
Button button3 = Button( 32, BUTTON_PULLDOWN );
boolean isDown3 = false;
const int buttonLed4 = 26; // blue
Button button4 = Button( 28, BUTTON_PULLDOWN );
boolean isDown4 = false;

// every half hour I want to take a ph reading, but I only want to send the data to the web service every hour
boolean timer_toggle = false;
boolean do_webservice = true;
void timers_up() {
  Serial.println( "timer's up" );
  if( timer_toggle ) {
    do_webservice = true;
    log_ph();
    timer_toggle = false;
  }else{
    do_webservice = false;
    log_ph();
    timer_toggle = true;
  }
}

//timed action stuff
//TimedAction timedAction = TimedAction( 30 * 1000, timers_up ); // 30 seconds
TimedAction timedAction = TimedAction( 1800000, timers_up ); // 1/2 hour

// ethercard stuff
#define REQUEST_RATE 5000 // milliseconds
// ethernet interface mac address
static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };
// ethernet interface ip address
//static byte myip[] = { 192,168,1,203 };
static byte myip[] = { 10,1,1,179 };
// gateway ip address
//static byte gwip[] = { 192,168,1,1 };
static byte gwip[] = { 10,1,1,21 };
// remote website ip address and port
// not sure why, but I have to supply the ip and the url
//static byte hisip[] = { 23,23,125,242 };
static byte hisip[] = { 192,168,1,157 };
char website[] PROGMEM = "hydromon.axexsoftware.com";
//char website[] PROGMEM = "192.168.1.157";
byte Ethernet::buffer[300];   // a very small tcp/ip buffer is enough here
static long timer;
// called when the client request is complete
static void my_result_cb( byte status, word off, word len ) {
  Serial.println( "sent data to website" );   
}

void ether_setup() {  
  lcd_clear();
  lcd_print( "Init Ethetnet" );
  
  if( ether.begin(sizeof Ethernet::buffer, mymac) == 0 ) {
    Serial.println( "Failed to access Ethernet controller");
    lcd_clear();
    lcd_print( "Ethernet Failed" );
  }else{
    lcd_second_line();
    lcd_print( "10.1.1.179" );

    ether.staticSetup(myip, gwip);
    
    ether.copyIp(ether.hisip, hisip);
    ether.printIp("Server: ", ether.hisip);

    while (ether.clientWaitingGw())
      ether.packetLoop(ether.packetReceive());
    Serial.println("Gateway found");
  
    lcd_second_line();
    lcd_print( "Gateway Found" );
  
    timer = - REQUEST_RATE; // start timing out right away
  
    // lookup the ip of the website
    if( !ether.dnsLookup( website ) )
      Serial.println("DNS failed");
    ether.printIp("Server: ", ether.hisip);
  
  }
}

void led_button_setup() {
  // set up the button LED:
  pinMode( buttonLed1,OUTPUT );
  pinMode( buttonLed2,OUTPUT );
  pinMode( buttonLed3,OUTPUT );
  pinMode( buttonLed4,OUTPUT );
  
  digitalWrite( buttonLed1, HIGH );  
  digitalWrite( buttonLed2, HIGH );  
  digitalWrite( buttonLed3, HIGH );  
  digitalWrite( buttonLed4, HIGH );
}

void lcd_setup() {
  lcd.begin(9600); // set up serial port for 9600 baud
  delay(500); // wait for display to boot up
  
  lcd_clear();
  lcd_print( "Welcome to" );
  lcd_second_line();
  lcd_print( "Hydromon !" );
  
  delay( 1000 * 2 );
  
  lcd_clear();
}

void ph_setup() {
  Serial3.begin( 38400 ); 
  delay( 1000 );
}

void setup () {
  Serial.begin(115200);  
  lcd_setup();
  ether_setup();
  led_button_setup();
  ph_setup();
  Serial.println( "setup finished" );
  lcd_clear();
  log_ph();
  lcd_clear();
  lcd_print( "Ready..." );
  log_ph();
}

void send_reading_to_webservice( char* reading ) {
  char uri[255];
  sprintf( uri, "?api_key=%s&data=%s", api_key, reading );
  ether.browseUrl( PSTR("/reading/submit/ph"), uri, website, &my_result_cb );
  do_webservice = false;
}

void send_data_to_lcd( String data ) {
  char charBuf[128];
  data.toCharArray(charBuf, 128);
  lcd_clear();
  lcd_print( charBuf );
}

char ph_action = 'x'; // x doesnt really man anything

boolean do_ph = true;

void log_ph() {  
  lcd_clear();
  lcd_print( "Reading pH" );
  
  Serial.println( "send command to ph probe" );
  // send the read command and then, send a \r to tell it the command is done
  ph_action = 'r';
  if( do_ph ) {
    Serial3.write( 'r' );
    Serial3.write( '\r' );
  }
  // it seems to need a bit of a delay or reading doesn't quite work right
  delay( 1000 );
}

void calibrate_red() {
  lcd_clear();
  lcd_print( "Calibrating Red" );
  
  Serial.println( "send red command to ph probe" );
  // send the read command and then, send a \r to tell it the command is done
  ph_action = 't';
  if( do_ph ) {
    Serial3.write( 't' ); // t for ph Ten solution
    Serial3.write( '\r' );
  }
  // it seems to need a bit of a delay or reading doesn't quite work right
  delay( 1000 );
  lcd_clear();
}

void calibrate_yellow() {
  lcd_clear();
  lcd_print( "Calibrating " );
  lcd_second_line();
  lcd_print( "Yellow" );
  
  Serial.println( "send yellow command to ph probe" );
  // send the read command and then, send a \r to tell it the command is done
  ph_action = 's';
  if( do_ph ) {
    Serial3.write( 's' ); // s for ph Seven solution
    Serial3.write( '\r' );
  }
  // it seems to need a bit of a delay or reading doesn't quite work right
  delay( 1000 );
  lcd_clear();
}

void calibrate_blue() {
  lcd_clear();
  lcd_print( "Calibrating Blue" );
  
  Serial.println( "send blue command to ph probe" );
  // send the read command and then, send a \r to tell it the command is done
  ph_action = 'f';
  if( do_ph ) {
    Serial3.write( 'f' ); // f for ph Four solution
    Serial3.write( '\r' );
  }
  // it seems to need a bit of a delay or reading doesn't quite work right
  delay( 1000 );
  lcd_clear();
}

void loop () {
  ether.packetLoop(ether.packetReceive());
  
  button_loop();
  
  timedAction.check();
}

void serialEvent3() {
  int a = Serial3.available();
  char chars[a];
  if( a ) {
    Serial.println( "ph data available" );
    String output = "";
    for( int i=0; i<a; i++ ) {
      char c = (char)Serial3.read();
      if( c != '\r' ) {
        output += c;
        chars[i] = c;
      }
    }
    if( ph_action == 'r' ) { // don't do this if we are calibrating, only if we are getting a reading
      Serial.print( "pH reading: " );
      Serial.println( output );
      if( do_webservice ) {
        send_reading_to_webservice( chars );
      }
      send_data_to_lcd( "pH: " + output );
    }
  }
}

void button_loop() {
  // button & led 1
  if( button1.isPressed() ) {
    digitalWrite( buttonLed1, LOW );
    if( !isDown1 ) {
      Serial.println( "green" );
      do_webservice = true;
      log_ph();
    }
    isDown1 = true;
  }else{
    digitalWrite( buttonLed1, HIGH );
    isDown1 = false;
  }
  
  // button & led 2
  if( button2.isPressed() ) {
    digitalWrite( buttonLed2, LOW );
    if( !isDown2 ) {
      Serial.println( "red" );
      calibrate_red();
    }
    isDown2 = true;
  }else{
    digitalWrite( buttonLed2, HIGH );
    isDown2 = false;
  }
  
  // button & led 3
  if( button3.isPressed() ) {
    digitalWrite( buttonLed3, LOW );
    if( !isDown3 ) {
      Serial.println( "yellow" );
      calibrate_yellow();
    }
    isDown3 = true;
  }else{
    digitalWrite( buttonLed3, HIGH );
    isDown3 = false;
  }
  
  // button & led 4
  if( button4.isPressed() ) {
    digitalWrite( buttonLed4, LOW );
    if( !isDown4 ) {
      Serial.println( "blue" );
      calibrate_blue();
    }
    isDown4 = true;
  }else{
    digitalWrite( buttonLed4, HIGH );
    isDown4 = false;
  } 
}

void lcd_home() {
  lcd.write(254); // move cursor to beginning of first line
  lcd.write(128);
}

void lcd_second_line() {
  lcd.write( 254 );
  lcd.write( 192 );
}

void lcd_clear() { 
  lcd_home();
  lcd.write("                "); // clear display
  lcd.write("                ");
  lcd_home();
}

void lcd_print( const char* output ) {
  lcd.write( output );
}
