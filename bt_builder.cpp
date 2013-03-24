// File Name : bt_builder.cpp
// Date : 03/06/2013
// Team : Amit & Mike


#include "bt_builder.h"


BtreeBuilder::~BtreeBuilder(){
	destroyBtree();
}


Status BtreeBuilder::insertBuilderKey(KeyId KeyValue) {
	Status ret;
	//Step 1 : Find the Child Pointer the KeyId needs to be inserted.
	const int type = root->get_type();
	KeyId * KeyValue_MoveUp= &KeyValue;
	if (type == LEAF) {
		// Traverse this node and insert your value
		ret = root->insertKey(KeyValue,0) ; // As defined in bt_node.cpp
		if (ret == LEAF_IS_FULL) {
			ret = splitNode(KeyValue,KeyValue_MoveUp,root,root->getPtr(MAX_NUM_KEYS-1),root->getPtr(MAX_NUM_KEYS));
			if (ret != OK)
				return FAIL;
		}	
		else if (ret != OK)
			return FAIL;
	}
	else {
		BtreeNode *temp = new BtreeIndex;
		//BtreeIndex *temp;  //NEED TO MAKE SURE IF THIS IS RIGHT DEFINITION
		root->searchKey(KeyValue,0,&temp);
		while(temp->get_type() != LEAF){
			temp->searchKey(KeyValue,0,&temp);
		}
		ret = temp->insertKey(KeyValue,0);
		if (ret == LEAF_IS_FULL) {
			ret = splitNode(KeyValue,KeyValue_MoveUp,temp,temp->getPtr(MAX_NUM_KEYS-1),temp->getPtr(MAX_NUM_KEYS));
			if (ret != OK)
				return FAIL;
		}	
		else if (ret != OK)
		return FAIL;
	}
	return OK; 
}

Status BtreeBuilder::deleteBuilderKey(KeyId) {
	return OK;
}

