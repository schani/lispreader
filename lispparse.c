/* $Id: lispparse.c 179 1999-12-05 21:10:10Z schani $ */
/*
 * lispparse.c
 *
 * Copyright (C) 1998 Mark Probst
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

#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

#include "lispparse.h"

#define TOKEN_OPEN_PAREN              1
#define TOKEN_CLOSE_PAREN             2
#define TOKEN_IDENT                   3
#define TOKEN_STRING                  4
#define TOKEN_INTEGER                 5
#define TOKEN_PATTERN_OPEN_PAREN      6
#define TOKEN_DOT                     7
#define TOKEN_TRUE                    8
#define TOKEN_FALSE                   9

#define MAX_TOKEN_LENGTH           1024

static char token_string[MAX_TOKEN_LENGTH + 1] = "";
static int token_length = 0;

static lisp_object_t end_marker = { LISP_TYPE_EOF };
static lisp_object_t dot_marker = { LISP_TYPE_INTEGER };

static void
_token_clear (void)
{
    token_string[0] = '\0';
    token_length = 0;
}

static void
_token_append (char c)
{
    assert(token_length < MAX_TOKEN_LENGTH);

    token_string[token_length++] = c;
    token_string[token_length] = '\0';
}

static int
_next_char (lisp_stream_t *stream)
{
    switch (stream->type)
    {
	case LISP_STREAM_FILE :
	    return getc(stream->v.file);

	case LISP_STREAM_STRING :
	    {
		char c = stream->v.string.buf[stream->v.string.pos];

		if (c == 0)
		    return EOF;

		++stream->v.string.pos;

		return c;
	    }
    }

    assert(0);
    return EOF;
}

static void
_unget_char (char c, lisp_stream_t *stream)
{
    switch (stream->type)
    {
	case LISP_STREAM_FILE :
	    ungetc(c, stream->v.file);
	    break;

	case LISP_STREAM_STRING :
	    --stream->v.string.pos;
	    break;

	default :
	    assert(0);
    }
}

static int
_scan (lisp_stream_t *stream)
{
    int c;

    _token_clear();

    do
    {
	c = _next_char(stream);
	if (c == EOF)
	    return 0;
    } while (isspace(c));

    switch (c)
    {
	case '(' :
	    return TOKEN_OPEN_PAREN;

	case ')' :
	    return TOKEN_CLOSE_PAREN;

	case '"' :
	    while (1)
	    {
		c = _next_char(stream);
		if (c == EOF)
		    return 0;
		if (c == '"')
		    break;
		if (c == '\\')
		{
		    c = _next_char(stream);

		    switch (c)
		    {
			case EOF :
			    return 0;
			
			case 'n' :
			    c = '\n';
			    break;

			case 't' :
			    c = '\t';
			    break;
		    }
		}

		_token_append(c);
	    }
	    return TOKEN_STRING;

	case '#' :
	    c = _next_char(stream);
	    if (c == EOF)
		return 0;

	    switch (c)
	    {
		case 't' :
		    return TOKEN_TRUE;

		case 'f' :
		    return TOKEN_FALSE;

		case '?' :
		    c = _next_char(stream);
		    if (c == EOF)
			return 0;

		    if (c == '(')
			return TOKEN_PATTERN_OPEN_PAREN;
		    return 0;
	    }
	    return 0;

	default :
	    if (isdigit(c))
	    {
		do
		{
		    _token_append(c);
		    c = _next_char(stream);
		} while (c != EOF && isdigit(c));
		if (c != EOF)
		    _unget_char(c, stream);

		return TOKEN_INTEGER;
	    }
	    else
	    {
		if (c == '.')
		{
		    c = _next_char(stream);
		    if (c != EOF && !isspace(c) && c != '(' && c != ')' && c != '"')
			_token_append('.');
		    else
		    {
			_unget_char(c, stream);
			return TOKEN_DOT;
		    }
		}
		do
		{
		    _token_append(c);
		    c = _next_char(stream);
		} while (c != EOF && !isspace(c) && c != '(' && c != ')' && c != '"');
		if (c != EOF)
		    _unget_char(c, stream);

		return TOKEN_IDENT;
	    }
    }

    assert(0);
    return 0;
}

lisp_object_t*
lisp_object_alloc (int type)
{
    lisp_object_t *obj = (lisp_object_t*)malloc(sizeof(lisp_object_t));

    obj->type = type;

    return obj;
}

lisp_stream_t*
lisp_stream_init_file (lisp_stream_t *stream, FILE *file)
{
    stream->type = LISP_STREAM_FILE;
    stream->v.file = file;

    return stream;
}

lisp_stream_t*
lisp_stream_init_string (lisp_stream_t *stream, char *buf)
{
    stream->type = LISP_STREAM_STRING;
    stream->v.string.buf = buf;
    stream->v.string.pos = 0;

    return stream;
}

lisp_object_t*
lisp_read (lisp_stream_t *in)
{
    int token = _scan(in);
    lisp_object_t *obj = 0;

    if (token == 0)
	return &end_marker;

    switch (token)
    {
	case TOKEN_OPEN_PAREN :
	case TOKEN_PATTERN_OPEN_PAREN :
	    {
		lisp_object_t *last = 0, *car;

		do
		{
		    car = lisp_read(in);
		    if (car == &dot_marker)
		    {
			if (last == 0)
			    return &end_marker;	/* parse error */

			car = lisp_read(in);
			if (car != &end_marker)
			{
			    last->v.cons.cdr = car;

			    car = lisp_read(in);
			    if (car != &end_marker)
				return &end_marker; /* parse error */
			}
			else
			    return &end_marker;	/* parse error */
		    }
		    else if (car != &end_marker)
		    {
			if (last == 0)
			    obj = last = lisp_object_alloc(token == TOKEN_OPEN_PAREN
							   ? LISP_TYPE_CONS
							   : LISP_TYPE_PATTERN_CONS);
			else
			    last = last->v.cons.cdr = lisp_object_alloc(token == TOKEN_OPEN_PAREN
									? LISP_TYPE_CONS
									: LISP_TYPE_PATTERN_CONS);
			last->v.cons.car = car;
			last->v.cons.cdr = 0;
		    }
		} while (car != &end_marker);
	    }
	    return obj;

	case TOKEN_CLOSE_PAREN :
	    return &end_marker;

	case TOKEN_IDENT :
	    obj = lisp_object_alloc(LISP_TYPE_IDENT);
	    obj->v.string = strdup(token_string);
	    return obj;

	case TOKEN_STRING :
	    obj = lisp_object_alloc(LISP_TYPE_STRING);
	    obj->v.string = strdup(token_string);
	    return obj;

	case TOKEN_INTEGER :
	    obj = lisp_object_alloc(LISP_TYPE_INTEGER);
	    obj->v.integer = atoi(token_string);
	    return obj;

	case TOKEN_DOT :
	    return &dot_marker;

	case TOKEN_TRUE :
	    obj = lisp_object_alloc(LISP_TYPE_BOOLEAN);
	    obj->v.integer = 1;
	    return obj;

	case TOKEN_FALSE :
	    obj = lisp_object_alloc(LISP_TYPE_BOOLEAN);
	    obj->v.integer = 0;
	    return obj;
    }

    assert(0);
    return &end_marker;
}

