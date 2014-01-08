-define(spawn(FUN), broadcast ! {add, spawn(FUN)}).

broadcaster(PIDS) ->
  receive
    {add, PID} -> broadcaster(PIDS++[PID]);
    Any        -> [PID ! Any || PID <- PIDS]
  end,
  broadcaster(PIDS).

broadcast_init() ->
  register(broadcast, spawn(fun()->broadcaster([]) end)).

main([]) ->
  broadcast_init(),
  start().
