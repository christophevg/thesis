// heartbeat.dsl
// author: Christophe VG

// example implementation of heartbeat detection algorithm

// the nodes modules provides functionality related to nodes' validation and
// communication
use nodes

const heartbeat_interval     = 1000    // 1 sec
const max_last_seen_interval = 1000    // 1 sec
const max_fail_count         =    3    // 3 strikes and you're out
const validation_interval    = 5000    // check nodes every 5 seconds

extend nodes with {
  last_seen  : timestamp = 0
, fail_count : byte      = 0
, trust      : boolean   = true    // we trust nodes until otherwise proven
, sequence   : byte      = 0
}

// validate is a function that is called depending on the validation strategy
// here added as an annotation. it is called for each node in de nodes 
// collection. it is defined as
@every(validation_interval)
nodes.validate = function() {
  if( now() - this.last_seen > max_last_seen_interval ) {
    // the heartbeat is late, let's track this incident
    this.fail_count++
  }

  if( this.fail_count > max_fail_count ) {
    this.trust = false
  }
}

// event handler when receiving data
nodes.receive = function(from, payload) {
 switch(payload) {
   match [ 'heartbeat', time, sequence, signature ]:
     if(sha1([sequence, time]) == signature) {
       this.nodes[from].last_seen = time
       this.nodes[from].sequence  = sequence
     } else {
       this.nodes[from].fail_count++
     }
     break
  }
}

@every(heartbeat_interval)
nodes.send = function() {
  time = now()
  * <- [ 'heartbeat', this.sequence, time, sha1([this.sequence,time]) ]
  this.sequence++
}
