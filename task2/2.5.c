#include <stdio.h>
#include <stdlib.h> 
#include <string.h>

typedef struct Node * link;
typedef struct Node
{
    char *elem;
    link next;
} node;
typedef link list;

void Print_List(list lp)
{
    if (lp==NULL)return;
    {
	Print_List(lp->next);
   	printf("%s ", lp->elem);	
    } 
}


void Delete_Word(list *lp)
{
    char *last = (*lp)->elem;
    link cur = *lp;
    link prev;
    prev = cur;
    cur = cur->next;

    while(cur!=NULL)
    {
	if (strcmp(cur->elem, last) == 0)
	{
	    prev->next = cur->next;
	    free(cur->elem);
	    free(cur);
	    cur = prev->next;
	}
	else
	{
	    prev = cur;
	    cur = cur->next;
	}
    }
}

void Add_Word(list *lp, char *slovo)
{
    link p = malloc(sizeof(node));
    p->elem = malloc(strlen(slovo) + 1);
    strcpy(p->elem, slovo);
    p->next = *lp;
    *lp = p;
}
char *strrev(char *word)
{
    char t;	
    int n = strlen(word)-1;
    for (int i = 0; i <= (strlen(word))/2 - 1; i++)
	{
	    t = word[i];
	    word[i] = word[n];
	    word[n] = t;
	    n--;
	}
    return word;
}
void Full_Reverse(list *lp)
{
    list prev = NULL;
    list cur = *lp;
    list next = NULL;
    while (cur != NULL)
    {
	next = cur->next;
	cur->next = prev;
	prev = cur;
	cur = next;
    }
    *lp = prev;
    list curr = *lp;
    while (curr != NULL)
    {
	strcpy(curr->elem, strrev(curr->elem));
	curr = curr->next;
    }
}

void Clear(list *lp)
{
    while(*lp != NULL)
    {
	link temp = (*lp)->next;
	free((*lp)->elem);
	free(*lp);
	*lp = temp;
    }
}



int main()
{
    char word[100];
    list lst = NULL; 
    while (scanf("%99s", word) == 1)
    {
	Add_Word(&lst, word);
    }
   
    if (lst != NULL)
    {
        Delete_Word(&lst);
	Full_Reverse(&lst);
        Print_List(lst);
	Clear(&lst);
    }
    printf("\n");
}





























