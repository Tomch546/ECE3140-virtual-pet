// LED setup and functions
#include <MKL46Z4.h>

void LED_Initialize(void) {

  SIM->SCGC5 |= (1 << 13) | (1 << 12); /* Enable Clock to Port D & E */
  PORTE->PCR[29] = (1 << 8);           /* Pin PTE29 is GPIO */
  PORTD->PCR[5] = (1 << 8);            /* Pin PTD5  is GPIO */

  PTE->PDOR = (1 << 29); /* switch Red LED off    */
  PTE->PDDR = (1 << 29); /* enable PTE29 as Output */

  PTD->PDOR = 1 << 5; /* switch Greed LED off  */
  PTD->PDDR = 1 << 5; /* enable PTD5 as Output */
}

void LEDRed_Toggle(void) {
  PTE->PTOR = 1 << 29; /* Red LED Toggle */
}

void LEDGreen_Toggle(void) {
  PTD->PTOR = 1 << 5; /* Green LED Toggle */
}

void LED_Off(void) {
  // Save and disable interrupts (for atomic LED change)
  uint32_t m;
  m = __get_PRIMASK();
  __disable_irq();

  PTD->PSOR = 1 << 5;  /* Green LED Off*/
  PTE->PSOR = 1 << 29; /* Red LED Off*/

  // Restore interrupts
  __set_PRIMASK(m);
}