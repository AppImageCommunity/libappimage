/**************************************************************************
 *
 * Copyright (c) 2004-18 Simon Peter
 *
 * All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 **************************************************************************/

#ident "AppImage by Simon Peter, http://appimage.org/"

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>

#include "xdg-basedir.h"

// own header
#include "appimage/appimage.h"

#if HAVE_LIBARCHIVE3 == 1 // CentOS
# include <archive3.h>
# include <archive_entry3.h>
#else // other systems

# include <archive.h>
# include <archive_entry.h>
#endif

#include <glob.h>
#include <appimage/appimage_legacy.h>

char* appimage_registered_desktop_file_path(const char* path, char* md5, bool verbose) {
    glob_t pglob = {};

    // if md5 has been calculated before, we can just use it to save these extra calculations
    // if not, we need to calculate it here
    if (md5 == NULL)
        md5 = appimage_get_md5(path);

    // sanity check
    if (md5 == NULL) {
        if (verbose)
            fprintf(stderr, "appimage_get_md5() failed\n");
        return NULL;
    }

    char* data_home = xdg_data_home();

    char* glob_pattern = NULL;
    asprintf(&glob_pattern, "%s/applications/appimagekit_%s-*.desktop", data_home, md5);

    glob(glob_pattern, 0, NULL, &pglob);

    char* rv = NULL;

    if (pglob.gl_pathc <= 0) {
        if (verbose) {
            fprintf(stderr, "No results found by glob()");
        }
    } else if (pglob.gl_pathc >= 1) {
        if (pglob.gl_pathc > 1 && verbose) {
            fprintf(stderr, "Too many results returned by glob(), returning first result found");
        }

        // need to copy value to be able to globfree() later on
        rv = strdup(pglob.gl_pathv[0]);
    }

    globfree(&pglob);

    return rv;
};
