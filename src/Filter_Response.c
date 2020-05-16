/****************************************************************
Author: Jose Augusto Amador Demeneghi
file: Filter_Response.c
Description:
This program is meant to output the frequency response of the
filters by passing a PRBS signal through the filters to obtain it
using a PC and finally getting the frequency response using the FFT.

A macro definition is used to easily change between the tested filter.

This program uses the CMSIS DSP library and the filter coefficients
were generated using MATLAB and the provided CMSIS compliant 
converter funtion (fir_coeffs.m). 

The only dependency is the audio.h header, which contains the
driver files to use DMA and the codec IC.

Many sections of this code are similiar to the main.c, so detailed
explanation is found in that file.
*****************************************************************/

#include "audio.h"
#include "lowpass.h"
#include "highpass.h"

// Use this macro definition to change the tested filter
// #define HPF or #define LPF
#define HPF

#if defined(LPF)
	#warning The tested filter is LP
#elif defined(HPF)
	#warning The tested filter is HP
#else
	#error No filter selected
#endif

// Buffer definition to store the output and filter state
float x[DMA_BUFFER_SIZE], y1[DMA_BUFFER_SIZE],y2[DMA_BUFFER_SIZE];
static float32_t LPfirStateF32[DMA_BUFFER_SIZE + Nlp - 1];
static float32_t HPfirStateF32[DMA_BUFFER_SIZE + Nhp - 1];

// Filter Instances
arm_fir_instance_f32 LPS, HPS;

void DMA_HANDLER (void)  /****** DMA Interruption Handler*****/
{
      if (dstc_state(0)){ //check interrupt status on channel 0

					if(tx_proc_buffer == (PONG))
						{
						dstc_src_memory (0,(uint32_t)&(dma_tx_buffer_pong));    //Soucrce address
						tx_proc_buffer = PING; 
						}
					else
						{
						dstc_src_memory (0,(uint32_t)&(dma_tx_buffer_ping));    //Soucrce address
						tx_proc_buffer = PONG; 
						}
				tx_buffer_empty = 1;                                        //Signal to main() that tx buffer empty					
       
				dstc_reset(0);			                                        //Clean the interrup flag
    }
    if (dstc_state(1)){ //check interrupt status on channel 1

					if(rx_proc_buffer == PONG)
					  {
						dstc_dest_memory (1,(uint32_t)&(dma_rx_buffer_pong));   //Destination address
						rx_proc_buffer = PING;
						}
					else
						{
						dstc_dest_memory (1,(uint32_t)&(dma_rx_buffer_ping));   //Destination address
						rx_proc_buffer = PONG;
						}
					rx_buffer_full = 1;   
						
				dstc_reset(1);		
    }
}

void proces_buffer(void) 
{
 int ii;
  uint32_t *txbuf;
	
	// Active buffer selection
  if(tx_proc_buffer == PING) txbuf = dma_tx_buffer_ping; 
  else txbuf = dma_tx_buffer_pong; 
	
	/************************This block does the processing ***************/
	
  for(ii=0; ii<DMA_BUFFER_SIZE ; ii++)
  {
		// The used signal is the PRBS instead of the Codec ADC
		x[ii]= prbs();
	}
	
	// The PRBS signal is filtered and outputed to a different buffer
  arm_fir_f32(&LPS,x,y1,DMA_BUFFER_SIZE);
  arm_fir_f32(&HPS,x,y2,DMA_BUFFER_SIZE);

	// The played signal is selected by using the macro definition above
	for(ii=0; ii<DMA_BUFFER_SIZE ; ii++)
  {
		#if defined(LPF)
			txbuf[ii] = (((short)(y1[ii])<<16 & 0xFFFF0000)) + ((short)(y1[ii]) & 0x0000FFFF);
		#elif defined(HPF)
			txbuf[ii] = (((short)(y2[ii])<<16 & 0xFFFF0000)) + ((short)(y2[ii]) & 0x0000FFFF);
		#endif
	}
	
	/*************** End of the processing block ****************************************/
	tx_buffer_empty = 0;
  rx_buffer_full = 0;
	}

int main (void) {    //Main function
	
	// Filter initialization
	arm_fir_init_f32(&LPS, Nlp, hlp, LPfirStateF32, DMA_BUFFER_SIZE);
	arm_fir_init_f32(&HPS, Nhp, hhp, HPfirStateF32, DMA_BUFFER_SIZE);
	// DMA and Codec Initialization
  audio_init ( hz32000, line_in, dma, DMA_HANDLER);
while(1){
	while (!(rx_buffer_full && tx_buffer_empty)){};
		proces_buffer();
	}
}


