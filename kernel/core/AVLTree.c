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
portCHAR			AVLTree_nodeCpm(ttag_info tagFirst, ttag_info tagSecond);
portCHAR			AVLTree_height(ttag_nodeptr ptagRoot);
ttag_nodeptr 	AVLTree_nodeAlloc(ttag_info tagNewInfo);
ttag_nodeptr 	AVLTree_leftRotate(ttag_nodeptr ptagRoot);
ttag_nodeptr 	AVLTree_rightRotate(ttag_nodeptr ptagRoot);
ttag_nodeptr 	AVLTree_rightLeftRotate(ttag_nodeptr ptagRoot);
ttag_nodeptr 	AVLTree_leftRightRotate(ttag_nodeptr ptagRoot);
ttag_nodeptr 	AVLTree_minValueNode(ttag_nodeptr ptagRoot);


/*********************************************************

    Operations implementation

*********************************************************/

/**
	This function insert the tagNewInfo to ptagRoot AVLTree

    @param	ptagRoot: a pointer to root os AVLTree
    		tagNewInfo: information to be inserted in AVLTree
    @return ttag_nodeptr: pointer to root after insert information.
    @author jponeticarvalho
    @date   21/08/2018
*/
ttag_nodeptr 	AVLTree_insertNode(ttag_nodeptr ptagRoot, ttag_info tagNewInfo){
	if(ptagRoot == NULL)
		return AVLTree_nodeAlloc(tagNewInfo);
	
	if(AVLTree_nodeCpm(tagNewInfo, ptagRoot->info) < 0) {
		ptagRoot->ptagLeft = AVLTree_insertNode(ptagRoot->ptagLeft, tagNewInfo);
	}
	else {
		ptagRoot->ptagRight = AVLTree_insertNode(ptagRoot->ptagRight, tagNewInfo);
	}
	
	portCHAR cBalanced = AVLTree_height(ptagRoot->ptagLeft) - AVLTree_height(ptagRoot->ptagRight);
	
	if (cBalanced > 1 && AVLTree_nodeCpm(tagNewInfo, ptagRoot->ptagLeft->info) < 0)
        return AVLTree_rightRotate(ptagRoot);
 
    if (cBalanced < -1 && AVLTree_nodeCpm(tagNewInfo, ptagRoot->ptagRight->info) > 0)
        return AVLTree_leftRotate(ptagRoot);
 
    if (cBalanced > 1 && AVLTree_nodeCpm(tagNewInfo, ptagRoot->ptagLeft->info) > 0)
    {
        ptagRoot->ptagLeft =  AVLTree_leftRotate(ptagRoot->ptagLeft);
        return AVLTree_rightRotate(ptagRoot);
    }
 
    if (cBalanced < -1 && AVLTree_nodeCpm(tagNewInfo, ptagRoot->ptagRight->info) < 0)
    {
        ptagRoot->ptagRight = AVLTree_rightRotate(ptagRoot->ptagRight);
        return AVLTree_leftRotate(ptagRoot);
    }
    
    return ptagRoot;
}

/**
	This function search in AVLTree the node that has tagSearchedInfo

    @param	ptagRoot: a pointer to root os AVLTree
    		tagInfo: a information to be searched in AVLTree
    @return ttag_nodeptr: pointer to node - Found.
						  NULL			  - Not Found
    @author jponeticarvalho
    @date   21/08/2018
*/
ttag_nodeptr AVLTree_getHandler(ttag_nodeptr ptagRoot, ttag_info tagSearchedInfo) {
	if(ptagRoot == NULL)
		return NULL;
	else {
		portCHAR cCompareValue = AVLTree_nodeCpm(ptagRoot->info, tagSearchedInfo);
		if(cCompareValue == 0) {
			return ptagRoot;
		}
		else if(cCompareValue < 0) {
			return AVLTree_getHandler(ptagRoot->ptagRight, tagSearchedInfo);
		}
		else {
			return AVLTree_getHandler(ptagRoot->ptagLeft, tagSearchedInfo);
		}
		
	}
}

