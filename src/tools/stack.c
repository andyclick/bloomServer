/*
 * stack.c -- Implementation of stack which can store anything.
 * Create: Xie Han, OS lab of Peking University <me@pku.edu>
 *
 * Simple is the best!
 *
 * Created: Sep 30 1:10am 2003. version 1.0.0
 *
 * Updated: Sep 30 1:40am 2003. version 1.2.0
 *		# Eliminiate the compile-time error when pushing a constant
 *		  into the stack.
 *		# Bug found! Pushing array name may lead to fatal error. When
 *		  you need to take an array name as a pointer and push it into
 *		  the stack, supposing "a" the name of an int array, please
 *		  use stack_push(stack, a + 0) or stack_push(stack, (int *)a)
 *		  in stead of stack_push(stack, a).
 *
 * Updated: Sep 30 3:17am 2003. version 1.2.1
 *		# The type of second argument of the macro function "stack_pop"
 *		  was changed to pointer to avoid confusions (It can be any
 *		  type before).
 *
 * Updated: National Day 5:00am 2003. version 1.2.2
 *		# Several bugs were killed.
 * 
 * Updated: Oct 3 7:54am 2003. version 1.2.3
 *		# "stack_peep" is added.
 *
 * Updated: Oct 12 8:04pm 2003. version 2.0.0
 *		# Everything is different. The second argument of "stack_pop" is
 *		  the data type which you want to pop. And the result will been
 *		  returned instead of stored in the pointer given as the second
 *		  argument. Same as "stack_peep".
 *		# NOTE!!! "stack_pop" and "stack_peep" will no longer examine
 *		  whether the stack has enough data to been popped (peeped).
 *		  Popping of peeping a empty stack will core your program.
 *		# Function "stack_isempty" and "stack_height" are added for the
 *		  users to examine how many data are left in the stack.
 *		# Function "stack_init" is obsoleted by "stack_new", and the latter
 *		  take an unsigned int argument for users to set the initial stack
 *		  size. If you are in doubt, use STACK_INITIAL_SIZE for the size.
 *		  "stack_new" returns a "stack_t", which is a pointer of
 *		  "struct __stack".
 *		# The internal structure of "struct __stack" is no longer hidden
 *		  to the users but declared in the .h file, because some macro
 *		  functions need to know it.
 *
 * Updated: Oct 13 1:00am 2003. version 2.0.1
 *		# A bug is found when reallocating space for the stack, fixed.
 *		# "stack_isempty" is renamed "stack_empty".
 *		# Mandatory type conversion is used when adding or minusing a
 *		  void pointer so that the module can live in C++.
 *		# The bug mentioned in version 1.2.0 lives on. Take care.
 *
 * Updated: Oct 19 1:46am 2003. version 2.1.0
 *		# A fatal bug found and I have to change the interface in order
 *		  to fix this bug. The way I used to caculate the size of an
 *		  element is wrong, becuase sizeof ('a') = 4... Now, users must
 *		  tell "stack_push" what's the data type he/she wanna push. And
 *		  this fixs the bug in introduced by version 1.2.0 too.
 *		# The date types of "base" and "top" are changed to "char *".
 *		  Mandatory type conversion is not needed any more.
 *
 * Updated: Oct 20 21:05pm 2003. version 2.1.1
 *		# Change, change, change! Internal function "__stack_push" no
 *		  longer exists. It's subsituted by "__stack_expand", and the
 *		  latter is called only when the stack has insufficient space
 *		  to store the element to be pushed and it fulfils only the task
 *		  of expanding the stack, not storing data. Macro "stack_push"
 *		  is responsible for storing data. This means that no need
 *		  expanding stack, no function called.
 *
 * Updated: Oct 20 1:22am 2003. version 2.2.0
 *		# "stack_peep" is renamed "stack_top";
 *		  "stack_new" is renamed "stack_create.
 */
#include <stdlib.h>
#include "stack.h"

stack_t *stack_create(unsigned int isize)
{
	stack_t *stack = (stack_t *)malloc(sizeof (stack_t));

	if (stack)
	{
		if (stack->base = (char *)malloc(isize))
		{
			stack->size = isize;
			stack->top = stack->base;
		}
		else
		{
			free(stack);
			stack = NULL;
		}
	}

	return stack;
}

int __stack_expand(stack_t *stack, unsigned int esize)
{
	char *new_base;
	unsigned int new_size = stack->size << 1;

	while (stack->top + esize > stack->base + new_size)
		new_size <<= 1;

	if (new_base = (char *)realloc(stack->base, new_size))
	{
		stack->top = stack->top - stack->base + new_base;
		stack->base = new_base;
		return stack->size = new_size;
	}

	return -1;
}

void stack_destroy(stack_t *stack)
{
	free(stack->base);
	free(stack);
}

