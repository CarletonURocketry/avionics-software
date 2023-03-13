/**
 * @file mcp23s17-registers.h
 * @desc Register descriptions for Microchip MCP23S17 IO Expander
 * @author Samuel Dewan
 * @date 2019-03-14
 * Last Author:
 * Last Edited On:
 */

#ifndef mcp23s17_registers_h
#define mcp23s17_registers_h

#define MCP23S17_ADDR       0b0100000


#define MCP23S17_IODIRA     0x00
#define MCP23S17_IODIRB     0x01

union MCP23S17_IODIR_Reg {
    struct {
        uint8_t IO0:1;
        uint8_t IO1:1;
        uint8_t IO2:1;
        uint8_t IO3:1;
        uint8_t IO4:1;
        uint8_t IO5:1;
        uint8_t IO6:1;
        uint8_t IO7:1;
    } bit;
    uint8_t reg;
};


#define MCP23S17_IOPOLA     0x02
#define MCP23S17_IOPOLB     0x03

union MCP23S17_IPOL_Reg {
    struct {
        uint8_t IP0:1;
        uint8_t IP1:1;
        uint8_t IP2:1;
        uint8_t IP3:1;
        uint8_t IP4:1;
        uint8_t IP5:1;
        uint8_t IP6:1;
        uint8_t IP7:1;
    } bit;
    uint8_t reg;
};


#define MCP23S17_GPINTENA   0x04
#define MCP23S17_GPINTENB   0x05

union MCP23S17_GPINTEN_Reg {
    struct {
        uint8_t GPINT0:1;
        uint8_t GPINT1:1;
        uint8_t GPINT2:1;
        uint8_t GPINT3:1;
        uint8_t GPINT4:1;
        uint8_t GPINT5:1;
        uint8_t GPINT6:1;
        uint8_t GPINT7:1;
    } bit;
    uint8_t reg;
};


#define MCP23S17_DEFVALA    0x06
#define MCP23S17_DEFVALB    0x07

union MCP23S17_DEFVAL_Reg {
    struct {
        uint8_t DEF0:1;
        uint8_t DEF1:1;
        uint8_t DEF2:1;
        uint8_t DEF3:1;
        uint8_t DEF4:1;
        uint8_t DEF5:1;
        uint8_t DEF6:1;
        uint8_t DEF7:1;
    } bit;
    uint8_t reg;
};


#define MCP23S17_INTCONA    0x08
#define MCP23S17_INTCONB    0x09

union MCP23S17_INTCON_Reg {
    struct {
        uint8_t IOC0:1;
        uint8_t IOC1:1;
        uint8_t IOC2:1;
        uint8_t IOC3:1;
        uint8_t IOC4:1;
        uint8_t IOC5:1;
        uint8_t IOC6:1;
        uint8_t IOC7:1;
    } bit;
    uint8_t reg;
};


#define MCP23S17_IOCON      0x0A
#define MCP23S17_IOCON_INTPOL_Pos   1
#define MCP23S17_IOCON_INTPOL_Msk   (0b1 << MCP23S17_IOCON_INTPOL_Pos)
#define MCP23S17_IOCON_ODR_Pos      2
#define MCP23S17_IOCON_ODR_Msk      (0b1 << MCP23S17_IOCON_ODR_Pos)
#define MCP23S17_IOCON_HAEN_Pos     3
#define MCP23S17_IOCON_HAEN_Msk     (0b1 << MCP23S17_IOCON_HAEN_Pos)
#define MCP23S17_IOCON_DISSLW_Pos   4
#define MCP23S17_IOCON_DISSLW_Msk   (0b1 << MCP23S17_IOCON_DISSLW_Pos)
#define MCP23S17_IOCON_SEQOP_Pos    5
#define MCP23S17_IOCON_SEQOP_Msk    (0b1 << MCP23S17_IOCON_SEQOP_Pos)
#define MCP23S17_IOCON_MIRROR_Pos   6
#define MCP23S17_IOCON_MIRROR_Msk   (0b1 << MCP23S17_IOCON_MIRROR_Pos)
#define MCP23S17_IOCON_BANK_Pos     7
#define MCP23S17_IOCON_BANK_Msk     (0b1 << MCP23S17_IOCON_BANK_Pos)

