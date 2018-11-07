/*
 * avltree.c
 *
 *  Created on: 4 de set de 2018
 *      Author: jpone
 */

#include "AVLTree.h"
/*********************************************************

    private constants.

*********************************************************/


/*********************************************************

    private types.

*********************************************************/


/*********************************************************

    private attributes.

*********************************************************/


/*********************************************************

    private operations.

*********************************************************/
WEAK portCHAR 		AVLTree_nodeCmp(void* pvFirst, void* pvSecond);
portCHAR			AVLTree_height(ttag_treeNodePtr ptagRoot);
ttag_treeNodePtr	AVLTree_nodeAlloc(void* pvPayload, ttag_treeNodePtr* pptagNodeHandler);
ttag_treeNodePtr	AVLTree_nodeDealloc(ttag_treeNodePtr ptagNode);
ttag_treeNodePtr 	AVLTree_leftRotate(ttag_treeNodePtr ptagRoot);
ttag_treeNodePtr 	AVLTree_rightRotate(ttag_treeNodePtr ptagRoot);
ttag_treeNodePtr 	AVLTree_rightLeftRotate(ttag_treeNodePtr ptagRoot);
ttag_treeNodePtr 	AVLTree_leftRightRotate(ttag_treeNodePtr ptagRoot);
ttag_treeNodePtr 	AVLTree_minValueNode(ttag_treeNodePtr ptagRoot);


/*********************************************************

    Operations implementation

*********************************************************/

/**
	This function insert the tagNewInfo to ptagRoot AVLTree

    @param	ptagRoot: a pointer to root of AVLTree
    		pvPayload: information to be inserted in AVLTree
    		pptagNodeHandler: a handler to be setted a node allocated
    @return ttag_treeNode: pointer to root after insert information.
    @author jponeticarvalho
    @date   21/08/2018
*/
ttag_treeNodePtr AVLTree_insertNode(ttag_treeNodePtr ptagRoot, void* pvPayload, ttag_treeNodePtr* pptagNodeHandler){
	if(ptagRoot == NULL)
		return AVLTree_nodeAlloc(pvPayload, pptagNodeHandler);

	if(AVLTree_nodeCmp(pvPayload, ptagRoot->pvPayload) == SMALLER) {
		ptagRoot->ptagLeft = AVLTree_insertNode(ptagRoot->ptagLeft, pvPayload, pptagNodeHandler);
	}
	else {
		ptagRoot->ptagRight = AVLTree_insertNode(ptagRoot->ptagRight, pvPayload, pptagNodeHandler);
	}

	portCHAR cBalanced = AVLTree_height(ptagRoot->ptagLeft);
	cBalanced -= AVLTree_height(ptagRoot->ptagRight);

//	portCHAR cCompareValue = AVLTree_nodeCmp(pvPayload, ptagRoot->ptagLeft->pvPayload);
	if (cBalanced > 1 && AVLTree_nodeCmp(pvPayload, ptagRoot->ptagLeft->pvPayload) < 0)
        return AVLTree_rightRotate(ptagRoot);

//	cCompareValue = AVLTree_nodeCmp(pvPayload, ptagRoot->ptagRight->pvPayload);
    if (cBalanced < -1 && AVLTree_nodeCmp(pvPayload, ptagRoot->ptagRight->pvPayload) > 0)
        return AVLTree_leftRotate(ptagRoot);

//    cCompareValue = AVLTree_nodeCmp(pvPayload, ptagRoot->ptagLeft->pvPayload);
    if (cBalanced > 1 && AVLTree_nodeCmp(pvPayload, ptagRoot->ptagLeft->pvPayload) > 0)
    {
        ptagRoot->ptagLeft =  AVLTree_leftRotate(ptagRoot->ptagLeft);
        return AVLTree_rightRotate(ptagRoot);
    }

//    cCompareValue = AVLTree_nodeCmp(pvPayload, ptagRoot->ptagRight->pvPayload);
    if (cBalanced < -1 && AVLTree_nodeCmp(pvPayload, ptagRoot->ptagRight->pvPayload) < 0)
    {
        ptagRoot->ptagRight = AVLTree_rightRotate(ptagRoot->ptagRight);
        return AVLTree_leftRotate(ptagRoot);
    }

    return ptagRoot;
}

