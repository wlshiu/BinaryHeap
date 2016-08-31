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

typedef void*   priq_priority_t;

/** callback functions to get/set/compare the priority of an element */
typedef priq_priority_t (*CB_PRIORITY_GET)(void *pNode);
typedef void (*CB_PRIORITY_SET)(void *pNode, priq_priority_t node_priority);

/**
 *  compare 2 nodes priority,
 *  return 'true'   => change nodes
 *         'false'  => keep state
 */
typedef int (*CB_PRIORITY_CMP)(priq_priority_t next_node, priq_priority_t cur_node);


/** callback functions to get/set the position of an element */
typedef size_t (*CB_POSITION_GET)(void *pNode);
typedef void (*CB_POSITION_SET)(void *pNode, int position);


/** debug callback function to print a entry */
typedef void (*CB_PRINT_ENTRY)(FILE *out, void *pNode);
//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================
typedef struct priq_init_info
{
    int         amount_nodes;

    CB_PRIORITY_GET     pf_pri_get;
    CB_PRIORITY_SET     pf_pri_set;
    CB_PRIORITY_CMP     pf_pri_cmp;

    CB_POSITION_GET     pf_pos_get;
    CB_POSITION_SET     pf_pos_set;

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


#ifdef __cplusplus
}
#endif

#endif
