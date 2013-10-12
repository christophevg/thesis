% creates figure, plots trust data and saves to requested file
% author: Christophe VG

function plot_trust(data_file, image_file)
  load(data_file);
  
  fh = figure;
  plot([0:size(trusts,1)-1], trusts(:,:,3), ...
       ['--m'; '-b'; '-g'; '-r'], 'lineWidth', 4);
  set(gca, 'ytick', 0:.1:1);
  legend(['initial trust'; 'cooperative'; 'uncooperative'; '2nd hand'])
  xlabel('Number of packets');
  ylabel('Trust between i and j');
  set(findall(fh, '-property', 'fontsize'), 'fontsize', 18);
  print(image_file, '-tight', '-color');
  close(fh);
end
