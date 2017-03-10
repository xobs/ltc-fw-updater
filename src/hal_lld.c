/*
    ChibiOS - Copyright (C) 2013-2015 Fabio Utzig

    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at

        http://www.apache.org/licenses/LICENSE-2.0

    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
*/

/**
 * @file    KL2x/hal_lld.c
 * @brief   Kinetis KL2x HAL Driver subsystem low level driver source.
 *
 * @addtogroup HAL
 * @{
 */

#define KINETIS_MCG_FLL_DMX32       1           /* Fine-tune for 32.768 kHz */
#define KINETIS_MCG_FLL_DRS         1           /* 1464x FLL factor */
#define KINETIS_MCG_FLL_OUTDIV1     1           /* Divide 48 MHz FLL by 1 => 48 MHz */
#define KINETIS_MCG_FLL_OUTDIV4     2           /* Divide OUTDIV1 output by 2 => 24 MHz */
#define KINETIS_SYSCLK_FREQUENCY    47972352UL  /* 32.768 kHz * 1464 (~48 MHz) */

#include "osal.h"
#include "hal.h"


/*===========================================================================*/
/* Driver local definitions.                                                 */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported variables.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver local variables and types.                                         */
/*===========================================================================*/

/* This should get copied to offset 0x400 */
__attribute__ ((used))
const uint8_t _cfm[0x10] = {
  0xFF,  /* NV_BACKKEY3: KEY=0xFF */
  0xFF,  /* NV_BACKKEY2: KEY=0xFF */
  0xFF,  /* NV_BACKKEY1: KEY=0xFF */
  0xFF,  /* NV_BACKKEY0: KEY=0xFF */
  0xFF,  /* NV_BACKKEY7: KEY=0xFF */
  0xFF,  /* NV_BACKKEY6: KEY=0xFF */
  0xFF,  /* NV_BACKKEY5: KEY=0xFF */
  0xFF,  /* NV_BACKKEY4: KEY=0xFF */
  0xFF,  /* NV_FPROT3: PROT=0xFF */
  0xFF,  /* NV_FPROT2: PROT=0xFF */
  0xFF,  /* NV_FPROT1: PROT=0xFF */
  0xFF,  /* NV_FPROT0: PROT=0xFF */
  0x7E,  /* NV_FSEC: KEYEN=1,MEEN=3,FSLACC=3,SEC=2 */
  0xFB,  /* NV_FOPT: ??=1,??=1,FAST_INIT=1,LPBOOT1=1,RESET_PIN_CFG=1,
                      NMI_DIS=0,EZPORT_DIS=1,LPBOOT0=1 */
  0xFF,
  0xFF
};

/*===========================================================================*/
/* Driver local functions.                                                   */
/*===========================================================================*/

/*===========================================================================*/
/* Driver interrupt handlers.                                                */
/*===========================================================================*/

/*===========================================================================*/
/* Driver exported functions.                                                */
/*===========================================================================*/

/**
 * @brief   Low level HAL driver initialization.
 *
 * @notapi
 */
void hal_lld_init(void) {
}

/**
 * @brief   KL2x clocks and PLL initialization.
 * @note    All the involved constants come from the file @p board.h.
 * @note    This function should be invoked just after the system reset.
 *
 * @special
 */
