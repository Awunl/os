#include <stdio.h>
#include <stdlib.h>

typedef struct tNode *tree;
typedef unsigned elemtype;
typedef struct tNode
{
    elemtype elem;
    tree left;
    tree right;
} node;

tree Make_Leaf(elemtype elem)
{
    tree temp = malloc(sizeof(node));
    temp->elem = elem;
    temp->left = NULL;
    temp->right = NULL;
    return temp;
}

void Add(tree *node, elemtype x)
{
    tree temp = *node;
    if (temp == NULL)
    {
        *node = Make_Leaf(x);  
	return;
    }

    while(1)
    {
        if (x < temp->elem)
        {
            if (temp->left == NULL)
            {
                temp->left = Make_Leaf(x);
                return;
            }
            temp = temp->left;
        }
        else if (x > temp->elem)
        {
            if (temp->right == NULL)             
	    {
                temp->right = Make_Leaf(x);
                return;
            }
            temp = temp->right;
        }
        else return;
    }
}

tree FindMin(tree node)
{
    while(node != NULL && node->left != NULL)
    {
        node = node->left;
    }
    return node;
}

void Delete(tree *node, elemtype x)
{
    tree temp = *node;
    tree parent = NULL;     
    if (temp == NULL) return;
    while (temp != NULL && temp->elem != x)
    {
        parent = temp;
        if (x < temp->elem) temp = temp->left;
        else temp = temp->right;
    }
    if (temp == NULL) return;      
    if (temp->left == NULL || temp->right == NULL) 
    {
        tree child;

        if (temp->left == NULL) 
        {
            child = temp->right;
        } 
        else 
        {
            child = temp->left;
        }

        if (parent == NULL) 
        {
            *node = child;
        }
        else if (parent->left == temp) 
        {
            parent->left = child;
        }
        else 
        {
            parent->right = child;
        }
        
        free(temp);
    }
    else 
    {
        tree next = FindMin(temp->right);
        elemtype t = next->elem;
        Delete(node, next->elem);
        temp->elem = t;
    }
}



int Find(tree node, elemtype x)
{
    if(node == NULL) return 0;
    else if(node->elem == x) return 1;
    else if(x < node->elem) return Find(node->left, x);
    else return Find(node->right, x);
}

void Clear(tree node)
{
    if(node != NULL)
    {
        Clear(node->left);
        Clear(node->right);
        free(node);
    }
}

int main()
{
    char operation;
    elemtype value;
    tree root = NULL;
    while(scanf(" %c%u", &operation, &value) == 2)
    {
        switch (operation)
        {
            case '+':
                Add(&root, value);
                break;
            case '-':
                Delete(&root, value);
                break;
            case '?':
                if (Find(root, value)) 
                    printf("%u yes\n", value);
                else 
                    printf("%u no\n", value);
                break; 
        }
    }
    Clear(root);
    return 0;
}
