% computes trust based on number of observations of coorperative and
% uncooperative actions. see paper 1
% author: Christophe VG

function trust = trust(a, b)
  trust = (a + 1) / (a + b + 2);
end
