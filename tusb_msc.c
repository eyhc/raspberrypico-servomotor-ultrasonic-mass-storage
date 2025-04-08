#include "tusb.h"
#include "ff.h"
#include "diskio.h"
#include "tusb_msc.h"

// callback function
static event_cb *cb_fct = NULL;
void usb_set_callback(event_cb *f) {
    cb_fct = f;
}


// Invoked when received SCSI_CMD_INQUIRY
void tud_msc_inquiry_cb(
    uint8_t lun,
    uint8_t vendor_id[8],
    uint8_t product_id[16],
    uint8_t product_rev[4]
) {
    const char vid[] = "TinyUSB";
    const char pid[] = "Mass Storage";
    const char rev[] = "1.0";

    memcpy(vendor_id  , vid, strlen(vid));
    memcpy(product_id , pid, strlen(pid));
    memcpy(product_rev, rev, strlen(rev));
}

// Invoked when received Test Unit Ready command.
bool tud_msc_test_unit_ready_cb(uint8_t lun)
{
    return true;
}

// Invoked when received SCSI_CMD_READ_CAPACITY_10 and SCSI_CMD_READ_FORMAT_CAPACITY
void tud_msc_capacity_cb(uint8_t lun, uint32_t* block_count, uint16_t* block_size)
{
    disk_ioctl(0, GET_SECTOR_COUNT, block_count);
    disk_ioctl(0, GET_SECTOR_SIZE, block_size);
}

// Callback invoked when received READ10 command.
int32_t tud_msc_read10_cb(
    uint8_t lun,
    uint32_t lba,
    uint32_t offset,
    void* buffer,
    uint32_t bufsize
) {
    DRESULT res;
    WORD ss;
    res = disk_ioctl(0, GET_SECTOR_SIZE, &ss);
    if (res != RES_OK) return -1;

    // Check for overflow of offset + bufsize
    if (offset + bufsize > ss) {
        return -1;
    }

    uint8_t temp_buffer[ss];
    res = disk_read(0, temp_buffer, lba, 1);
    if (res != RES_OK) return -1;

    memcpy(buffer, temp_buffer + offset, bufsize);

    // reading callback
    if (cb_fct) {
        event e = {.type=READ_CALL, .lun=lun, .lba=lba, .offset=offset, .bufsize=bufsize};
        (*cb_fct)(e);
    }

    return (int32_t) bufsize;
}

bool tud_msc_is_writable_cb (uint8_t lun) {
    return true;
}

// Callback invoked when received WRITE10 command.
int32_t tud_msc_write10_cb(
    uint8_t lun,
    uint32_t lba,
    uint32_t offset,
    uint8_t* buffer,
    uint32_t bufsize
) {
    DRESULT res;
    WORD ss;
    res = disk_ioctl(0, GET_SECTOR_SIZE, &ss);
    if (res != RES_OK) return -1;

    // Check for overflow of offset + bufsize
    if (offset + bufsize > ss) {
        return -1;
    }
    
    uint8_t temp_buffer[ss];
    res = disk_read(0, temp_buffer, lba, 1);
    if (res != RES_OK) return -1;

    memcpy(temp_buffer + offset, buffer, bufsize);

    res = disk_write(0, temp_buffer, lba, 1);
    if (res != RES_OK) return -1;

    // writing callback
    if (cb_fct) {
        event e = {.type=WRITE_CALL, .lun=lun, .lba=lba, .offset=offset, .bufsize=bufsize};
        (*cb_fct)(e);
    }
    
    return (int32_t) bufsize;
}

// Callback invoked when received an SCSI command not in built-in list below
int32_t tud_msc_scsi_cb(
    uint8_t lun,
    uint8_t const scsi_cmd[16],
    void* buffer,
    uint16_t bufsize
) {
    tud_msc_set_sense(lun, SCSI_SENSE_ILLEGAL_REQUEST, 0x20, 0x00);
    return (int32_t) -1;
}
