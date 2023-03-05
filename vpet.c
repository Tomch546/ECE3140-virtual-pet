// Main FSM file
// written by az292, tc575

// Serial Output
#include "board.h"
#include "fsl_debug_console.h"
#include "fsl_device_registers.h"
#include "led.h"
#include "pin_mux.h"
#include <MKL46Z4.h>
#include "touch_slider.h"

// Time Delay
#include <stdio.h>
#include <time.h>

// Variables
int run;
volatile unsigned int inputA;
volatile unsigned int inputB;
int state;
int eatcounter;
int playcounter;
int direction;
volatile int state_idle;
volatile int state_idle_direction;
int bgcounter;

// Switch constants
const int SWITCH_1_PIN = 3; // switch 2 pin is 12
const int SWITCH_2_PIN = 12;
const int RED_LED_PIN = 29;
const int GREEN_LED_PIN = 5;
SIM_Type *global_SIM = SIM;
PORT_Type *global_PORTE = PORTE;
GPIO_Type *global_PTE = PTE;
PORT_Type *global_PORTC = PORTC;
GPIO_Type *global_PTC = PTC;

void setupswitch() {

  // setup switch 1
  SIM->SCGC5 |= SIM_SCGC5_PORTC_MASK;                 // Enable the clock to port C
  PORTC->PCR[SWITCH_1_PIN] &= ~PORT_PCR_MUX(0b111);   // Clear PCR Mux bits for PTC3
  PORTC->PCR[SWITCH_1_PIN] |= PORT_PCR_MUX(0b001);    // Set up PTC3 as GPIO
  PTC->PDDR &= ~GPIO_PDDR_PDD(1 << SWITCH_1_PIN);     // make it input
  PORTC->PCR[SWITCH_1_PIN] |= PORT_PCR_PE(1);         // Turn on the pull enable
  PORTC->PCR[SWITCH_1_PIN] |= PORT_PCR_PS(1);         // Enable the pullup resistor
  PORTC->PCR[SWITCH_1_PIN] &= ~PORT_PCR_IRQC(0b1111); // Clear IRQC bits for PTC3
  PORTC->PCR[SWITCH_1_PIN] |= PORT_PCR_IRQC(0b1011);  // trigger once

  // Setup switch 2
  PORTC->PCR[SWITCH_2_PIN] &= ~PORT_PCR_MUX(0b111);   // Clear PCR Mux bits for PTC2
  PORTC->PCR[SWITCH_2_PIN] |= PORT_PCR_MUX(0b001);    // Set up PTC2 as GPIO
  PTC->PDDR &= ~GPIO_PDDR_PDD(1 << SWITCH_2_PIN);     // make it input
  PORTC->PCR[SWITCH_2_PIN] |= PORT_PCR_PE(1);         // Turn on the pull enable
  PORTC->PCR[SWITCH_2_PIN] |= PORT_PCR_PS(1);         // Enable the pullup resistor
  PORTC->PCR[SWITCH_2_PIN] &= ~PORT_PCR_IRQC(0b1111); // Clear IRQC bits for PTC2
  PORTC->PCR[SWITCH_2_PIN] |= PORT_PCR_IRQC(0b1011);  // trigger once
}

void setupPIT() {
  // PIT 0
  SIM->SCGC6 |= SIM_SCGC6_PIT_MASK;            // Enable clock to PIT module
  PIT->MCR &= ~PIT_MCR_MDIS_MASK;              // Enable the standard timer
  PIT->CHANNEL[0].TCTRL |= PIT_TCTRL_TEN_MASK; // Enable the timer
  PIT->CHANNEL[0].TCTRL |= PIT_TCTRL_TIE_MASK; // Enable the timer interrupt
  PIT->CHANNEL[0].LDVAL = 200000000;           // Set load value of zeroth PIT

  NVIC_EnableIRQ(PIT_IRQn); // Enable ISR with the PIT interrupt
}

void PIT_IRQHandler(void) {
  LEDGreen_Toggle();

  if (state_idle == 1) {
    state_idle = 0;

    if (state_idle_direction == 0) {
      PRINTF("state3\r\n"); // jump left
      state_idle_direction = 1;
    }
    else {
      PRINTF("state13\r\n"); // jump right
      state_idle_direction = 0;
    }
  }

  if (bgcounter == 3) {
	  PRINTF("b1\r\n");
	  bgcounter ++;
  }
  else if (bgcounter == 7) {
	  PRINTF("b2\r\n");
	  bgcounter ++;
  }
  else if (bgcounter == 11) {
	  PRINTF("b3\r\n");
	  bgcounter = 0;
  }
  else {
	  bgcounter ++;
  }

  PIT->CHANNEL[0].TFLG |= PIT_TFLG_TIF_MASK; // Write 1 to flag to clear
}

void LCD_TimeDelay(uint32_t count) {
  while (count--) {
    __NOP();
  }
}

