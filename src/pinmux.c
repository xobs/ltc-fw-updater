#include <stdio.h>
#include "memio.h"
#include "kl02.h"

void setupPinMux(void) {
  /* PTB5 - Audio */
  writel((0 << 8), PORTB_PCR5);

  /* PTB6 - Prog Status Green -- output, slow slew rate */
  writel((1 << 8) | (1 << 2), PORTB_PCR6);
  /* PTB6 - configure as output */
  writel(readl(GPIOB_PDDR) | (1 << 6), GPIOB_PDDR);

  /* PTA5 - Prog Status Red -- output, slow slew rate */
  writel((1 << 8) | (1 << 2), PORTA_PCR5);
  /* PTA5 - configure as output */
  writel(readl(GPIOA_PDDR) | (1 << 5), GPIOA_PDDR);

  /* PTA0 - SWCLK [Alternate Function 3] */
  writel((3 << 8), PORTA_PCR0);

  /* PTA2 - SWDIO [Alternate Function 3] */
  writel((3 << 8), PORTA_PCR2);
}
