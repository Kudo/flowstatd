/*
 * Code reuses from flow-tools ( http://code.google.com/p/flow-tools/ )
 * The License will follow origin one.
*/

/*
 * Copyright (c) 2001 Mark Fullmer and The Ohio State University
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *      $Id: ftio.c,v 1.47 2003/02/24 00:51:47 maf Exp $
 */


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
struct fttime ftltime(uint32_t sys, uint32_t secs, uint32_t nsecs, uint32_t t)
{
    uint32_t sys_s, sys_m;
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

