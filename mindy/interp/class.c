/**********************************************************************\
*
*  Copyright (C) 1994, Carnegie Mellon University
*  All rights reserved.
*
*  This code was produced by the Gwydion Project at Carnegie Mellon
*  University.  If you are interested in using this code, contact
*  "Scott.Fahlman@cs.cmu.edu" (Internet).
*
***********************************************************************
*
* $Header: /home/housel/work/rcs/gd/src/mindy/interp/class.c,v 1.2 1994/04/09 13:35:45 wlott Exp $
*
* This file does whatever.
*
\**********************************************************************/

#include <stdarg.h>

#include "mindy.h"
#include "gc.h"
#include "type.h"
#include "list.h"
#include "sym.h"
#include "bool.h"
#include "obj.h"
#include "error.h"
#include "def.h"
#include "print.h"
#include "class.h"

obj_t obj_ClassClass = 0;



/* Class constructors. */

obj_t make_builtin_class(int scavenge(struct object *ptr),
			 obj_t transport(obj_t object))
{
    obj_t res = alloc(obj_ClassClass, sizeof(struct class));

    init_class_type_stuff(res);
    CLASS(res)->abstract_p = FALSE;
    CLASS(res)->scavenge = scavenge;
    CLASS(res)->transport = transport;
    CLASS(res)->print = NULL;
    /* These really want to be an obj_t, but we don't have any good obj_t's */
    /* to fill them in with yet. */
    CLASS(res)->debug_name = NULL;
    CLASS(res)->superclasses = NULL;
    CLASS(res)->cpl = NULL;
    CLASS(res)->direct_subclasses = NULL;
    CLASS(res)->all_subclasses = NULL;

    return res;
}

static int scav_lose(struct object *ptr)
{
    lose("Found an instance of an abstract class?\n");
    return 0;
}

static obj_t trans_lose(obj_t obj)
{
    lose("Found an instance of an abstract class?\n");
    return NULL;
}

obj_t make_abstract_class(void)
{
    obj_t res = make_builtin_class(scav_lose, trans_lose);

    CLASS(res)->abstract_p = TRUE;

    return res;
}


/* CPL computation. */

struct cpd {
    obj_t class;
    struct cpd_chain *supers;
    struct cpd_chain *after;
    int count;
};

struct cpd_chain {
    struct cpd *cpd;
    struct cpd_chain *next;
};

static struct cpd_chain *cpds = NULL;
static int class_count = 0;

static void push_cpd(struct cpd *cpd, struct cpd_chain **chain)
{
    struct cpd_chain *new = (struct cpd_chain *)malloc(sizeof(struct cpd));

    new->cpd = cpd;
    new->next = *chain;
    *chain = new;
}

static struct cpd *pop_cpd(struct cpd_chain **chainptr)
{
    struct cpd_chain *chain = *chainptr;
    struct cpd *cpd = chain->cpd;

    *chainptr = chain->next;
    free(chain);

    return cpd;
}

static void free_cpd_chain(struct cpd_chain *chain)
{
    while (chain != NULL) {
	struct cpd_chain *next = chain->next;
	free(chain);
	chain = next;
    }
}

static struct cpd *find_cpd(obj_t class);

static struct cpd *compute_cpd(obj_t class, obj_t supers)
{
    struct cpd *cpd = (struct cpd *)malloc(sizeof(struct cpd));

    cpd->class = class;
    cpd->supers = NULL;
    cpd->after = NULL;
    cpd->count = NULL;
    push_cpd(cpd, &cpds);
    class_count++;

    if (supers != obj_Nil) {
	struct cpd *prev_super_cpd = find_cpd(HEAD(supers));
	push_cpd(prev_super_cpd, &cpd->supers);
	push_cpd(prev_super_cpd, &cpd->after);
	prev_super_cpd->count++;
	while ((supers = TAIL(supers)) != obj_Nil) {
	    struct cpd *super_cpd = find_cpd(HEAD(supers));
	    push_cpd(super_cpd, &cpd->supers);
	    push_cpd(super_cpd, &cpd->after);
	    push_cpd(super_cpd, &prev_super_cpd->after);
	    super_cpd->count += 2;
	    prev_super_cpd = super_cpd;
	}
    }
    return cpd;
}

static struct cpd *find_cpd(obj_t class)
{
    struct cpd_chain *ptr;

    for (ptr = cpds; ptr != NULL; ptr = ptr->next)
	if (ptr->cpd->class == class)
	    return ptr->cpd;

    return compute_cpd(class, CLASS(class)->superclasses);
}

static struct cpd *tie_breaker(struct cpd_chain **candidates, obj_t rcpl)
{
    obj_t remaining, supers;
    struct cpd_chain **prev, *ptr;

    for (remaining = rcpl; remaining != obj_Nil; remaining = TAIL(remaining)) {
	supers = CLASS(HEAD(remaining))->superclasses;
	for (prev = candidates; (ptr = *prev) != NULL; prev = &ptr->next)
	    if (memq(ptr->cpd->class, supers))
		return pop_cpd(prev);
    }
    lose("Can't happen.\n");
    return NULL;
}