union MCP23S17_IOCON_Reg {
    struct {
        uint8_t :1;         /*!< bit:      0  Reserved                        */
        uint8_t INTPOL:1;   /*!< bit:      1  Interrupt pin polarity          */
        uint8_t ODR:1;      /*!< bit:      2  Interrupt pin open drain        */
        uint8_t HAEN:1;     /*!< bit:      3  Hardware address enable         */
        uint8_t DISSLW:1;   /*!< bit:      4  SDA output slew rate control    */
        uint8_t SEQOP:1;    /*!< bit:      5  Address pointer increment       */
        uint8_t MIRROR:1;   /*!< bit:      6  Interrupt pin mirror            */
        uint8_t BANK:1;     /*!< bit:      7  Register addressing mode        */
    } bit;
    uint8_t reg;
};


#define MCP23S17_GPPUA      0x0C
#define MCP23S17_GPPUB      0x0D

union MCP23S17_GPPU_Reg {
    struct {
        uint8_t PU0:1;
        uint8_t PU1:1;
        uint8_t PU2:1;
        uint8_t PU3:1;
        uint8_t PU4:1;
        uint8_t PU5:1;
        uint8_t PU6:1;
        uint8_t PU7:1;
    } bit;
    uint8_t reg;
};


#define MCP23S17_INTFA      0x0E
#define MCP23S17_INTFB      0x0F

union MCP23S17_INTF_Reg {
    struct {
        uint8_t INT0:1;
        uint8_t INT1:1;
        uint8_t INT2:1;
        uint8_t INT3:1;
        uint8_t INT4:1;
        uint8_t INT5:1;
        uint8_t INT6:1;
        uint8_t INT7:1;
    } bit;
    uint8_t reg;
};


#define MCP23S17_INTCAPA    0x10
#define MCP23S17_INTCAPB    0x11

union MCP23S17_INTCAP_Reg {
    struct {
        uint8_t ICP0:1;
        uint8_t ICP1:1;
        uint8_t ICP2:1;
        uint8_t ICP3:1;
        uint8_t ICP4:1;
        uint8_t ICP5:1;
        uint8_t ICP6:1;
        uint8_t ICP7:1;
    } bit;
    uint8_t reg;
};


#define MCP23S17_GPIOA      0x12
#define MCP23S17_GPIOB      0x13

union MCP23S17_GPIO_Reg {
    struct {
        uint8_t GP0:1;
        uint8_t GP1:1;
        uint8_t GP2:1;
        uint8_t GP3:1;
        uint8_t GP4:1;
        uint8_t GP5:1;
        uint8_t GP6:1;
        uint8_t GP7:1;
    } bit;
    uint8_t reg;
};


#define MCP23S17_OLATA      0x14
#define MCP23S17_OLATB      0x15

union MCP23S17_OLAT_Reg {
    struct {
        uint8_t OL0:1;
        uint8_t OL1:1;
        uint8_t OL2:1;
        uint8_t OL3:1;
        uint8_t OL4:1;
        uint8_t OL5:1;
        uint8_t OL6:1;
        uint8_t OL7:1;
    } bit;
    uint8_t reg;
};

// Ignoring warnings about inefficient alignment for this struct
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpacked"

/** Register file for MCP23S17 IO Expander. */
struct mcp23s17_register_map {
    /** Data direction, 1 is input, 0 is output */
    union MCP23S17_IODIR_Reg IODIR[2];
    
    /** Input polarity, 1 is inverted, 0 is not inverted */
    union MCP23S17_IPOL_Reg IPOL[2];
    
    /** Interrupt enable, 1 is enabled, 0 is disabled */
    union MCP23S17_GPINTEN_Reg GPINTEN[2];
    
    /** Interrupt level select, 1 is low, 0 is high */
    union MCP23S17_DEFVAL_Reg DEFVAL[2];
    
    /** Interrupt mode, 1 is level, 0 is pin change */
    union MCP23S17_INTCON_Reg INTCON[2];
    
    /** Alias of IOCON,  */
    uint8_t IOCON_ALT;
    /** Device configuration register */
    union MCP23S17_IOCON_Reg IOCON;
    
    /** Pull-up configuration, 1 is enabled, 0 is disabled */
    union MCP23S17_GPPU_Reg GPPU[2];
    
    /** Interrupt flags, 1 is interrupt pending */
    union MCP23S17_INTF_Reg INTF[2];
    
    /** Pin values at last interrupt */
    union MCP23S17_INTCAP_Reg INTCAP[2];
    
    /** Pin values (writes to these registers go to OLATA/B) */
    union MCP23S17_GPIO_Reg GPIO[2];
    
    /** Output latches */
    union MCP23S17_OLAT_Reg OLAT[2];
} __attribute__((packed));

// Stop ignoring warnings about inefficient alignment
#pragma GCC diagnostic pop

#endif /* mcp23s17_registers_h */
