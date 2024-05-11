#include "list.h"

/**
 * @brief Initialize a list item.
 * @param pxItem
 */
void vListInitialiseItem(ListItem_t* const pxItem)
{
    pxItem->pvContainer = NULL;
}
/**
 * @brief Initialize a root list item.
 * @param pxList
 */
void vListInitialise(List_t* const pxList)
{
    pxList->uxNumberOfItems = (UBaseType_t)0U;
    pxList->pxIndex = (ListItem_t*)&(pxList->xListEnd);
    pxList->xListEnd.xItemValue = portMAX_DELAY;
    pxList->xListEnd.pxNext = (MiniListItem_t*)pxList->pxIndex;
    pxList->xListEnd.pxPrevious = (MiniListItem_t*)pxList->pxIndex;
}

void vListInsertEnd(List_t* const pxList, ListItem_t* const pxNewListItem)
{
    ListItem_t* const pxIndex = pxList->pxIndex;

    pxNewListItem->pxNext = pxIndex;
    pxNewListItem->pxPrevious = pxIndex->pxPrevious;

    pxIndex->pxPrevious->pxNext = pxNewListItem;
    pxIndex->pxPrevious = pxNewListItem;

    pxNewListItem->pvContainer = (void*)pxList;

    (pxList->uxNumberOfItems)++;
}

/**
 * @brief Insert a new list item ascendingly.
 * @param pxList
 * @param pxNewListItem
 */
void vListInsert(List_t* const pxList, ListItem_t* const pxNewListItem)
{
    ListItem_t* pxIterator;
    const TickType_t xValueOfInsertion = pxNewListItem->xItemValue;

    // THe next item of end is the first item of the list.
    if (xValueOfInsertion == portMAX_DELAY) {
        pxIterator = pxList->xListEnd.pxPrevious;
    } else {
        for (pxIterator = (ListItem_t*)&(pxList->xListEnd); pxIterator->pxNext->xItemValue <= xValueOfInsertion; pxIterator = pxIterator->pxNext) {
            // Empty loop to find the right position.
        }
    }

    pxNewListItem->pxNext = pxIterator->pxNext;
    pxNewListItem->pxNext->pxPrevious = pxNewListItem;
    pxNewListItem->pxPrevious = pxIterator;
    pxIterator->pxNext = pxNewListItem;

    pxNewListItem->pvContainer = (void*)pxList;

    (pxList->uxNumberOfItems)++;
}

UBaseType_t uxListRemove(ListItem_t* const pxItemToRemove)
{
    List_t* const pxList = (List_t*)pxItemToRemove->pvContainer;

    pxItemToRemove->pxNext->pxPrevious = pxItemToRemove->pxPrevious;
    pxItemToRemove->pxPrevious->pxNext = pxItemToRemove->pxNext;

    if (pxItemToRemove == pxList->pxIndex) {
        pxList->pxIndex = pxItemToRemove->pxPrevious;
    }

    pxItemToRemove->pvContainer = NULL;
    (pxList->uxNumberOfItems)--;

    return pxList->uxNumberOfItems;
}