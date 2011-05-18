/* dbashcfg.h
 * 
 * Copyright 2009 Martin Read
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef DBASHCFG_HH
#define DBASHCFG_HH

/* Compile-time configuration options for Martin's Dungeon Bash. */

/* Define MULTIUSER if creating a multi-user installation. */
//#define MULTIUSER

/* COMPRESSOR and DECOMPRESSOR are invoked by the load/save code. */
/* Note that they must be willing to overwrite old files. */
#define COMPRESSOR "/bin/gzip -f"

#define DECOMPRESSOR "/bin/gunzip -f"

#define COMPRESSED_SUFFIX ".gz"

#define INNER_FULL_VER(ma, mi, p)  #ma "." #mi "." #p
#define INNER_SHORT_VER(ma, mi)  #ma "." #mi
#define FULL_VERSTRING(maj, min, patch) INNER_FULL_VER(maj, min, patch)
#define SHORT_VERSTRING(maj, min) INNER_SHORT_VER(maj, min)
#define LONG_VERSION FULL_VERSTRING(MAJVERS, MINVERS, PATCHVERS)
#define SHORT_VERSION SHORT_VERSTRING(MAJVERS, MINVERS)

/* change WIZARD_MODE to 1 if you want the wizard mode commands. */
#define WIZARD_MODE 1

/* Set the compiled default display according to taste. */
#define COMPILED_DEFAULT_DISPLAY "tty"

/* The INCLUDE_LANG_xyz directives don't do anything yet, because I haven't
 * written any of the localization support. */
#define INCLUDE_LANG_en_GB

#define LOG_MESSAGES
#endif


