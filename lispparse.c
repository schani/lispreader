/* $Id: lispparse.c 174 1999-11-18 23:00:09Z schani $ */
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
		    if (c == EOF)
			return 0;
		}

		_token_append(c);
	    }
	    return TOKEN_STRING;

	case '#' :
	    c = _next_char(stream);
	    if (c == EOF)
		return 0;

	    if (c == 't')
		return TOKEN_TRUE;
	    if (c == 'f')
		return TOKEN_FALSE;

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
		if (c == '?')
		{
		    c = _next_char(stream);
		    if (c == '(')
			return TOKEN_PATTERN_OPEN_PAREN;
		    else
			_token_append('?');
		}
		else if (c == '.')
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
	    lisp_free(obj->v.cons.car);
	    lisp_free(obj->v.cons.cdr);
	    break;
    }

    free(obj);
}

static int _match (lisp_object_t *pattern, lisp_object_t *obj, lisp_object_t **vars, int *varpos);

static int
_match_pattern (lisp_object_t *pattern, lisp_object_t *obj, lisp_object_t **vars, int *varpos)
{
    char *type;
    int pos = (*varpos)++;

    assert(lisp_car(pattern) != 0 && lisp_type(lisp_car(pattern)) == LISP_TYPE_IDENT);

    if (vars != 0)
	vars[pos] = 0;

    type = lisp_ident(lisp_car(pattern));

    if (strcmp(type, "any") == 0)
	;
    else if (strcmp(type, "ident") == 0)
    {
	if (obj == 0 || lisp_type(obj) != LISP_TYPE_IDENT)
	    return 0;
    }
    else if (strcmp(type, "string") == 0)
    {
	if (obj == 0 || lisp_type(obj) != LISP_TYPE_STRING)
	    return 0;
    }
    else if (strcmp(type, "integer") == 0)
    {
	if (obj == 0 || lisp_type(obj) != LISP_TYPE_INTEGER)
	    return 0;
    }
    else if (strcmp(type, "boolean") == 0)
    {
	if (obj == 0 || lisp_type(obj) != LISP_TYPE_BOOLEAN)
	    return 0;
    }
    else if (strcmp(type, "list") == 0)
    {
	if (obj == 0 || lisp_type(obj) != LISP_TYPE_CONS)
	    return 0;
    }
    else if (strcmp(type, "or") == 0)
    {
	int value = 0;

	pattern = lisp_cdr(pattern);
	while (pattern != 0)
	{
	    int result;

	    assert(lisp_type(pattern) == LISP_TYPE_PATTERN_CONS);
	    
	    result = _match(lisp_car(pattern), obj, vars, varpos);
	    value = value || result;

	    pattern = lisp_cdr(pattern);
	}

	if (!value)
	    return 0;
    }
    else
	assert(0);

    if (vars != 0)
	vars[pos] = obj;

    return 1;
}

static int
_match (lisp_object_t *pattern, lisp_object_t *obj, lisp_object_t **vars, int *varpos)
{
    if (pattern == 0)
	return obj == 0;

    if (obj == 0)
	return 0;

    if (lisp_type(pattern) == LISP_TYPE_PATTERN_CONS)
	return _match_pattern(pattern, obj, vars, varpos);

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

		result1 = _match(lisp_car(pattern), lisp_car(obj), vars, varpos);
		result2 = _match(lisp_cdr(pattern), lisp_cdr(obj), vars, varpos);

		return result1 && result2;
	    }
	    break;

	default :
	    assert(0);
    }

    return 0;
}

int
lisp_match (char *pattern_string, lisp_object_t *obj, lisp_object_t **vars)
{
    lisp_stream_t stream;
    lisp_object_t *pattern;
    int varpos = 0;
    int result;

    pattern = lisp_read(lisp_stream_init_string(&stream, pattern_string));

    assert(pattern == 0 || lisp_type(pattern) != LISP_TYPE_EOF);

    result = _match(pattern, obj, vars, &varpos);

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
	    fprintf(out, "%s", lisp_ident(obj));
	    break;

	case LISP_TYPE_STRING :
	    fprintf(out, "\"%s\"", lisp_string(obj));
	    break;

	case LISP_TYPE_CONS :
	case LISP_TYPE_PATTERN_CONS :
	    fprintf(out, lisp_type(obj) == LISP_TYPE_CONS ? "(" : "?(");
	    while (obj != 0)
	    {
		lisp_dump(lisp_car(obj), out);
		obj = lisp_cdr(obj);
		if (obj != 0)
		{
		    if (lisp_type(obj) != LISP_TYPE_CONS
			&& lisp_type(obj) != LISP_TYPE_PATTERN_CONS)
		    {
			fprintf(out, " . ");
			lisp_dump(obj, out);
			break;
		    }
		    else
			fprintf(out, " ");
		}
	    }
	    fprintf(out, ")");
	    break;

	case LISP_TYPE_BOOLEAN :
	    if (lisp_boolean(obj))
		fprintf(out, "#t");
	    else
		fprintf(out, "#f");
	    break;

	default :
	    assert(0);
    }
}
