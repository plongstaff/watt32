/*
 *   Copyright 1996, 1997, 1998 Michiel Boland.
 *
 *   Redistribution and use in source and binary forms, with or
 *   without modification, are permitted provided that the following
 *   conditions are met:
 *
 *   1. Redistributions of source code must retain the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer.
 *
 *   2. Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials
 *      provided with the distribution.
 *
 *   3. The name of the author may not be used to endorse or promote
 *      products derived from this software without specific prior
 *      written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY
 *   EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 *   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 *   PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR
 *   BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 *   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 *   TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 *   ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *   LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 *   IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 *   THE POSSIBILITY OF SUCH DAMAGE.
 */

/* As */

static const char rcsid[] = "$Id: dump.c,v 1.6 1998/09/20 20:16:30 boland Exp $";

#include "mathopd.h"

static void dump_children(struct virtual *v)
{
    while (v) {
        log_d("VHB %s %lu %lu", v->fullname, v->nrequests, v->nwritten);
        v = v->next;
    }
}

static void dump_servers(struct server *s)
{
    while (s) {
        log_d("SAH %s:%d %lu %lu", inet_ntoa(s->addr), s->port, s->naccepts, s->nhandled);
        dump_children(s->children);
        s = s->next;
    }
}

void dump(void)
{
    log_d("*** Start of dump");
    log_d("SCM %lu %lu %d", startuptime, current_time, maxconnections);
    maxconnections = nconnections;
    dump_servers(servers);
    log_d("*** End of dump");
}