/**
	This function search in AVLTree the node that has tagSearchedInfo

    @param	ptagRoot: a pointer to root os AVLTree
    		pvPayloadSearched: a information to be searched in AVLTree
    @return ttag_treeNode: pointer to node - Found.
						  NULL			  - Not Found
    @author jponeticarvalho
    @date   21/08/2018
*/
ttag_treeNodePtr AVLTree_getHandler(ttag_treeNodePtr ptagRoot, void* pvPayloadSearched) {
	if(ptagRoot == NULL)
		return NULL;
	else {
		portCHAR cCompareValue = AVLTree_nodeCmp(pvPayloadSearched, ptagRoot->pvPayload);
		if(cCompareValue == EQUALS) {
			return ptagRoot;
		}
		else if(cCompareValue == EQUALS || cCompareValue == BIGGER) {
			return AVLTree_getHandler(ptagRoot->ptagRight, pvPayloadSearched);
		}
		else {
			return AVLTree_getHandler(ptagRoot->ptagLeft, pvPayloadSearched);
		}

	}
}

/**
	This function remove the ptagNodeHandler node of AVLTree

    @param	ptagRoot: a pointer to root os AVLTree
    		ptagNodeHandler: a node to be removed from AVLTree
    @return ttag_treeNodePtr: pointer to root after remove node.
    @author jponeticarvalho
    @date   25/08/2018
*/
ttag_treeNodePtr AVLTree_removeSpecificNode(ttag_treeNodePtr ptagRoot, ttag_treeNodePtr* ptagNodeHandler) {

	if(ptagRoot == NULL)
		return NULL;

	if(ptagRoot == *ptagNodeHandler) {
		if( (ptagRoot->ptagLeft == NULL) || (ptagRoot->ptagRight == NULL) ) {
			ttag_treeNodePtr ptagTemp = ptagRoot->ptagLeft ? ptagRoot->ptagLeft:ptagRoot->ptagRight;

			if (ptagTemp == NULL){
				ptagTemp = ptagRoot;
				ptagRoot = NULL;
			}
			else
				*ptagRoot = *ptagTemp;
			(*ptagNodeHandler) = NULL;
			AVLTree_nodeDealloc(ptagTemp);
		}
		else{
			ttag_treeNodePtr ptagTemp = AVLTree_minValueNode(ptagRoot->ptagRight);

			ptagRoot->pvPayload = ptagTemp->pvPayload;

			ptagRoot->ptagRight = AVLTree_removeSpecificNode(ptagRoot->ptagRight, ptagTemp);
		}
	}
	else {
		portCHAR cCompareValue = AVLTree_nodeCmp(ptagRoot->pvPayload, (*ptagNodeHandler)->pvPayload);
		if(cCompareValue > 0) {
			ptagRoot->ptagLeft = AVLTree_removeSpecificNode(ptagRoot->ptagLeft, ptagNodeHandler);
		}
		else {
			ptagRoot->ptagRight = AVLTree_removeSpecificNode(ptagRoot->ptagRight, ptagNodeHandler);
		}
	}

	if (ptagRoot == NULL)
		return ptagRoot;

	portCHAR cBalanced = AVLTree_height(ptagRoot->ptagLeft) - AVLTree_height(ptagRoot->ptagRight);
	portCHAR cSubBalanced = 0;

	if(ptagRoot->ptagLeft == NULL)
		cSubBalanced = 0;
	else
		cSubBalanced = AVLTree_height(ptagRoot->ptagLeft->ptagLeft) - AVLTree_height(ptagRoot->ptagLeft->ptagRight);
	if(cBalanced > 1 && cSubBalanced >= 0) {
		return AVLTree_rightRotate(ptagRoot);
	}

	if(cBalanced > 1 && cSubBalanced < 0) {
		ptagRoot->ptagLeft = AVLTree_leftRotate(ptagRoot->ptagLeft);
		return AVLTree_rightRotate(ptagRoot);
	}

	if(ptagRoot->ptagRight == NULL)
		cSubBalanced = 0;
	else
		cSubBalanced = AVLTree_height(ptagRoot->ptagRight->ptagLeft) - AVLTree_height(ptagRoot->ptagRight->ptagRight);
    if(cBalanced < -1 && cSubBalanced <= 0) {
		return AVLTree_leftRotate(ptagRoot);
	}

	if(cBalanced < -1 && cSubBalanced > 0) {
		ptagRoot->ptagRight = AVLTree_rightRotate(ptagRoot);
		return AVLTree_leftRotate(ptagRoot);
	}

    return ptagRoot;
}

