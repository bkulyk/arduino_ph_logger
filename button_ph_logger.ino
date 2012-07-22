#include <EtherCard.h>
#include <Button.h>

const int modeButton = 22; // pushbutton for calibration mode
const int buttonLed = 23; // LED for the button
Button button = Button(modeButton,BUTTON_PULLDOWN);

#define REQUEST_RATE 5000 // milliseconds
// ethernet interface mac address
static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };
// ethernet interface ip address
static byte myip[] = { 192,168,1,203 };
// gateway ip address
static byte gwip[] = { 192,168,1,1 };
// remote website ip address and port
//static byte hisip[] = { 74,125,79,99 };
static byte hisip[] = { 192,168,1,157 };
// remote website name
char website[] PROGMEM = "192.168.1.157";

byte Ethernet::buffer[300];   // a very small tcp/ip buffer is enough here
static long timer;

// called when the client request is complete
static void my_result_cb( byte status, word off, word len ) { }

void ether_setup() {
  if (ether.begin(sizeof Ethernet::buffer, mymac) == 0) 
    Serial.println( "Failed to access Ethernet controller");

  ether.staticSetup(myip, gwip);

  ether.copyIp(ether.hisip, hisip);
  ether.printIp("Server: ", ether.hisip);

  while (ether.clientWaitingGw())
    ether.packetLoop(ether.packetReceive());
  Serial.println("Gateway found");
  
  timer = - REQUEST_RATE; // start timing out right away
}

void led_button_setup() {
  pinMode(buttonLed,OUTPUT);
  digitalWrite( buttonLed, HIGH ); 
}

void ph_setup() {
  Serial3.begin( 38400 ); 
  delay( 1000 );
}

void setup () {
  Serial.begin(115200);  
  ether_setup();
  led_button_setup();
  ph_setup();
  Serial.println( "setup finished" );
}

boolean isDown = false;

void send_data_to_webservice( String data ) { 
  String uri = "?data=";
  uri += data;
  
  char charBuf[128];
  uri.toCharArray(charBuf, 128);
  
  ether.browseUrl( PSTR("/index.html"), charBuf, website, &my_result_cb );
}

void log_ph() {
  Serial.println( "send command to ph probe" );
  Serial3.write( 'r' );
  Serial3.write( '\r' );
  delay( 1000 );
}

void loop () {
  ether.packetLoop(ether.packetReceive());
  
  if( button.isPressed() ) {
    digitalWrite( buttonLed, LOW );
    if( !isDown )
      log_ph();
    isDown = true;
  }else{
    digitalWrite( buttonLed, HIGH );
    isDown = false;
  }
    
}

void serialEvent3() {
  int a = Serial3.available();
  if( a ) {
    Serial.println( "ph data available" );
    String output = "";
    //String output = "";
    for( int i=0; i<a; i++ ) {
      output += (char)Serial3.read();
      //Serial.print( (char)Serial3.read() );
    }
    Serial.print( "pH reading: " );
    Serial.println( output );
    send_data_to_webservice( output );
  }
}
