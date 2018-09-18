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
#include "event.h"
#include "list.h"

#include "Log.h"

/*********************************************************

    public constants.

*********************************************************/


/*********************************************************

    public types.

*********************************************************/
typedef struct tag_Event {

	xList pxSubscriberList;	/*< Event handler separated by event types. */
	portCHAR* pcEventName;
	portHASH_TYPE pxHash;

}ttag_Event;


typedef struct tag_node
{
	ttag_Event Event;
	struct tag_node *ptagRight;
	struct tag_node *ptagLeft;
}ttag_node;

typedef ttag_node* ttag_nodeptr;

/*********************************************************

    public operations.

*********************************************************/
ttag_nodeptr 	AVLTree_insertNode(ttag_nodeptr ptagRoot, ttag_Event tagNewInfo);
ttag_nodeptr 	AVLTree_getHandler(ttag_nodeptr ptagRoot, ttag_Event tagSearchedInfo);
ttag_nodeptr 	AVLTree_removeSpecificNode(ttag_nodeptr ptagRoot, ttag_nodeptr ptagNodeHandler);
ttag_nodeptr 	AVLTree_removeNode(ttag_nodeptr ptagRoot, ttag_Event tagInfo);
ttag_nodeptr	AVLTree_clearTree(ttag_nodeptr ptagRoot);
void			AVLTree_printTree(ttag_nodeptr ptagRoot);
void			AVLTree_printNode(ttag_nodeptr ptagRoot);

#endif /* KERNEL_CORE_AVLTREE_H_ */
