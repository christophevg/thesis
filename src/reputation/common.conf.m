% common configuration for all experiments
% author: Christophe VG

global seed     = 123;        % allow repetition of experiments

global aging    = 0.98;       % old reputation info must fade away
global threshold = 0.9;       % threshold of trust after which indirect 
                              % reputation is taken into account

global packets  = 50;         % number of packets per simulation
global max_iterations = 100;  % number of simulations to average over
