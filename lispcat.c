/* $Id$ */
/*
 * lispcat.c
 *
 * Copyright (C) 2004 Mark Probst <schani@complang.tuwien.ac.at>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#include <stdio.h>
#include <lispreader.h>

int 
main (void)
{
    lisp_object_t *obj;
    lisp_stream_t stream;

    if (lisp_stream_init_file(&stream, stdin) == 0)
    {
	fprintf(stderr, "could not init stream\n");
	return 1;
    }

    for (;;)
    {
	obj = lisp_read(&stream);

	switch (lisp_type(obj))
	{
	    case LISP_TYPE_EOF :
		goto done;

	    case LISP_TYPE_PARSE_ERROR :
		fprintf(stderr, "parse error\n");
		return 1;

	    default :
		lisp_dump(obj, stdout);
		lisp_free(obj);
	}
    }

 done:
    return 0;
}
