#include "flowd.h"

/*
 * function: ftltime
 *
 * Flow exports represent time with a combination of uptime of the
 * router, real time, and offsets from the router uptime.  ftltime
 * converts from the PDU to a standard unix seconds/milliseconds
 * representation
 *
 * returns: struct fttime
 */
struct fttime ftltime(int sys, int secs, int nsecs, int t)
{

  int sys_s, sys_m;
  struct fttime ftt;

  /* sysUpTime is in milliseconds, convert to seconds/milliseconds */
  sys_s = sys / 1000;
  sys_m = sys % 1000;


  /* unix seconds/nanoseconds to seconds/milliseconds */
  ftt.secs = secs;
  ftt.msecs = nsecs / 1000000L;

  /* subtract sysUpTime from unix seconds */
  ftt.secs -= sys_s;

  /* borrow a second? */
  if (sys_m > ftt.msecs) {
    -- ftt.secs;
    ftt.msecs += 1000;
  }
  ftt.msecs -= sys_m;

  /* add offset which is in milliseconds */
  ftt.secs += t / 1000;
  ftt.msecs += t % 1000;

  /* fix if milliseconds >= 1000 */
  if (ftt.msecs >= 1000) {
    ftt.msecs -= 1000;
    ftt.secs += 1;
  }

  return ftt;

} /* ftltime */

