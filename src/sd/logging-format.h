/**
 * @file logging-format.h
 * @desc Tools for parsing and marshalling data logging format.
 * @author Samuel Dewan
 * @date 2021-07-11
 * Last Author:
 * Last Edited On:
 */

#ifndef logging_format_h
#define logging_format_h

#include <stdint.h>

#define LOGGING_FORMAT_VERSION  1

#define LOGGING_SB_MAGIC        "CUInSpac"
#define LOGGING_SB_NUM_FLIGHTS  32

/**
 *  Format for Superblock of CU InSpace logging data partition.
 */
union logging_superblock {
    struct {
        /** Magic number to identify superblock */
        char magic[8];
        /** Partion format version number */
        uint32_t version:8;
        /** Indicates if the first flight of this partition is continued from
            the last flight in a previous partion */
        uint32_t continued:1;
        uint32_t RESERVED:23;
        /** Number of blocks in the partition */
        uint32_t partition_length;
        uint32_t RESERVED2[20];
        struct {
            /** First block in the flight, indexed within the partion (i.e. the
                block after the superblock is block 1) */
            uint32_t first_block;
            /** The number of blocks in the flight */
            uint32_t num_blocks;
            /** First UTC time recieved while recording data for thsi flight */
            uint32_t timestamp;
        } flights[32];
        uint32_t RESERVED3[6];
        /** Second copy of magic number to identify superblock */
        char magic2[8];
    };
    uint8_t raw[512];
};




enum logging_block_class {
    LOGGING_BLOCK_CLASS_METADATA = 0x0,
    LOGGING_BLOCK_CLASS_TELEMETRY = 0x1,
    LOGGING_BLOCK_CLASS_DIAG = 0x2
};

enum logging_metadata_block_type {
    LOGGING_METADATA_TYPE_SPACER = 0x0
};

enum logging_diag_block_type {
    LOGGING_DIAG_TYPE_MSG = 0x0
};


//
//
//  Data Block Header Functions
//
//

#define LOGGING_BLOCK_HEADER_LENGTH 4

static inline enum logging_block_class logging_block_class(const uint8_t *head)
{
    return (enum logging_block_class)(head[0] & 0x3f);
}

static inline uint16_t logging_block_type(const uint8_t *head)
{
    return ((uint16_t)head[0] >> 6) | ((uint16_t)head[1] << 2);
}

static inline uint16_t logging_block_length(const uint8_t *head)
{
    return ((uint16_t)head[2]) | ((uint16_t)head[3] << 8);
}

static inline uint8_t *logging_block_marshal_header(uint8_t *buffer,
                                                enum logging_block_class class,
                                                uint16_t type, uint16_t length)
{
    buffer[0] = (class & 0x3f) | ((type & 0x3) << 6);
    buffer[1] = type >> 2;
    buffer[2] = (length >> 0) & 0xff;
    buffer[3] = (length >> 8) & 0xff;
    return buffer;
}



#endif /* logging_format_h */