void
lisp_free (lisp_object_t *obj)
{
    if (obj == 0)
	return;

    switch (obj->type)
    {
	case LISP_TYPE_IDENT :
	case LISP_TYPE_STRING :
	    free(obj->v.string);
	    break;

	case LISP_TYPE_CONS :
	case LISP_TYPE_PATTERN_CONS :
	    lisp_free(obj->v.cons.car);
	    lisp_free(obj->v.cons.cdr);
	    break;

	case LISP_TYPE_PATTERN_VAR :
	    lisp_free(obj->v.pattern.sub);
	    break;
    }

    free(obj);
}

static int
_compile_pattern (lisp_object_t **obj, int *index)
{
    if (*obj == 0)
	return 1;

    switch (lisp_type(*obj))
    {
	case LISP_TYPE_PATTERN_CONS :
	    {
		struct { char *name; int type; } types[] =
						 {
						     { "any", LISP_PATTERN_ANY },
						     { "ident", LISP_PATTERN_IDENT },
						     { "string", LISP_PATTERN_STRING },
						     { "integer", LISP_PATTERN_INTEGER },
						     { "boolean", LISP_PATTERN_BOOLEAN },
						     { "list", LISP_PATTERN_LIST },
						     { "or", LISP_PATTERN_OR },
						     { 0, 0 }
						 };
		char *type_name;
		int type;
		int i;
		lisp_object_t *pattern;

		if (lisp_type(lisp_car(*obj)) != LISP_TYPE_IDENT)
		    return 0;

		type_name = lisp_ident(lisp_car(*obj));
		for (i = 0; types[i].name != 0; ++i)
		{
		    if (strcmp(types[i].name, type_name) == 0)
		    {
			type = types[i].type;
			break;
		    }
		}

		if (types[i].name == 0)
		    return 0;

		if (type != LISP_PATTERN_OR && lisp_cdr(*obj) != 0)
		    return 0;

		pattern = lisp_object_alloc(LISP_TYPE_PATTERN_VAR);
		pattern->v.pattern.type = type;
		pattern->v.pattern.index = (*index)++;
		pattern->v.pattern.sub = 0;

		if (type == LISP_PATTERN_OR)
		{
		    lisp_object_t *cdr = lisp_cdr(*obj);

		    if (!_compile_pattern(&cdr, index))
		    {
			lisp_free(pattern);
			return 0;
		    }

		    pattern->v.pattern.sub = cdr;

		    (*obj)->v.cons.cdr = 0;
		}

		lisp_free(*obj);

		*obj = pattern;
	    }
	    break;

	case LISP_TYPE_CONS :
	    if (!_compile_pattern(&(*obj)->v.cons.car, index))
		return 0;
	    if (!_compile_pattern(&(*obj)->v.cons.cdr, index))
		return 0;
	    break;
    }

    return 1;
}

