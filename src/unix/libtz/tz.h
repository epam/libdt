/* Copyright (c) 2013, EPAM Systems. All rights reserved.

Authors: 
Ilya Storozhilov <Ilya_Storozhilov@epam.com>,
Andrey Kuznetsov <Andrey_Kuznetsov@epam.com>, 
Maxim Kot <Maxim_Kot@epam.com>

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met: 

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer. 
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution. 

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */
#ifndef TZ_H
#define TZ_H

#if BUILDING_LIBTZ && HAVE_VISIBILITY
#define LIBTZ_DLL_EXPORTED __attribute__((__visibility__("default")))
#else
#define LIBTZ_DLL_EXPORTED
#endif

LIBTZ_DLL_EXPORTED const struct state *tz_alloc(register const char *name);
LIBTZ_DLL_EXPORTED void tz_free(const struct state *const sp);

LIBTZ_DLL_EXPORTED struct tm *tz_localtime(const struct state *const sp, const time_t *const timep);
LIBTZ_DLL_EXPORTED struct tm *tz_localtime_r(const struct state *const sp, const time_t *const timep, struct tm *tmp);

LIBTZ_DLL_EXPORTED time_t tz_mktime(const struct state *const sp, struct tm *const tmp);

#endif /* TZ_H */
