// heartbeat.dsl
// author: Christophe VG

// example implementation of heartbeat detection algorithm.
// this file includes extensive comments to clarify the DSL and some of the
// inner workings of the code generator.

// define constants to configure the solution. types are inferred from the
// value (integer, float (with .), boolean (true/false), "string", 'atom')
// or can be explicited using the ": <type>" typing information
const heartbeat_interval     = 1000    // 1 sec
const max_last_seen_interval = 1000    // 1 sec
const max_fail_count         =    3    // 3 strikes and you're out
const validation_interval    = 5000    // check nodes every 5 seconds

// imports general purpose functionality. this is done explicit to avoid lookup
// through all possible collections of functions. function collections are
// simple C modules of which the public functions can be called. using python-
// based transformations arguments are mapped back and forth.
from crypto import sha1
from time   import now

// the nodes modules provides functionality related to nodes' validation and
// communication. modules consist of a combination of python code and C
// templates and drive the code generation.
use nodes

// modules can be extended. the implications can differ. here the internal
// representation of a node is extended with some properties that are specific
// to this algorithm. the nodes module already implements things like unique
// identification of nodes. internal modification of the module allows for
// different network stacks and properties to be implemented (e.g. Zigbee,...)
// from the outside (=here), all this is abstracted into nodes in general.
extend nodes with {
  last_seen  : timestamp = 0
, fail_count : byte      = 0
, trust      : boolean   = true    // we trust nodes until otherwise proven
, sequence   : byte      = 0
}

// validate is a function that is called depending on the validation strategy
// here added as an annotation. it is called for each node in the known nodes
// collection. it is defined as a method called upon an instance of a node,
// which allows the self reference to this.
nodes.validate = function() @every(validation_interval) {
  // validate if the time that passed since the last heartbeat isn't too long
  if( now() - this.last_seen > max_last_seen_interval ) {
    // the heartbeat is late, let's track this incident
    this.fail_count++
  }

  // make sure that the number of failures doesn't exceed our limits
  if( this.fail_count > max_fail_count ) {
    this.trust = false
  }
}

// event handler when receiving data. called by the nodes module onto the node
// representing the actual node that is running this code. a reference to the
// sender is provided, aswell as the FULL payload. to extract the part that is
// of interest, matching can be used.
nodes.receive = function(from, payload) {
  // payload is a list of data. we can consider one or more cases
  case payload {
    // e.g. we can check if we find an atom and three variables after is
    contains [ 'heartbeat', time, sequence, signature ]:
      // validate signature
      if(sha1([sequence, time]) == signature) {
        from.last_seen = time
        from.sequence  = sequence
      } else {
        from.fail_count++
        break
      }
      // validate time
      // TODO
      break
  }
}

// adding a send function to the nodes, with an execution strategy to have it
// executed every some milliseconds.
nodes.send = function() @every(heartbeat_interval) {
  time = now()
  // <- is an operator representing "send to"
  // * here represents all possible nodes (aka broadcasting)
  // single quoted strings are so called atoms. in code these strings uniquely
  // identify something. when generated, these are typically replaced by magic
  // numbers or simply removed in favor of a known order.
  * <- [ 'heartbeat', this.sequence, time, sha1([this.sequence,time]) ]
  this.sequence++
}