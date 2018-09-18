#ifndef AVLTREE
#define AVLTREE

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*********************************************************

    public constants.

*********************************************************/


/*********************************************************

    public types.

*********************************************************/
typedef struct tag_Info {
	
	portUCHAR ucValue;
	portCHAR cMyString[10];
	
}ttag_info;


typedef struct tag_node
{
	ttag_info info;
	struct tag_node *ptagRight;
	struct tag_node *ptagLeft;
}ttag_node;

typedef ttag_node* ttag_nodeptr;

/*********************************************************

    public operations.

*********************************************************/
ttag_nodeptr 	AVLTree_insertNode(ttag_nodeptr ptagRoot, ttag_info tagNewInfo);
ttag_nodeptr 	AVLTree_getHandler(ttag_nodeptr ptagRoot, ttag_info tagSearchedInfo);
ttag_nodeptr 	AVLTree_removeSpecificNode(ttag_nodeptr ptagRoot, ttag_nodeptr ptagNodeHandler);
ttag_nodeptr 	AVLTree_removeNode(ttag_nodeptr ptagRoot, ttag_info tagInfo);
ttag_nodeptr	AVLTree_clearTree(ttag_nodeptr ptagRoot);
void			AVLTree_printTree(ttag_nodeptr ptagRoot);
void			AVLTree_printNode(ttag_nodeptr ptagRoot);

#endif
