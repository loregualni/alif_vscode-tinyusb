#include "RTE_Components.h"
#include CMSIS_device_header

#include "tusb.h"

#define CTRL_BUFFER_SIZE 64
#define BULK_BUFFER_SIZE 64

#define IS_VENDOR_REQUEST_CMD(req, req_type, req_cmd) \
    ((req->bmRequestType == req_type) &&              \
     (req->bRequest == req_cmd))

#define VENDOR_REQUEST_IN 0xC0
#define VENDOR_REQUEST_READ 0x01
#define IS_VENDOR_REQUEST_READ(req) \
    IS_VENDOR_REQUEST_CMD(req, VENDOR_REQUEST_IN, VENDOR_REQUEST_READ)

#define VENDOR_REQUEST_OUT 0x40
#define VENDOR_REQUEST_WRITE 0x02
#define IS_VENDOR_REQUEST_WRITE(req) \
    IS_VENDOR_REQUEST_CMD(req, VENDOR_REQUEST_OUT, VENDOR_REQUEST_WRITE)

bool tud_vendor_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const *request)
{
    static uint8_t in_buffer[CTRL_BUFFER_SIZE] = "";
    static uint8_t out_buffer[CTRL_BUFFER_SIZE] = "";
    static uint32_t packet_id = 0;

    if (IS_VENDOR_REQUEST_READ(request))
    {
        if (stage == CONTROL_STAGE_SETUP)
        {
            sprintf(out_buffer, "%s from DEVICE (%d)", in_buffer, packet_id++);
            return tud_control_xfer(rhport, request, out_buffer, strlen(out_buffer));
        }
    }
    else if (IS_VENDOR_REQUEST_WRITE(request))
    {
        if (stage == CONTROL_STAGE_SETUP)
        {
            memset(in_buffer, 0x00, CTRL_BUFFER_SIZE);
            return tud_control_xfer(rhport, request, in_buffer, request->wLength);
        }
    }
    else
    {
        // Unknown command, cause stall
        return false;
    }

    return true;
}

void vendor_task(void)
{
    static uint8_t in_buffer[BULK_BUFFER_SIZE] = "";
    static uint8_t out_buffer[BULK_BUFFER_SIZE] = "";
    static int packet_id = 0;

    if (tud_vendor_available())
    {
        memset(in_buffer, 0x00, BULK_BUFFER_SIZE);
        tud_vendor_read(in_buffer, BULK_BUFFER_SIZE);

        sprintf(out_buffer, "%s from DEVICE (%d)", in_buffer, packet_id++);
        tud_vendor_write(out_buffer, strlen(out_buffer));
    }
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
