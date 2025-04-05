#include "tusb.h"
#include "ff.h"
#include "diskio.h"

// whether host does safe-eject
static bool ejected = false;


// Invoked when received SCSI_CMD_INQUIRY
// Application fill vendor id, product id and revision with string up
//  to 8, 16, 4 characters respectively
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
// return true allowing host to read/write this LUN e.g SD card inserted
bool tud_msc_test_unit_ready_cb(uint8_t lun)
{
    // RAM disk is ready until ejected
    if (ejected) {
        // Additional Sense 3A-00 is NOT_FOUND
        tud_msc_set_sense(lun, SCSI_SENSE_NOT_READY, 0x3a, 0x00);
        return false;
    }
    return true;
}

// Invoked when received SCSI_CMD_READ_CAPACITY_10 and SCSI_CMD_READ_FORMAT_CAPACITY
//  to determine the disk size
// Application update block count and block size
void tud_msc_capacity_cb(uint8_t lun, uint32_t* block_count, uint16_t* block_size)
{
    disk_ioctl(0, GET_SECTOR_COUNT, block_count);
    disk_ioctl(0, GET_SECTOR_SIZE, block_size);
}

// Invoked when received Start Stop Unit command
// - Start = 0 : stopped power mode, if load_eject = 1 : unload disk storage
// - Start = 1 : active mode, if load_eject = 1 : load disk storage
bool tud_msc_start_stop_cb(uint8_t lun, uint8_t power_condition, bool start, bool load_eject)
{
    if (load_eject) {
        if (start) {
            // load disk storage
            ejected = false;
        }
        else {
            // unload disk storage
            ejected = true;
        }
    }
    return true;
}

// Callback invoked when received READ10 command.
// Copy disk's data to buffer (up to bufsize) and return number of copied bytes.
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

    return (int32_t) bufsize;
}

bool tud_msc_is_writable_cb (uint8_t lun) {
    return true;
}

// Callback invoked when received WRITE10 command.
// Process data in buffer to disk's storage and return number of written bytes
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
    
    return (int32_t) bufsize;
}

// Callback invoked when received an SCSI command not in built-in list below
// - READ_CAPACITY10, READ_FORMAT_CAPACITY, INQUIRY, MODE_SENSE6, REQUEST_SENSE
// - READ10 and WRITE10 has their own callbacks
int32_t tud_msc_scsi_cb(
    uint8_t lun,
    uint8_t const scsi_cmd[16],
    void* buffer,
    uint16_t bufsize
) {
    // read10 & write10 has their own callback and MUST not be handled here
    void const* response = NULL;
    int32_t resplen = 0;

    // most scsi handled is input
    bool in_xfer = true;

    switch (scsi_cmd[0]) {
        default:
          // Set Sense = Invalid Command Operation
          tud_msc_set_sense(lun, SCSI_SENSE_ILLEGAL_REQUEST, 0x20, 0x00);

          // negative means error -> tinyusb could stall and/or response with failed status
          resplen = -1;
        break;
    }

    // return resplen must not larger than bufsize
    if (resplen > bufsize) resplen = bufsize;

    if (response && (resplen > 0)) {
        if(in_xfer)
        {
          memcpy(buffer, response, (size_t) resplen);
        }
        else { /* SCSI output */ }
    }

    return (int32_t) resplen;
}