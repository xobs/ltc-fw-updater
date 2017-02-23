#ifndef __ORCHARD_DEMOD__
#define __ORCHARD_DEMOD__

#define DEMOD_DEBUG 1  // for now, this is the only mode that works...
// to turn off the debug flag, we have to figure out how to back multi-threading into
// the code base. Proceed with caution. Note dataReadyFlag is unprotected but used
// as a sync point, and also measure CPU overhead

#define NB_SAMPLES 8   // number of samples per demod frame
#define NB_FRAMES  32   // number of frames to process at once (trade memory off against OS overhead)

#define BAUD_RATE 8000
#define SAMPLE_RATE 74957  // of the rx side
#define SAMPLE_RATE_TX 44100

#define COS_BITS   14
#define COS_BASE   (1 << COS_BITS)

#define F_LO   8666
#define F_HI   12500

#define FSK_FILTER_SIZE (SAMPLE_RATE / BAUD_RATE)  // <-- this can be 8 or so with optimal bauds
#define FSK_FILTER_BUF_SIZE (FSK_FILTER_SIZE * 2)

#define DMBUF_DEPTH (NB_SAMPLES * NB_FRAMES) 

typedef int32_t demod_sample_t;

extern demod_sample_t dm_buf[];

/* bit I/O for data pumps */
typedef void (*put_bit_func)(int bit);

typedef struct {
    demod_sample_t filter_buf[FSK_FILTER_BUF_SIZE];
    uint8_t buf_ptr;

    int32_t baud_incr;
    int32_t baud_pll, baud_pll_adj, baud_pll_threshold;
    demod_sample_t lastsample;
    int16_t shift;
} FSK_demod_state;

typedef struct {
    /* parameters */
    uint32_t f_lo,f_hi;
    uint32_t sample_rate;
    uint32_t baud_rate;

    /* local variables */
    int32_t filter_size;
    demod_sample_t filter_lo_i[FSK_FILTER_SIZE];
    demod_sample_t filter_lo_q[FSK_FILTER_SIZE];
    
    demod_sample_t filter_hi_i[FSK_FILTER_SIZE];
    demod_sample_t filter_hi_q[FSK_FILTER_SIZE];
} FSK_demod_const;

extern const FSK_demod_const fsk_const;

static inline int dsp_dot_prod(int16_t *tab1, const int16_t *tab2, 
                               int16_t n, int sum)
{
    int i;

    for(i=0;i<n;i++) {
        sum += tab1[i] * tab2[i];
    }

    return sum;
}

void demodInit(void);
void FSKdemod(demod_sample_t *samples, uint32_t nb, put_bit_func put_bit);

#endif
