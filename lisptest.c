/* $Id: lisptest.c 181 1999-12-21 12:54:22Z schani $ */

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
	    lisp_object_t *vars[5];

	    lisp_dump(obj, stdout);
	    fprintf(stdout, "\n");

	    if (lisp_match_string("(beidel #?(or (heusl #?(integer)) #?(string)) #?(boolean) . #?(list))",
				  obj, vars))
	    {
		lisp_dump(vars[0], stdout);
		fprintf(stdout, "\n");
		lisp_dump(vars[1], stdout);
		fprintf(stdout, "\n");
		lisp_dump(vars[2], stdout);
		fprintf(stdout, "\n");
		lisp_dump(vars[3], stdout);
		fprintf(stdout, "\n");
		lisp_dump(vars[4], stdout);
		fprintf(stdout, "\n");
	    }

	    lisp_free(obj);
	}
	else
	    break;
    }

    return 0;
}
