% configuration for experiment with malicious forwarders
% author: Christophe VG

clear all;

source('common.conf.m');

global plot_legend = ['0% failure'; '5% failure'; '10% failure'; '30% failure'; ...
                      '0% with 2nd'; '5% with 2nd'; '10% with 2nd'; '30% with 2nd'];
global plot_colors = ['-r'; '-g'; '-b'; '-m'; ...
                      '--r'; '--g'; '--b'; '--m' ];

global wait_for_observations = 15;
