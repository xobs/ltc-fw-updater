#ifndef __ANALOG_H__
#define __ANALOG_H__

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "esplanade-demod.h"

#define MIC_SAMPLE_DEPTH  (DMBUF_DEPTH * 2)  // have to double-buffer around demodulation buffer depth

typedef uint16_t adcsample_t;

extern volatile uint8_t dataReadyFlag;
extern volatile adcsample_t *bufloc;
extern volatile size_t buf_n;

void analogUpdateTemperature(void);
int32_t analogReadTemperature(void);

void analogUpdateMic(void);

void analogStart(void);
void analogStop(void);

#endif /*__ANALOG_H__*/