void __early_init(void) {
#if !KINETIS_NO_INIT
  /* Disable COP watchdog */
  SIM->COPC = 0;

  /* Enable PORTA and PORGB */
  SIM->SCGC5 |= SIM_SCGC5_PORTA | SIM_SCGC5_PORTB;

  /* --- MCG mode: FEI (default out of reset) ---
     f_MCGOUTCLK = f_int * F
     F is the FLL factor selected by C4[DRST_DRS] and C4[DMX32] bits.
     Typical f_MCGOUTCLK = 21 MHz immediately after reset.
     C4[DMX32]=0 and C4[DRST_DRS]=00  =>  FLL factor=640.
     C3[SCTRIM] and C4[SCFTRIM] factory trim values apply to f_int. */

  /* System oscillator drives 32 kHz clock (OSC32KSEL=0) */
  //  SIM->SOPT1 &= ~SIM_SOPT1_OSC32KSEL_MASK;

  /*
   * FLL Enabled External (FEE) MCG Mode
   * 24 MHz core, 12 MHz bus - using 32.768 kHz crystal with FLL.
   * f_MCGOUTCLK = (f_ext / FLL_R) * F
   *             = (32.768 kHz ) *
   *  FLL_R is the reference divider selected by C1[FRDIV]
   *  F is the FLL factor selected by C4[DRST_DRS] and C4[DMX32].
   *
   * Then the core/system and bus/flash clocks are divided:
   *   f_SYS = f_MCGOUTCLK / OUTDIV1 = 48 MHz / 1 = 48 MHz
   *   f_BUS = f_MCGOUTCLK / OUTDIV1 / OUTDIV4 =  MHz / 4 = 24 MHz
   */

  SIM->SOPT2 =
          SIM_SOPT2_TPMSRC(1);  /* MCGFLLCLK clock or MCGPLLCLK/2 */
          /* PLLFLLSEL=0 -> MCGFLLCLK */

  /* The MCGOUTCLK is divided by OUTDIV1 and OUTDIV4:
   * OUTDIV1 (divider for core/system and bus/flash clock)
   * OUTDIV4 (additional divider for bus/flash clock) */
  SIM->CLKDIV1 =
          SIM_CLKDIV1_OUTDIV1(KINETIS_MCG_FLL_OUTDIV1 - 1) |
          SIM_CLKDIV1_OUTDIV4(KINETIS_MCG_FLL_OUTDIV4 - 1);

  /* EXTAL0 and XTAL0 */
  //  PORTA->PCR[18] &= ~0x01000700; /* Set PA18 to analog (default) */  // defaults should already be good
  //  PORTA->PCR[19] &= ~0x01000700; /* Set PA19 to analog (default) */

  OSC0->CR = 0xC;

  /* From KL25P80M48SF0RM section 24.5.1.1 "Initializing the MCG". */
  /* To change from FEI mode to FEE mode: */
  /* (1) Select the external clock source in C2 register.
         Use low-power OSC mode (HGO0=0) which enables internal feedback
         resistor, for 32.768 kHz crystal configuration.  */
  MCG->C2 =
          MCG_C2_RANGE0(0) |  /* low frequency range (<= 40 kHz) */
          MCG_C2_EREFS0;      /* external reference (using a crystal) */
  /* (2) Write to C1 to select the clock mode. */
  MCG->C1 = /* Clear the IREFS bit to switch to the external reference. */
          MCG_C1_CLKS_FLLPLL |  /* Use FLL for system clock, MCGCLKOUT. */
          MCG_C1_FRDIV(0);      /* Don't divide 32kHz ERCLK FLL reference. */
  MCG->C6 = 0;  /* PLLS=0: Select FLL as MCG source, not PLL */

  /* Loop until S[OSCINIT0] is 1, indicating the
     crystal selected by C2[EREFS0] has been initialized. */
  while ((MCG->S & MCG_S_OSCINIT0) == 0)
    ;
  /* Loop until S[IREFST] is 0, indicating the
     external reference is the current reference clock source. */
  while ((MCG->S & MCG_S_IREFST) != 0)
    ;  /* Wait until external reference clock is FLL reference. */
  /* (1)(e) Loop until S[CLKST] indicates FLL feeds MCGOUTCLK. */
  while ((MCG->S & MCG_S_CLKST_MASK) != MCG_S_CLKST_FLL)
    ;  /* Wait until FLL has been selected. */

  /* --- MCG mode: FEE --- */
  /* Set frequency range for DCO output (MCGFLLCLK). */
  MCG->C4 = (KINETIS_MCG_FLL_DMX32 ? MCG_C4_DMX32 : 0) |
            MCG_C4_DRST_DRS(KINETIS_MCG_FLL_DRS);

  /* Wait for the FLL lock time; t[fll_acquire][max] = 1 ms */
  /* TODO - not implemented - is it required? Freescale example code
     seems to omit it. */

#else
  #error "Unimplemented"
#endif /* !KINETIS_NO_INIT */
}

/**
 * @brief   Platform early initialization.
 * @note    All the involved constants come from the file @p board.h.
 * @note    This function is meant to be invoked early during the system
 *          initialization, it is usually invoked from the file
 *          @p board.c.
 *
 * @special
 */
void platform_early_init(void) {

}

/** @} */
