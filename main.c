#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "binary_heap.h"

//////////////////////////////////
typedef struct node
{
    priq_priority_t     priority;
    int                 val;
    int                 pos;
} node_t;

//////////////////////////////////
static int
_cmp_priority(
    priq_priority_t     *pPri_a,
    priq_priority_t     *pPri_b)
{
    return (pPri_a->u.u32_value > pPri_b->u.u32_value);
}


static priq_priority_t*
_get_priority(void  *pNode)
{
    return &((node_t*)pNode)->priority;
}


static void
_set_priority(
    void                *pNode,
    priq_priority_t     *pPri)
{
    ((node_t*)pNode)->priority = *pPri;
}


static int
_get_positon(void *pNode)
{
    return ((node_t*)pNode)->pos;
}


static void
_set_position(void *pNode, int pos)
{
    ((node_t*)pNode)->pos = pos;
}

static void
_log_node(void  *pOut_dev, void  *pNode, void *pExtra)
{
    node_t  **ppTarget_node = (node_t**)pExtra;
    node_t  *pCur_node = (node_t*)pNode;

    if( ppTarget_node && pCur_node->val == 2 )
        *ppTarget_node = pCur_node;

    fprintf(pOut_dev, "pri: %7d, val: %d\n",
            pCur_node->priority.u.u32_value, pCur_node->val);
}
//////////////////////////////////
int main()
{
    int                 i = 0;
    priq_t              *pHPriq = 0;
    priq_init_info_t    init_info = {0};
    priq_priority_t     new_pri = {{0}};
    node_t              test_node[10] = {{.val = 0}};
    node_t              *pNode = 0;

    srand(123);

    init_info.amount_nodes = 10;
    init_info.cb_pri_get = _get_priority;
    init_info.cb_pri_set = _set_priority;
    init_info.cb_pri_cmp = _cmp_priority;
    init_info.cb_pos_get = _get_positon;
    init_info.cb_pos_set = _set_position;
    priq_create(&pHPriq, &init_info);

    printf("---------- push data --------------\n");
    for(i = 0; i < 10; i++)
    {
        node_t      *pCur_node = &test_node[i];

        pCur_node->priority.u.u32_value = rand();
        pCur_node->val                  = i;

        priq_node_push(pHPriq, (void*)pCur_node);
        printf("\tpri= %7d, valud= %d\n", pCur_node->priority.u.u32_value, i);
    }

    printf("--------- log data --------------\n");
    priq_print(pHPriq, stderr, (void*)&pNode, _log_node);

    printf("--------- change priority --------------\n");
    new_pri.u.u32_value = 111;

    printf("change pri %7d -> 111, pos= %d\n", pNode->priority.u.u32_value, pNode->pos);
    priq_node_change_priority(pHPriq, &new_pri, (void*)pNode);


    printf("--------- pop data --------------\n");

    while( !priq_node_pop(pHPriq, (void**)&pNode) )
    {
        printf("  pri= %7d, val= %d\n", pNode->priority.u.u32_value, pNode->val);
    }

    priq_destroy(&pHPriq);
    return 0;
}