/**
	This function remove the node that contains tagInfo of AVLTree

    @param	ptagRoot: a pointer to root os AVLTree
    		pvPayload: a information to be removed from AVLTree
    @return ttag_treeNodePtr: pointer to root after remove node.
    @author jponeticarvalho
    @date   21/08/2018
*/
ttag_treeNodePtr AVLTree_removeNode(ttag_treeNodePtr ptagRoot, void* pvPayload) {
	if(ptagRoot == NULL) {
		return ptagRoot;
	}

	ttag_treeNodePtr ptagHandler = AVLTree_getHandler(ptagRoot, pvPayload);

	if(ptagHandler == NULL)
		return NULL;
	else
		return AVLTree_removeSpecificNode(ptagRoot, ptagHandler);
}

/**
	This function deallocate all node of AVLTree

    @param	ptagRoot: a pointer to root os AVLTree
    @return ttag_treeNodePtr: pointer to root after deallocate nodes.
    @author jponeticarvalho
    @date   21/08/2018
*/
ttag_treeNodePtr AVLTree_clearTree(ttag_treeNodePtr ptagRoot) {
	if(ptagRoot != NULL) {
		ptagRoot->ptagRight = AVLTree_clearTree(ptagRoot->ptagRight);
		ptagRoot->ptagLeft = AVLTree_clearTree(ptagRoot->ptagLeft);
		AVLTree_nodeDealloc(ptagRoot);
	}
	ptagRoot = NULL;
	return ptagRoot;
}

/**
	This function print all node of AVLTree

    @param	ptagRoot: a pointer to root os AVLTree
    @return ttag_nodeptr: pointer to root after deallocate nodes.
    @author jponeticarvalho
    @date   21/08/2018
*/
void AVLTree_printTree(ttag_treeNodePtr ptagRoot) {
	if(ptagRoot != NULL) {
		AVLTree_printNode(ptagRoot);
		AVLTree_printTree(ptagRoot->ptagLeft);
		AVLTree_printTree(ptagRoot->ptagRight);
	}
}

/**
	This function print all node of AVLTree

    @param	ptagRoot: a pointer to root os AVLTree
    @return void
    @author jponeticarvalho
    @date   21/08/2018
*/
void AVLTree_printNode(ttag_treeNodePtr ptagRoot) {
	if(ptagRoot != NULL) {
		char buffer[50];
		sprintf(buffer,"Value: %d\nMyString: '%p'\n", ptagRoot->pvPayload);
		Log_print(LOG_FACILITY_USER_LEVEL_MESSAGES, LOG_SEVERITY_INFORMATIONAL, (char*)buffer);
	}
}

/**
	This function compares who is larger, the first term, or the second

    @param	pvFirst: first term to be compared
			pvSecond: second term to be compared
    @return char: 	BIGGER  to first larger then second
    				EQUALS  to first equals then second
					SMALLER to first smaller then second
    @author jponeticarvalho
    @date   21/08/2018
*/
portCHAR AVLTree_nodeCmp(void* pvFirst, void* pvSecond) {
	if(pvFirst > pvSecond) {
		return BIGGER;
	}
	else if(pvFirst < pvSecond) {
		return SMALLER;
	}
	else {
		return EQUALS;
	}
}

/**
	This function calculate the AVLtree Node height

    @param	ptagRoot: a pointer to node to discover height
    @return char: height of ptagRoot
    @author jponeticarvalho
    @date   21/08/2018
*/
portCHAR AVLTree_height(ttag_treeNodePtr ptagRoot) {
	if(ptagRoot == NULL)
		return -1;
	portCHAR leftHeight = AVLTree_height(ptagRoot->ptagLeft);
	portCHAR rightHeight = AVLTree_height(ptagRoot->ptagRight);
	if(leftHeight > rightHeight) {
		return leftHeight + 1;
	}
	else {
		return rightHeight + 1;
	}
}

