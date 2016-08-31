/**
 * Copyright (c) 2016 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file binery_heap.c
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2016/08/31
 * @license
 * @description
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "binary_heap.h"
#include "pthread.h"

/**
 *  binary heap: binary tree map to an array
 *
 *               a          level 0 (2^0)
 *             /  \
 *            b    c        level 1 (2^1)
 *           / \  / \
 *          d  e  f         level 2 (2^2)
 *
 *   array:  [a, b, c, d, e, f]
 *
 */

//=============================================================================
//                  Constant Definition
//=============================================================================

//=============================================================================
//                  Macro Definition
//=============================================================================
#define err(str, args...)       fprintf(stderr, "%s[#%d] " str, __func__, __LINE__, ## args)


#define LEFT(x)         ((x) << 1)
#define RIGHT(x)        (((x) << 1) + 1)
#define PARENT(x)       ((x) >> 1)


#ifndef MEMBER_OFFSET
    #define MEMBER_OFFSET(type, member)     (unsigned long)&(((type *)0)->member)
#endif

#ifndef STRUCTURE_POINTER
    #define STRUCTURE_POINTER(type, ptr, member)    (type*)((unsigned long)ptr - MEMBER_OFFSET(type, member))
#endif

#define priq_l_verify_handle(handle, err_code)          \
            do{ if(handle==NULL){                       \
                err("%s", "input Null pointer !!\n");   \
                return err_code;}                       \
            }while(0)



#define priq_mutex_init(pMtx)               pthread_mutex_init(pMtx, NULL)
#define priq_mutex_deinit(pMtx)             pthread_mutex_destroy(pMtx)
#define priq_mutex_lock(pMtx)               pthread_mutex_lock(pMtx)
#define priq_mutex_unlock(pMtx)             pthread_mutex_unlock(pMtx)
//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct priq_dev
{
    priq_t              hPriq;

    pthread_mutex_t     mutex;

    int                 cur_node_cnt;
    int                 avail_nodes;

    CB_PRIORITY_GET     pf_pri_get;
    CB_PRIORITY_SET     pf_pri_set;
    CB_PRIORITY_CMP     pf_pri_cmp;

    CB_POSITION_GET     pf_pos_get;
    CB_POSITION_SET     pf_pos_set;


    void                **ppNode_list;

} priq_dev_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================
static void
_bubble_up(
    priq_dev_t  *pDev,
    int         start_idx)
{
    return;

    #if 0
    size_t parent_node;
    void *moving_node = q->d[i];
    pqueue_pri_t moving_pri = q->getpri(moving_node);

    for (parent_node = parent(i);
         ((i > 1) && q->cmppri(q->getpri(q->d[parent_node]), moving_pri));
         i = parent_node, parent_node = parent(i))
    {
        q->d[i] = q->d[parent_node];
        q->setpos(q->d[i], i);
    }

    q->d[i] = moving_node;
    q->setpos(moving_node, i);
    #endif
}

