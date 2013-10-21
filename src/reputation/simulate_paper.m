% performs the simulation from the original paper
% author: Christophe VG

function [a, b] = get_observation_cooperative()
   a  = 1;
   b  = 0;
end

function [a, b] = get_observation_uncooperative()
  a  = 0;
  b  = 1;
end

% we track 4 trust evolutions:
% 1. initial trust = always 0.5
% 2. cooperative forwarder
% 3. uncooperative forwarder
% 4. cooperative forwarder with neighbour 2nd hand information

trusts  = zeros(packets+1, 4);

trusts(:,1) = ones(1) / 2;
trusts(:,2) = simulate(packets, @(o)get_observation_cooperative(),   false)';
trusts(:,3) = simulate(packets, @(o)get_observation_uncooperative(), false)';
trusts(:,4) = simulate(packets, @(o)get_observation_cooperative(),   true)';

save( '-binary', 'paper.mat', 'trusts');
