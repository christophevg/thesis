% configuration for experiment with failing forwarders
% author: Christophe VG

clear all;

global seed    = 123;
global packets = 50;

global plot_legend = ['10% failure'; '30% failure'; '50% failure'; ...
                      '70% failure'; '90% failure' ];
global plot_colors = ['-r'; '-b'; '-g'; '--b'; '--r'];
