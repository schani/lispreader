/*
 * pools.c
 *
 * lispreader
 *
 * Copyright (C) 2002-2005 Mark Probst
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <string.h>
#include <assert.h>

#include "pools.h"

int
init_pools (pools_t *pools)
{
    int i;

    pools->active_pool = 0;
    pools->fill_ptr = 0;

    for (i = 0; i < NUM_POOLS; ++i)
	pools->pools[i] = 0;

    pools->pools[0] = (long*)malloc(GRANULARITY * FIRST_POOL_SIZE);
    if (pools->pools[0] == 0)
	return 0;

    memset(pools->pools[0], 0, GRANULARITY * FIRST_POOL_SIZE);

    return 1;
}

void
reset_pools (pools_t *pools)
{
    pools->active_pool = 0;
    pools->fill_ptr = 0;
}

void
free_pools (pools_t *pools)
{
    int i;

    /* printf("alloced %d pools\n", active_pool + 1); */
    for (i = 0; i < NUM_POOLS; ++i)
	if (pools->pools[i] != 0)
	    free(pools->pools[i]);

    init_pools(pools);
}

#ifdef __GNUC__
void*
_pools_alloc (pools_t *pools, size_t size)
#else
void*
pools_alloc (pools_t *pools, size_t size)
#endif
{
    size_t pool_size;
    void *p;

    pool_size = FIRST_POOL_SIZE << pools->active_pool;
    size = (size + GRANULARITY - 1) / GRANULARITY;

    while (pools->fill_ptr + size >= pool_size)
    {
	++pools->active_pool;
	assert(pools->active_pool < NUM_POOLS);

	pools->fill_ptr = 0;

	if (pools->pools[pools->active_pool] == 0)
	{
	    pools->pools[pools->active_pool] = (long*)malloc(GRANULARITY * (FIRST_POOL_SIZE << pools->active_pool));
	    if (pools->pools[pools->active_pool])
		return 0;
	}
    }

    assert(pools->fill_ptr + size < pool_size);

    memset(pools->pools[pools->active_pool], 0, GRANULARITY * (FIRST_POOL_SIZE << pools->active_pool));

    p = pools->pools[pools->active_pool] + pools->fill_ptr;
    pools->fill_ptr += size;

    return p;
}