void checkInputs() {
  if ((PORTC->PCR[SWITCH_1_PIN] & PORT_PCR_ISF(1)) != 0) {
    PTE->PTOR = GPIO_PTOR_PTTO(1 << RED_LED_PIN);
    inputA = 1;
    PORTC->PCR[SWITCH_1_PIN] |= PORT_PCR_ISF(1);
  }
  else if ((PORTC->PCR[SWITCH_2_PIN] & PORT_PCR_ISF(1)) != 0) {
    PTE->PTOR = GPIO_PTOR_PTTO(1 << RED_LED_PIN);
    inputB = 1;
    PORTC->PCR[SWITCH_2_PIN] |= PORT_PCR_ISF(1);
  }
}

int check_slider() {
  uint16_t temp[4];
  uint16_t curr_value, prev_value, i, j, empty;
  int cmp;
  empty = 0;
  while (Touch_Scan_LH() < 50) {
    empty = empty + 1;
    LCD_TimeDelay(0x8FFFU);
    if (empty > 75)
      return 0;
  };
  i = 0;
  j = 0;
  prev_value = 0;
  curr_value = 0;
  cmp = 0;
  while (i < 8) {
    while (j < 4) {
      int value = Touch_Scan_LH(); // Get the touch sensor input
      if ((value < 9999) & (value > 0)) {
        temp[j] = value;
        LCD_TimeDelay(0x2000U);
        curr_value = curr_value + temp[j];
        j = j + 1;
      }
    }
    curr_value = curr_value / 4;
    //    displayDecimal(curr_value);
    LCD_TimeDelay(0xFFFFFU);
    if (i > 0)
      cmp = cmp + (curr_value > prev_value);
    prev_value = curr_value;
    curr_value = 0;
    i = i + 1;
    j = 0;
  }

  LCD_TimeDelay(0xFFFFFU);
  if (cmp >= 3) {
    return 1; // swipe right
  }
  else if (cmp <= 2) {
    return 2; // swipe left
  }
  else {
    return 0; // no turn
  }
}

// Setup
void initialize() {
  // Initialize board hardware
  LED_Initialize();
  BOARD_InitPins();
  BOARD_BootClockRUN();
  BOARD_InitDebugConsole();
  Touch_Init();
  setupPIT();
  setupswitch();
  // initialize variables
  state = 1;
  run = 1;
  eatcounter = 0;
  playcounter = 0;
  inputA = 0;
  inputB = 0;
  state_idle = 0;
  state_idle_direction = 0;
  bgcounter = 0;
}

// Delay function
void longdelay(void) {
  int j;
  int i;
  for (i = 0; i < 9; i++) {
    for (j = 0; j < 1000000; j++) {
    }
  }
}
void shortdelay(void) {
  int j;
  int i;
  for (i = 0; i < 3; i++) {
    for (j = 0; j < 1000000; j++) {
    }
  }
}

//////
// Main FSM
//////
void FSM() {
  while (run == 1) {

    switch (state) {

      // Hatch State = 1
    case 1:
      PRINTF("b1\r\n"); // setup background
      PRINTF("state1\r\n");
      shortdelay();
      state = 2;
      break;

      // State = 2
    case 2:
      checkInputs();
      shortdelay();
      direction = check_slider();
      // play -> input 1
      if (inputA == 1) {
        inputA = 0;
        LEDRed_Toggle();
        state = 5;
      }
      // eat -> input 2
      else if (inputB == 1) {
        inputB = 0;
        LEDRed_Toggle();
        state = 4;
      }
      // bounce -> slider input
      else if (direction != 0) {
        state = 6;
      }
      // idle -> directly
      else {
        state = 3;
      }
      break;

      // Idle State = 3
    case 3:
      state_idle = 1;
      shortdelay();
      state = 2;
      break;

      // Eat State = 4, 15
    case 4:
      if (eatcounter == 0){
      	  PRINTF("state4\r\n");
      	  eatcounter = 1;
      }
      else {
    	  PRINTF("state15\r\n");
    	  eatcounter = 0;
      }
      longdelay();
      longdelay();
      longdelay();
      state = 2;
      break;

      // Play State = 5, 16
    case 5:
      if (playcounter == 0){
    	  PRINTF("state5\r\n");
    	  playcounter = 1;
      }
      else {
    	  PRINTF("state16\r\n");
    	  playcounter = 0;
      }
      longdelay();
      longdelay();
      state = 2;
      break;

      // Bounce State = 6, 14
    case 6:
      if (direction == 2) {
        PRINTF("state6\r\n");
        longdelay();
        shortdelay();
      } else if (direction == 1) {
        PRINTF("state14\r\n");
        longdelay();
        shortdelay();
      }
      state = 2;
      break;

    default:
      state = 1;
    }
  }
}

// Main function
int main(void) {
  initialize();
  FSM();

  // End Sequence
  while (1) {
  }
}