int
lisp_compile_pattern (lisp_object_t **obj)
{
    int index = 0;

    return _compile_pattern(obj, &index);
}

static int
_match_pattern (lisp_object_t *pattern, lisp_object_t *obj, lisp_object_t **vars)
{
    assert(lisp_type(pattern) == LISP_TYPE_PATTERN_VAR);

    switch (pattern->v.pattern.type)
    {
	case LISP_PATTERN_ANY :
	    break;

	case LISP_PATTERN_IDENT :
	    if (obj == 0 || lisp_type(obj) != LISP_TYPE_IDENT)
		return 0;
	    break;

	case LISP_PATTERN_STRING :
	    if (obj == 0 || lisp_type(obj) != LISP_TYPE_STRING)
		return 0;
	    break;

	case LISP_PATTERN_INTEGER :
	    if (obj == 0 || lisp_type(obj) != LISP_TYPE_INTEGER)
		return 0;
	    break;

	case LISP_PATTERN_BOOLEAN :
	    if (obj == 0 || lisp_type(obj) != LISP_TYPE_BOOLEAN)
		return 0;
	    break;

	case LISP_PATTERN_LIST :
	    if (obj == 0 || lisp_type(obj) != LISP_TYPE_CONS)
		return 0;

	case LISP_PATTERN_OR :
	    {
		lisp_object_t *sub;

		for (sub = pattern->v.pattern.sub; sub != 0; sub = lisp_cdr(sub))
		{
		    assert(lisp_type(sub) == LISP_TYPE_CONS);

		    if (!lisp_match_pattern(lisp_car(sub), obj, vars))
			return 0;
		}
	    }
	    break;

	default :
	    assert(0);
    }

    if (vars != 0)
	vars[pattern->v.pattern.index] = obj;

    return 1;
}

int
lisp_match_pattern (lisp_object_t *pattern, lisp_object_t *obj, lisp_object_t **vars)
{
    if (pattern == 0)
	return obj == 0;

    if (obj == 0)
	return 0;

    if (lisp_type(pattern) == LISP_TYPE_PATTERN_VAR)
	return _match_pattern(pattern, obj, vars);

    if (lisp_type(pattern) != lisp_type(obj))
	return 0;

    switch (lisp_type(pattern))
    {
	case LISP_TYPE_IDENT :
	    return strcmp(lisp_ident(pattern), lisp_ident(obj)) == 0;

	case LISP_TYPE_STRING :
	    return strcmp(lisp_string(pattern), lisp_string(obj)) == 0;

	case LISP_TYPE_INTEGER :
	    return lisp_integer(pattern) == lisp_integer(obj);

	case LISP_TYPE_CONS :
	    {
		int result1, result2;

		result1 = lisp_match_pattern(lisp_car(pattern), lisp_car(obj), vars);
		result2 = lisp_match_pattern(lisp_cdr(pattern), lisp_cdr(obj), vars);

		return result1 && result2;
	    }
	    break;

	default :
	    assert(0);
    }

    return 0;
}

