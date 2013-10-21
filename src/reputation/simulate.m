% simulates handling of packets and computes evolution of trust
% author: Christophe VG

function [trusts] = simulate(packets, get_observation, with_2nd)
  packets = packets + 1;        % include 0
  trusts  = zeros(1, packets);  % packets x trust

  % initial situation
  prev_a  = prev_b  = a  = b  = 0;
  prev_ak = prev_bk = ak = bk = 0;

  % initial trust
  trusts(1) = trust(a, b);

  % simulate packets
  for packet = 2:packets
    prev_a = a; prev_b = b; prev_ak = ak; prev_bk = bk;

    [obsv_a, obsv_b] = get_observation(packet);
    
    if with_2nd
      [ak, bk] = update(prev_ak, prev_bk, obsv_a, obsv_b, 0, 0, 0);
      id = 1;
    else
      id = 0;
    end

    [a, b] = update(prev_a,  prev_b,  obsv_a, obsv_b, id, ak, bk);

    trusts(packet) = trust(a, b);
  end
end