/**
	This function remove the ptagNodeHandler node of AVLTree

    @param	ptagRoot: a pointer to root os AVLTree
    		ptagHandler: a node to be removed from AVLTree
    @return ttag_nodeptr: pointer to root after remove node.
    @author jponeticarvalho
    @date   25/08/2018
*/
ttag_nodeptr AVLTree_removeSpecificNode(ttag_nodeptr ptagRoot, ttag_nodeptr ptagNodeHandler) {
	
	if(ptagRoot == NULL)
		return NULL;
	
	if(ptagRoot == ptagNodeHandler) {
		if( (ptagRoot->ptagLeft == NULL) || (ptagRoot->ptagRight == NULL) ) {
			ttag_nodeptr ptagTemp = ptagRoot->ptagLeft ? ptagRoot->ptagLeft:ptagRoot->ptagRight;
		
			if (ptagTemp == NULL){
				ptagTemp = ptagRoot;
				ptagRoot = NULL;
			}
			else
				*ptagRoot = *ptagTemp;
			free(ptagTemp);
		}
		else{
			ttag_nodeptr ptagTemp = AVLTree_minValueNode(ptagRoot->ptagRight);
		
			ptagRoot->info.ucValue = ptagTemp->info.ucValue;
		
			ptagRoot->ptagRight = AVLTree_removeSpecificNode(ptagRoot->ptagRight, ptagTemp);
		}
	}
	else {
		if(AVLTree_nodeCpm(ptagRoot->info, ptagNodeHandler->info) > 0) {
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
    		tagInfo: a information to be removed from AVLTree
    @return ttag_nodeptr: pointer to root after remove node.
    @author jponeticarvalho
    @date   21/08/2018
*/
ttag_nodeptr AVLTree_removeNode(ttag_nodeptr ptagRoot, ttag_info tagInfo) {
	if(ptagRoot == NULL) {
		return ptagRoot;
	}
	
	ttag_nodeptr ptagHandler = AVLTree_getHandler(ptagRoot, tagInfo);
	
	if(ptagHandler == NULL)
		return NULL;
	else
		return AVLTree_removeSpecificNode(ptagRoot, ptagHandler);
}

/**
	This function deallocate all node of AVLTree

    @param	ptagRoot: a pointer to root os AVLTree
    @return ttag_nodeptr: pointer to root after deallocate nodes.
    @author jponeticarvalho
    @date   21/08/2018
*/
ttag_nodeptr AVLTree_clearTree(ttag_nodeptr ptagRoot) {
	if(ptagRoot != NULL) {
		ptagRoot->ptagRight = AVLTree_clearTree(ptagRoot->ptagRight);
		ptagRoot->ptagLeft = AVLTree_clearTree(ptagRoot->ptagLeft);
		free(ptagRoot);
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
void AVLTree_printTree(ttag_nodeptr ptagRoot) {
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
void AVLTree_printNode(ttag_nodeptr ptagRoot) {
	if(ptagRoot != NULL) {
		uportCHAR buffer[50];
		sprintf(buffer,"Value: %d\nMyString: '%s'\n", ptagRoot->info.ucValue, ptagRoot->info.cMyString);
		Log_print(LOG_FACILITY_USER_LEVEL_MESSAGES, LOG_SEVERITY_INFORMATIONAL, (char*)buffer);
	}
}

/**
	This function compares who is larger, the first term, or the second 

    @param	tagFirst: first term to be compared
			tagSecond: second term to be compared
    @return portCHAR: 	 1 to first larger then second
    				 0 to first equals then second
					-1 to first smaller then second
    @author jponeticarvalho
    @date   21/08/2018
*/
portCHAR AVLTree_nodeCpm(ttag_info tagFirst, ttag_info tagSecond) {
	if(tagFirst.ucValue > tagSecond.ucValue) {
		return 1;
	}
	else if(tagFirst.ucValue < tagSecond.ucValue) {
		return -1;
	}
	else {
		return 0;
	}
}

/**
	This function calculate the AVLtree Node height

    @param	ptagRoot: a pointer to node to discover height 
    @return portCHAR: height of ptagRoot
    @author jponeticarvalho
    @date   21/08/2018
*/
portCHAR AVLTree_height(ttag_nodeptr ptagRoot) {
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

    @param	tagNewInfo: info to set in new node
    @return ttag_nodeptr: a pointer to node allocated
    @author jponeticarvalho
    @date   21/08/2018
*/
ttag_nodeptr AVLTree_nodeAlloc(ttag_info tagNewInfo) {
	ttag_nodeptr ptagNewNode = (ttag_nodeptr)malloc(sizeof(ttag_node));
	
	ptagNewNode->info = tagNewInfo;
	ptagNewNode->ptagRight = NULL;
	ptagNewNode->ptagLeft = NULL;
	
	return ptagNewNode;
}

/**
	This function rotate to left subtree rooted with ptagRoot

    @param	ptagRoot: a root of tree
    @return ttag_nodeptr: a pointer to new ptagRoot after left rotate
    @author jponeticarvalho
    @date   21/08/2018
*/
ttag_nodeptr AVLTree_leftRotate(ttag_nodeptr ptagRoot) {
	ttag_nodeptr ptagNewRoot = ptagRoot->ptagRight;
	ttag_nodeptr ptagNewRightSubtree = ptagNewRoot->ptagLeft;
	
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
ttag_nodeptr AVLTree_rightRotate(ttag_nodeptr ptagRoot) {
	ttag_nodeptr ptagNewRoot = ptagRoot->ptagLeft;
	ttag_nodeptr ptagNewLeftSubtree = ptagNewRoot->ptagRight;
	
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
ttag_nodeptr AVLTree_rightLeftRotate(ttag_nodeptr ptagRoot) {
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
ttag_nodeptr AVLTree_leftRightRotate(ttag_nodeptr ptagRoot) {
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
ttag_nodeptr AVLTree_minValueNode(ttag_nodeptr ptagRoot) {
	ttag_nodeptr ptagMinimun = ptagRoot;
	
	while(ptagMinimun->ptagLeft != NULL) {
		ptagMinimun = ptagMinimun->ptagLeft;
	}
	
	return ptagMinimun;
}
