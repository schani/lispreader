/* $Id: docexample.c 184 2000-04-12 22:28:53Z schani $ */
/*
 * docexample.c
 *
 * Copyright (C) 1999 Mark Probst
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

#include <lispreader.h>

int
main (void)
{
    lisp_object_t *obj;
    lisp_stream_t stream;

    lisp_stream_init_file(&stream, stdin);

    while (1)
    {
        int type;

        obj = lisp_read(&stream);
        type = lisp_type(obj);
        if (type != LISP_TYPE_EOF && type != LISP_TYPE_PARSE_ERROR)
        {
            lisp_object_t *vars[2];

            if (lisp_match_string("(+ #?(or #?(integer) #?(real)) "
                                     "#?(or #?(integer) #?(real)))",
                                  obj, vars))
                printf("%f\n", lisp_real(vars[0])
                               + lisp_real(vars[3]));
        }
        else if (type == LISP_TYPE_PARSE_ERROR)
            printf("parse error\n");
        lisp_free(obj);

        if (type == LISP_TYPE_EOF)
            break;
    }

    return 0;
}
