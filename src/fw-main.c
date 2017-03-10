#include "kl02x.h"
#include "flash.h"
#include "esplanade-analog.h"
#include "esplanade-demod.h"
#include "esplanade-mac.h"
#include "esplanade-updater.h"

/* Defined in crt-v6m.S */
extern void Reset_Handler(void);

/* Defined in esplanade-analog.c */
extern void ADC0ISR(void);

/* Defined in pinmux.c */
extern void setupPinMux(void);

/* Defined in __aeabi.c */
void *memset_aligned(void *dst0, int val, size_t length);

/* Defined in __aeabi.c */
void *memcpy_aligned(void *dest, const void *src, size_t n);

/* Defined in linker script */
extern uint32_t __main_stack_end__;

extern void analogStart(void);
extern void analogUpdateMic(void);

static void phy_demodulate(void) {
  int frames;

  for (frames = 0; frames < NB_FRAMES; frames++)
    // putBitMac is callback to MAC layer
    FSKdemod(dm_buf + (frames * NB_SAMPLES), NB_SAMPLES, putBitMac);

  dataReadyFlag = 0;
}

/* Main entrypoint from system reboot */
int main(void) {
  unsigned int i;

  SIM->SCGC5 |= SIM_SCGC5_PORTA | SIM_SCGC5_PORTB;
  SIM->SCGC6 |= SIM_SCGC6_FTF | SIM_SCGC6_ADC0;
  setupPinMux();
  analogStart();
  demodInit();

  /* Start sampling the microphone pin */
  __enable_irq();
  analogUpdateMic();
  NVIC_EnableIRQ(ADC0_IRQn);

  while (1) {
    while (!pktReady) {
      if (dataReadyFlag) {
        // copy from the double-buffer into a demodulation buffer
        for (i = 0 ; i < buf_n; i++) {
          dm_buf[i] = (int16_t) (((int16_t) bufloc[i]) - 2048);
        }
        // call handler, which includes the demodulation routine
        phy_demodulate();
      }
    }

    // unstripe the transition xor's used to keep baud sync
    if (pkt.header.type == PKTTYPE_DATA_OS) {
      /* We don't xor the header or the ending hash, but xor everything else */
      for (i = sizeof(pkt.header);
           i < sizeof(pkt.data_pkt) - sizeof(pkt.data_pkt.hash);
           i++) {
        if ((i % 16) == 7)
          ((uint8_t *)&pkt)[i] ^= 0x55;
        else if ((i % 16) == 15)
          ((uint8_t *)&pkt)[i] ^= 0xAA;
      }
    }

    updaterPacketProcess(&pkt);
  }
}

static void error(uint32_t reason) {
  (void)reason;
  while (1);
}

/* Some functions (particularly flash functions) opreate out of RAM */
static void init_ramtext(void) {
  extern uint32_t _ramtext_start, _ramtext_flash_start, _ramtext_end;
  memcpy_aligned(&_ramtext_start, &_ramtext_flash_start, ((uint32_t)&_ramtext_end) - ((uint32_t)&_ramtext_start));
}

void __init_ram_areas(void) {
  extern uint32_t _bss_start, _bss_end;
  extern uint32_t _data_start, _data_flash_start, _data_end;

  memset_aligned(&_bss_start, 0, ((uint32_t)&_bss_end) - ((uint32_t)&_bss_start));
  memcpy_aligned(&_data_start, &_data_flash_start, ((uint32_t)&_data_end) - ((uint32_t)&_data_start));
}

void __default_exit(void) {
  error(7);
}

/* Main entrypoint from LtC OS */
__attribute__((naked, noreturn))
void Esplanade_Main(void) {

  extern const uint8_t _cfm[0x10];
  static uint32_t val;

  /* Ensure interrupts are disabled, so that the OS' scheduler doesn't
   * interfere with what we're doing here.
   */
  __disable_irq();

  /* Allow us to call flash commands, which run from RAM */
  init_ramtext();

  /* Erase sector 0 and sector 1.  It is very bad if we crash here. */
  if (F_ERR_OK != flashEraseSectors(0, 2)) {
    error(1); /* XXX bad */
  }

  /* Program the configuration management bits */
  if (F_ERR_OK != flashProgram(&_cfm, (uint8_t *)0x400, sizeof(_cfm))) {
    error(2); /* XXX also bad */
  }

  /* Program the stack offset */
  val = (uint32_t)&__main_stack_end__;
  if (F_ERR_OK != flashProgram((const uint8_t *)&val, (uint8_t *)0, 4)) {
    error(3); /* XXX also bad */
  }

  /* Program the interrupt handlers */
  val = (uint32_t)Reset_Handler;
  val |= 1; /* Enable Thumb mode */
  if (F_ERR_OK != flashProgram((const uint8_t *)&val, (uint8_t *)4, 4)) {
    error(4); /* XXX also bad */
  }
  val = (uint32_t)Reset_Handler;
  val |= 1; /* Enable Thumb mode */
  if (F_ERR_OK != flashProgram((const uint8_t *)&val, (uint8_t *)8, 4)) {
    error(5); /* XXX also bad */
  }

  /* Program the analog ISR handler */
  val = (uint32_t)ADC0ISR;
  val |= 1; /* Enable Thumb mode */
  if (F_ERR_OK != flashProgram((const uint8_t *)&val, (uint8_t *)0x7c, 4)) {
    error(6); /* XXX also bad */
  }

  /* Zero-out the OS storage area */
  updaterInitialize();

  /* This will break into a debugger, if one is attached.
   * Or it will reboot the system if one is not.
   */
  asm("bkpt #0");
  while (1) {
    ;
  }
}
