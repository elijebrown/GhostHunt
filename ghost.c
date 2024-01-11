#include "defs.h"

int createGhost(GhostType **ghost){
    *ghost = malloc(sizeof(GhostType));
    if (!checkMalloc(*ghost)) return C_FALSE;
    return C_TRUE;
}

void initGhost(GhostType* ghost, HouseType* house){
    ghost->ghost = randomGhost();
    // pick random room, starts at 1 because van is 0
    int roomIndex = randInt(1, (house->rooms.size));

    RoomNodeType* current = house->rooms.head;
    for (int i = 1; i < roomIndex; ++i) {
        current = current->next;
    }
    ghost->room = current->data;
    ghost->bored = 0;

    //log 
    l_ghostInit(ghost->ghost, ghost->room->name);
    return;
}

/*
	Assumes there's no hunters in the same room
	(i.e. no hunter-related info to update)
*/
void moveGhost(GhostType *ghost){
    RoomType* newRoom = NULL; // Room to move to
    RoomType* currentRoom = ghost->room;

    // Choose a random adjacent room
    int rNum = randInt(0, ghost->room->adj->size);
    RoomNodeType* temp = ghost->room->adj->head;
    for(int i = 0; i < rNum; ++i){
        temp = temp->next;
    }
    newRoom = temp->data;

    // Determine lock order based on memory addresses
    RoomType* firstLock = (currentRoom < newRoom) ? currentRoom : newRoom;
    RoomType* secondLock = (currentRoom < newRoom) ? newRoom : currentRoom;

    // Acquire locks in order
    sem_wait(&firstLock->sem);
    if (firstLock != secondLock) {
        sem_wait(&secondLock->sem);
    }

    // Move ghost to new room
    currentRoom->ghost = NULL;
    newRoom->ghost = ghost;
    ghost->room = newRoom;

    // Log movement
    l_ghostMove(ghost->room->name);

    // Release locks in reverse order
    if (firstLock != secondLock) {
        sem_post(&secondLock->sem);
    }
    sem_post(&firstLock->sem);
}


void leaveEvidence(GhostType *ghost){
    EvidenceType ev = chooseEvidence(ghost->ghost);
    sem_wait(&ghost->room->sem); //lock room
    addEvidence(ghost->room, ev);
    sem_post(&ghost->room->sem); //unlock room
    // log
    l_ghostEvidence(ev, ghost->room->name);
    return;
}

int doGhostAction(GhostType *ghost){
    int action;
    if(ghost->room->numHunters > 0){
        ghost->bored = 0;
        action = randInt(0, 2);
        switch(action){
            case 1:
                leaveEvidence(ghost);
                break;
            default:
                break;
        }
    }
    else{
        action = randInt(0, 4);
        ghost->bored++;
        switch(action){
            case 0:
                moveGhost(ghost);
                break;
            case 1:
                leaveEvidence(ghost);
                break;
            default: // do nothing
                break;
        }
    }
    // check if bored to death!!
    if(ghost->bored >= BOREDOM_MAX){
        l_ghostExit(LOG_BORED); //exit the thread!! send exit flag
        ghost->room->ghost = NULL; //remove from room
        return C_TRUE;
    }
    return C_FALSE;
}

void cleanupGhost(GhostType* ghost){
    ghost->room = NULL;
    return free(ghost);
}

void* ghostFunction(void* args){
    GhostParamsType *p = (GhostParamsType *)args;
    // create ghost
    GhostType* ghost;
    if(createGhost(&ghost) == C_FALSE)pthread_exit(NULL);
    // init ghost
    initGhost(ghost, p->house);
    // loop
    int exit = C_FALSE; // exit flag
    while(C_OK){
        usleep(GHOST_WAIT); //delay for simulation
        
        exit = doGhostAction(ghost); //action loop
        if (exit == C_TRUE) break; //exit if bored to death
    }
    return (void*)ghost;
}


// ghost.c
// void initGhost(GhostType* ghost);
// void moveGhost(GhostType* ghost);
// void leaveEvidence(GhostType* ghost);
// void createGhost(GhostType** ghost);