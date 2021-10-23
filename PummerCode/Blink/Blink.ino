#include <avr/sleep.h>
#include <avr/wdt.h>  // Watchdog Timer included to wake MCU from sleep

#define CELL_READ 3  // PB3
#define LED1 0 //PB0
#define LED2 1 //PB1

// Utility Macro
#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif

#define SINE_LENGTH 17
uint8_t sine_pattern[] = {0,25,50,74,98,120,142,162,180,197,212,225,236,244,250,254,255};
#define EXP_DECAY_LENGTH 7
uint8_t exp_decay[]= {255,94,35,13,5,2,0};
#define EXP_LENGTH 6
uint8_t exp_pattern[] = {2,5,13,35,94,255};
#define HEARTBEAT_LENGTH 19
uint8_t heartbeat[] = {50,100,150,200,255,200,150,100,50,0,0,25,50,100,127,100,50,25,0};

volatile bool wdt_flag = 1;
uint8_t wdt_delay = 9;

#define PATTERN1 0
#define PATTERN2 1
bool led2_flag = PATTERN1;

// 0=16ms, 1=32ms,2=64ms,3=128ms,4=250ms,5=500ms
// 6=1 sec,7=2 sec, 8=4 sec, 9= 8sec

void setup_watchdog(int ii) {
  byte bb;
  int ww;
  if (ii > 9 ) ii=9;
  bb=ii & 7;
  if (ii > 7) bb|= (1<<5);
  bb|= (1<<WDCE);
  ww=bb;
  MCUSR &= ~(1<<WDRF);
  // start timed sequence
  WDTCR |= (1<<WDCE) | (1<<WDE);
  // set new watchdog timeout value
  WDTCR = bb;
  WDTCR |= _BV(WDTIE);
}

// Watchdog Interrupt Service / is executed when watchdog timed out

ISR(WDT_vect) {
  wdt_flag=1;  // set global flag
}

void system_sleep(void) 
{
  setup_watchdog(wdt_delay); 
  cbi(ADCSRA,ADEN);                    // switch Analog to Digitalconverter OFF
  set_sleep_mode(SLEEP_MODE_PWR_DOWN); // sleep mode is set here
  sleep_enable();
  sleep_mode();                        // System sleeps here
  sleep_disable();                     // System continues execution here when watchdog timed out 
  sbi(ADCSRA,ADEN);                    // switch Analog to Digitalconverter ON

}

void flash_light(uint8_t pin, uint8_t levels[], uint8_t num, uint8_t interval) {
  pinMode(pin, OUTPUT);
  for(uint8_t i = 0; i < num; i++){
    analogWrite(pin, levels[i]);
    delay(interval);
  }
}

void heartbeat_pattern(uint8_t pin){
  flash_light(pin, heartbeat, HEARTBEAT_LENGTH, 30);
  analogWrite(pin, 0);
  pinMode(pin, INPUT);
}

void slow_blink_pattern(uint8_t pin){
  flash_light(pin, sine_pattern, SINE_LENGTH, 80);
  flash_light(pin, exp_decay, EXP_DECAY_LENGTH, 80);
  analogWrite(pin, 0);
  pinMode(pin, INPUT);
}

void fast_blink_pattern(uint8_t pin){
  flash_light(pin, exp_pattern, EXP_LENGTH, 30);
  flash_light(pin, exp_decay, EXP_DECAY_LENGTH, 30);
  analogWrite(pin, 0);
  pinMode(pin, INPUT);
}


void setup() {
  // Setup non-used pins with pullup resistors
  pinMode(2, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);

  // Solar Panel Pin
  pinMode(CELL_READ, INPUT);
  setup_watchdog(wdt_delay);  // Set Timer to 8s
  
  
}
void loop() {
  if (wdt_flag == 1) {
    wdt_flag = 0;

    if (digitalRead(CELL_READ) == 0) {
      heartbeat_pattern(LED1);
      delay(250);
      fast_blink_pattern(LED2);
      
      wdt_delay = 8;
    } else {
      wdt_delay = 9;
    }
    
    system_sleep();
  }
} 
