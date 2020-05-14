#include<stdio.h>
#include<stdlib.h>
#include<malloc.h> 

/*
 * Tugas GSLC 6 - Datstruk
 * Binary char tree
 * Matthew Christopher Albert - 2301848981 - LD01
 */

struct tnode{
	char *value;
	struct tnode *left, *right;
}*temp = NULL;

struct tnode_info{
	struct tnode *parent, *own;
	int height;
};

int compareStrings(char *s1, char *s2) {
    int i;
    for (i = 0; s1[i] == s2[i]; i++) {
        if (s1[i] == '\0')
            return 0; // s1 == s2
    }
    if ((unsigned char)s1[i] < (unsigned char)s2[i]){
        return -1; // s1 < s2
	}else{
        return 1; // s1 > s2
	}
}

struct tnode *create_new_node(char value[]){
	struct tnode *node = (struct tnode*)malloc(sizeof(struct tnode));
	node->value = value;
	node->left = NULL;
	node->right = NULL;
	return node;
}

void append(struct tnode **head, char value[]){
	struct tnode *node = create_new_node(value);
	if( *head == NULL ){
		*head = node;
	}else{
		temp = *head;
		int placed = 0;
		while( placed == 0 ){
			char *cval = temp->value;
			if( compareStrings(cval,value) == 1 ){
				//go left
				if( temp->left == NULL ){
					temp->left = node;
					placed = 1;
				}else temp = temp->left;
			}else{
				//go right
				if( temp->right == NULL ){
					temp->right = node;
					placed = 1;
				}else temp = temp->right;
			}
		}
	}
	printf("Insert %s\n",value);
}

struct tnode_info *find(struct tnode **head, char value[]){
	struct tnode_info *node = NULL;
	struct tnode *parent = NULL;
	int height = 0;
	temp = *head;
	while( temp != NULL ){
		if( temp->value == value ){
			node = (struct tnode_info*)malloc(sizeof(struct tnode_info));
			node->own = temp;
			node->parent = parent;
			node->height = height;
			return node;
		}else{
			char *cval = temp->value;
			parent = temp;
			if( compareStrings(cval,value) == 1 ){
				//go left
				temp = temp->left;
			}else{
				//go right
				temp = temp->right;
			}
			height++;
		}
	}
	return NULL;
}

void print_node_info(struct tnode **head, char value[]){
	struct tnode_info *node = find(&*head, value);
	printf("Node info %s\n",node->own->value);
	printf("Height: %i\n",node->height);
	printf("Parent: %s\n",node->parent != NULL? node->parent->value : "NULL");
	printf("Left: %s\n",node->own->left != NULL? node->own->left->value : "NULL");
	printf("Right: %s\n",node->own->right != NULL? node->own->right->value : "NULL");
}

struct tnode* find_minimum(struct tnode *root)
{
    if(root == NULL)
        return NULL;
    else if(root->left != NULL)
        return find_minimum(root->left);
    return root;
}

struct tnode* delete_node(struct tnode *root, char x[])
{
    //searching for the item to be deleted
    if(root==NULL){return NULL;}
	int cres = compareStrings(root->value, x);
    if (cres == 1){
        root->left = delete_node(root->left, x);
	}
    else if(cres == -1){
        root->right = delete_node(root->right, x);
	}
    else
    {
        //No Children
        if(root->left==NULL && root->right==NULL)
        {
            free(root);
            return NULL;
        }

        //One Child
        else if(root->left==NULL || root->right==NULL)
        {
            if(root->left==NULL)
                temp = root->right;
            else
                temp = root->left;
            free(root);
            return temp;
        }

        //Two Children
        else
        {
            temp = find_minimum(root->right);
            root->value = temp->value;
            root->right = delete_node(root->right, temp->value);
        }
    }
    return root;
}

//delete wrapper
void delete_n(struct tnode **head, char x[]){
	*head = delete_node(*head, x);
    printf("deleted %s\n",x); 
}


//binary print using inorder traversal geekforgeeks method
void print2DUtil(struct tnode *root, int space){ 
    if (root == NULL) 
        return; 
    space += 10; 
    print2DUtil(root->right, space); 
  
    // Print current node after space 
    printf("\n"); 
    for (int i = 10; i < space; i++)
        printf(" "); 
    printf("%s\n", root->value); 
  
    // Process left child 
    print2DUtil(root->left, space); 
} 
  
//print wrapper
void print2D(struct tnode *root) { 
   print2DUtil(root, 0); 
} 

int main(){
	struct tnode *head = NULL;
	append(&head, "aa");
	print2D(head); 
	append(&head, "z");
	print2D(head); 
	append(&head, "za");
	print2D(head); 
	append(&head, "b");
	print2D(head); 
	append(&head, "c");
	print2D(head); 
	append(&head, "ca");
	print2D(head); 
	append(&head, "ba");
	print2D(head); 
	delete_n(&head,"z");
	print2D(head); 
	delete_n(&head,"b");
	print2D(head); 
	delete_n(&head,"ca");
	print2D(head); 
}