static void
_percolate_down(
    priq_dev_t  *pDev,
    int         start_idx)
{
    return;

    #if 0
    size_t child_node;
    void *moving_node = q->d[i];
    pqueue_pri_t moving_pri = q->getpri(moving_node);

    while ((child_node = maxchild(q, i)) &&
           q->cmppri(moving_pri, q->getpri(q->d[child_node])))
    {
        q->d[i] = q->d[child_node];
        q->setpos(q->d[i], i);
        i = child_node;
    }

    q->d[i] = moving_node;
    q->setpos(moving_node, i);
    #endif
}
//=============================================================================
//                  Public Function Definition
//=============================================================================
priq_err_t
priq_create(
    priq_t              **ppHPriq,
    priq_init_info_t    *pInit_info)
{
    priq_err_t      rval = PRIQ_ERR_OK;
    priq_dev_t      *pDev = 0;

    do {
        if( !ppHPriq || !(*ppHPriq) || !pInit_info )
        {
            err("%s", "input null pointer\n");
            rval = PRIQ_ERR_INVALID_PARAM;
            break;
        }

        if( !pInit_info->pf_pri_get || !pInit_info->pf_pri_set || !pInit_info->pf_pri_cmp ||
            !pInit_info->pf_pos_get || !pInit_info->pf_pos_set )
        {
            err("%s", "callback can't be null \n");
            rval = PRIQ_ERR_INVALID_PARAM;
            break;
        }


        if( !(pDev = malloc(sizeof(priq_dev_t))) )
        {
            err("malloc hanlde fail, size= %d\n", sizeof(priq_dev_t));
            rval = PRIQ_ERR_MALLOC_FAIL;
            break;
        }

        memset(pDev, 0x0, sizeof(priq_dev_t));

        if( priq_mutex_init(&pDev->mutex) )
        {
            err("%s", "mutex init fail\n");
            rval = PRIQ_ERR_UNKNOWN;
            break;
        }

        // element 0 isn't used for mapping indxe and count.
        pDev->avail_nodes  = pInit_info->amount_nodes + 1;
        pDev->cur_node_cnt = 1;

        pDev->pf_pri_get = pInit_info->pf_pri_get;
        pDev->pf_pri_set = pInit_info->pf_pri_set;
        pDev->pf_pri_cmp = pInit_info->pf_pri_cmp;
        pDev->pf_pos_get = pInit_info->pf_pos_get;
        pDev->pf_pos_set = pInit_info->pf_pos_set;

        if( !(pDev->ppNode_list = malloc(sizeof(void*) * pDev->avail_nodes)) )
        {
            err("malloc node list fail, size= %d\n", sizeof(void*) * pDev->avail_nodes);
            rval = PRIQ_ERR_MALLOC_FAIL;
            break;
        }

        pDev->hPriq.remain_num = pDev->cur_node_cnt - 1;
        //------------------------
        *ppHPriq = &pDev->hPriq;

    } while(0);

    if( rval )
    {
        priq_t  *pHPriq = &pDev->hPriq;
        priq_destroy(&pHPriq);
    }

    return rval;
}

priq_err_t
priq_destroy(priq_t  **ppHPriq)
{
    priq_err_t      rval = PRIQ_ERR_OK;

    do {
        priq_dev_t          *pDev = 0;
        pthread_mutex_t     mutex;

        if( !ppHPriq || !(*ppHPriq) )
        {
            rval = PRIQ_ERR_INVALID_PARAM;
            break;
        }

        pDev = STRUCTURE_POINTER(priq_dev_t, (*ppHPriq), hPriq);

        priq_mutex_lock(&pDev->mutex);

        *ppHPriq = 0;
        mutex = pDev->mutex;

        if( pDev->ppNode_list )
            free(pDev->ppNode_list);

        free(pDev);

        priq_mutex_unlock(&mutex);
        priq_mutex_deinit(&mutex);

    } while(0);

    return rval;
}

priq_err_t
priq_node_push(
    priq_t      *pHPriq,
    void        *pNode)
{
    priq_err_t      rval = PRIQ_ERR_OK;
    priq_dev_t      *pDev = STRUCTURE_POINTER(priq_dev_t, pHPriq, hPriq);

    priq_l_verify_handle(pHPriq, PRIQ_ERR_INVALID_PARAM);
    priq_l_verify_handle(pNode, PRIQ_ERR_INVALID_PARAM);

    priq_mutex_lock(&pDev->mutex);

    do {
        int     idx = 0;
        if( pDev->cur_node_cnt >= pDev->avail_nodes )
        {
            err("queue full %d/%d\n", pDev->cur_node_cnt, pDev->avail_nodes);
            rval = PRIQ_ERR_QUEUE_FULL;
            break;
        }

        idx = pDev->cur_node_cnt++;
        pDev->ppNode_list[idx] = pNode;
        _bubble_up(pDev, idx);

    } while(0);

    priq_mutex_unlock(&pDev->mutex);

    return rval;
}

