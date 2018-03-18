#ifndef BOARD_H
#define BOARD_H


//#define DO_DEBUG

#ifdef DO_DEBUG
    // NodeMCU debug
    #define TV_SYNC	15
    #define PS2_DATA	13
    #define PS2_CLK	12
#else
    // ESP-01 LUT
    #define TV_SYNC	1
    #define PS2_DATA	2
    #define PS2_CLK	0
#endif


#endif
