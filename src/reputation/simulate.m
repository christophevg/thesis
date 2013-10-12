% simulates handling of packets and computes trusts
% author: Christophe VG

% 4 evolutions of trust are tracked:
% 1. initial trust = always 0.5
% 2. cooperative forwarder
% 3. uncooperative forwarder
% 4. cooperative forwarder with neighbour 2nd hand information

% requires update(a,b,Da,Db,ak,bk) and trust(a,b) functions

function simulate(packets, filename)
  packets = packets + 1;          % include 0
  trusts = zeros(packets, 4, 5);  % packet x 4 x (a, b, T, ak, bk)

  % initial trust with no packets send
  trusts(1,1,3) = trust(0, 0); % initial trust
  trusts(1,2,3) = trust(0, 0); % coorperative
  trusts(1,3,3) = trust(0, 0); % uncooperative
  trusts(1,4,3) = trust(0, 0); % cooperative + 2nd hand

  % simulate packets
  for packet = 2:packets
    % initial trust
    trusts(packet,1,3) = trust(0, 0);
    % cooperative trust
    [a, b] = update(trusts(packet-1,2,1), ...           % previous a
                          trusts(packet-1,2,2), ...     % previous b
                          1,0, ...                      % new observation a, b
                          0, ...                        % indirect observation a
                          0, 0);                        % reputation neighbour
    trusts(packet,2,:) = [a, b, trust(a, b), 0, 0];
    % uncooperative trust
    [a, b] = update(trusts(packet-1,3,1), ...
                          trusts(packet-1,3,2), ...
                          0,1, ...
                          0, ...
                          0,0);
    trusts(packet,3,:) = [a, b, trust(a, b), 0, 0];
    % cooperative trust + 2nd hand
    [ak, bk] = update(trusts(packet-1,4,4), ...
                            trusts(packet-1,4,5), ...
                            1,0, ...
                            0, ...
                            0,0);
    [a, b] = update(trusts(packet-1,4,1), ...
                          trusts(packet-1,4,2), ...
                          1,0, ...
                          1, ...
                          ak, bk);
    trusts(packet,4,:) = [a, b, trust(a, b), ak, bk];
  end

  save( '-binary', filename, 'trusts');
end
