/* $Id: lisptest.c 178 1999-11-18 23:02:04Z schani $ */

#include "lispparse.h"

int
main (void)
{
    lisp_object_t *obj;
    lisp_stream_t stream;

    lisp_stream_init_file(&stream, stdin);

    while (1)
    {
	obj = lisp_read(&stream);
	if (obj == 0 || lisp_type(obj) != LISP_TYPE_EOF)
	{
	    lisp_object_t *vars[3];

	    lisp_dump(obj, stdout);
	    fprintf(stdout, "\n");

	    if (lisp_match("(beidel ?(integer) ?(boolean) . ?(list))", obj, vars))
	    {
		lisp_dump(vars[0], stdout);
		fprintf(stdout, "\n");
		lisp_dump(vars[1], stdout);
		fprintf(stdout, "\n");
		lisp_dump(vars[2], stdout);
		fprintf(stdout, "\n");
	    }

	    lisp_free(obj);
	}
	else
	    break;
    }

    return 0;
}
