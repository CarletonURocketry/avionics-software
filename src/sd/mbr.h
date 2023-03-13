/**
 * @file mbr.h
 * @desc Master Boot Record parsing functions
 * @author Samuel Dewan
 * @date 2021-07-30
 * Last Author:
 * Last Edited On:
 */

#ifndef mbr_h
#define mbr_h

#include <stdint.h>
#include <string.h>


#define MBR_MAX_NUM_PARTITIONS   4


// MARK: Types

struct mbr_chs_address {
    uint16_t cylinder;
    uint8_t head;
    uint8_t sector;
};

enum mbr_partition_type {
    MBR_PART_TYPE_EMPTY = 0x00,
    MBR_PART_TYPE_EXFAT = 0x70,
    MBR_PART_TYPE_CUINSPACE = 0x89
};


#define MBR_CHS_ADDR_TOO_LARGE  (struct mbr_chs_address){ .cylinder = 1023, \
                                                          .head = 254, \
                                                          .sector = 63 }


// MARK: MBR Parsing Functions

static inline int mbr_is_valid(uint8_t const *mbr)
{
    return (mbr[510] == 0x55) && (mbr[511] == 0xAA);
}

static inline uint8_t const *mbr_get_partition_entry(uint8_t const *mbr,
                                                     uint8_t partition_num)
{
    return mbr + 446 + (16 * partition_num);
}


static inline int mbr_part_is_valid(uint8_t const *entry)
{
    return ((entry[0] & ~(1 << 7)) == 0) && (entry[4] != 0);
}

static inline int mbr_part_is_bootable(uint8_t const *entry)
{
    return entry[0] & (1 << 7);
}

static inline struct mbr_chs_address mbr_part_first_sector_chs_addr(
                                                        uint8_t const *entry)
{
    return (struct mbr_chs_address){
        .cylinder = (uint16_t)entry[3] | ((((uint16_t)entry[2]) & 0xc0) << 2),
        .head = entry[1],
        .sector = entry[2] & 0x3f
    };
}

static inline enum mbr_partition_type mbr_part_type(uint8_t const *entry)
{
    return (enum mbr_partition_type)entry[4];
}

static inline struct mbr_chs_address mbr_part_last_sector_chs_addr(
                                                        uint8_t const *entry)
{
    return (struct mbr_chs_address){
        .cylinder = (uint16_t)entry[7] | ((((uint16_t)entry[6]) & 0xc0) << 2),
        .head = entry[5],
        .sector = entry[6] & 0x3f
    };
}

static inline uint32_t mbr_part_first_sector_lba(uint8_t const *entry)
{
    return (((uint32_t)entry[8] << 0) |
            ((uint32_t)entry[9] << 8) |
            ((uint32_t)entry[10] << 16) |
            ((uint32_t)entry[11] << 24));
}

static inline uint32_t mbr_part_num_sectors(uint8_t const *entry)
{
    return (((uint32_t)entry[12] << 0) |
            ((uint32_t)entry[13] << 8) |
            ((uint32_t)entry[14] << 16) |
            ((uint32_t)entry[15] << 24));
}


// MARK: MBR Creating Functions

static inline void mbr_init(uint8_t *mbr)
{
    memset(mbr, 0, 512);
    mbr[510] = 0x55;
    mbr[511] = 0xAA;
}

static inline void mbr_set_chs_addr(uint8_t *dest, struct mbr_chs_address addr)
{
    dest[0] = addr.head;
    dest[1] = (addr.sector & 0x3f) | (((uint8_t)(addr.cylinder >> 2)) & 0xc0);
    dest[2] = (uint8_t)(addr.cylinder & 0xff);
}

static inline void mbr_init_partition(uint8_t *mbr, uint8_t partition_num,
                                      enum mbr_partition_type type,
                                      uint32_t start, uint32_t length)
{
    uint8_t *const entry = mbr + 446 + (16 * partition_num);

    entry[0] = 0;
    mbr_set_chs_addr(entry + 1, MBR_CHS_ADDR_TOO_LARGE);
    entry[4] = (uint8_t)type;
    mbr_set_chs_addr(entry + 5, MBR_CHS_ADDR_TOO_LARGE);
    entry[8] = (uint8_t)(start >> 0);
    entry[9] = (uint8_t)(start >> 8);
    entry[10] = (uint8_t)(start >> 16);
    entry[11] = (uint8_t)(start >> 24);
    entry[12] = (uint8_t)(length >> 0);
    entry[13] = (uint8_t)(length >> 8);
    entry[14] = (uint8_t)(length >> 16);
    entry[15] = (uint8_t)(length >> 24);
}




#endif /* mbr_h */
