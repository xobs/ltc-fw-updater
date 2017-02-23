#include "flash.h"

/* Defined in crt-v6m.S */
extern void Reset_Handler(void);

/* Defined in linker script */
extern uint32_t __main_stack_end__;

/* Main entrypoint from system reboot */
__attribute__((naked, noreturn))
int main(void) {
  while (1) {
    ;
  }
}

static void abort(uint32_t reason) {
  (void)reason;
  while (1);
}

void __init_ram_areas(void) {
  #warning "Implement __init_ram_areas()"
}

void __default_exit(void) {
  abort(6);
}

/* Main entrypoint from LtC OS */
__attribute__((naked, noreturn))
void Esplanade_Main(void) {

  extern const uint8_t _cfm[0x10];

  /* Ensure interrupts are disabled, so that the OS' scheduler doesn't
   * interfere with what we're doing here.
   */
  asm("cpsid i");

  /* Erase sector 0 and sector 1.  It is very bad if we crash here. */
  if (F_ERR_OK != flashEraseSectors(0, 2)) {
    abort(1); /* XXX bad */
  }

  /* Program the configuration management bits */
  if (F_ERR_OK != flashProgram(_cfm, (uint8_t *)0x400, sizeof(_cfm))) {
    abort(2); /* XXX also bad */
  }

  /* Program the stack offset */
  if (F_ERR_OK != flashProgram((const uint8_t *)&__main_stack_end__, (uint8_t *)0, 4)) {
    abort(4); /* XXX also bad */
  }

  /* Program the interrupt handlers */
  if (F_ERR_OK != flashProgram((const uint8_t *)Reset_Handler, (uint8_t *)4, 4)) {
    abort(4); /* XXX also bad */
  }
  if (F_ERR_OK != flashProgram((const uint8_t *)Reset_Handler, (uint8_t *)8, 4)) {
    abort(5); /* XXX also bad */
  }

  while (1) {
    ;
  }
}
