% configuration for experiment with malicious forwarders
% author: Christophe VG

clear all;

source('common.conf.m');

global plot_legend = ['0% fouten'; '5% fouten'; '10% fouten'; '30% fouten'; ...
                      '0% met 2e hand'; '5% met 2e hand'; '10% met 2e hand'; '30% met 2e hand'];
global plot_colors = ['-r'; '-g'; '-b'; '-m'; ...
                      '--r'; '--g'; '--b'; '--m' ];

global wait_for_observations = 15;
