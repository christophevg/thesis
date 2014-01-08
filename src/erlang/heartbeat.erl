-module(heartbeat).

-export([]).

-include("broadcast.hrl").
-include("util.hrl").

start() ->
  % spawn three heartbeat processes with some time apart
  spawn(fun()->heartbeat(1, 1000) end),
  timer:sleep(100),
  spawn(fun()->heartbeat(2, 1000) end),
  timer:sleep(100),
  spawn(fun()->heartbeat(3, 1000) end),

  % let them beat for 10 seconds
  timer:sleep(10000).

% heartbeat consists of three processes:
% 1. heartbeat_sender: simply broadcasts heartbeats
% 2. heartbeat_processor: checks if all is fine, but does so when instructed by
% 3. heartbeat_scheduler: send periodic requests to processor to do his thing
%                         it is therefore also spawned from 
% parameter N is simply a number to uniquely identify each process-bundle
heartbeat(N, Interval) ->
  ?spawn(fun()->heartbeat_processor(N)              end),
   spawn(fun()->heartbeat_sender   (N, Interval, 1) end).

heartbeat_sender(N, Interval, Seq) ->
  timer:sleep(Interval),
  broadcast ! {heartbeat, N, Seq},
  heartbeat_sender(N, Interval, Seq+1).

heartbeat_processor(N) ->
  PID = self(),
  spawn(fun()->heartbeat_scheduler(PID) end),
  heartbeat_processor(N, dict:new()).

heartbeat_processor(N, Nodes) ->
  receive
    {heartbeat, Node, Seq} when Node /= N ->
      io:format("~B - received ~B from ~B @ ~B~n", [N, Seq, Node, ts()]),
      UpdatedNodes = dict:store(Node, [Seq, ts()], Nodes),
      heartbeat_processor(N, UpdatedNodes);
    process ->
      io:format("~B - processing ~B nodes~n", [N, dict:size(Nodes)]),
      dict:map(fun(Key, [Seq, Time]) ->
                % TODO add actual validation
                io:format("    ~B = ~B @ ~B~n", [Key, Seq, Time]) end,
               Nodes
              ),
      heartbeat_processor(N, Nodes)
  end.

heartbeat_scheduler(PROCESSOR) ->
  timer:sleep(2000),
  PROCESSOR ! process,
  heartbeat_scheduler(PROCESSOR).
