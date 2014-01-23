// reputation.dsl
// author: Christophe VG

// example implementation of a reputation-based intrusion detection algorithm
// the algorithm checks if a message that is sent to another node is actually
// propagated further to a next node. if not, that node is considered non-
// cooperative and gets a bad reputation

// for more information about the DSL and its implementated considerations
// see heartbeat.dsl. only functional and additional DSL aspects will be added
// in comments here.

const forward_timeout     = 1000
const validation_interval = 5000
const max_fail_count      =    3
const aging_weight        =    0.98     // factor to age reputation
const indirect_threshold  =    0.9      // lower limit before including indirect

use nodes

extend nodes with {
  queue      : [timestamp, byte*]*   = []
  msg_count  : byte                  = 0
  // alpha and beta are the parameters of the Beta distribution used to 
  // represent the reputation of a node
  alpha      : float                 = 0.0
  beta       : float                 = 0.0
  // trust is the single value [0-1] computed from alpha and beta
  trust      : float                 = 0.0
}

// we can react on "events". events are actions taken by nodes. actions are
// statements performed by nodes (e.g. sending data, raising of events,...)
after nodes transmit do function(from, to, hop, payload) {
  // only if we expect the addressee to actually route the message futher...
  if( hop == to ) { return }

  // count the total number of packets that are being sent and for which we
  // expect to see a forward action
  if ! payload.contains(['reputation']) {
    hop.msg_count++
  }

  // we add the payload to a queue of payloads we expect to be forwarded by
  // the hop.
  hop.queue.push( [ now() + forward_timeout, payload ] )
}

after nodes receive do function(from, to, payload) {
  case payload {
    contains [ 'reputation', of, alpha, beta ] :
      if( from.trust > indirect_threashold ) {
        // taking into account of indirect reputation information
        weight = (2 * from.alpha) /
                 ( (from.beta+2) * (alpha + beta + 2) * 2 * from.alpha )
        of.alpha += weight * alpha
        of.beta  += weight * beta
      }
      break
    else
      // when we see a payload that matches one we were expecting from the 
      // sender we remove it from its queue by removing any items in the queue 
      // that match the payload
      // the remove function takes a matching argument, which here uses the "_"
      // don't care operator
      from.qeueu.remove([ _, payload ])
  }
}

// validation consists of counting the number of non-cooperative actions of a 
// node (aka non-forwarded payloads) and recomputing the reputation taking into
// account these new 
@every(validation_interval)
with nodes do function() {
  // the remove function returns the number of removals, which we simply
  // use as failure count. the matching here uses the don't care operator for
  // the payload and uses a boolean expression to match passed timeouts
  failures = this.queue.remove([ < now(), _ ])

  // update the reputation parameters
  this.alpha = (aging_weight * this.alpha) + this.msg_count - failures
  this.beta  = (aging_weight * this.beta ) + failures

  // and compute trust
  this.trust = (this.alpha + 1) / (this.alpha + this.beta + 2)

  // notify bad node
  if(this.trust < 0.25) {
    send to this [ 'excluded' ]
  }

  // reset message counter
  this.msg_count = 0;
}

// of all nodes we track, we send out reputation information
@every(broadcast_interval)
with nodes do function() {
 send to * [ 'reputation', this, this.alpha, this.beta ]
}