int
lisp_match_string (char *pattern_string, lisp_object_t *obj, lisp_object_t **vars)
{
    lisp_stream_t stream;
    lisp_object_t *pattern;
    int result;

    pattern = lisp_read(lisp_stream_init_string(&stream, pattern_string));

    if (pattern != 0 && lisp_type(pattern) == LISP_TYPE_EOF)
	return 0;

    if (!lisp_compile_pattern(&pattern))
    {
	lisp_free(pattern);
	return 0;
    }

    result = lisp_match_pattern(pattern, obj, vars);

    lisp_free(pattern);

    return result;
}

int
lisp_type (lisp_object_t *obj)
{
    return obj->type;
}

int
lisp_integer (lisp_object_t *obj)
{
    assert(obj->type == LISP_TYPE_INTEGER);

    return obj->v.integer;
}

char*
lisp_ident (lisp_object_t *obj)
{
    assert(obj->type == LISP_TYPE_IDENT);

    return obj->v.string;
}

char*
lisp_string (lisp_object_t *obj)
{
    assert(obj->type == LISP_TYPE_STRING);

    return obj->v.string;
}

int
lisp_boolean (lisp_object_t *obj)
{
    assert(obj->type == LISP_TYPE_BOOLEAN);

    return obj->v.integer;
}

lisp_object_t*
lisp_car (lisp_object_t *obj)
{
    assert(obj->type == LISP_TYPE_CONS || obj->type == LISP_TYPE_PATTERN_CONS);

    return obj->v.cons.car;
}

lisp_object_t*
lisp_cdr (lisp_object_t *obj)
{
    assert(obj->type == LISP_TYPE_CONS || obj->type == LISP_TYPE_PATTERN_CONS);

    return obj->v.cons.cdr;
}

int
lisp_list_length (lisp_object_t *obj)
{
    int length = 0;

    while (obj != 0)
    {
	assert(obj->type == LISP_TYPE_CONS || obj->type == LISP_TYPE_PATTERN_CONS);

	++length;
	obj = obj->v.cons.cdr;
    }

    return length;
}

lisp_object_t*
lisp_list_nth (lisp_object_t *obj, int index)
{
    while (index > 0)
    {
	assert(obj != 0);
	assert(obj->type == LISP_TYPE_CONS || obj->type == LISP_TYPE_PATTERN_CONS);

	--index;
	obj = obj->v.cons.cdr;
    }

    assert(obj != 0);

    return obj->v.cons.car;
}

void
lisp_dump (lisp_object_t *obj, FILE *out)
{
    if (obj == 0)
    {
	fprintf(out, "()");
	return;
    }

    switch (lisp_type(obj))
    {
	case LISP_TYPE_INTEGER :
	    fprintf(out, "%d", lisp_integer(obj));
	    break;

	case LISP_TYPE_IDENT :
	    fputs(lisp_ident(obj), out);
	    break;

	case LISP_TYPE_STRING :
	    {
		char *p;

		fputc('"', out);
		for (p = lisp_string(obj); *p != 0; ++p)
		{
		    if (*p == '"' || *p == '\\')
			fputc('\\', out);
		    fputc(*p, out);
		}
		fputc('"', out);
	    }
	    break;

	case LISP_TYPE_CONS :
	case LISP_TYPE_PATTERN_CONS :
	    fputs(lisp_type(obj) == LISP_TYPE_CONS ? "(" : "?(", out);
	    while (obj != 0)
	    {
		lisp_dump(lisp_car(obj), out);
		obj = lisp_cdr(obj);
		if (obj != 0)
		{
		    if (lisp_type(obj) != LISP_TYPE_CONS
			&& lisp_type(obj) != LISP_TYPE_PATTERN_CONS)
		    {
			fputs(" . ", out);
			lisp_dump(obj, out);
			break;
		    }
		    else
			fputc(' ', out);
		}
	    }
	    fputc(')', out);
	    break;

	case LISP_TYPE_BOOLEAN :
	    if (lisp_boolean(obj))
		fputs("#t", out);
	    else
		fputs("#f", out);
	    break;

	default :
	    assert(0);
    }
}