static obj_t slow_compute_cpl(obj_t class, obj_t superclasses)
{
    struct cpd_chain *candidates;
    struct cpd *candidate;
    obj_t rcpl;
    int count;
    struct cpd_chain *after;

    cpds = NULL;
    class_count = 0;
    candidates = NULL;
    push_cpd(compute_cpd(class, superclasses), &candidates);
    free_cpd_chain(cpds);
    cpds = NULL;

    rcpl = obj_Nil;
    for (count = 0; count < class_count; count++) {
	if (candidates == NULL)
	    error("Inconsistent CPL");
	if (candidates->next != NULL)
	    candidate = tie_breaker(&candidates, rcpl);
	else
	    candidate = pop_cpd(&candidates);

	rcpl = pair(candidate->class, rcpl);

	free_cpd_chain(candidate->supers);
	for (after = candidate->after; after != NULL; after = after->next) {
	    after->cpd->count--;
	    if (after->cpd->count == 0)
		push_cpd(after->cpd, &candidates);
	}
	free_cpd_chain(candidate->after);
	free(candidate);
    }

    return nreverse(rcpl);
}

static obj_t compute_cpl(obj_t class, obj_t superclasses)
{
    if (superclasses == obj_Nil)
	return list1(class);
    else if (TAIL(superclasses) == obj_Nil)
	return pair(class, CLASS(HEAD(superclasses))->cpl);
    else
	return slow_compute_cpl(class, superclasses);
}


/* Class initialization. */

void setup_class_supers(obj_t class, obj_t supers)
{
    obj_t cpl, scan;

    CLASS(class)->superclasses = supers;
    cpl = compute_cpl(class, supers);
    CLASS(class)->cpl = cpl;

    for (scan = TAIL(cpl); scan != obj_Nil; scan = TAIL(scan)) {
	obj_t super = HEAD(scan);
	CLASS(super)->all_subclasses
	    = pair(class, CLASS(super)->all_subclasses);
    }
    for (scan = supers; scan != obj_Nil; scan = TAIL(scan)) {
	obj_t super = HEAD(scan);
	CLASS(super)->direct_subclasses
	    = pair(class, CLASS(super)->direct_subclasses);
    }
}

void init_builtin_class(obj_t class, char *name, ...)
{
    obj_t super, supers;
    va_list ap;

    supers = obj_Nil;
    va_start(ap, name);
    while ((super = va_arg(ap, obj_t)) != NULL)
	supers = pair(super, supers);
    va_end(ap);
    supers = nreverse(supers);

    CLASS(class)->debug_name = symbol(name);
    setup_class_supers(class, supers);
    CLASS(class)->direct_subclasses = obj_Nil;
    CLASS(class)->all_subclasses = obj_Nil;

    define_class(name, class);
}


/* Dylan functions. */

static obj_t direct_superclasses(obj_t class)
{
    return CLASS(class)->superclasses;
}

static obj_t direct_subclasses(obj_t class)
{
    return CLASS(class)->direct_subclasses;
}

static obj_t all_superclasses(obj_t class)
{
    return CLASS(class)->cpl;
}


/* Printer support. */

static void print_class(obj_t class)
{
    obj_t debug_name = CLASS(class)->debug_name;

    if (debug_name != NULL && debug_name != obj_False)
	printf("{class %s}", sym_name(debug_name));
    else
	printf("{anonymous class 0x%08lx}", (unsigned long)class);
}


/* GC stuff. */

static int scav_class(struct object *o)
{
    struct class *c = (struct class *)o;

    scavenge(&c->debug_name);
    scavenge(&c->superclasses);
    scavenge(&c->cpl);
    scavenge(&c->direct_subclasses);
    scavenge(&c->all_subclasses);

    return sizeof(struct class);
}

static obj_t trans_class(obj_t class)
{
    return transport(class, sizeof(struct class));
}

void scavenge_class_roots(void)
{
    scavenge(&obj_ClassClass);
}


/* Init stuff. */

void make_class_classes(void)
{
    obj_ClassClass = ptr_obj(0);
    obj_ClassClass = make_builtin_class(scav_class, trans_class);
    CLASS(obj_ClassClass)->class = obj_ClassClass;
}

void init_class_classes(void)
{
    init_builtin_class(obj_ClassClass, "<class>", obj_TypeClass, NULL);
    def_printer(obj_ClassClass, print_class);
}

void init_class_functions(void)
{
    define_method("all-superclasses", list1(obj_ClassClass), FALSE, obj_False,
		  obj_ObjectClass, all_superclasses);
    define_method("direct-superclasses", list1(obj_ClassClass), FALSE,
		  obj_False, obj_ObjectClass, direct_superclasses);
    define_method("direct-subclasses", list1(obj_ClassClass), FALSE,
		  obj_False, obj_ObjectClass, direct_subclasses);
}