Status BtreeBuilder::splitNode(KeyId KeyValue, KeyId *& KeyValue_MoveUp, BtreeNode * FullNode_Ptr, BtreeNode *& lChild_Ptr, BtreeNode *& rChild_Ptr) {
	Status ret;
	int count=0; //TEST. WHAT IS ACTUALLY GOING TO BE ITS VALUE. Now sure what is 2nd arg in insertKey fun.
	if (FullNode_Ptr->get_type() == LEAF) {
		//BtreeNode *New_SplitNode;
		BtreeLeaf *New_SplitNode ;
		New_SplitNode->set_type(LEAF); //FIND OUT IF WE NEED IT TEST
		for (int i = MAX_NUM_KEYS/2 ; i < MAX_NUM_KEYS ; i++) {
			ret = New_SplitNode->insertKey(FullNode_Ptr->getKey(i),0);
			if(ret != OK)
				return FAIL;
			FullNode_Ptr->setKey(-1,i);
			FullNode_Ptr->set_keyCount(FullNode_Ptr->get_keyCount()-1);
		}
		ret = New_SplitNode->insertKey(KeyValue,0);
		if(ret != OK)
			return FAIL;
		if (FullNode_Ptr == root) {
			BtreeNode *New_IndexNode = new BtreeIndex; //TEST if this is correct.
			//BtreeIndex New_IndexNode;
			New_IndexNode->set_type(INDEX); //FIND OUT IF WE STILL NEED IT TEST
			ret = New_IndexNode->insertKey(New_SplitNode->getKey(0), count, FullNode_Ptr, New_SplitNode);
			if (ret == OK)
				root = New_IndexNode;
			else
				return FAIL;
		}
		else { // When it is not root, we need to insert entry for new leaf node in parent node.
			//ret = (FullNode_Ptr->get_parentPtr())->insertKey(New_IndexNode->getKey(0),count, FullNode_Ptr, New_IndexNode);
			ret = (FullNode_Ptr->get_parentPtr())->insertKey(New_SplitNode->getKey(0),count, FullNode_Ptr, New_SplitNode);
			if (ret != OK)
				return FAIL;
			else if (ret == INDEX_IS_FULL) {  // This is a recursive call to splitNode to handle this case.
				ret = splitNode(New_SplitNode->getKey(0),count,FullNode_Ptr->get_parentPtr(),FullNode_Ptr, New_SplitNode);
				//New_SplitNode above may need Change. TEST
				if (ret != OK)
					return FAIL;
			}
			else
				return FAIL;
		}
		return OK;
	}
	else if (FullNode_Ptr->get_type() == INDEX) {
		BtreeNode *New_IndexNode = new BtreeIndex;
		//BtreeIndex New_IndexNode;
		New_IndexNode->set_type(INDEX); //TEST IF WE REALLY NEED IT.
		BtreeNode* FullNode_Ptr_last = FullNode_Ptr->getPtr(MAX_NUM_KEYS-1); //DO WE REALLY NEED IT.
		int i;
		for (i = MAX_NUM_KEYS/2 ; i < MAX_NUM_KEYS ; i++) {
			ret = New_IndexNode->insertKey(FullNode_Ptr->getKey(i), count, FullNode_Ptr->getPtr(i), FullNode_Ptr->getPtr(i+1));
			if(ret != OK)
				return FAIL;
			FullNode_Ptr->setKey(-1,i);
			FullNode_Ptr->setPtr(NULL,i);
			//Since keys share children we cannot clear i+1 until the end.
			//FullNode_Ptr->setPtr(NULL,i+1);
			FullNode_Ptr->set_keyCount(FullNode_Ptr->get_keyCount()-1);
		}
		//we set the last child pointer to null
		FullNode_Ptr->setPtr(NULL,i);
		Status ret = New_IndexNode->insertKey(KeyValue,0, FullNode_Ptr_last, New_IndexNode);
		if (ret != OK)
			return FAIL;
		if (FullNode_Ptr == root) { //If Full Index Node was root, we create a new Index node and mark it as root after moving our first pointer fron New_IndexNode to it.
			BtreeNode *New_IndexNode_root = new BtreeIndex;
			//BtreeIndex New_IndexNode_root;
			ret = New_IndexNode_root->insertKey(New_IndexNode->getKey(0), count, New_IndexNode->getPtr(0),New_IndexNode->getPtr(1));
			if (ret != OK)
				return FAIL;
			if (ret == OK) {
				root = New_IndexNode_root;
				//Shifting New_IndexNode one position to Left after its first element is move to root.	
				for (int i = 0 ; i < MAX_NUM_KEYS ; i++) 
					 New_IndexNode->setKey(New_IndexNode->getKey(i+1),i);
				New_IndexNode->set_keyCount(New_IndexNode->get_keyCount()-1);
			}
		}
		else {
			//ret = (FullNode_ptr->get_parentPtr())->insertKey(NewSplitNode->getKey(0),int count, FullNode_ptr, New_SplitNode);
			ret = (FullNode_Ptr->get_parentPtr())->insertKey(New_IndexNode->getKey(0),count, FullNode_Ptr, New_IndexNode);
			if (ret != OK)
				return FAIL;
			else if (ret == INDEX_IS_FULL) {
				ret = splitNode(New_IndexNode->getKey(0), FullNode_Ptr->get_parentPtr(),FullNode_Ptr,New_IndexNode);
				if (ret != OK)
					return FAIL;		
			}
			else
				return FAIL;
		}
		return OK;
		//Create a New Root Node because of Index Node Split and Take the first element from New_IndexNode and move it to Parent Nod.
	}
}		


BtreeScan* BtreeBuilder::openScan(KeyId lo_key, KeyId hi_key) {

  // Cases:
  //      (1) lo_key = NULL, hi_key = NULL
  //              scan the whole index
  //      (2) lo_key = NULL, hi_key!= NULL
  //              range scan from min to the hi_key
  //      (3) lo_key!= NULL, hi_key = NULL
  //              range scan from the lo_key to max
  //      (4) lo_key!= NULL, hi_key!= NULL, lo_key = hi_key
  //              exact match ( might not unique)
  //      (5) lo_key!= NULL, hi_key!= NULL, lo_key < hi_key
  //              range scan from lo_key to hi_key

  BtreeScan *scan = new BtreeScan();

  BtreeNode *current = root;
  BtreeNode *tmp;

  int leaf_pos;

  if(lo_key == NULL){
    scan->set_pos(0);
    while(current->get_type() != LEAF){
      current->searchKey(-1, 0, &tmp);
      current = tmp;
    }
  } else {
    while(current->get_type() != LEAF){
      current->searchKey(lo_key, 0, &tmp);
      current = tmp;
    }
    //I will need to be able to search a leaf for a key
    current->searchKey(lo_key, &leaf_pos);
    scan->set_pos(leaf_pos);
  }

  scan->set_leaf(current);
  scan->set_endKey(hi_key);

  return scan;

}

Status BtreeBuilder::findStartPosition(BtreeScan* , KeyId) {

}

Status BtreeBuilder::destroyBtree () {
  delete root;
  return OK;
}




