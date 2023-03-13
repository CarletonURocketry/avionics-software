/**
 * @file gnss.h
 * @desc XA1110 GNSS Receiver Driver
 * @author Tauheed ELahee
 * @date 2019-04-05
 * @last-edited: 2019-07-25
 */

#ifndef gnss_h
#define gnss_h

#include "global.h"
#include "src/console/console.h"

/* Define in order to parse store the satellite numbers of in use satellites */
#define GNSS_STORE_IN_USE_SAT_SVS
/* Define in order to parse additional information from GSV packets for
   debugging purposes */
#define GNSS_STORE_IN_VIEW_SAT_INFO
#define GNSS_MAX_SATS_IN_VIEW   16

#define GPS_SV_OFFSET       0
#define GLONASS_SV_OFFSET   65

/**
 *  Type of fix reported by the GNSS module.
 */
enum gnss_fix_type {
    GNSS_FIX_UNKOWN,
    GNSS_FIX_NOT_AVAILABLE,
    GNSS_FIX_2D,
    GNSS_FIX_3D
};

/**
 *  Quality of fix reported by the GNSS module.
 */
enum gnss_fix_quality {
    GNSS_QUALITY_INVALID,
    GNSS_QUALITY_GPS_FIX,
    GNSS_QUALITY_DGPS_FIX,
    GNSS_QUALITY_PPS_FIX,
    GNSS_QUALITY_REAL_TIME_KINEMATIC,
    GNSS_QUALITY_FLOAT_RTK,
    GNSS_QUALITY_DEAD_RECKONING,
    GNSS_QUALITY_MANUAL_INPUT,
    GNSS_QUALITY_SIMULATION
};

/**
 *  Antenna in use by GNSS module.
 */
enum gnss_antenna {
    GNSS_ANTENNA_UNKOWN,
    GNSS_ANTENNA_INTERNAL,
    GNSS_ANTENNA_EXTERNAL
};

/**
 *  Data received from GNSS module.
 */
extern struct gnss {
    // System timestamps
    /** System time at which most recent NMEA sentence was received */
    uint32_t last_sentence;
    /** System time at which most recent valid GNSS fix was received */
    uint32_t last_fix;
    /** System time at which most recent GNSS metadata was received */
    uint32_t last_meta;
#ifdef GNSS_STORE_IN_VIEW_SAT_INFO
    /** System time at which most recent GSV sentence was received */
    uint32_t last_gsv;
#endif
    
    // GNSS Fix
    /** Latitude in 100 microminutes per least significant bit */
    int32_t latitude;
    /** Longitude in 100 microminutes per least significant bit */
    int32_t longitude;
    
    // GNSS Time
    /** UTC time received from GNSS module in seconds since Unix epoch */
    uint32_t utc_time;
    
    // Additional GNSS data
    /** Altitude above sea level in millimeters */
    int32_t altitude;
    /** Speed over ground in hundredths of a knot */
    int16_t speed;
    /** Course over ground in hundredths of a degree */
    int16_t course;
    
    // Metadata
#ifdef GNSS_STORE_IN_USE_SAT_SVS
    /** Bitfield for GPS satellite PRNs used in last fix */
    uint32_t gps_sats_in_use;
    /** Bitfield for GLONASS satellite PRNs used in last fix */
    uint32_t glonass_sats_in_use;
#endif
    /** Position Dilution of Precision (PDOP) */
    uint16_t pdop;
    /** Horizontal Dilution of Precision (HDOP) */
    uint16_t hdop;
    /** Vertical Dilution of Precision (VDOP) */
    uint16_t vdop;
#ifdef GNSS_STORE_IN_VIEW_SAT_INFO
    /** Information on GPS satellites in view */
    struct {
        /** Elevation in degrees */
        uint8_t elevation;
        /** Signal to noise ration in dB-Hz */
        uint8_t snr;
        /** Pseudo-random noise sequence */
        uint16_t prn:5;
        /** Azimuth in degrees */
        uint16_t azimuth:9;
    } in_view_gps_satellites[GNSS_MAX_SATS_IN_VIEW];
    /** Information on GLONASS satellites in view */
    struct {
        /** Elevation in degrees */
        uint8_t elevation;
        /** Signal to noise ration in dB-Hz */
        uint8_t snr;
        /** Satellite ID */
        uint16_t sat_id:5;
        /** Azimuth in degrees */
        uint16_t azimuth:9;
    } in_view_glonass_satellites[GNSS_MAX_SATS_IN_VIEW];
    /** Number of GPS satellites in view */
    uint8_t num_gps_sats_in_view:4;
    /** Number of GLONASS satellites in view */
    uint8_t num_glonass_sats_in_view:4;
#endif
    /** Number of satellites used in fix */
    uint8_t num_sats_in_use;
    /** Antenna in use */
    enum gnss_antenna antenna:2;
    /** Type of last fix */
    enum gnss_fix_type fix_type:2;
    /** Quality of last fix */
    enum gnss_fix_quality fix_quality:4;
} gnss_xa1110_descriptor;





/**
 *  Configures the descriptor structure with all the necessary data for the GNSS
 *  reciever to work. Begin the process of sending any commands to the module
 *  that are necessary to initialize it.
 *
 *  @param console Console used to communicate with GNSS module
 */
extern uint8_t init_gnss_xa1110(struct console_desc_t* console);


#endif /* gnss_h */