priq_err_t
priq_node_pop(
    priq_t      *pHPriq,
    void        **ppNode)
{
    priq_err_t      rval = PRIQ_ERR_OK;
    priq_dev_t      *pDev = STRUCTURE_POINTER(priq_dev_t, pHPriq, hPriq);

    priq_l_verify_handle(pHPriq, PRIQ_ERR_INVALID_PARAM);
    priq_l_verify_handle(ppNode, PRIQ_ERR_INVALID_PARAM);

    priq_mutex_lock(&pDev->mutex);

    do {
        if( pDev->cur_node_cnt == 1 )
        {
            err("%s", "queue is empty \n");
            rval = PRIQ_ERR_QUEUE_EMPTY;
            break;
        }

        *ppNode = pDev->ppNode_list[1];

        pDev->ppNode_list[1] = pDev->ppNode_list[--pDev->cur_node_cnt];
        _percolate_down(pDev, 1);

    } while(0);

    priq_mutex_unlock(&pDev->mutex);

    return rval;
}

priq_err_t
priq_node_change_priority(
    priq_t              *pHPriq,
    priq_priority_t     new_pri,
    void                *pNode)
{
    priq_err_t      rval = PRIQ_ERR_OK;
    priq_dev_t      *pDev = STRUCTURE_POINTER(priq_dev_t, pHPriq, hPriq);

    priq_l_verify_handle(pHPriq, PRIQ_ERR_INVALID_PARAM);
    priq_l_verify_handle(pNode, PRIQ_ERR_INVALID_PARAM);

    priq_mutex_lock(&pDev->mutex);

    do {




    } while(0);

    priq_mutex_unlock(&pDev->mutex);

    return rval;
}

priq_err_t
priq_node_peek(
    priq_t              *pHPriq,
    void                **ppNode)
{
    priq_err_t      rval = PRIQ_ERR_OK;
    priq_dev_t      *pDev = STRUCTURE_POINTER(priq_dev_t, pHPriq, hPriq);

    priq_l_verify_handle(pHPriq, PRIQ_ERR_INVALID_PARAM);
    priq_l_verify_handle(ppNode, PRIQ_ERR_INVALID_PARAM);

    priq_mutex_lock(&pDev->mutex);

    do {
        if( pDev->cur_node_cnt == 1 )
        {
            err("%s", "queue is empty \n");
            rval = PRIQ_ERR_QUEUE_EMPTY;
            break;
        }



    } while(0);

    priq_mutex_unlock(&pDev->mutex);

    return rval;
}

priq_err_t
priq_node_remove(
    priq_t              *pHPriq,
    void                *pNode)
{
    priq_err_t      rval = PRIQ_ERR_OK;
    priq_dev_t      *pDev = STRUCTURE_POINTER(priq_dev_t, pHPriq, hPriq);

    priq_l_verify_handle(pHPriq, PRIQ_ERR_INVALID_PARAM);
    priq_l_verify_handle(pNode, PRIQ_ERR_INVALID_PARAM);

    priq_mutex_lock(&pDev->mutex);

    do {




    } while(0);

    priq_mutex_unlock(&pDev->mutex);

    return rval;
}


void
priq_print(
    priq_t          *pHPriq,
    FILE            *out,
    CB_PRINT_ENTRY  pf_print)
{
    #if 0
    pqueue_t *dup;
	void *e;

    dup = pqueue_init(q->size,
                      q->cmppri, q->getpri, set_pri,
                      q->getpos, set_pos);
    dup->size = q->size;
    dup->avail = q->avail;
    dup->step = q->step;

    memcpy(dup->d, q->d, (q->size * sizeof(void *)));

    while ((e = pqueue_pop(dup)))
		print(out, e);

    pqueue_free(dup);
    #endif
}

