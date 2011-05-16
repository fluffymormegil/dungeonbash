// msgpass.cc - message-passing functions for Martin's Dungeon Bash
// // Copyright 2011 Martin Read
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
// NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
// THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//

#define MSGPASS_CC

#include "dunbash.hh"
#include "msgpass.hh"
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/uio.h>
#include <signal.h>

int client_socket;
int engine_socket;
int child_pid;

const uint32_t dunbash::Message::max_size = 16384;

static void logger_kill();

const char * const msg_tags[] = {
    "no-op",
    "continuation",
    "shutdown",
    "text message",
    "command",
    "dance",
    "command request",
    "dance request",
    "tick report"
};

ssize_t dunbash::Message::emit(int fd)
{
    uint32_t tmptype = htonl(uint32_t(flavour));
    uint32_t tmpsize = htonl(uint32_t(size));
    iovec outbound[3] = {
        { &tmptype, 4 },
        { &tmpsize, 4 },
        { payload, size }
    };
    return writev(fd, outbound, 3);
}

dunbash::Message kill_msg;

static void logger_kill()
{
    kill_msg.emit(engine_socket);
}

void msgpass_init()
{
    int sockets[2];
    int i;
    i = socketpair(AF_LOCAL, SOCK_STREAM, 0, sockets);
    if (i < 0)
    {
        perror("dungeonbash: msgpass_init: socketpair");
        exit(1);
    }
    engine_socket = sockets[0];
    client_socket = sockets[1];
    kill_msg.flavour = Msg_type_shutdown;
    kill_msg.size = 0;
    i = fork();
    switch (i)
    {
    case 0:
        close(engine_socket);
        logger();
        break;
    case -1:
        perror("dungeonbash: msgpass_init: fork");
        exit(1);
    default:
        child_pid = i;
        atexit(logger_kill);
        close(client_socket);
        break;
    }
}

// msgpass.cc
