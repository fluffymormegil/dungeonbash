// logger.cc - testing message consumer for Martin's Dungeon Bash
// 
// Copyright 2011 Martin Read
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

#define LOGGER_CC
#include "dunbash.hh"
#include "msgpass.hh"
#include <libmormegil/stlprintf.hh>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <time.h>
#include <errno.h>

void logger()
{
    fd_set read_fds;
    dunbash::Message msg;
    uint32_t tmpflav;
    uint32_t tmpsiz;
    iovec iov[2] = {
        { &tmpflav, 4 },
        { &tmpsiz, 4 }
    };
    int logfd = open("logger.log", O_WRONLY | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
    if (logfd == -1)
    {
        exit(1);
    }
    dup2(logfd, STDERR_FILENO);
    fprintf(stderr, "logger started at %llx\n", (long long) time(0));
    FD_ZERO(&read_fds);
    do 
    {
        int i;
        ssize_t ri;
        FD_SET(client_socket, &read_fds);
        i = pselect(client_socket + 1, &read_fds, 0, 0, 0, 0);
        if (i > 0)
        {
            if (FD_ISSET(client_socket, &read_fds))
            {
                ri = readv(client_socket, iov, 2);
                switch (ri)
                {
                case -1:
                    perror("logger: readv");
                    if (errno == EIO)
                    {
                        fprintf(stderr, "game engine went away.\n");
                        _exit(0);
                    }
                    break;
                case 0:
                    // 0 bytes on read = EOF reached = engine closed its end of
                    // the socket.
                    fprintf(stderr, "logger received EOF at %llx\n", (long long) time(0));
                    _exit(0);
                case 8:
                    // Yay! message!
                    msg.size = ntohl(tmpsiz);
                    msg.flavour = Msg_type(ntohl(tmpflav));
                    msg.payload = new unsigned char [msg.size];
                    ri = read(client_socket, msg.payload, msg.size);
                    logger_process_msg(msg);
                    delete[] msg.payload;
                    break;
                default:
                    fprintf(stderr, "short read (%lld bytes) fetching message header!\n", (long long) ri);
                    _exit(1);
                    break;
                }
            }
            else
            {
                // can't happen
                fprintf(stderr, "YOUR OPERATING SYSTEM'S IMPLEMENTATION OF SELECT() IS BROKEN.\n");
                _exit(0);
            }
        }
        else if (i == 0)
        {
            perror("logger: pselect(0)");
            _exit(0);
        }
        else if (i == -1)
        {
            perror("logger: pselect");
            if (errno == EIO)
            {
                fprintf(stderr, "game engine went away.\n");
                _exit(0);
            }
        }
    } while (1);
}

void logger_process_msg(const dunbash::Message& msg)
{
    uint32_t tmpflav = msg.flavour;
    if (tmpflav >= Total_msg_types)
    {
        fprintf(stderr, "Invalid message flavour %#x (%#x bytes long)\n", tmpflav, msg.size);
    }
    else
    {
        fprintf(stderr, "Message flavour %s (%#x bytes long)\n", msg_tags[tmpflav], msg.size);
    }
    if (msg.flavour == Msg_type_shutdown)
    {
        fprintf(stderr, "shutdown message received, shutting down...\n");
        _exit(0);
    }
}

// logger.cc
