/*
 * AVLTree.h
 *
 *  Created on: 4 de set de 2018
 *      Author: jpone
 */

#ifndef KERNEL_CORE_AVLTREE_H_
#define KERNEL_CORE_AVLTREE_H_


#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "portmacro.h"
#include "EventOS.h"
#include "list.h"

#include "Log.h"

/*********************************************************

    public constants.

*********************************************************/
#define WEAK __attribute__ ((weak))

#define BIGGER	1
#define SMALLER -1
#define EQUALS	0

/*********************************************************

    public types.

*********************************************************/
typedef struct tag_treeNode
{
	void* pvPayload;
	struct tag_treeNode *ptagRight;
	struct tag_treeNode *ptagLeft;
}ttag_treeNode;

typedef ttag_treeNode* ttag_treeNodePtr;

/*********************************************************

    public operations.

*********************************************************/
ttag_treeNodePtr	AVLTree_insertNode(ttag_treeNodePtr ptagRoot, void* pvPayload, ttag_treeNodePtr* pptagNodeHandler);
ttag_treeNodePtr 	AVLTree_getHandler(ttag_treeNodePtr ptagRoot, void* pvPayloadSearched);
ttag_treeNodePtr 	AVLTree_removeSpecificNode(ttag_treeNodePtr ptagRoot, ttag_treeNodePtr ptagNodeHandler);
ttag_treeNodePtr 	AVLTree_removeNode(ttag_treeNodePtr ptagRoot, void* pvPayload);
ttag_treeNodePtr	AVLTree_clearTree(ttag_treeNodePtr ptagRoot);
void				AVLTree_printTree(ttag_treeNodePtr ptagRoot);
WEAK void			AVLTree_printNode(ttag_treeNodePtr ptagRoot);

#endif /* KERNEL_CORE_AVLTREE_H_ */
