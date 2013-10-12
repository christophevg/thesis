% computation of updated a and b reputation parameters (see paper 1)
% author: Christophe VG

function [new_a, new_b] = update(a, b, Da, Db, IDa, ak, bk)
  aging    = 0.98;
  treshold = 0.9;
  
  % direct information
  new_a = (aging * a) + Da;
  new_b = (aging * b) + Db;
  
  % 2nd hand information only for direct (IDb=0) and when we trust the source
  if trust(ak, bk) >= treshold
    new_a = new_a + ( ((2*ak) / ((bk + 2)*(IDa + 2)+(2*ak))) * IDa);
  end
end
