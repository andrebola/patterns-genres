/*
 * priority_queue.h - A priority queue implementation for the geometric P2 and
 *                    P3 algorithms.
 *
 * Version 2007-05-28
 *
 *
 * Copyright (C) 2007 Niko Mikkila and Mika Turkia
 *
 * University of Helsinki, Department of Computer Science, C-BRAHMS project
 *
 * Contact: mikkila@cs.helsinki.fi
 *
 *
 * This file is part of geometric-cbmr,
 * C-BRAHMS Geometric algorithms for Content-Based Music Retrieval.
 *
 * Geometric-cbmr is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * Geometric-cbmr is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * geometric-cbmr; if not, write to the Free Software Foundation, Inc., 59
 * Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */


#ifndef __PRIORITY_QUEUE_H__
#define __PRIORITY_QUEUE_H__

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>

#include "config.h"
#include "geometric_P3.h"

#ifdef __cplusplus
extern "C" {
#endif


/**
 * A bitfield structure for masking double variables into their components.
 */
typedef struct {
    unsigned int mantissa2:32;
    unsigned int mantissa1:20;
    unsigned int exponentbias1023:11;
    unsigned int signbit:1;
} doubleMask;


/**
 * Double variable mask.
 */
typedef union {
    double asDouble;
    doubleMask asMask;
} doubleAndMask;


/**
 * A priority queue node.
 */
typedef struct {
    unsigned int index;
    int key1;
    int key2;
    void *pointer;
} pqnode;


/**
 * Priority queue root.
 */
typedef struct {
    unsigned int size;
    unsigned int nodecount;
    pqnode *nodes;
    pqnode **tree;
} pqroot;



/*
 * Function definitions.
 * These are are here beceuse most of the functions should be inlined for
 * maximum performance and currently using the "static inline" directive seems
 * to be the most portable way to do it. This can be changed after GCC works
 * in C99 mode by default.
 */


/**
 * Calculates a base 2 logarithm of the given value.
 *
 * @param n the input value
 *
 * @return log_2(n)
 */
static INLINE unsigned int pq_log_2(unsigned int n) {
    doubleAndMask fm;
    fm.asDouble = (double) n;
    return (fm.asMask.exponentbias1023 - 1023);
}


/**
 * Creates a priority queue. The queue is stored to an array and each
 * node in the queue can be accessed directly with an index.
 *
 * @param size queue size
 *
 * @return the created queue
 */
static INLINE pqroot *pq_create(unsigned int size) {
    unsigned int i, j, leaves;
    int level, n;
    pqroot *pq;

    leaves = 1 << (pq_log_2(size-1) + 1);

    pq = (pqroot *) malloc(sizeof(pqroot));
    pq->nodes = (pqnode *) malloc(leaves * sizeof(pqnode));
    pq->tree = (pqnode **) malloc(2 * leaves * sizeof(pqnode *));
    pq->size = size;
    pq->nodecount = leaves;

    /* generate an empty tree */
    for (i = 0; i < leaves; ++i) {
        pq->tree[leaves + i] = &pq->nodes[i];
        pq->nodes[i].index = i;
        pq->nodes[i].key1 = INT_MAX;
        pq->nodes[i].key2 = INT_MAX;
        pq->nodes[i].pointer = NULL;
    }
    j = leaves >> 1;
    level = 2;
    n = leaves - level;
    for (i=leaves-1; i>0; --i) {
        pq->tree[i] = &pq->nodes[n];
        n -= level;
        if (i == j) {
            level <<= 1;
            j >>= 1;
            n = leaves - level;
        }
    }

    return pq;
}

/**
 * Releases the memory buffers allocated for a priority eueue.
 *
 * @param pq the queue to free
 */
static INLINE void pq_free(pqroot *pq) {
    free(pq->nodes);
    free(pq->tree);
    free(pq);
}


/**
 * Updates the priority queue after a change in node's key values.
 *
 * @param pq the priority queue
 * @param n changed node
 */
static INLINE void pq_update(pqroot *pq, pqnode *n) {
    pqnode **tree = pq->tree;

    unsigned int i = pq->nodecount + n->index;

    while (i > 1) {
        unsigned int pair;
        /* If index is odd then its pair is the previous even entry, otherwise
           the pair is the next odd entry. */
        if (i & 1) {
            pqnode *n2;
            pair = i - 1;
            n2 = tree[pair];
            i >>= 1;
            if ((n->key1 < n2->key1) || ((n->key1 == n2->key1) &&
                    (n->key2 < n2->key2))) {
                tree[i] = n;
            } else {
                tree[i] = n2;
                n = n2;
            }
        } else {
            pqnode *n2;
            pair = i + 1;
            n2 = tree[pair];
            i >>= 1;
            /* Sort the nodes so that one with the smaller index comes first
               if other keys are equal; hence the "<=" in this branch. */
            if ((n->key1 < n2->key1) || ((n->key1 == n2->key1) &&
                    (n->key2 <= n2->key2))) {
                tree[i] = n;
            } else {
                tree[i] = n2;
                n = n2;
            }
        }
    }
}


/**
 * Updates the priority queue after a change in node's key value.
 * This version only checks the primary key when updating and it is optimized
 * for the P2 algorithm.
 *
 * Note: This function is called often in the inner loop of the algorithms,
 * so optimizing it further can result in a significant speedup.
 *
 * @param pq the priority queue
 * @param n changed node
 */
static INLINE void pq_update_key1_p2(pqroot *pq, pqnode *n) {
    pqnode **tree = pq->tree;

    unsigned int i = pq->nodecount + n->index;

    while (i > 1) {
        pqnode *n2;
        /* If index is odd then its pair is the previous even entry, otherwise
           the pair is the next odd entry. */
        /* FIXME: This does not seem to work on big-endian processors */
        int odd = i & 1;
        unsigned int pair = i + 1 - (odd << 1);

        n2 = tree[pair];
        i >>= 1;
        if (n->key1 + odd > n2->key1) n = n2;
        tree[i] = n;
    }
}


/**
 * Updates the priority queue after a change in node's key value.
 * This version only checks the primary key when updating and it is optimized
 * for the P3 algorithm.
 *
 * Note: This function is called often in the inner loop of the algorithms,
 * so optimizing it further can result in a significant speedup.
 *
 * @param pq the priority queue
 * @param n changed node
 */
static INLINE void pq_update_key1_p3(pqroot *pq, pqnode *n) {
    pqnode **tree = pq->tree;

    unsigned int i = pq->nodecount + n->index;

    while (i > 1) {
        pqnode *n2;
        /* If index is odd then its pair is the previous even entry, otherwise
           the pair is the next odd entry. */
        unsigned int pair = i + 1 - ((i & 1) << 1);

        n2 = tree[pair];
        i >>= 1;
        if (n->key1 >= n2->key1) n = n2;
        tree[i] = n;
    }
}


#if 0
/**
 * Updates the priority queue after a change in node's key value.
 * This is an unoptimized version. Use pq_update_key1_p2() instead.
 *
 * @param pq the priority queue
 * @param n changed node
 */
static INLINE void pq_update_key1_unoptimized(pqroot *pq, pqnode *n) {
    pqnode **tree = pq->tree;

    unsigned int i = pq->nodecount + n->index;

    while (i > 1) {
        unsigned int pair;
        /* If index is odd then its pair is the previous even entry, otherwise
           the pair is the next odd entry. */
        if (i & 1) {
            pqnode *n2;
            pair = i - 1;
            n2 = tree[pair];
            i >>= 1;
            if (n->key1 < n2->key1) {
                tree[i] = n;
            } else {
                tree[i] = n2;
                n = n2;
            }
        } else {
            pqnode *n2;
            pair = i + 1;
            n2 = tree[pair];
            i >>= 1;
            /* Sort the nodes so that one with the smaller index comes first
               if other keys are equal; hence the "<=" in this branch. */
            if (n->key1 <= n2->key1) {
                tree[i] = n;
            } else {
                tree[i] = n2;
                n = n2;
            }
        }
    }
}
#endif


/**
 * A macro that returns the requested node from a priority queue.
 * The caller should make sure that the index does not exceed queue size.
 *
 * @param pq the priority queue
 * @param i index of the node to retrieve
 *
 * @raturns a node in the queue
 */
#define pq_getnode(pq, i) &pq->nodes[i]

/* An inline function version of the macro above. */
#if 0
static INLINE pqnode *pq_getnode(pqroot *pq, unsigned int i) {
    return &pq->nodes[i];
}
#endif


/**
 * A macro that returns the first (minimum) node of the given priority
 * queue.
 *
 * @param pq the priority queue
 *
 * @raturns the first node
 */

#define pq_getmin(pq) pq->tree[1]

/* An inline function version of the macro above. */
#if 0
static INLINE pqnode *pq_getmin(pqroot *pq) {
    /* the minimum is always in the root */
    return pq->tree[1];
}
#endif



/* Test functions. */

#if 0

/**
 * Prints a representation of the given priority queue to standard output.
 *
 * @param pq the priority queue
 */
static void pq_printtree(pqroot *pq) {
    int level = 1;
    unsigned int i;
    unsigned int j = 2;
    printf("leaves: %d\n", pq->nodecount);
    for (i=1; i<pq->nodecount*2; ++i) {
        if (i == j) {
            printf("\n");
            level <<= 1;
            j += level;
        }
        printf("%d:%d ", pq->tree[i]->index,
                pq->tree[i]->key1);
    }
}

/**
 * A small test program for the priority queue data structure.
 */
static void pq_test() {
    pqroot *pq;
    int i;
    unsigned int seed = time(NULL);
    srand(seed);

    pq = pq_create(10);
    printf("\n\n");
    pq_printtree(pq);
    printf("\n");

    printf("PQ: adding keys...\n");
    for (i=0; i<10; ++i) {
        pqnode *n;
        int key = rand();
        key = (int) (((float) key / (float) RAND_MAX) * 1000.0F - 500.0F);
        /* int key = i % 3333; */
        n = pq_getnode(pq, i);
        n->key1 = key;
        pq_update_key1_p2(pq, n);
        printf("%d ", key);
    }
    printf("\n\n");
    pq_printtree(pq);
    printf("\n");
    printf("\nPQ: getting keys...\n");
    for (i=0; i<10; ++i) {
        pqnode *n = pq_getmin(pq);
        printf("min: %d:%d\n", n->index, n->key1);
        n->key1 = INT_MAX;
        pq_update_key1_p2(pq, n);
    }
    pq_free(pq);
}

#endif

#ifdef __cplusplus
}
#endif


#endif

