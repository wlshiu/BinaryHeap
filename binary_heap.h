/**
 * Copyright (c) 2016 Wei-Lun Hsu. All Rights Reserved.
 */
/** @file binery_heap.h
 *
 * @author Wei-Lun Hsu
 * @version 0.1
 * @date 2016/08/31
 * @license
 * @description
 */

#ifndef __binery_heap_H_wuIPI8x5_l1Wz_Hfxs_sPuj_uVrSwgZmMTo3__
#define __binery_heap_H_wuIPI8x5_l1Wz_Hfxs_sPuj_uVrSwgZmMTo3__

#ifdef __cplusplus
extern "C" {
#endif


//=============================================================================
//                  Constant Definition
//=============================================================================
/**
 *  error code
 */
typedef enum priq_err
{
    PRIQ_ERR_OK                 = 0,
    PRIQ_ERR_MALLOC_FAIL,
    PRIQ_ERR_INVALID_PARAM,
    PRIQ_ERR_QUEUE_FULL,
    PRIQ_ERR_QUEUE_EMPTY,
    PRIQ_ERR_UNKNOWN,
} priq_err_t;

/**
 *  priority type
 */
typedef struct priq_priority
{
    union {
        void                *ptr;
        unsigned int        u32_value;
        unsigned long long  u64_value;
    } u;
} priq_priority_t;

/** callback functions to get/set/compare the priority of an element */
typedef priq_priority_t* (*CB_PRIORITY_GET)(void *pNode);
typedef void (*CB_PRIORITY_SET)(void *pNode, priq_priority_t *pNode_priority);

/**
 *  compare 2 nodes priority,
 *  return 'true'   => change nodes
 *         'false'  => keep state
 */
typedef int (*CB_PRIORITY_CMP)(priq_priority_t *pNext_node, priq_priority_t *pCur_node);


/** callback functions to get/set the position of an element in internal node list */
typedef int (*CB_POSITION_GET)(void *pNode);
typedef void (*CB_POSITION_SET)(void *pNode, int index);


/** debug callback function to print a entry */
typedef void (*CB_PRINT_ENTRY)(void *pOut_dev, void *pNode, void *pExtra);
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
/**
 *  init info
 */
typedef struct priq_init_info
{
    int         amount_nodes;

    CB_PRIORITY_GET     cb_pri_get;
    CB_PRIORITY_SET     cb_pri_set;
    CB_PRIORITY_CMP     cb_pri_cmp;

    CB_POSITION_GET     cb_pos_get;
    CB_POSITION_SET     cb_pos_set;

} priq_init_info_t;

/**
 *  priority queue handle
 */
typedef struct priq
{
    int         remain_num;
} priq_t;
//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================
priq_err_t
priq_create(
    priq_t              **ppHPriq,
    priq_init_info_t    *pInit_info);


priq_err_t
priq_destroy(priq_t  **ppHPriq);


priq_err_t
priq_node_push(
    priq_t      *pHPriq,
    void        *pNode);


priq_err_t
priq_node_pop(
    priq_t      *pHPriq,
    void        **ppNode);


priq_err_t
priq_node_change_priority(
    priq_t              *pHPriq,
    priq_priority_t     *pNew_pri,
    void                *pNode);


priq_err_t
priq_node_peek(
    priq_t              *pHPriq,
    void                **ppNode);


priq_err_t
priq_node_remove(
    priq_t      *pHPriq,
    void        *pNode);


priq_err_t
priq_print(
    priq_t          *pHPriq,
    void            *pOut_device,
    void            *pExtra,
    CB_PRINT_ENTRY  cb_print);


#ifdef __cplusplus
}
#endif

#endif
