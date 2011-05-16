// msgpass.cc - message-passing functions for Martin's Dungeon Bash
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

#define MSGPASS_CC

#include "dunbash.hh"
#include "msgpass.hh"
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/uio.h>

const uint32_t dunbash::Message::max_size = 16384;

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

dunbash::Message *next_message(int fd)
{
    uint32_t header_buf[2];
    ssize_t i;
    dunbash::Message *msg;
    i = read(fd, header_buf, 8);
    if (i != 8)
    {
        return 0;
    }
    msg = new dunbash::Message;
    uint32_t prospective_msg_type = htonl(header_buf[0]);
    uint32_t size = htonl(header_buf[1]);
    if (size > dunbash::Message::max_size)
    {
        // Either we've lost frame sync due to a bug, or we are being
        // attacked. Annoyingly, there is no handy "discard all data
        // available for reading from this socket" syscall.
    }
    msg->flavour = Msg_type(prospective_msg_type);
    msg->size = size;
    msg->payload = new unsigned char[size];
    i = read(fd, msg->payload, size);
    if (i == -1)
    {
        // Read error.
        delete msg;
        return 0;
    }
    else if (i == 0)
    {
        // End of file. Shouldn't happen.
        delete msg;
        return 0;
    }
    else if (i < (ssize_t) size)
    {
        // hmmmm not sure what to do.
    }
    return msg;
}

// msgpass.cc