/**
	Its a helper function that allocates a new node with the given information
	and a NULL ptagLeft and ptagRight pointers.

    @param	pvPayload: info to set in new node
    		pptagNodeHandler: a handler to be setted a node allocated
    @return ttag_nodeptr: a pointer to node allocated
    @author jponeticarvalho
    @date   21/08/2018
*/
ttag_treeNodePtr AVLTree_nodeAlloc(void* pvPayload, ttag_treeNodePtr* pptagNodeHandler) {
	ttag_treeNodePtr ptagNewNode = (ttag_treeNodePtr)pvPortMalloc(sizeof(ttag_treeNode));
	(*pptagNodeHandler) = ptagNewNode;

	ptagNewNode->pvPayload 	= pvPayload;
	ptagNewNode->ptagRight 	= NULL;
	ptagNewNode->ptagLeft 	= NULL;

	return ptagNewNode;
}

/**
	Its a helper function that deallocates a node.

    @param	ptagNode: node to be deallocated
    @return ttag_treeNode: a pointer to node deallocated
    			NULL - deallocated suceffuly
    @author jponeticarvalho
    @date   21/08/2018
*/
ttag_treeNodePtr AVLTree_nodeDealloc(ttag_treeNodePtr ptagNode) {

	if(ptagNode == NULL) return NULL;

	vPortFree(ptagNode);
	ptagNode = NULL;

	return ptagNode;
}

/**
	This function rotate to left subtree rooted with ptagRoot

    @param	ptagRoot: a root of tree
    @return ttag_nodeptr: a pointer to new ptagRoot after left rotate
    @author jponeticarvalho
    @date   21/08/2018
*/
ttag_treeNodePtr AVLTree_leftRotate(ttag_treeNodePtr ptagRoot) {
	ttag_treeNodePtr ptagNewRoot = ptagRoot->ptagRight;
	ttag_treeNodePtr ptagNewRightSubtree = ptagNewRoot->ptagLeft;

	ptagNewRoot->ptagLeft = ptagRoot;
	ptagRoot->ptagRight = ptagNewRightSubtree;

	return ptagNewRoot;
}


/**
	This function rotate to right subtree rooted with ptagRoot

    @param	ptagRoot: a root of tree
    @return ttag_nodeptr: a pointer to new ptagRoot after right rotate
    @author jponeticarvalho
    @date   21/08/2018
*/
ttag_treeNodePtr AVLTree_rightRotate(ttag_treeNodePtr ptagRoot) {
	ttag_treeNodePtr ptagNewRoot = ptagRoot->ptagLeft;
	ttag_treeNodePtr ptagNewLeftSubtree = ptagNewRoot->ptagRight;

	ptagNewRoot->ptagRight = ptagRoot;
	ptagRoot->ptagLeft = ptagNewLeftSubtree;

	return ptagNewRoot;
}

/**
	This function rotate to right and left subtree rooted with ptagRoot

    @param	ptagRoot: a root of tree
    @return ttag_nodeptr: a pointer to new ptagRoot after right and left rotate
    @author jponeticarvalho
    @date   21/08/2018
*/
ttag_treeNodePtr AVLTree_rightLeftRotate(ttag_treeNodePtr ptagRoot) {
	ptagRoot->ptagRight = AVLTree_rightRotate(ptagRoot->ptagRight);
	return AVLTree_leftRotate(ptagRoot);
}

/**
	This function rotate to left and right subtree rooted with ptagRoot

    @param	ptagRoot: a root of tree
    @return ttag_nodeptr: a pointer to new ptagRoot after left and right rotate
    @author jponeticarvalho
    @date   21/08/2018
*/
ttag_treeNodePtr AVLTree_leftRightRotate(ttag_treeNodePtr ptagRoot) {
	ptagRoot->ptagLeft = AVLTree_leftRotate(ptagRoot->ptagLeft);
	return AVLTree_rightRotate(ptagRoot);
}

/**
	This function search a minimun value node in the AVLTree

    @param	ptagRoot: a root of subtree
    @return ttag_nodeptr: a pointer to a node that have a minimun value.
    @author jponeticarvalho
    @date   25/08/2018
*/
ttag_treeNodePtr AVLTree_minValueNode(ttag_treeNodePtr ptagRoot) {
	ttag_treeNodePtr ptagMinimun = ptagRoot;

	while(ptagMinimun->ptagLeft != NULL) {
		ptagMinimun = ptagMinimun->ptagLeft;
	}

	return ptagMinimun;
}

