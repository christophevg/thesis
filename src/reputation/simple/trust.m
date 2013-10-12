% simple relative trust computation as alternative to "more" complex version
% from paper 1
% author: Christophe VG

function trust = trust(a, b)
  if a + b == 0
    trust = 0.5;
  else
    trust = a / (a + b);
  end
end
