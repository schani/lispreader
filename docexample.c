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

            if (lisp_match_string("(+ #?(integer) #?(integer))",
                                  obj, vars))
                printf("%d\n", lisp_integer(vars[0])
                               + lisp_integer(vars[1]));

        }
        else if (type == LISP_TYPE_PARSE_ERROR)
            printf("parse error\n");
        lisp_free(obj);

        if (type == LISP_TYPE_EOF)
            break;
    }

    return 0;
}
