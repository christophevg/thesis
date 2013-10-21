% performs a simulation of malicious forwarders
% author: Christophe VG

function [a, b] = get_observation_pct_fail(observation, pct)
  global wait_for_observations;
  
  if ( (observation < wait_for_observations) || (rand > pct) )
    a = 1;
  else
    a = 0;
  end
  b = 1 - a;
end

trusts  = zeros(packets+1, 1);

trusts(:,1)  = simulate_average(packets, @(o)get_observation_pct_fail(o, .0),  false)';
trusts(:,2)  = simulate_average(packets, @(o)get_observation_pct_fail(o, .05), false)';
trusts(:,3)  = simulate_average(packets, @(o)get_observation_pct_fail(o, .10), false)';
trusts(:,4)  = simulate_average(packets, @(o)get_observation_pct_fail(o, .30), false)';

trusts(:,5)  = simulate_average(packets, @(o)get_observation_pct_fail(o, .0),   true)';
trusts(:,6)  = simulate_average(packets, @(o)get_observation_pct_fail(o, .05),  true)';
trusts(:,7)  = simulate_average(packets, @(o)get_observation_pct_fail(o, .10),  true)';
trusts(:,8)  = simulate_average(packets, @(o)get_observation_pct_fail(o, .30),  true)';

save( '-binary', 'malicious.mat', 'trusts');
