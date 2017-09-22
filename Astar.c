#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#define MAX_DEPTH 10

int solution[MAX_DEPTH]; // list of move to solve the 8 puzzle
int nbMinMoves; // number of minimal moves to solve the problem
int grid[9]; // grid representing puzzle
int depth;
int nodesGenerated; // Total of nodes generated to find the solution

typedef struct node_t { // State of the puzzle
    struct node_t *pred; // the predecessor of the node
    int f;  // The true evaluation of the node ( f = g + h )
    int g;  // the cost of getting from the initial state to the current node    
    int h;  // heureustic value (The actual cost of getting from the current node to the goal)
    int depth; // height of the current node in the tree
    int grid[9]; // representes the puzzle
    int blank; // the position of the empty tile
    int move; // the move that I have perform on the previous node to get this node
} node_t;

typedef struct {    // list contains the possible moves of the empty tile
	int length;     // number of possible moves
	unsigned int moves[4]; // list of the possible moves
} move_t;

const move_t moves[9] = { // list of possible moves of empty tile in each postion in the grid
	/* 0 */{ 2, { 1, 3 } }, // if empty tile in cell 0 => number of possible moves = 2 ,, possible moves (empty tile to cell 1 or to cell 3)
	/* 1 */{ 3, { 0, 2, 4 } },  // if empty tile in cell 1 => number of possible moves = 3 ,, possible moves (empty tile to cell 0 or to cell 2 or to cell 4)
	/* 2 */{ 2, { 1, 5 } },
	/* 3 */{ 3, { 0, 4, 6 } },
	/* 4 */{ 4, { 1, 3, 5, 7 } },
	/* 5 */{ 3, { 2, 4, 8 } },
	/* 6 */{ 2, { 3, 7 } },
	/* 7 */{ 3, { 4, 6, 8 } },
    /* 8 */{ 2, { 5, 7 } } 
};

typedef struct {    // list of nodes
	int numElements; // number of nodes in the list
	node_t *elements[10000];
} list_t;

list_t openList; // openList stores the nodes generated but not expanded yet
list_t closedList; // closedList stores the nodes that have been visited
list_t solutionList; //stores the nodes from goal node to inital node (the inverse of the solution)

/*  allocate a space for a node */
node_t *nodeAlloc(void){
	node_t *node;

    node = (node_t *)malloc(sizeof(node_t));
	node->pred = NULL;
	node->f = node->g = node->h = 0;
	return node;
}

/* free the space allocaated by node */
void nodeFree(node_t *node){	
	free(node);
	return;
}

/* return the number of nodes in the list of nodes list */
int listCount(list_t *list){
	return list->numElements;
}

/* initialize the list of nodes list */
void initList(list_t *list){
    int i;
    
	list->numElements = 0;
	for (i = 0; i < 10000; i++) {
		list->elements[i] = (node_t *)0;
	}
	return;
}

/* check if node  is in the list of nodes list, and return its position in the list and store it in pos */
int inList(list_t *list, int *grid, int *pos){
	int i, j;

	for (i = 0; i < list->numElements; i++) {
		if (list->elements[i] != (node_t *)0) { 
			for (j = 0; j < 9; j++) {
				if (list->elements[i]->grid[j] != grid[j]) break;
			}
			if (j == 9) {
				if (pos) *pos = i;
				return 1;
			}
		}
	}
	return 0;
}

/* delete the node in position pos from the list */
node_t *delList(list_t *list, int pos){
	node_t *node;

	node = list->elements[pos];
	list->numElements--;
	list->elements[pos] = list->elements[list->numElements];
	list->elements[list->numElements] = NULL;

	return node;
}

/* search the best node to process in the list */
node_t *getBestNode(list_t *list){
	int i;
	int position = -1;
	double best_f = 1000000.0;
	node_t *node;

	for (i = 0; i < list->numElements; i++)
	    if (list->elements[i])
	        if (list->elements[i]->f < best_f) {
		        position = i;
		        best_f = list->elements[position]->f;
	}
	node = delList(list, position);

	return node;
}

/* put the node in the list */
void putList(list_t *list, node_t *node){
    if (list->numElements >= 10000)
	    return;
    list->elements[list->numElements++] = node;
}

/* free the space allocated for the list */
void cleanupList(list_t *list){
	int i;

	for (i = 0; i < 10000; i++) {
		if (list->elements[i] != (node_t *)0) {
			nodeFree(list->elements[i]);
			list->numElements--;
		}
	}
	return;
}

/* evaluate how much the node is close to the goal state (heurestic function) */
double evaluateNode(node_t *node){
	int i;
	const int test[8] = { 1, 2, 3, 4, 5, 6, 7, 8 };
	int  score = 0;

	for (i = 0; i < 8; i++) {
		score += (node->grid[i] != test[i]);
	}
	return (double)score;
}

