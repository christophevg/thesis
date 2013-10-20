% performs a number of simulations and averages the results
% author: Christophe VG

function [average] = simulate_average(packets, get_observation, with_2nd)
  global seed;
  global max_iterations;

  trusts = zeros(packets+1, 1);

  for iteration = 1:max_iterations
    rand('seed', seed + 3 * iteration);
    
    trusts = trusts + simulate(packets, get_observation, with_2nd)';
  end
  
  average = trusts / max_iterations;
end
