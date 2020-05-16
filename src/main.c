/****************************************************************
Author: Jose Augusto Amador Demeneghi
file: main.c
Description:
This is the main file containing the DSP algorithm described in 
the report and the Simulink Model. This program uses macro 
definitions for the tunnable parameters and the output results
to keep terminology between files.

This program uses the CMSIS DSP library and the filter coefficients
were generated using MATLAB and the provided CMSIS compliant 
converter funtion (fir_coeffs.m). 

The only dependency is the audio.h header, which contains the
driver files to use DMA and the codec IC.
*****************************************************************/

#include "audio.h"
#include "lowpass.h"
#include "highpass.h"

// Output Signal macros
#define OUT_YES	1
#define OUT_NO	-1
#define OUT_NA	0

// MATLAB Parameters
// InputThreshold -> 0.006
// YNThreshold -> 0.8
// MaxWindow -> 22

// Converting parameters from MATLAB for real system
// Considering an 16-bit ADC and 3.3V VCC
// InputThreshold -> 2^16/3.3 * 0.006 -> 120
#define InputThreshold ((float) 120) 
	
// Since the YNThreshold is the power spectrum ratio
// It remains unchanged. YNThreshold -> 0.8
// This value was then optimized as 0.7
#define YNThreshold	((float) 0.7) 

// The Window size is not affected by the ADC quantization
#define MaxWindow	22

// Declaration of signals in the MATLAB model
float x0[DMA_BUFFER_SIZE], x1[DMA_BUFFER_SIZE], x2[DMA_BUFFER_SIZE];
float x3, x4, x5, x6;
int8_t x7, x8, x9, y[2] = {0};

// Debug variables
float debug_maxRMS = 0; // Max input RMS value
float debug_maxYN = 0; 	// Max Energy Spectrum ratio

// State buffers for the filters
static float32_t LPfirStateF32[DMA_BUFFER_SIZE + Nlp - 1];
static float32_t HPfirStateF32[DMA_BUFFER_SIZE + Nhp - 1];

// Filter Instances 
arm_fir_instance_f32 LPS, HPS;

// Moving Maximum function as shown in the model (MovMaximum)
int8_t movingMax(int8_t in){
	// Static buffer to store previous values
	// Static attribute for it have global characteristics
	// but availability only in the function scope
	static int8_t window[MaxWindow] = {0};
	int j;
	int8_t max = -1;
	
	// Store the current value in the buffer
	window[0] = in;
	
	// Get the maximum value in the buffer
	for(j = 0; j < MaxWindow; ++j){
		if (window[j] > max) {
			max = window[j];
		}
	}
	
	// Shift the buffer
	for(j = 1; j < MaxWindow; ++j){
		window[j] = window[j-1];
	}
	window[0] = 0;
	
	return max;
}

 /****** DMA Interruption Handler*****/
void DMA_HANDLER (void) {
	// Channel 0 handles the path from the main thread to the DAC
	// Check interrupt status on channel 0
	if (dstc_state(0)) {
		// If the PONG buffer is the active one... 
		if(tx_proc_buffer == (PONG)){
			// Send the Pong buffer to the DAC
			dstc_src_memory (0,(uint32_t)&(dma_tx_buffer_pong));
			// And set the PING buffer as active buffer
			tx_proc_buffer = PING; 
		// If the PING buffer is the active buffer...
		} else {
			// Send the PING buffer to the DAC
			dstc_src_memory (0,(uint32_t)&(dma_tx_buffer_ping)); 
			// And set the PONG buffer as active
			tx_proc_buffer = PONG; 
		}
		tx_buffer_empty = 1; // Set the empty buffer flag				
		dstc_reset(0);			 // Reset the DMA channel interrupt flag
	}
	// Channel 1 handles the path from the ADC to the main thread
	// Check interrupt status on channel 1
	if (dstc_state(1)) {
		// If the PONG buffer is the active one... 
		if(rx_proc_buffer == PONG) {
			// Receive the ADC samples from the PONG buffer
			dstc_dest_memory (1,(uint32_t)&(dma_rx_buffer_pong));
			// And set the PING buffer as active
			rx_proc_buffer = PING;
		// If the PING buffer is the active buffer...
		} else {
			// Receive the ADC samples from the PING buffer
			dstc_dest_memory (1,(uint32_t)&(dma_rx_buffer_ping)); 
			// And set the PONG buffer as active
			rx_proc_buffer = PONG;
			}
		rx_buffer_full = 1; // Set the full buffer flag	
		dstc_reset(1);			// Reset the DMA channel interrupt flag
	}
}

