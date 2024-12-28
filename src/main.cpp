#include <Arduino.h>

//#include <driver/rtc_io.h>
#include "driver/gpio.h"
#include "esp_sleep.h"

#define M0 0
#define M1 1
#define AUX 2
#define LED 8

#define TXD2 7 
#define RXD2 6

#define LID 4 //RTC wakeup is available on GPIO0-GPIO5
#define DOOR 5

#define FULL 0x55
#define EMPTY 0xAA
#define ACKNOWLEDGE 0x25



//Will wake up when pins 15 or 12 go HIGH
#define DEEP_SLEEP_GPIO_ANY_HIGH


// keep track of how many times we've come out of deep sleep
RTC_DATA_ATTR int sleep_count = 0;

enum boxStatus {
  empty,
  full
};

enum programstat {
  boxinit,
  boxready,
  boxfilled,
  boxemptied,
  boxfull,
  boxempty,
  waitackfull,
  waitackempty
};

RTC_DATA_ATTR programstat programStatus = boxinit;
RTC_DATA_ATTR boxStatus mailBoxStaus = empty;
RTC_DATA_ATTR bool transmitted = false;
RTC_DATA_ATTR bool acknowledged = false;
RTC_DATA_ATTR unsigned long transmissionTime = millis();
RTC_DATA_ATTR int retransmissions = 0;
RTC_DATA_ATTR bool opening_pressed = false;
RTC_DATA_ATTR bool door_pressed = false;

void receive() {
  byte _receivedCode = 0;
  if (Serial1.available() > 0) {
    while (Serial1.available()) {
      _receivedCode = Serial1.read();
      Serial.print(_receivedCode, HEX);
      if (_receivedCode == ACKNOWLEDGE) {
        acknowledged = true;
      }
    }
  }
}

// dump out the pin that woke us from EXT1 deep sleep
void show_gpio_wakeup_reason()
{

  uint64_t gpio_wakeup_reason = esp_sleep_get_gpio_wakeup_status();

  for (int i = 0; i < GPIO_NUM_MAX; i++)
  {
    if (gpio_wakeup_reason & (1ULL << i))
    {
      Serial.printf("GPIO %d\n", i);
      if (i == LID) {

        if (mailBoxStaus == empty)
        {programStatus = boxfilled;
        }
        else 
        {programStatus = boxfull;
        }
        
      } 
      else if (i == DOOR) {
        if (mailBoxStaus == full)
        {programStatus = boxemptied;
        }
        else 
        {programStatus = boxempty;
        }
      }
      else {
        programStatus = boxready;
      }
      
    }
  }
  

}



// work out why we were woken up
void show_wake_reason()
{
  sleep_count++;
  auto cause = esp_sleep_get_wakeup_cause();
  switch (cause)
  {
  case ESP_SLEEP_WAKEUP_UNDEFINED:
    Serial.println("Undefined"); // most likely a boot up after a reset or power cycly
    break;
  case ESP_SLEEP_WAKEUP_EXT0:
    Serial.println("Wakeup reason: EXT0");
    break;
  case ESP_SLEEP_WAKEUP_EXT1:
    Serial.println("Wakeup reason: EXT1");
    show_gpio_wakeup_reason();
    break;
  case ESP_SLEEP_WAKEUP_TIMER:
    Serial.println("Wakeup reason: TIMER");
    break;
  case ESP_SLEEP_WAKEUP_TOUCHPAD:
    Serial.println("Wakeup reason: TOUCHPAD");
    break;
  case ESP_SLEEP_WAKEUP_ULP:
    Serial.println("Wakeup reason: ULP");
    break;
  default:
    Serial.printf("Wakeup reason: %d\n", cause);
  }
  Serial.printf("Count %d\n", sleep_count);
}


// count down 3 seconds and then go to sleep
void enter_sleep()
{
  Serial.println("Going to sleep...");
  delay(1000);
  Serial.println("3");
  delay(1000);
  Serial.println("2");
  delay(1000);
  Serial.println("1");
  delay(1000);

#ifdef DEEP_SLEEP_GPIO_ANY_HIGH
  pinMode(GPIO_NUM_15, INPUT_PULLDOWN);
  gpio_hold_en(GPIO_NUM_15);
  pinMode(GPIO_NUM_12, INPUT_PULLDOWN);
  gpio_hold_en(GPIO_NUM_12);
  esp_deep_sleep_enable_gpio_wakeup((1 << GPIO_NUM_12) | (1 << GPIO_NUM_15), ESP_GPIO_WAKEUP_GPIO_HIGH);
#endif

  esp_deep_sleep_start();
}

