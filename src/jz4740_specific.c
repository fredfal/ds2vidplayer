/***************************************
* Contains jz4740 optimized functions
*
* There are mxu optimization, and ipu
* optimizations
*
***************************************/
#include <stdio.h>

#include <ds2io.h>

#include "jz4740.h"

#include "jz4740_mxu.h"
#include "jz4740_ipu.h"
#include "jz4740_specific.h"

#define PHYSADDR(x) ((x) & 0x1fffffff)

/*****************************************************************************
* 
*                       MXU related functions
*
******************************************************************************/
/*
* From http://wiki.dingoonity.org/index.php?title=Development:MXU#Enabling_MXU
* "Before the MXU can be used, it must be enabled. This is done by setting bit 0 (the lowest bit) of xr16 to 1."
*
* It return the previous value of xr16, it should be saved to be later restored
*
*/
unsigned int
enable_jz4740_mxu ()
{
  unsigned int mxucr, nval;
  mxucr = S32M2I (xr16);
  nval = mxucr | 0x7;
  S32I2M (xr16, nval);
  return mxucr;
}

/* Restore the old value of xr16 register */
unsigned int
disable_jz4740_mxu (unsigned int mxucr)
{
  S32I2M (xr16, mxucr);
  return 0;
};

/*****************************************************************************
* 
*                       IPU related functions
*
* We assume here that the input is in YV12 i.e. YCbCr 4:2:0 planar format
* http://wiki.multimedia.cx/index.php?title=YCbCr_4:2:0
* The output format is always nintendo DS screen
*
******************************************************************************/

int
enable_jz4740_ipu ()
{
  __cpm_start_ipu ();
  __cpm_start_dmac ();
  __dmac_enable_module ();
  reset_ipu (IPU_P_BASE);
}

// Not working function (yet ?)
int
frameconversion_jz4740_ipu (unsigned char *source_address,
			    unsigned char *destination_address,
			    int source_width, int source_height)
{
  // We wait for the last image conversion completion
  printf ("STOP %d - %d - %d - %d\n", polling_end_flag (IPU_P_BASE),
	  ipu_is_enabled (IPU_P_BASE), __dmac_test_halt_error (),
	  __dmac_test_addr_error ());

  if ((source_width < 33) || (source_height < 33))
    {
      printf ("Input frame resolution %dx%d is too small - skipping frame\n");
      return 1;
    }

  __dcache_writeback_all ();

  stop_ipu (IPU_P_BASE);
  clear_end_flag (IPU_P_BASE);

  //disable irq
  disable_irq (IPU_P_BASE);

  // Resize
  disable_rsize (IPU_P_BASE);
  //REG32(IPU_P_BASE + REG_CTRL) |= (0 << V_SCALE_SHIFT) | (0 << H_SCALE_SHIFT);

  // set input and output color Format
  REG32 (IPU_P_BASE + REG_D_FMT) = INFMT_YCbCr420 | OUTFMT_RGB555;

  // Set source size and stride
  REG32 (IPU_P_BASE + REG_IN_FM_GS) =
    IN_FM_W (source_width) | IN_FM_H (source_height);
  REG32 (IPU_P_BASE + REG_Y_STRIDE) = source_width;
  REG32 (IPU_P_BASE + REG_UV_STRIDE) =
    U_STRIDE (source_width) | V_STRIDE (source_width);

  // Set source and destination addresses
  int source_size = source_width * source_height;
  REG32 (IPU_P_BASE + REG_Y_ADDR) = PHYSADDR ((unsigned int) source_address);
  REG32 (IPU_P_BASE + REG_U_ADDR) =
    PHYSADDR ((unsigned int) source_address + source_size);
  REG32 (IPU_P_BASE + REG_V_ADDR) =
    PHYSADDR ((unsigned int) source_address + source_size + source_size / 4);
  REG32 (IPU_P_BASE + REG_OUT_ADDR) =
    PHYSADDR ((unsigned int) destination_address);

  // Set destination size and stride
  REG32 (IPU_P_BASE + REG_OUT_GS) =
    OUT_FM_W (SCREEN_WIDTH << 1) | OUT_FM_H (SCREEN_HEIGHT << 1);
  REG32 (IPU_P_BASE + REG_OUT_STRIDE) = SCREEN_WIDTH << 1;

  // set YUV conversion matrix
  REG32 (IPU_P_BASE + REG_CSC_C0_COEF) = YUV_CSC_C0;
  REG32 (IPU_P_BASE + REG_CSC_C1_COEF) = YUV_CSC_C1;
  REG32 (IPU_P_BASE + REG_CSC_C2_COEF) = YUV_CSC_C2;
  REG32 (IPU_P_BASE + REG_CSC_C3_COEF) = YUV_CSC_C3;
  REG32 (IPU_P_BASE + REG_CSC_C4_COEF) = YUV_CSC_C4;

  // Start image conversion and return before waiting conversion completion
  run_ipu (IPU_P_BASE);
  printf ("RUN: %d - %d\n", polling_end_flag (IPU_P_BASE),
	  ipu_is_enabled (IPU_P_BASE));
  printf ("w:%d - h:%d\n", source_width, source_height);
  printf ("IY:%x IU:%x IV:%x\n", REG32 (IPU_P_BASE + REG_Y_ADDR),
	  REG32 (IPU_P_BASE + REG_U_ADDR), REG32 (IPU_P_BASE + REG_V_ADDR));
  printf ("%x %x %x\n", source_address,
	  (unsigned int) source_address + source_width,
	  (unsigned int) (source_address + source_width));

  while ((!polling_end_flag (IPU_P_BASE)) && ipu_is_enabled (IPU_P_BASE))
    {
      printf ("RUN: %d - %d - %d - %d\n", polling_end_flag (IPU_P_BASE),
	      ipu_is_enabled (IPU_P_BASE), __dmac_test_halt_error (),
	      __dmac_test_addr_error ());
    };

  __dcache_invalidate_all ();

  return 0;
}
