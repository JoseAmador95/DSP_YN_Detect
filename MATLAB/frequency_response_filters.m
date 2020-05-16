%% Filter Frequency Response Comparison
% This script is for comparing the ideal filter design with the resulting
% frequency response in the microcontroller considering the Codec IC on
% board. 

clear

%% Live Script
% This file can be read as a MATLAB Live script, to open as Live
% Right click the file in the folder browser and select Open as Live Script.

%% 1 - Get the Frequency Response of the Filter Implementation
% The Frequency Response of the filters coded in the microcontroller is
% estimated by using an audio file containing the filter being subjected to
% a frequenncy rich signal using a PRBS generator function inside the
% microcontroller. The output data was obtained by playing the filtered
% PRBS signal on a pair of headphones (Flat Eq) and by recording the sound
% using an external microphone in the PC. Unfortunately, this added some
% noise and frequency impairments to the resulting response. Some
% compensation to these impairments are described in the following
% comments.

% Get samples and sampling rate from the recorded files.
[HPF.y, Fs] = audioread('hpf.wav');
[LPF.y,  ~] = audioread('lpf.wav');

% Get the FFT magnitude response in dB from the sample array.
LPF.FR.real = mag2db(abs(fft(LPF.y)));
HPF.FR.real = mag2db(abs(fft(HPF.y)));

% The magnitude is normalized in order to set the passband region to 0 dB.
LPF.FR.real = LPF.FR.real - 53; 
HPF.FR.real = HPF.FR.real - 53;

%% 2 - Filter Definition
% This section describes the filter design. The code in sections 1.1 and
% 1.2 were created using MATLAB's Filter Designer App in order to get the
% filter coefficients. Only the filter sample rate was defined while the
% remaining shared parameters were left as default. Then, the filter
% response is obtained and compensated for it to be in the same vertical
% region than the real response.

Fs_system = 32000;      % Filter Sample Rate
Dstop = 0.0001;         % Stopband Attenuation
Dpass = 0.057501127785; % Passband Ripple
dens  = 20;             % Density Factor

% 1.1 - Low-Pass Filter
LPF.pb = 2500;          % Passband Frequency
LPF.sb = 3500;          % Stopband Frequency
% Get filter features
[LPF.N, LPF.Fo, LPF.Ao, LPF.W] = firpmord([LPF.pb, LPF.sb]/(Fs_system/2), [1 0], [Dpass, Dstop]);
% Get the filter coefficients
LPF.b  = firpm(LPF.N, LPF.Fo, LPF.Ao, LPF.W, {dens});

% 1.2 - High-Pass Filter
HPF.sb = 3500;          % Stopband Frequency
HPF.pb = 4500;          % Passband Frequency
% Get filter features
[HPF.N, HPF.Fo, HPF.Ao, HPF.W] = firpmord([HPF.sb, HPF.pb]/(Fs_system/2), [0 1], [Dstop, Dpass]);
% Get filter coefficients
HPF.b  = firpm(HPF.N, HPF.Fo, HPF.Ao, HPF.W, {dens});

% 1.3 - Ideal Frequency Response
% The frequency array is computed using the FFT length and sampling
% frequency of the real filters.
LPF.f = linspace(0, Fs, length(LPF.FR.real));
HPF.f = linspace(0, Fs, length(HPF.FR.real));

% Get the ideal frequency magnitude responses in dB
HPF.FR.ideal = mag2db(abs(freqz(HPF.b, length(HPF.f))));
LPF.FR.ideal = mag2db(abs(freqz(LPF.b, length(LPF.f))));

% The magnitude is upscaled for the passband region to match 0 dB.
LPF.FR.ideal = LPF.FR.ideal + 108;
HPF.FR.ideal = HPF.FR.ideal + 108;

%% Response Display
subplot(2,1,1)
hold on
plot(HPF.f, HPF.FR.real)
plot(linspace(0, Fs_system/2, length(HPF.FR.ideal)), HPF.FR.ideal, 'linewidth', 2);
axis([0 Fs_system/2 -inf inf])
grid on
xlabel 'Frequency (Hz)'
ylabel 'Magnitude (dB)'
title 'HPF Response Comparison'
legend('Measured', 'Ideal', 'location', 'southeast')
hold off

subplot(2,1,2)
hold on
plot(LPF.f, LPF.FR.real)
plot(linspace(0, Fs_system/2, length(LPF.FR.ideal)), LPF.FR.ideal, 'linewidth', 2);
axis([0 Fs_system/2 -inf inf])
grid on
xlabel 'Frequency (Hz)'
ylabel 'Magnitude (dB)'
title 'LPF Response Comparison'
legend('Measured', 'Ideal', 'location', 'northeast')
hold off