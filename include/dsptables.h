#ifndef __DSPTABLES_H__
#define __DSPTABLES_H__
////// THIS FILE IS AUTOMATICALLY GENENERATED DO NOT EDIT
////// 'make dsptables' to build this file

  const FSK_demod_const fsk_const = {
      8666,  12500,  // f_lo, f_hi
     74957,          // sample_rate
      8000,          // baud_rate
         9,          // filter_size
    // filer_lo_i
    {16384, 12248, 1928, -9365, -15930, -14452, -5677, 5963, 14593, },
    // filer_lo_q
    {0, 10882, 16270, 13443, 3829, -7717, -15368, -15260, -7447, },
    // filer_hi_i
    {16384, 8183, -8209, -16383, -8157, 8234, 16383, 8132, -8260, },
    // filer_hi_q
    {0, 14193, 14179, -29, -14208, -14164, 59, 14223, 14149, }
  };
#endif /*__DSPTABLES_H__*/
