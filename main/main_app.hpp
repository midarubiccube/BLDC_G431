#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void main_setup(void);
void main_loop(void);

void disable_irq_nest();
void enable_irq_nest();

enum class DataType : uint8_t {
    COMMON_COMAND = 0x01,
    POWERBOARD_COMANND = 0x02,
    BLCD_COMANND = 0x03,
    MOTORBOARDC_COMAND = 0x04,
    // Add other data types as needed
};


#pragma pack(push, 1)
struct BLDCPacket {
    uint8_t mode;
    uint16_t rps_target;          
    float  angle_target;
    uint16_t rps;          
};
#pragma pack(pop)

#ifdef __cplusplus
}
#endif
static uint32_t irq_cnt = 0;

static volatile uint8_t adc2_rank_index = 0;