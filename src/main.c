#include "RTE_Components.h"
#include CMSIS_device_header

#include "power.h"
#include "pinconf.h"

#include "tusb.h"

static uint8_t charbuf[512];

static cdc_line_coding_t lc __attribute__((used));

void main (void)
{
    *(volatile uint32_t*) 0x4900C004 |= 8; // 12_0 + 12_3 as output (blue + red LED)
    *(volatile uint32_t*) 0x49007004 |= 16; // 7_4 as output (green LED)

    // *(volatile uint32_t*) 0x49004004 |= 1; // 4_0 out
    // *(volatile uint32_t*) 0x49004000 |= 1; // 4_0 hi

    tusb_init();

    while (1) {
        tud_task();
        if (tud_cdc_available()) {
            uint32_t len = tud_cdc_read(charbuf, sizeof(charbuf));
            tud_cdc_write(charbuf, len);
            tud_cdc_write_flush();
        }
    }
}

void tud_cdc_line_coding_cb(uint8_t itf, cdc_line_coding_t const* p_line_coding)
{
    // pass LC state to the main loop; for some reason this is called twice
    lc = *p_line_coding;
}

void tud_cdc_line_state_cb(uint8_t itf, bool dtr, bool rts)
{
    // signal DTR state using green LED
    // if (true == dtr) {
    //     *(volatile uint32_t*) 0x49007000 |= 16;
    // } else {
    //     *(volatile uint32_t*) 0x49007000 &= ~16;
    // }
}

// Stubs to suppress missing stdio definitions for nosys
#define TRAP_RET_ZERO  {__BKPT(0); return 0;}
int _close(int val) TRAP_RET_ZERO
int _lseek(int val0, int val1, int val2) TRAP_RET_ZERO
int _read(int val0, char * val1, int val2) TRAP_RET_ZERO
int _write(int val0, char * val1, int val2) TRAP_RET_ZERO
int _fstat(int val0, void * val1) TRAP_RET_ZERO
int _isatty(int val0) TRAP_RET_ZERO
int _getpid(void) TRAP_RET_ZERO
void _kill(int val0, int val1) {__BKPT(0);}