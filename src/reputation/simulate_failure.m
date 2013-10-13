% performs a simulation of failing forwarders
% author: Christophe VG

function [a, b] = get_observation_pct_fail(pct)
  if rand > pct
    a = 1;
  else
    a = 0;
  end
  b = 1 - a;
end

trusts  = zeros(packets+1, 1);

trusts(:,1) = simulate_average(packets, @()get_observation_pct_fail(.10), true)';
trusts(:,2) = simulate_average(packets, @()get_observation_pct_fail(.30), true)';
trusts(:,3) = simulate_average(packets, @()get_observation_pct_fail(.50), true)';
trusts(:,4) = simulate_average(packets, @()get_observation_pct_fail(.70), true)';
trusts(:,5) = simulate_average(packets, @()get_observation_pct_fail(.90), true)';

save( '-binary', 'failure.mat', 'trusts');
