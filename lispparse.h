/* $Id: lispparse.h 179 1999-12-05 21:10:10Z schani $ */
/*
 * lispparse.h
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

#ifndef __LISPPARSE_H__
#define __LISPPARSE_H__

#include <stdio.h>

#define LISP_STREAM_FILE       1
#define LISP_STREAM_STRING     2

#define LISP_TYPE_EOF           0
#define LISP_TYPE_INTERNAL      0
#define LISP_TYPE_IDENT         1
#define LISP_TYPE_INTEGER       2
#define LISP_TYPE_STRING        3
#define LISP_TYPE_CONS          4
#define LISP_TYPE_PATTERN_CONS  5
#define LISP_TYPE_BOOLEAN       6
#define LISP_TYPE_PATTERN_VAR   7

#define LISP_PATTERN_ANY        1
#define LISP_PATTERN_IDENT      2
#define LISP_PATTERN_STRING     3
#define LISP_PATTERN_INTEGER    4
#define LISP_PATTERN_BOOLEAN    5
#define LISP_PATTERN_LIST       6
#define LISP_PATTERN_OR         7

typedef struct
{
    int type;

    union
    {
	FILE *file;
	struct
	{
	    char *buf;
	    int pos;
	} string;
    } v;
} lisp_stream_t;


typedef struct _lisp_object_t lisp_object_t;
struct _lisp_object_t
{
    int type;

    union
    {
	struct
	{
	    struct _lisp_object_t *car;
	    struct _lisp_object_t *cdr;
	} cons;

	char *string;
	int integer;

	struct
	{
	    int type;
	    int index;
	    struct _lisp_object_t *sub;
	} pattern;
    } v;
};

lisp_stream_t* lisp_stream_init_file (lisp_stream_t *stream, FILE *file);
lisp_stream_t* lisp_stream_init_string (lisp_stream_t *stream, char *buf);

lisp_object_t* lisp_read (lisp_stream_t *in);
void lisp_free (lisp_object_t *obj);

int lisp_compile_pattern (lisp_object_t **obj);
int lisp_match_pattern (lisp_object_t *pattern, lisp_object_t *obj, lisp_object_t **vars);
int lisp_match_string (char *pattern_string, lisp_object_t *obj, lisp_object_t **vars);

int lisp_type (lisp_object_t *obj);
int lisp_integer (lisp_object_t *obj);
char* lisp_ident (lisp_object_t *obj);
char* lisp_string (lisp_object_t *obj);
int lisp_boolean (lisp_object_t *obj);
lisp_object_t* lisp_car (lisp_object_t *obj);
lisp_object_t* lisp_cdr (lisp_object_t *obj);
int lisp_list_length (lisp_object_t *obj);
lisp_object_t* lisp_list_nth (lisp_object_t *obj, int index);
void lisp_dump (lisp_object_t *obj, FILE *out);

#endif
