//
// Created by Guillaume Noailles on 29/01/18.
//
#include <zconf.h>
#include <assert.h>
#include <malloc_custom.h>
#include <memset_custom.h>

void* HEAD = NULL;


void* malloc_custom(size_t size)
{
	if(size <= 0)
		return NULL;

	size = ALIGNED_SIZE(size);
	t_block *block = find_block(size);
	if (block == NULL)
	{
		block = (t_block*) sbrk(0);

		if (extend_heap(size) == (t_block *) -1)
			return NULL;

		block->size = size;
		initialize_block(block);
	}

	return block + 1;
}

void* calloc_custom(size_t nitems, size_t size)
{
	size = ALIGNED_SIZE(size * nitems);
	void* ptr = malloc_custom(size);

	MEMSET(ptr, 0, size);
	return ptr;
}


void initialize_block(t_block *block)
{
	if (HEAD != NULL)
	{
		t_block* ptr = NULL;
		for(ptr = (t_block*)HEAD; ptr->next != NULL; ptr = ptr->next);
		ptr->next = block;
		block->prev = ptr;
		block->next = NULL;
	}
	else
	{
		HEAD = block;
		block->prev = NULL;
		block->next = NULL;
	}
	block->free = false;
}

t_block* extend_heap(size_t size)
{
	assert(!(size & ALIGN_MASK));
	return (t_block*)(sbrk(sizeof(t_block) + size));
}

t_block* find_block(size_t size)
{
	assert(!(size & ALIGN_MASK));
	if (HEAD != NULL)
	{
		for(t_block* ptr = (t_block*)HEAD; ptr != NULL; ptr = ptr->next)
		{
			if(ptr->size >= size && ptr->free == true)
			{
				split_block(ptr, size);
				return ptr;
			}
		}
	}
	return NULL;
}

void split_block(t_block *b, size_t size)
{
	assert(!(size & ALIGN_MASK));
	if (b != NULL && b->size > size + sizeof(t_block))
	{
		t_block* block = (t_block*)((char*)b + sizeof(t_block) + size);

		block->size = b->size - size - sizeof(t_block);
		b->size = size;

		if(b->next)
			b->next->prev = block;

		block->next = b->next;
		b->next = block;
		block->prev = b;
		block->free = true;
	}
}

void try_to_fusion()
{
	if (HEAD != NULL)
	{
		for(t_block* ptr = (t_block*)HEAD; ptr != NULL; ptr = ptr->next)
		{
			if(ptr->prev != NULL && ptr->free == true && ptr->prev->free == true)
			{
				ptr->prev->size += sizeof(t_block) + ptr->size;
				if(ptr->next != NULL)
					ptr->next->prev = ptr->prev;

				ptr->prev->next = ptr->next;

				MEMSET(ptr, 0, sizeof(t_block));
			}
		}
	}
}