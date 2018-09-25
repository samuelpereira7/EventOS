
/*
    EventOS V1.0 - Copyright (C) 2014 Edielson Prevato Frigieri.

       This file is part of the EventOS distribution.

    EventTOS is free software; you can redistribute it and/or modify it under
    the terms of the MIT License (MIT).
    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.

 */

#include <stdlib.h>
#include "EventOS.h"
#include "list.h"

/*********************************************************
    Operations implementation
*********************************************************/

void vList_initialize( xList* pxList )
{
	if( pxList == NULL ) return;

	/* The list structure contains an item which is used to mark the
	end of the list - the sentinel.  To initialize the list, the list end
	is inserted as the only list entry. */
	pxList->pxIndex = ( xListNode* ) &( pxList->xListSentinel);

	/* The list end value is the highest possible value in the list to
	ensure it remains at the end of the list. */
	pxList->xListSentinel.xNodeValue = portMAX_BASETYPE;

	/* The list sentinel next and previous pointers point to itself so we know
	when the list is empty. */
	pxList->xListSentinel.pxNext = ( xListNode * ) &( pxList->xListSentinel );
	pxList->xListSentinel.pxPrevious = ( xListNode * ) &( pxList->xListSentinel );

	pxList->uxNumberOfNodes = 0;
}

void vList_deinitialize( xList* pxList )
{
	if( pxList == NULL ) return;

	while( pxList->uxNumberOfNodes > 0 )
	{
		vList_remove( pxList->xListSentinel.pxNext );
	}
}

void vList_initializeNode( xListNode* pxNode )
{
	/* Make sure the list node is not recorded as being on a list. */
	pxNode->pvContainer = NULL;
}

void vList_insertHead( xList* pxList, xListNode* pxNewListNode )
{
	volatile xListNode* pxIndex;

	/* Insert a new list node into xList, but rather than sort the list,
	makes the new list node the header node, but the last to be removed. */

	//Points to the sentinel
	pxIndex = pxList->pxIndex;

	pxNewListNode->pxNext = pxIndex->pxNext;
	pxIndex->pxNext->pxPrevious = ( volatile xListNode* ) pxNewListNode;
	pxIndex->pxNext = ( volatile xListNode* ) pxNewListNode;
	pxNewListNode->pxPrevious = pxList->pxIndex;

	/* Remember which list the node is in. */
	pxNewListNode->pvContainer = ( void* ) pxList;

	( pxList->uxNumberOfNodes )++;
}

/*-----------------------------------------------------------*/
void vList_insert( xList* pxList, xListNode *pxNewListNode )
{
	volatile xListNode* pxIterator;
	portBASE_TYPE xValueToInsert;

	/* Insert the new list node into the list, sorted in xNodeValue order. */
	xValueToInsert = pxNewListNode->xNodeValue;

	/* if the node item value is equal to portMAX_BASETYPE, the node goes to the end of the list, just after the sentinel. */
	if( xValueToInsert == portMAX_BASETYPE || listIS_EMPTY( pxList ) )
	{
		//point to the list tail, i.e., the last node
		pxIterator = pxList->pxIndex;
	}
	else
	{
		//TODO change the iteration to run from tail to head
		for( pxIterator = ( xListNode* ) &( pxList->xListSentinel); pxIterator->pxPrevious->xNodeValue > xValueToInsert; pxIterator = pxIterator->pxPrevious )
		{
			/* There is nothing to do here, we are just iterating to the
			wanted insertion position. */
		}
	}

	pxNewListNode->pxPrevious = pxIterator->pxPrevious;
	pxIterator->pxPrevious->pxNext = ( volatile xListNode* ) pxNewListNode;
	pxIterator->pxPrevious = ( volatile xListNode* ) pxNewListNode;
	pxNewListNode->pxNext = pxIterator;

	/* Remember which list the item is in.  This allows fast removal of the
	item later. */
	pxNewListNode->pvContainer = ( void * ) pxList;

	( pxList->uxNumberOfNodes )++;
}
/*-----------------------------------------------------------*/

void vList_remove( xListNode* pxNodeToRemove )
{
	xList* pxList;

	pxNodeToRemove->pxPrevious->pxNext = pxNodeToRemove->pxNext;
	pxNodeToRemove->pxNext->pxPrevious = pxNodeToRemove->pxPrevious;

	/* The list item knows which list it is in.  Obtain the list from the list
	item. */
	pxList = ( xList*) pxNodeToRemove->pvContainer;
	( pxList->uxNumberOfNodes )--;

	pxNodeToRemove->pvContainer = NULL;
}
/*-----------------------------------------------------------*/

