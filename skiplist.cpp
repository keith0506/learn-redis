#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <ctime>
using namespace std;
 
#define MAX_LEVEL 16
#define newNodeOfLevel(l) (Node*)malloc(sizeof(Node) + l * sizeof(Node*));

struct Node{
    int key;
    int value;
    Node *forward[1];
};
struct skiplist{
    int level; // max level ot the list
    Node *header;
};
 
int randomLevel()
{
    int k=0;
    while (rand()%2) k++;
    k = k < MAX_LEVEL ? k : MAX_LEVEL;
    return k;
}
 
void printSL(skiplist *sl)
{
    Node *p,*q;
    int k=sl->level;
    for(int i=k; i >= 0; i--)
    {
        p = sl->header->forward[i];
        printf("level[%d]: ",i);
        while(p->forward[i] != NULL)
        {
            printf("%d -> ",p->value);
            p = p->forward[i];
        }
        printf("%d\n",p->value);
    }
    printf("\n");
}
 
Node* create(int level,int key,int value){
    Node *node = newNodeOfLevel(level);
    node->key = key;
    node->value = value;
    return node;
}
 
skiplist* createSkipList(){
    skiplist *sl = (skiplist*)malloc(sizeof(skiplist));
    sl->level = 0;
    sl->header = create(MAX_LEVEL,0,0);
    for(int i = 0 ; i <= MAX_LEVEL ; i ++){
        // malloc continuous memory, no overstep
        sl->header->forward[i]= NULL;
    }
    return sl;
}
 
bool insert(skiplist* sl,int key,int value){
    Node *update[MAX_LEVEL];
    Node *p,*q;
    p = sl->header;
    int k = sl->level;
    for(int i = k ; i >= 0 ; i --){ // Traversal each level
        // find insert position
        while(p->forward[i] != NULL && (p->forward[i]->key < key)){
            p = p->forward[i];
        }
        update[i] = p;
    }
    q = p->forward[0];
    if(q && q->key == key){
        return false;
    }
 
    k = randomLevel();
    printf("NewLevel: %d, CurLevel: %d\n",k,sl->level);
    if(k > sl->level){
        // update the max level
        for(int i = sl->level + 1 ; i <= k ; i ++)
            update[i] = sl->header;
        sl->level = k;
    }
 
    q = create(k,key,value);
    for(int i = 0 ; i <= k ; i ++){
        q->forward[i] = update[i]->forward[i];
        update[i]->forward[i] = q;
    }
    printSL(sl);
    return true;
}
 
bool remove(skiplist *sl,int key){
    Node *update[MAX_LEVEL];
    Node *p,*q;
    p = sl->header;
    int k = sl->level;
    for(int i = k ; i >=0 ; i --){
        while(p->forward[i] != NULL && (p->forward[i]->key < key)){
            p = p->forward[i];
        }
        update[i] = p; // update caches the previous of target node with the same level
    }
    q = p->forward[0];
    if(q && q->key == key){
        for(int i = 0 ; i <= sl->level; i ++){
            if(update[i]->forward[i] != q)
                break;
            update[i]->forward[i] = q->forward[i];
        }
        free(q);
        for(int i = sl->level ; i > 0 ; i --){
            if(sl->header->forward[i] == NULL){
                sl->level --;
            }
        }
        return true;
    }
    else{
        return false;
    }
}
 
int search(skiplist *sl,int key){
    Node *p,*q;
    p = sl->header;
    int k = sl->level;
    for(int i = k ; i >= 0 ; i --){
        while((q = p->forward[i]) && q->key <= key){
            if(q->key == key)
                return q->value;
            p = q;
        }
    }
    return -1;
}
 
int main(){
    srand((int)time(0));
    skiplist *sl=createSkipList();
    for(int i=1;i<=16;i++)
    {
        bool a = insert(sl,i,i*3);
    }
    int i=search(sl,10);
    printf("i=%d\n",i);
    bool b=remove(sl,6);
    return 0;
}
