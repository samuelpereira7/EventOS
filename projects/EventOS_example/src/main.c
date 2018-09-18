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
	ttag_info myInfo;

	myInfo.ucValue = 10;
	strcpy(myInfo.cMyString,"1 insert");
	ptagRoot = AVLTree_insertNode(ptagRoot, myInfo);

	myInfo.ucValue = 20;
	strcpy(myInfo.cMyString,"2 insert");
	ptagRoot = AVLTree_insertNode(ptagRoot, myInfo);

	myInfo.ucValue = 30;
	strcpy(myInfo.cMyString,"3 insert");
	ptagRoot = AVLTree_insertNode(ptagRoot, myInfo);

	myInfo.ucValue = 40;
	strcpy(myInfo.cMyString,"4 insert");
	ptagRoot = AVLTree_insertNode(ptagRoot, myInfo);

	myInfo.ucValue = 50;
	strcpy(myInfo.cMyString,"5 insert");
	ptagRoot = AVLTree_insertNode(ptagRoot, myInfo);

	myInfo.ucValue = 25;
	strcpy(myInfo.cMyString,"6 insert");
	ptagRoot = AVLTree_insertNode(ptagRoot, myInfo);

	AVLTree_printTree(ptagRoot);

	printf("\nTesting Delete Method...\n\n");

	myInfo.ucValue = 30;
	strcpy(myInfo.cMyString,"3 insert");
	ptagRoot = AVLTree_removeNode(ptagRoot, myInfo);

	myInfo.ucValue = 40;
	strcpy(myInfo.cMyString,"4 insert");
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
