-module(broadcast).

-export([]).

-include("broadcast.hrl").

start() ->  
  ?spawn(fun()->anode() end),
  ?spawn(fun()->anode() end),
  ?spawn(fun()->anode() end),

  timer:sleep(100),

  broadcast ! {hello, self()},

  timer:sleep(1000).

anode() ->
  receive
    {hello, From} ->
      io:format("~p got hello from ~p~n", [self(), From]),
      broadcast ! {reply, self(), From},
      anode();
    {reply, PID, FROM} ->
      io:format("~p got reply from ~p (hello from ~p)~n", [self(), PID, FROM]),
      anode()
  end.
