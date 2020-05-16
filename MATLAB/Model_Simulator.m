%% Model Simulator
% This script runs the Simulink Model using every provided test file. It is
% meant to be ran multiple times as trial and error while tweaking the
% Tunning Parameters in 1.3. The result of each simulation is displayed in
% the command window. 
clear

%% Live Script
% This file can be read as a MATLAB Live script, to open as Live
% Right click the file in the folder browser and select Open as Live Script.

%% Before Running the Script/Model
% The model requires to change the path to the actualFilename.wav file
% since the audio source block requires an absolute path. To do this, open
% the Simulink model, double click on the audio source block and change the
% file name parameter.
% Also, the model is provided for an earlier verison of MATLAB (2019a).
% Change the ModelName variable to match the file name.

%% 1 - Model Parameters
% These variables are used in the Simulink file, so at least this section
% must be executed before running the model. The variables in section 1.3
% are the ones being optimized using trial and error.

% ----- 1.1 System Parameters -----
Fs_file = 24000;            % Audio file sample rate
Fs_system = 32000;          % DSP system sample rate
DMA_BlockSize = 128;        % DMA Block Size
ModelName = 'YN_Detector';  % Model Name
SimulationTime = 1.2;       % Simulation time

% ----- 1.2 Filter Parameters -----
HPF.sb = 3500;              % HPF stopband freq       
HPF.pb = 4500;              % HPF passband freq
LPF.pb = 2500;              % LPF passband freq
LPF.sb = 3500;              % LPF stopband freq

% ----- 1.3 Tunning Parameters -----
InputThreshold = 0.006;     % Input RMS Threshold
YNThreshold    = 0.8;       % Energy Ratio Threshold
MaxWindow      = 22;        % Window Length

%% 2 - Simulation
% This section runs the Simulink model multiple times changing the input
% audio file, the result is logged and displayed in the console window.

% ----- 2.1 Variable declaration -----
n_y = 25;                   % Number of Yes files
n_n = 24;                   % Number of No files
error = 0;                  % Total number of erros
errorY = 0;                 % Number of errors in Yes files
errorN = 0;                 % Number of errors in No files

%% 2.2 Iterative simulation
% The following two For loops iterate through every Yes and No file,
% updating the used filename (This because the Input Audio File Block does
% not support a variable file path). Then, the model is ran and the output
% is logged and displayed.

% ----- 2.2.1 For Loop for Yes files -----
for i = 1:n_y % Iterate as many times as Yes files
    % Set the file path, name and extension
    filename = strcat('good\y', string(i), '.wav');
    % Overwrite actualFilename.wav with the current Yes file
    audiowrite('actualFilename.wav', audioread(filename), Fs_file);
    % Simulate the model with the updated audio file
    simulation = sim(ModelName, SimulationTime);
    % Log and display the result
    if simulation.result == 1
        result = 'correct';
    else
        result = 'incorrect';
        errorY = errorY + 1;
    end
    fprintf('Result for y%u is %s -> %d\n', i, result, simulation.result)
end

% ----- 2.2.2 For Loop for No files -----
for i = 1:n_n % Iterate as many times as No files
    % Set the file path, name and extension
    filename = strcat('good\n', string(i), '.wav');
    % Overwrite actualFilename.wav with the current No file
    audiowrite('actualFilename.wav', audioread(filename), Fs_file);
    % Simulate the model with the updated audio file
    simulation = sim(ModelName, SimulationTime);
    % Log and display the result
    if simulation.result == -1
        result = 'correct';
    else
        result = 'incorrect';
        errorN = errorN + 1;
    end
    fprintf('Result for n%u is %s -> %d\n', i, result, simulation.result)
end

%% 3 - Overall Result
fprintf('Total errors for YES detection -> %u/%u\n', errorY, n_y)
fprintf('Total errors for NO detection -> %u/%u\n', errorN, n_n)
fprintf('Total errors -> %u/%u\n', errorY+errorN, n_y+n_n)