void enter_full_sleep()
{
  Serial.println("Going to full sleep...");
  delay(1000);
  Serial.println("3");
  delay(1000);
  Serial.println("2");
  delay(1000);
  Serial.println("1");
  delay(1000);

#ifdef DEEP_SLEEP_GPIO_ANY_HIGH
  pinMode(GPIO_NUM_15, INPUT_PULLDOWN);
  gpio_hold_en(GPIO_NUM_15);
  pinMode(GPIO_NUM_12, INPUT_PULLDOWN);
  gpio_hold_en(GPIO_NUM_12);
  esp_deep_sleep_enable_gpio_wakeup((1 << GPIO_NUM_15), ESP_GPIO_WAKEUP_GPIO_HIGH);
#endif

  esp_deep_sleep_start();
}

void enter_empty_sleep()
{
  Serial.println("Going to empty sleep...");
  delay(1000);
  Serial.println("3");
  delay(1000);
  Serial.println("2");
  delay(1000);
  Serial.println("1");
  delay(1000);

#ifdef DEEP_SLEEP_GPIO_ANY_HIGH
  pinMode(GPIO_NUM_15, INPUT_PULLDOWN);
  gpio_hold_en(GPIO_NUM_15);
  pinMode(GPIO_NUM_12, INPUT_PULLDOWN);
  gpio_hold_en(GPIO_NUM_12);
  esp_deep_sleep_enable_gpio_wakeup((1 << GPIO_NUM_12), ESP_GPIO_WAKEUP_GPIO_HIGH);
#endif

  esp_deep_sleep_start();
}


void setup(void)
{
  Serial.begin(9600);
  Serial1.begin(9600, SERIAL_8N1, RXD2, TXD2);
  delay(1000);
  // we've just started up - show the reason why
  show_wake_reason();

  pinMode(M0, OUTPUT);
  pinMode(M1, OUTPUT);
  pinMode(AUX, INPUT_PULLUP);
  pinMode(LED, OUTPUT);

  digitalWrite(LED, HIGH);

// Check if first setup
    if(programStatus == boxinit){
      // setup the radio
        Serial.println("Setting up radio");
        digitalWrite(M0, HIGH);
        digitalWrite(M1, HIGH);
        byte data[] = { 0xC0, 0x0, 0x1, 0x1D, 0x34, 0x40 }; // 914MHz, Chan 1, 9.6k uart, 19.2k air
        delay(100);
        for (unsigned int i = 0; i < sizeof(data); i++) {
            Serial1.write(data[i]);
            Serial.println(data[i], HEX);
        }


    } else {
        

    }

    delay(100);
    digitalWrite(M0, LOW);
    digitalWrite(M1, LOW);
    Serial1.write(EMPTY);
    Serial1.flush();

  
}

void loop()
{
  switch (programStatus) {
    
    case boxinit:
// We just started - no powerdown yet 
      Serial.println("Boxinit");
      opening_pressed = false;
      door_pressed = false;
      programStatus = boxready;
      enter_sleep();
      break;

    case boxready:
// wait for first action
      Serial.println("BoxReady");
      opening_pressed = false;
      door_pressed = false;
      programStatus = boxready;
      enter_sleep();
      break;

    case boxfilled:
      Serial.println("Boxfilled");
// Send message to indicate Full    
      Serial.println("Boxfilled");
      Serial1.write(FULL);
      Serial1.flush();
      transmissionTime = millis();
      retransmissions = 0;
      programStatus = waitackfull;
      break;

    case boxemptied:
// Send message to indicate Empty    
      Serial.println("Boxemptied");
      Serial1.write(EMPTY);
      Serial1.flush();
      transmissionTime = millis();
      retransmissions = 0;
      programStatus = waitackempty;
      break;

    case boxfull:
    //Box is full and we have ack
      Serial.println("Boxfull");
      mailBoxStaus = full;
      retransmissions = 0;
      opening_pressed = false;
      door_pressed = false;
      enter_full_sleep();
      break;

    case boxempty:
    // Box is empty and we have ack
      Serial.println("Boxempty");
      mailBoxStaus = empty;
      retransmissions = 0;
      opening_pressed = false;
      door_pressed = false;
      enter_empty_sleep();
      break;

    case waitackfull:
    // Lid opened; wait for ack  
      //Serial.println("Waitackfull");
      if (acknowledged) {
        acknowledged = false;
        programStatus = boxfull;
      } else {
        if ((millis() - transmissionTime > 1000) && (retransmissions < 5)) {  // 5 retransmissions max every second
          Serial1.write(FULL);
          Serial1.flush();
          retransmissions++;
          transmissionTime = millis();
        }
        if (retransmissions >= 5) programStatus = boxfull;  // go to status anyway
      }
      break;

    case waitackempty:
    //door opened; wait for ack
      //Serial.println("Waitackempty");
      if (acknowledged) {
        acknowledged = false;
        programStatus = boxempty;
      } else {
        if ((millis() - transmissionTime > 1000) && (retransmissions < 5)) {  // 5 retransmissions max every second
          Serial1.write(EMPTY);
          Serial1.flush();
          retransmissions++;
          transmissionTime = millis();
        }
        if (retransmissions >= 5) programStatus = boxempty;  // go to status anyway
      }
      break;

    default:
      Serial.println("Default");
      break;
  }

  receive();
}