void proces_buffer(void) {
	static int16_t Lbuffer[DMA_BUFFER_SIZE];
	int ii;
	
	// Ping-Pong Buffer selection
  uint32_t *txbuf, *rxbuf;
  if(tx_proc_buffer == PING) txbuf = dma_tx_buffer_ping; 
  else txbuf = dma_tx_buffer_pong; 
  if(rx_proc_buffer == PING) rxbuf = dma_rx_buffer_ping; 
  else rxbuf = dma_rx_buffer_pong; 
	
	/************************This block does the processing ***************/
	
  for(ii=0; ii<DMA_BUFFER_SIZE ; ii++) {
		// Get the samples from the active buffer
		audio_IN = rxbuf[ii]; 
		// Only left channel is used
		Lbuffer[ii] = (uint16_t) (audio_IN & 0x0000FFFF);
		x0[ii] = (float32_t) Lbuffer[ii];
	}
	
	/* ----- MATLAB Model Algorithm ------ */
	// Each lines translates to a Simulink block
  arm_fir_f32(&HPS,x0,x1,DMA_BUFFER_SIZE); 					// HPF
  arm_fir_f32(&LPS,x0,x2,DMA_BUFFER_SIZE); 					// LPF
	
	arm_rms_f32(x1, DMA_BUFFER_SIZE, &x3); 						// RMS_1
	arm_rms_f32(x2, DMA_BUFFER_SIZE, &x4);						// RMS_2
	arm_rms_f32(x0, DMA_BUFFER_SIZE, &x6);						// RMS_3
	
	x5 = (x4 > InputThreshold) ? x4 :  1;							// S2
	x7 = (x3/x5 > YNThreshold) ? OUT_YES  : OUT_NO;		// S1
	x8 = (x6 > InputThreshold) ? x7 :  OUT_NA;				// S3
	x9 = (x8) ? x7 : y[1];														// S4
	y[0] = movingMax(x9);															// MovMax
	y[1] = y[0];																			// Delay
	
	/* ----- Debug Variables -------- */
	if(x6 > debug_maxRMS) 
		debug_maxRMS = x6;

	if(x3/x5 > debug_maxYN)
		debug_maxYN = x3/x5;
	
	// The filtered signals are outputted for debugging
	for(ii=0; ii<DMA_BUFFER_SIZE ; ii++){
		txbuf[ii] = (((short)(x1[ii])<<16 & 0xFFFF0000)) + ((short)(x2[ii]) & 0x0000FFFF);	
	}
	
	/*************** End of the processing block ****************************************/
	tx_buffer_empty = 0; 	// Reset empty Tx buffer flag
  rx_buffer_full = 0;		// Reset full Rx buffer flag
	}

int main (void) {    //Main function
	// Set RGB LED GPIO 
	init_LED();
	// Turn off LEDs
	// Since a common annode RGB LED is used, HIGH means off
	gpio_set(LED_B, HIGH);
	gpio_set(LED_R, HIGH);
	gpio_set(LED_G, HIGH);
	
	// Initialize filters with coefficients and state buffers
	arm_fir_init_f32(&LPS, Nlp, hlp, LPfirStateF32, DMA_BUFFER_SIZE); 
	arm_fir_init_f32(&HPS, Nhp, hhp, HPfirStateF32, DMA_BUFFER_SIZE);
	
	// Initialize the ADC and DAC using DMA and setting the callback function
  audio_init ( hz32000, line_in, dma, DMA_HANDLER);
	delay_ms(2000); // Wait for the system to stabilize
	while(1){
		// Wait for both flags to be set
		while (!(rx_buffer_full && tx_buffer_empty));
		proces_buffer(); // Proccess data
		
		// Set LEDs
		// Green if Yes
		if(y[0] == 1){
			gpio_set(LED_G, LOW);
			gpio_set(LED_R, HIGH);
			
		}
		// Red if No
		else if(y[0] == -1) {
			gpio_set(LED_G, HIGH);
			gpio_set(LED_R, LOW);
		}
	}
}


