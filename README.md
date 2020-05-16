# Yes/No Detector
This repository holds the Keil uVision project and MATLAB/Simulink simulation files for the DSP Lab project assignment.

# MATLAB/Simulink
The simulation files are available in the /MATLAB directory. These files include:
* Test audio files
* Output signal from the microcontroller (PRBS -> Filter -> Codec IC -> Microphone)
* Frequency Response for test files
* Frequency Respose for filters and Codec IC
* Simulink Model
* Model Simulator

## Model
The Simulink model includes the DSP algorithm to distinguish between Yes and No. Before running it the file name path must be updated in the audio source block to match the "actualFilename.wav" path.
The model is available for both 2019a and 2020a version of MATLAB.

# Microcontroller Deployment
The C source files for the S6E2CC microcontroller are found in the /src directory. These files include:
* The main program
* Header files with the filter coefficients
* Filter test file

The remaining files were left as provided

## Main and Test programs
The project has two programs, the main one with the DSP algorithm and the filter test file. By default, the test file is not included in the build, so in order to test the filter the main program must be excluded from the build and then add the test.

