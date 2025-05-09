#include "RTE_Components.h"
#include CMSIS_device_header

#include "tusb.h"

static uint8_t control_buf[64];

// Handle control request
bool tud_vendor_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request)
{
    if (request->bmRequestType == 0xC0 && request->bRequest == 0x01)
    {
        // Vendor-specific control IN (device to host)
        if (stage == CONTROL_STAGE_SETUP)
        {
            // Prepare data (echo back wValue)
            return tud_control_xfer(rhport, request, "LIOM", 4);
        }
        else if (stage == CONTROL_STAGE_DATA)
        {
            return true;
        }
        else
        {
            return false;
        }
    }
    else if (request->bmRequestType == 0x40 && request->bRequest == 0x02)
    {
        // Vendor-specific control OUT (host to device)
        if (stage == CONTROL_STAGE_SETUP)
        {
            return true;
        }
        else if (stage == CONTROL_STAGE_DATA)
        {
            // Store received data
            uint16_t len = tud_control_xfer(rhport, request, control_buf, request->wLength);
            // Echo back in next control IN
            return len;
        }
        else
        {
            return false;
        }
    }
    else
    {
        // Unknown request, cause stall
        return false;
    }

    // failure, cause stall
    return false;
}

void vendor_task(void)
{
    static uint8_t bulk_buffer[512] = "";
    static int packet_id = 0;
    sprintf(bulk_buffer, "ShowCar packet %d", packet_id++);
    tud_vendor_write(bulk_buffer, strlen(bulk_buffer) + 1);
}

void main(void)
{
    *(volatile uint32_t *)0x4900C004 |= 8;  // 12_0 + 12_3 as output (blue + red LED)
    *(volatile uint32_t *)0x49007004 |= 16; // 7_4 as output (green LED)

    tusb_init();

    while (1)
    {
        tud_task();
        vendor_task();
    }
}

// Stubs to suppress missing stdio definitions for nosys
#define TRAP_RET_ZERO \
    {                 \
        __BKPT(0);    \
        return 0;     \
    }
int _close(int val) TRAP_RET_ZERO;
int _lseek(int val0, int val1, int val2) TRAP_RET_ZERO;
int _read(int val0, char *val1, int val2) TRAP_RET_ZERO;
int _write(int val0, char *val1, int val2) TRAP_RET_ZERO;
int _fstat(int val0, void *val1) TRAP_RET_ZERO;
int _isatty(int val0) TRAP_RET_ZERO;
int _getpid(void) TRAP_RET_ZERO;
void _kill(int val0, int val1) { __BKPT(0); }
