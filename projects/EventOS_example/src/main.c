/*
 * main.c
 *
 *  Created on: 05/09/2014
 *      Author: edielson
 */
#include <stdlib.h>
#include <stdio.h>

#include "EventOS.h"
#include "event.h"
#include "AVLTree.h"

#include "Application.h"
#include "Log.h"

int main()
{
	Log_print(LOG_FACILITY_USER_LEVEL_MESSAGES, LOG_SEVERITY_INFORMATIONAL, (char*)pxEvent_getVersion());

	ttag_nodeptr ptagRoot = NULL;
	ttag_Event myInfo;

	myInfo.xHash = 10;
	myInfo.pcEventName = (portCHAR*) pvPortMalloc( 11 )
	strcpy(myInfo.pcEventName,"1 insert");
	ptagRoot = AVLTree_insertNode(ptagRoot, myInfo);

	myInfo.xHash = 20;
	myInfo.pcEventName = (portCHAR*) pvPortMalloc( 11 )
	strcpy(myInfo.pcEventName,"2 insert");
	ptagRoot = AVLTree_insertNode(ptagRoot, myInfo);

	myInfo.xHash = 30;
	myInfo.pcEventName = (portCHAR*) pvPortMalloc( 11 )
	strcpy(myInfo.pcEventName,"3 insert");
	ptagRoot = AVLTree_insertNode(ptagRoot, myInfo);

	myInfo.xHash = 40;
	myInfo.pcEventName = (portCHAR*) pvPortMalloc( 11 )
	strcpy(myInfo.pcEventName,"4 insert");
	ptagRoot = AVLTree_insertNode(ptagRoot, myInfo);

	myInfo.xHash = 50;
	myInfo.pcEventName = (portCHAR*) pvPortMalloc( 11 )
	strcpy(myInfo.pcEventName,"5 insert");
	ptagRoot = AVLTree_insertNode(ptagRoot, myInfo);

	myInfo.xHash = 25;
	myInfo.pcEventName = (portCHAR*) pvPortMalloc( 11 )
	strcpy(myInfo.pcEventName,"6 insert");
	ptagRoot = AVLTree_insertNode(ptagRoot, myInfo);

	AVLTree_printTree(ptagRoot);

	printf("\nTesting Delete Method...\n\n");

	myInfo.xHash = 30;
	strcpy(myInfo.pcEventName,"3 insert");
	ptagRoot = AVLTree_removeNode(ptagRoot, myInfo);

	myInfo.xHash = 40;
	strcpy(myInfo.pcEventName,"4 insert");
	ptagRoot = AVLTree_removeNode(ptagRoot, myInfo);

	AVLTree_printTree(ptagRoot);

	ptagRoot = AVLTree_clearTree(ptagRoot);

	if(ptagRoot == NULL) {
		printf("\n\nSucefully deallocated\n");
	}

	for(;;);

	/*Should never reach here*/
	return 0;
}
