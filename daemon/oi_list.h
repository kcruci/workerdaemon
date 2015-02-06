#ifndef __OI_LIST_H__
#define __OI_LIST_H__

/**
 * oi_list.h
 * most stolen from <linux/list.h>
 */


#define DIM(a)	(sizeof(a)/sizeof(a[0]))

typedef struct _ListHead
{
	struct _ListHead * pstNext;
	struct _ListHead * pstPrev;
}ListHead;

#define INIT_LIST_HEAD(ptr) do { \
	(ptr)->pstNext = (ptr); (ptr)->pstPrev = (ptr); \
} while (0)

#define LIST_HEAD_INIT(name) { &(name), &(name) }
#define LIST_HEAD(name) \
	ListHead name = LIST_HEAD_INIT(name)

/*
 * Insert a new entry between two known consecutive entries. 
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
#define _LIST_ADD(node, prev, next) do { \
	(next)->pstPrev = (node); \
		(node)->pstNext = (next); \
		(node)->pstPrev = (prev); \
		(prev)->pstNext = (node); \
} while (0)
	
/**
 * list_add - add a new entry
 * @new: new entry to be added
 * @head: list head to add it after
 *
 * Insert a new entry after the specified head.
 * This is good for implementing stacks.
 */
#define LIST_ADD(node, head) do { \
	ListHead * pstNew = (node); \
		ListHead * pstPrev = (head); \
		ListHead * pstNext = (head)->pstNext; \
		_LIST_ADD(pstNew, pstPrev, pstNext); \
} while (0)
//Warning: this macro won't work some occasion, not like the function call in <linux/list.h>!
//#define LIST_ADD(node, head) _LIST_ADD(node, head, (head)->pstNext)

/**
 * list_add_tail - add a new entry
 * @new: new entry to be added
 * @head: list head to add it before
 *
 * Insert a new entry before the specified head.
 * This is useful for implementing queues.
 */
#define LIST_ADD_TAIL(node, head) do { \
	ListHead * pstNew = (node); \
		ListHead * pstPrev = (head)->pstPrev; \
		ListHead * pstNext = (head); \
		_LIST_ADD(pstNew, pstPrev, pstNext); \
} while (0)

//Warning: this macro won't work some occasion, not like the function call in <linux/list.h>!
//#define LIST_ADD_TAIL(node, head) _LIST_ADD(node, (head)->pstPrev, head)

/*
 * Delete a list entry by making the prev/next entries
 * point to each other.
 *
 * This is only for internal list manipulation where we know
 * the prev/next entries already!
 */
#define _LIST_DEL(prev, next) do { \
	(next)->pstPrev = (prev); \
		(prev)->pstNext = (next); \
} while (0)

/**
 * list_del - deletes entry from list.
 * @entry: the element to delete from the list.
 * Note: list_empty on entry does not return true after this, the entry is in an undefined state.
 */
#define LIST_DEL(entry) do { \
	ListHead * pstPrev = (entry)->pstPrev; \
		ListHead * pstNext = (entry)->pstNext; \
		_LIST_DEL(pstPrev, pstNext); \
} while (0)
//Warning: this macro may not work some occasion, not like the function call in <linux/list.h>!
//#define LIST_DEL(entry) _LIST_DEL((entry)->pstPrev, (entry)->pstNext)

/**
 * list_del_init - deletes entry from list and reinitialize it.
 * @entry: the element to delete from the list.
 */
#define LIST_DEL_INIT(entry) do { \
	LIST_DEL(entry); \
		INIT_LIST_HEAD(entry); \
} while (0)

/**
 * list_empty - tests whether a list is empty
 * @head: the list to test.
 */
#define LIST_EMPTY(head) ((head)->pstNext == (head))

/**
 * list_splice - join two lists
 * @list: the new list to add.
 * @head: the place to add it in the first list.
 */
#define LIST_SPLICE(list, head) do { \
	ListHead * first = (list)->pstNext; \
		if (first != (list)) { \
			ListHead * last = (list)->pstPrev; \
				ListHead * at = (head)->pstNext; \
				first->pstPrev = (head); \
				last->pstNext = at; \
				st->pstPrev = last; \
		} \
} while (0)

/**
 * list_entry - get the struct for this entry
 * @ptr:	the &struct list_head pointer.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the list_struct within the struct.
 */
#define LIST_ENTRY(ptr, type, member) \
	((type *)((char *)(ptr)-(unsigned long)(&((type *)0)->member)))

/**
 * list_for_each	-	iterate over a list
 * @pos:	the &struct list_head to use as a loop counter.
 * @head:	the head for your list.
 */
#define LIST_FOR_EACH(pos, head) \
	for ((pos) = (head)->pstNext; (pos) != (head); pos = (pos)->pstNext)
        	
/**
 * list_for_each_safe	-	iterate over a list safe against removal of list entry
 * @pos:	the &struct list_head to use as a loop counter.
 * @n:		another &struct list_head to use as temporary storage
 * @head:	the head for your list.
 */
#define LIST_FOR_EACH_SAFE(pos, n, head) \
	for ((pos) = (head)->pstNext, (n) = (pos)->pstNext; (pos) != (head); \
		(pos) = (n), (n) = (pos)->pstNext)

/**
 * list_for_each_prev	-	iterate over a list in reverse order
 * @pos:	the &struct list_head to use as a loop counter.
 * @head:	the head for your list.
 */
#define LIST_FOR_EACH_PREV(pos, head) \
	for ((pos) = (head)->pstPrev; (pos) != (head); (pos) = (pos)->pstPrev)
        	

#endif//__OI_LIST_H__
