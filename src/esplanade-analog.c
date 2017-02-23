#include "esplanade-analog.h"
#include "esplanade-demod.h"
#include "kl02x.h"

static adcsample_t mic_sample[MIC_SAMPLE_DEPTH];
static uint32_t adc_current_index;
static const volatile uint32_t *adc_ra;

/* Global variables, used by the main loop */
volatile uint8_t dataReadyFlag;
volatile adcsample_t *bufloc;
volatile size_t buf_n;

int analogISR(void) {

  /* Read the sample into the buffer */
  mic_sample[adc_current_index++] = *adc_ra;//ADC->RA;

  /* If buffer 2 has filled, deliver samples and move to buffer 1 */
  if (adc_current_index == MIC_SAMPLE_DEPTH) {
    /* Invokes the callback passing the 2nd half of the buffer.*/
    size_t half = MIC_SAMPLE_DEPTH / 2;
    size_t half_index = half * 1;

    bufloc = mic_sample + half_index;
    buf_n = half;
    adc_current_index = 0;
    dataReadyFlag++;
  }

  /* If buffer 1 has filled up, deliver the samples and continue to buffer 2 */
  else if (adc_current_index == (MIC_SAMPLE_DEPTH / 2)) {
    bufloc = mic_sample;
    buf_n = MIC_SAMPLE_DEPTH / 2;
    dataReadyFlag++;
  }

  // Return 0 to indicate we don't want the normal handler to fire
  return 0;
}

static void calibrate(void) {
  /* Clock Divide by 8, Use Bus Clock Div 2 */
  /* At 48MHz this results in ADCCLK of 48/8/2 == 3MHz */
  ADC0->CFG1 = ADCx_CFG1_ADIV(ADCx_CFG1_ADIV_DIV_8) |
      ADCx_CFG1_ADICLK(ADCx_CFG1_ADIVCLK_BUS_CLOCK_DIV_2);

  /* Use software trigger and disable DMA etc. */
  ADC0->SC2 = 0;

  /* Enable Hardware Average, Average 32 Samples, Calibrate */
  ADC0->SC3 = ADCx_SC3_AVGE |
      ADCx_SC3_AVGS(ADCx_SC3_AVGS_AVERAGE_32_SAMPLES) |
      ADCx_SC3_CAL;

  /* FIXME: May take several ms. Use an interrupt instead of busy wait */
  /* Wait for calibration completion */
  while (!(ADC0->SC1A & ADCx_SC1n_COCO))
    ;

  uint16_t gain = ((ADC0->CLP0 + ADC0->CLP1 + ADC0->CLP2 +
      ADC0->CLP3 + ADC0->CLP4 + ADC0->CLPS) / 2) | 0x8000;
  ADC0->PG = gain;

  gain = ((ADC0->CLM0 + ADC0->CLM1 + ADC0->CLM2 +
      ADC0->CLM3 + ADC0->CLM4 + ADC0->CLMS) / 2) | 0x8000;
  ADC0->MG = gain;

}
/*
 * ADC conversion group.
 * Mode:        Linear buffer, 8 samples of 1 channel, SW triggered.
 */
/*
static const ADCConversionGroup adcgrpmic = {
  true, // circular buffer mode
  1, // just one channel
  adc_mic_end_cb,  // callback
  NULL,  // error callback
  
  
  
};
*/

void analogUpdateMic(void) {
 /* Set clock speed and conversion size */

  // CFG1 register
  // SYSCLK = 48MHz.
  // BUSCLK = SYSCLK / 4 = 12MHz
  // ADCCLK = SYSCLK / 2 / 1 = 6 MHz
  
  // ADLPC = 0 (normal power)
  // ADLSMP = 0 (short sample time)
  // ADHSC = 0 (normal conversion sequence)
  // -> 5 ADCK cycles + 5 bus clock cycles = SFCadder
  // 20 ADCK cycles per sample
  ADC0->CFG1 = ADCx_CFG1_ADIV(ADCx_CFG1_ADIV_DIV_2) |
               ADCx_CFG1_ADICLK(ADCx_CFG1_ADIVCLK_BUS_CLOCK_DIV_2) |
               ADCx_CFG1_MODE(ADCx_CFG1_MODE_12_OR_13_BITS);  // 12 bits per sample

  /* Set averaging */
  // SC3 register
  //      48 MHz sysclk
  // /2   24 MHz busclk
  // /2   12 MHz after prescaler
  // /2   6 MHz after adiv
  // /20  300ksps after base sample time @ 12 bps
  // /4   75ksps after averaging by factor of 4
  ADC0->SC3 = ADCx_SC3_ADCO |   // continuous conversions
              ADCx_SC3_AVGE |
              ADCx_SC3_AVGS(ADCx_SC3_AVGS_AVERAGE_4_SAMPLES); // 4 sample average

  /* Enable Interrupt, Select Channel */
  ADC0->SC1A = ADCx_SC1n_AIEN | ADCx_SC1n_ADCH(ADC_DAD1); // microphone input channel
}

void analogStart(void) {
  /* Ungate the ADC */
  SIM->SCGC6 |= SIM_SCGC6_ADC0;

  calibrate();

  adc_ra = &ADC0->RA;
}

void analogStop(void) {
  /* Disable everything */
  ADC0->CFG1 = 0;
  ADC0->CFG2 = 0;
  ADC0->SC2  = 0;
  ADC0->SC3  = 0;

  /* Disable Interrupt, Disable Channel */
  ADC0->SC1A = ADCx_SC1n_ADCH(ADCx_SC1n_ADCH_DISABLED);

  /* If in ready state then disables the ADC clock.*/
  SIM->SCGC6 &= ~SIM_SCGC6_ADC0;
}
