#ifndef _JZ4740_SPECIFIC_H_
#define _JZ4740_SPECIFIC_H_

#include "jz4740_ipu.h"

// function about REG_CTRL
#define stop_ipu(IPU_V_BASE) REG32(IPU_V_BASE + REG_CTRL) &= ~IPU_EN;
#define run_ipu(IPU_V_BASE) REG32(IPU_V_BASE + REG_CTRL) |= IPU_EN;
#define reset_ipu(IPU_V_BASE) REG32(IPU_V_BASE + REG_CTRL) |= IPU_RESET;
#define disable_irq(IPU_V_BASE) REG32(IPU_V_BASE + REG_CTRL) &= ~FM_IRQ_EN;
#define disable_rsize(IPU_V_BASE) REG32(IPU_V_BASE + REG_CTRL) &= ~RSZ_EN;
#define enable_rsize(IPU_V_BASE) REG32(IPU_V_BASE + REG_CTRL) |= RSZ_EN;
#define ipu_is_enabled(IPU_V_BASE) (REG32(IPU_V_BASE + REG_CTRL) & IPU_EN)

// function about REG_STATUS
#define clear_end_flag(IPU_V_BASE) REG32(IPU_V_BASE +  REG_STATUS) &= ~OUT_END
#define polling_end_flag(IPU_V_BASE) (REG32(IPU_V_BASE + REG_STATUS) & OUT_END)

unsigned int enable_jz4740_mxu ();
unsigned int disable_jz4740_mxu (unsigned int mxucr);

int frameconversion_jz4740_ipu (unsigned char *source_address,
				unsigned char *destination_address,
				int source_width, int source_height);

#endif
