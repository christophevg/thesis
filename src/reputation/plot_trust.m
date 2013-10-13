% creates figure, plots trust data and saves to requested file
% author: Christophe VG

function plot_trust(data_file, image_file)
  global plot_colors;
  global plot_legend;     % from configuration file

  load(data_file);
  
  fh = figure;
  plot([0:size(trusts,1)-1], trusts, plot_colors, 'lineWidth', 4);
  axis([0 size(trusts,1)-1 0 +1]);
  set(gca, 'ytick', 0:.1:1);  % make sure every 0.1 tick is shown

  legend(plot_legend);
  xlabel('Number of packets');
  ylabel('Trust between i and j');

  % prepare for and save to file
  set(findall(fh, '-property', 'fontsize'), 'fontsize', 18);
  print(image_file, '-tight', '-color');
  close(fh);
end
