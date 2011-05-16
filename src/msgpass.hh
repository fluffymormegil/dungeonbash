// msgpass.hh - message-passing functions for Martin's Dungeon Bash
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

#ifndef MSGPASS_HH
#define MSGPASS_HH

enum Msg_type
{
    Msg_type_no_op,
    Msg_type_continuation,
    Msg_type_shutdown,
    Msg_type_text,
    Msg_type_cmd,
    Msg_type_dance,
    Msg_type_cmdreq,
    Msg_type_dancereq,
    Msg_type_tick_report,
    Total_msg_types
};

namespace dunbash
{
    struct Message
    {
        static const uint32_t max_size;
        Msg_type flavour;
        uint32_t size;
        unsigned char *payload;
        Message() : flavour(Msg_type_no_op), size(0), payload(0) { }
        ~Message() { if (payload) delete[] payload; }
        ssize_t emit(int fd);
    };
}

extern int client_socket;
extern int engine_socket;
extern const char * const msg_tags[];
extern void logger();
extern void logger_process_msg(const dunbash::Message& msg);

extern void msgpass_init();

#endif

// msgpass.hh