/* put the puzzle puzz in  a new node */
node_t *initNode(int *puzz) 
{
    int i;
    node_t *node;

    node = nodeAlloc();
    for (i = 0; i < 9 ; i++) {
        node->grid[i] = puzz[i];
        
        if (node->grid[i] == 0) {
            node->blank = i;
        }
    }	
    node->f = node->h = evaluateNode(node);
    node->depth = 0;
    return node;
}

/* get a child node generated from parent. Index is to indicate wich move is performed to get the child node */
node_t *getChildNode(node_t *node, int index){
    node_t *child = (node_t *)0;
    int blankSpot;
    int i;
    int move;
    blankSpot = node->blank;

    if (index < moves[blankSpot].length) {
        int moveFrom;

        child = nodeAlloc();
        for (i = 0; i < 9; i++) {
        child->grid[i] = node->grid[i];
        }
        child->blank = node->blank;
        moveFrom = moves[blankSpot].moves[index];
        child->grid[child->blank] = child->grid[moveFrom];
        child->grid[moveFrom] = 0;
        child->blank = moveFrom;
        move = moveFrom - blankSpot;
        switch (move){
            case 3:
                child->move = 1;
                break;
            case -3:
                child->move = 2;
                break;
            case  1:
                child->move = 3;
                break;
            case -1:
                child->move = 4;
                break;

		}
		nodesGenerated++;
    }

    return child;
}

/* get the list of moves to sovle the puzzle  */
void getSolution(list_t *solList, node_t *goal){
    node_t *revList[10000];
	int i = 0, j,k;
	while (goal) {
		revList[i++] = goal;
		goal = goal->pred;
	}
	for (j = i - 2,k=0; j >= 0;k++, j--) {
        solution[k]=revList[j]->move;
        solList->elements[k]=revList[j];
        solList->numElements++;
	}
    nbMinMoves = i - 1;
}

/* apply astar algorithm to find the solution */
void astar(void){
	node_t *cur_node, *child, *temp;
	int i;

	while (listCount(&openList)) { // while the list contains nodes generated
		cur_node = getBestNode(&openList); // get the best node in the list
		putList(&closedList, cur_node); // put the node in the list of visited nodes
		if (cur_node->h == 0) { // if the current node is the goal node get the solution and stop the algorithm
			getSolution(&solutionList,cur_node); 
			return;
		}
		else {
			if (cur_node->depth >= MAX_DEPTH) continue;
			for (i = 0; i < 4; i++) {   // generate the child nodes of the current node
				child= getChildNode(cur_node, i);
				if (child != (node_t *)0) {
					int pos;

					if (inList(&closedList, child->grid, NULL)) { // if the child node is in the list of visited nodes discart it
						nodeFree(child);
						continue;
					}
					child->depth = cur_node->depth + 1;
					child->h = evaluateNode(child);
					child->g = (double)child->depth;
					child->f = (child->g *1) + (child->h ); 
					if (inList(&openList, child->grid, &pos)) {
						temp = delList(&openList, pos);
						if (temp->g < child->g) {
							nodeFree(child);
							putList(&openList, temp);
							continue;
						}
						nodeFree(temp);
					}
					child->pred = cur_node;
					putList(&openList, child);
				}
			}
		}
	}

	return;
}

/* functions to print solution */
void printNode(node_t *node){
    int i;

    for(i=0;i<9;i++){
        if(node->grid[i]==-1)
            printf("*   ");
        else
            printf("%d  ",node->grid[i]);
        if((i+1)%3==0)
            printf("\n");
    }
    printf("-------------------------\n");
    printf("F=%d, H=%d, moves=%d",node->f,node->h,node->depth);
    printf("\n-------------------------\n");
}

void printList(list_t *list){
	int i;
	printf("************************************");
	printf("\n The solution of the puzzle is \n");
	printf("************************************\n\n\n");
	if(list->numElements == 0)
		printf("NO SOLUTION FOUND FOR THIS PUZZLE WITH MAX DEPTH = %d\n", MAX_DEPTH);
    for(i=0; i< list->numElements; i++)
        printNode(list->elements[i]);
}

int main(){
    node_t *node;
    int puzzle[9] = { 1, 5, 2, 4, 8, 3, 0, 7, 6}; 
    initList(&openList);
    initList(&closedList);
    initList(&solutionList);
    depth=0;
	node = initNode(puzzle); // this is the initial node
	printf("\n\n******* INITIAL NODE *******\n");
    printNode(node);
    putList(&openList, node);
    astar();
	printList(&solutionList);
	printf("\n number of nodes generated to find the solution : %d\n", nodesGenerated);
	cleanupList(&openList);
    cleanupList(&closedList);
    return 0;
}