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

#define priq_verify_handle(handle, err_code)            \
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

    int                 node_cnt;
    long                max_nodes;

    CB_PRIORITY_GET     cb_pri_get;
    CB_PRIORITY_SET     cb_pri_set;
    CB_PRIORITY_CMP     cb_pri_cmp;

    CB_POSITION_GET     cb_pos_get;
    CB_POSITION_SET     cb_pos_set;


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
    long        idx)
{
    long                parent_idx = 0l;
    void                **ppNode_list = pDev->ppNode_list;
    void                *pCur_node = 0;
    priq_priority_t     *pCur_node_pri = 0;
    CB_PRIORITY_GET     cb_pri_get = pDev->cb_pri_get;
    CB_PRIORITY_CMP     cb_pri_cmp = pDev->cb_pri_cmp;
    CB_POSITION_SET     cb_pos_set = pDev->cb_pos_set;

    pCur_node     = ppNode_list[idx];
    pCur_node_pri = cb_pri_get(pCur_node);

    for(parent_idx = PARENT(idx);
        (idx > 1) && cb_pri_cmp(cb_pri_get(ppNode_list[parent_idx]), pCur_node_pri);
        idx = parent_idx, parent_idx = PARENT(idx))
    {
        ppNode_list[idx] = ppNode_list[parent_idx];
        cb_pos_set(ppNode_list[idx], (int)idx);
    }

    ppNode_list[idx] = pCur_node;
    cb_pos_set(pCur_node, (int)idx);

    return;
}

static long
_get_child_idx(
    priq_dev_t  *pDev,
    long        idx)
{
    long                child_idx = LEFT(idx);
    void                **ppNode_list = pDev->ppNode_list;
    CB_PRIORITY_GET     cb_pri_get = pDev->cb_pri_get;
    CB_PRIORITY_CMP     cb_pri_cmp = pDev->cb_pri_cmp;

    if( child_idx >= pDev->node_cnt )
        return 0l;

    // choice left or right child node
    if( (child_idx + 1) < pDev->node_cnt &&
        cb_pri_cmp(cb_pri_get(ppNode_list[child_idx]), cb_pri_get(ppNode_list[child_idx+1])) )
        child_idx++; // select right child node

    return child_idx;
}

