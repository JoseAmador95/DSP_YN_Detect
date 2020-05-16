%% Frequeny Response of Test Files
% This script gets the sample data from every test file and zero pads them
% to have the same length. The padded array is added to one of two matrices
% containing all the yes and no samples to then be converted to a timetable
% object. The Timetibles can be used in MATLAB's Signal Analyzer app 
% (APPS > Signal Processing and Communications > Signal Analizer)
% to see the time and frequency response of every file to determine the
% filter requirements.
clear

%% Live Script
% This file can be read as a MATLAB Live script, to open as Live
% Right click the file in the folder browser and select Open as Live Script.

%% 1 - Variable declaration
n_y = 25;                   % Number of Yes files
n_n = 24;                   % Number of No files
maxLen = 26090;             % Length of the longest file
Fs = 24000;                 % File sample rate
yy = zeros(n_y, maxLen);    % Yes matrix initial allocation
nn = zeros(n_n, maxLen);    % No matrix initial allocation

%% 2 - Matrix definition
% The folowing For loops read the data from the test files and add them
% into the Yes and No matrices. Since the matrices were previously
% allocated, the inserted arrays are automatically zero padded.

for i = 1:n_y
    x = audioread(strcat('good/y', string(i), '.wav'));
    yy(i,1:length(x)) = x;
end

for i = 1:n_n
    x = audioread(strcat('good/n', string(i), '.wav'));
    nn(i,1:length(x)) = x;
end

%% 3 - Timetable Conversion
No = array2timetable(nn', 'RowTimes', seconds(0:1/Fs: (maxLen-1)/Fs));
Yes = array2timetable(yy', 'RowTimes', seconds(0:1/Fs: (maxLen-1)/Fs));
signalAnalyzer(Yes, No) % Open the Signal Analyzer