static void
_percolate_down(
    priq_dev_t  *pDev,
    long        idx)
{
    long                child_idx = 0l;
    void                **ppNode_list = pDev->ppNode_list;
    void                *pCur_node = 0;
    priq_priority_t     *pCur_node_pri = 0;
    CB_PRIORITY_GET     cb_pri_get = pDev->cb_pri_get;
    CB_PRIORITY_CMP     cb_pri_cmp = pDev->cb_pri_cmp;
    CB_POSITION_SET     cb_pos_set = pDev->cb_pos_set;

    pCur_node     = ppNode_list[idx];
    pCur_node_pri = cb_pri_get(pCur_node);

    while( (child_idx = _get_child_idx(pDev, idx)) &&
            cb_pri_cmp(pCur_node_pri, cb_pri_get(ppNode_list[child_idx])) )
    {
        ppNode_list[idx] = ppNode_list[child_idx];
        cb_pos_set(ppNode_list[idx], (int)idx);

        idx = child_idx;
    }

    ppNode_list[idx] = pCur_node;
    cb_pos_set(pCur_node, (int)idx);

    return;
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
        if( !ppHPriq || (*ppHPriq) || !pInit_info )
        {
            err("%s", "input null pointer\n");
            rval = PRIQ_ERR_INVALID_PARAM;
            break;
        }

        if( !pInit_info->cb_pri_get || !pInit_info->cb_pri_set || !pInit_info->cb_pri_cmp ||
            !pInit_info->cb_pos_get || !pInit_info->cb_pos_set )
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
        pDev->max_nodes = pInit_info->amount_nodes + 1;
        pDev->node_cnt  = 1;

        pDev->cb_pri_get = pInit_info->cb_pri_get;
        pDev->cb_pri_set = pInit_info->cb_pri_set;
        pDev->cb_pri_cmp = pInit_info->cb_pri_cmp;
        pDev->cb_pos_get = pInit_info->cb_pos_get;
        pDev->cb_pos_set = pInit_info->cb_pos_set;

        if( !(pDev->ppNode_list = malloc(sizeof(void*) * pDev->max_nodes)) )
        {
            err("malloc node list fail, size= %d\n", sizeof(void*) * pDev->max_nodes);
            rval = PRIQ_ERR_MALLOC_FAIL;
            break;
        }
        memset(pDev->ppNode_list, 0x0, sizeof(void*) * pDev->max_nodes);

        pDev->hPriq.remain_num = pDev->node_cnt - 1;
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

    priq_verify_handle(pHPriq, PRIQ_ERR_INVALID_PARAM);
    priq_verify_handle(pNode, PRIQ_ERR_INVALID_PARAM);

    priq_mutex_lock(&pDev->mutex);

    do {
        int     idx = 0;

        if( pDev->node_cnt >= pDev->max_nodes )
        {
            err("queue full %d/%d\n", pDev->node_cnt, pDev->max_nodes);
            rval = PRIQ_ERR_QUEUE_FULL;
            break;
        }

        idx = pDev->node_cnt++;
        pDev->ppNode_list[idx] = pNode;

        _bubble_up(pDev, idx);

        pDev->hPriq.remain_num = pDev->node_cnt - 1;

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

    priq_verify_handle(pHPriq, PRIQ_ERR_INVALID_PARAM);
    priq_verify_handle(ppNode, PRIQ_ERR_INVALID_PARAM);

    priq_mutex_lock(&pDev->mutex);

    do {
        *ppNode = NULL;

        if( pDev->node_cnt == 1 )
        {
            err("%s", "queue is empty \n");
            rval = PRIQ_ERR_QUEUE_EMPTY;
            break;
        }

        *ppNode = pDev->ppNode_list[1];

        pDev->ppNode_list[1] = pDev->ppNode_list[--pDev->node_cnt];
        _percolate_down(pDev, 1);

        pDev->hPriq.remain_num = pDev->node_cnt - 1;

    } while(0);

    priq_mutex_unlock(&pDev->mutex);

    return rval;
}

priq_err_t
priq_node_change_priority(
    priq_t              *pHPriq,
    priq_priority_t     *pNew_pri,
    void                *pNode)
{
    priq_err_t      rval = PRIQ_ERR_OK;
    priq_dev_t      *pDev = STRUCTURE_POINTER(priq_dev_t, pHPriq, hPriq);

    priq_verify_handle(pHPriq, PRIQ_ERR_INVALID_PARAM);
    priq_verify_handle(pNew_pri, PRIQ_ERR_INVALID_PARAM);
    priq_verify_handle(pNode, PRIQ_ERR_INVALID_PARAM);

    priq_mutex_lock(&pDev->mutex);

    do {
        int                 cur_idx = 0;
        priq_priority_t     cur_node_pri = {{0}};

        cur_node_pri = *(pDev->cb_pri_get(pNode));

        pDev->cb_pri_set(pNode, pNew_pri);
        cur_idx = pDev->cb_pos_get(pNode);

        if( pDev->cb_pri_cmp(&cur_node_pri, pNew_pri) )
            _bubble_up(pDev, cur_idx);
        else
            _percolate_down(pDev, cur_idx);

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

    priq_verify_handle(pHPriq, PRIQ_ERR_INVALID_PARAM);
    priq_verify_handle(ppNode, PRIQ_ERR_INVALID_PARAM);

    priq_mutex_lock(&pDev->mutex);

    do {
        *ppNode = NULL;

        if( pDev->node_cnt == 1 )
        {
            err("%s", "queue is empty \n");
            rval = PRIQ_ERR_QUEUE_EMPTY;
            break;
        }

        *ppNode = pDev->ppNode_list[1];

    } while(0);

    priq_mutex_unlock(&pDev->mutex);

    return rval;
}

priq_err_t
priq_node_remove(
    priq_t      *pHPriq,
    void        *pNode)
{
    priq_err_t      rval = PRIQ_ERR_OK;
    priq_dev_t      *pDev = STRUCTURE_POINTER(priq_dev_t, pHPriq, hPriq);

    priq_verify_handle(pHPriq, PRIQ_ERR_INVALID_PARAM);
    priq_verify_handle(pNode, PRIQ_ERR_INVALID_PARAM);

    priq_mutex_lock(&pDev->mutex);

    do {
        long         cur_idx = 0l;

        cur_idx = (long)pDev->cb_pos_get(pNode);
        pDev->ppNode_list[cur_idx] = pDev->ppNode_list[--pDev->node_cnt];

        if( pDev->cb_pri_cmp(pDev->cb_pri_get(pNode), pDev->cb_pri_get(pDev->ppNode_list[cur_idx])))
            _bubble_up(pDev, cur_idx);
        else
            _percolate_down(pDev, cur_idx);

    } while(0);

    priq_mutex_unlock(&pDev->mutex);

    return rval;
}

void
priq_node_search(
    priq_t          *pHPriq,
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

static void
_print_set_pos(void *pNode, int idx)
{
    /* when print, do nothing */
    return;
}


static void
_print_set_pri(void *pNode, priq_priority_t *pPri)
{
    /* when print, do nothing */
    return;
}

priq_err_t
priq_print(
    priq_t          *pHPriq,
    void            *pOut_device,
    void            *pExtra,
    CB_PRINT_ENTRY  cb_print)
{
    priq_err_t      rval = PRIQ_ERR_OK;
    priq_dev_t      *pDev = STRUCTURE_POINTER(priq_dev_t, pHPriq, hPriq);

    priq_verify_handle(pHPriq, PRIQ_ERR_INVALID_PARAM);

    priq_mutex_lock(&pDev->mutex);

    do {
        priq_dev_t          dup_dev = {{0}};
        void                **ppNode_list = 0, *pNode = 0;

        if( pDev->node_cnt == 1 )
        {
            err("%s", "queue is empty \n");
            break;
        }

        dup_dev.max_nodes  = pDev->max_nodes;
        dup_dev.node_cnt   = pDev->node_cnt;
        dup_dev.cb_pri_get = pDev->cb_pri_get;
        dup_dev.cb_pri_cmp = pDev->cb_pri_cmp;
        dup_dev.cb_pos_get = pDev->cb_pos_get;

        dup_dev.cb_pri_set = _print_set_pri;
        dup_dev.cb_pos_set = _print_set_pos;

        if( !(ppNode_list = malloc(sizeof(void*) * (dup_dev.node_cnt + 1))) )
        {
            err("malloc node list fail, size= %d\n", sizeof(void*) * (dup_dev.node_cnt + 1));
            break;
        }

        dup_dev.ppNode_list = ppNode_list;

        memcpy(ppNode_list, pDev->ppNode_list, sizeof(void*) * dup_dev.node_cnt);

        while( dup_dev.node_cnt > 1 &&
               (pNode = ppNode_list[1]) )
        {
            ppNode_list[1] = ppNode_list[--dup_dev.node_cnt];

            _percolate_down(&dup_dev, 1);

            cb_print(pOut_device, pNode, pExtra);
        }

        free(ppNode_list);
    } while(0);

    priq_mutex_unlock(&pDev->mutex);
    return rval;
}

