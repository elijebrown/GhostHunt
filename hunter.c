#include "defs.h"

int doHunterAction(HunterType *hunter) {
	//check for spookies!!
	if(hunter->room->ghost !=NULL){
		hunter->bored = 0;
		hunter->fear++;
	}
	else{
		hunter->bored++;
	}
	int action = randInt(0, 5);
	// most likely? don't need to update boredom or fear here because its done in moveHunter and moveGhost
	switch(action){
		case 0:
			moveHunter(hunter);
			break;
		case 1:
			collectEvidence(hunter);
			break;
		case 2: {
			GhostClass spirit = reviewEvidence(hunter);
			if(spirit != GH_UNKNOWN){ // exit simulation, sufficient ev!
				//log
				l_hunterExit(hunter->name, LOG_EVIDENCE);
				hunter->room->numHunters--;
				removeHunter(hunter, hunter->room);
				return C_TRUE; //exit thread
			}
		}
			break;
		default:
			break;
	}
	//check if hunter is a scaredy cat or bored to death
	if(hunter->fear >= FEAR_MAX || hunter->bored >= BOREDOM_MAX){
		if(hunter->fear >= FEAR_MAX){
			//log
			l_hunterExit(hunter->name, LOG_FEAR);
		}
		else{
			//log
			l_hunterExit(hunter->name, LOG_BORED);
		}
		//exit thread
		hunter->room->numHunters--;
		removeHunter(hunter, hunter->room);
		return C_TRUE;
	}
	return C_FALSE;
}

int canCollect(HunterType *hunter) {
	// IFF evidence in room and hunter has correct equipment
	if(findEvidence(hunter->room->evidence, hunter->equipment) != NULL){
		return C_TRUE;
	}
	return C_FALSE;
}

void collectEvidence(HunterType *hunter) {
	sem_t* sem = &hunter->room->sem;
	sem_wait(sem); //lock
	if (canCollect(hunter) == C_FALSE){
		sem_post(sem); //unlock
		return;
	}
	// remove evidence from room
	removeEvidence(hunter->room->evidence, hunter->equipment);
	sem_post(sem); //unlock room evidence
	// add to house, as needed
	sem_wait(&hunter->evidence->sem); //lock house evidence
	addEvidenceHouse(hunter->evidence, hunter->equipment);
	sem_post(&hunter->evidence->sem); //unlock house evidence
	// log
	l_hunterCollect(hunter->name, hunter->equipment, hunter->room->name);
	return;
}

int createHunter(HunterType** hunter) {
	*hunter = malloc(sizeof(HunterType));
	if (!checkMalloc(*hunter)) return C_NOK;
	return C_OK;
}

void initHunter(char* name, HunterType* hunter, HouseType* house) {
	//name
	strncpy(hunter->name, name, MAX_STR);
	//lock room semaphore as well so no one else can leave
	sem_wait(&(house->rooms.head->data->sem)); //lock
	switch(house->numHunters){
		case 0:
			hunter->equipment = EMF;
			break;
		case 1:
			hunter->equipment = FINGERPRINTS;
			break;
		case 2:
			hunter->equipment = TEMPERATURE;
			break;
		case 3:
			hunter->equipment = SOUND;
			break;
		default:
			hunter->equipment = EV_UNKNOWN;
			break;
	}
	// room -> head of list (van)
	hunter->room = house->rooms.head->data;
	//hunters share the same evidence list from house
	hunter->evidence = &house->evidence;
	hunter->fear = 0;
	hunter->bored = 0;
	//log
	l_hunterInit(hunter->name, hunter->equipment);
	// add hunter to room and increment
	addHunter(hunter, hunter->room);
	house->numHunters++; //increment house count
	sem_post(&house->rooms.head->data->sem); //unlock
	return;
}

void moveHunter(HunterType* hunter) {
    RoomType* newRoom = NULL; // Room to move to
    RoomType* currentRoom = hunter->room;

    // Choose a random adjacent room
    int rNum = randInt(0, hunter->room->adj->size);
    RoomNodeType* temp = hunter->room->adj->head;
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

    // Move hunter to new room
    currentRoom->numHunters--;
    removeHunter(hunter, currentRoom);
    newRoom->numHunters++;
    addHunter(hunter, newRoom);
    hunter->room = newRoom;

    // Log movement
    l_hunterMove(hunter->name, newRoom->name);

    // Release locks in reverse order
    if (firstLock != secondLock) {
        sem_post(&secondLock->sem);
    }
    sem_post(&firstLock->sem);
}

GhostClass reviewEvidence(HunterType *hunter) {
	sem_t* sem = &hunter->evidence->sem;
	sem_wait(sem); //lock
	if(hunter->evidence->size < SUFFICIENT_EV){
		//log
		l_hunterReview(hunter->name, LOG_INSUFFICIENT);
		sem_post(sem); //unlock
		return GH_UNKNOWN;
	}
	//evidence flags
	int emf = 0;
	int fing = 0;
	int temp = 0;
	int sound = 0;
	//traverse evidence list
	EvidenceNodeType* tempNode = hunter->evidence->head;
	int size = hunter->evidence->size;
	for(int i = 0; i < size; ++i){
		switch(tempNode->data){
			case EMF:
				emf = 1;
				break;
			case FINGERPRINTS:
				fing = 1;
				break;
			case TEMPERATURE:
				temp = 1;
				break;
			case SOUND:
				sound = 1;
				break;
			default:
				break;
		}
		tempNode = tempNode->next;
	}
	sem_post(sem); //unlock
	// check flags
	GhostClass spirit = convictSpirit(emf, fing, temp, sound);

	if(spirit != GH_UNKNOWN){ // if sufficient evidence
		//log
		l_hunterReview(hunter->name, LOG_SUFFICIENT);
		return spirit;
	}
	else{
		//log
		l_hunterReview(hunter->name, LOG_INSUFFICIENT);
		return GH_UNKNOWN;
	}
}

void addHunter(HunterType* hunter, RoomType* room) {
	room->numHunters++;
	// insert at corresponding index
	switch(hunter->equipment){
		case EMF:
			room->hunters[0] = hunter;
			break;
		case FINGERPRINTS:
			room->hunters[1] = hunter;
			break;
		case TEMPERATURE:
			room->hunters[2] = hunter;
			break;
		case SOUND:
			room->hunters[3] = hunter;
			break;
		default: // unknown type, break. This should not happen, needed for C compiler
			break;
	}
}

void removeHunter(HunterType *hunter, RoomType* room) {
	room->numHunters--;
	// remove from corresponding index
	switch(hunter->equipment){
		case EMF:
			room->hunters[0] = NULL;
			break;
		case FINGERPRINTS:
			room->hunters[1] = NULL;
			break;
		case TEMPERATURE:
			room->hunters[2] = NULL;
			break;
		case SOUND:
			room->hunters[3] = NULL;
			break;
		default: // unknown type, break. This should not happen, needed for C compiler
			break;
	}
}

void cleanupHunter(HunterType *hunter) {
	// set fields to null just for fun
	hunter->room = NULL;
	hunter->evidence = NULL;
	free(hunter);
	return;
}

void* hunterFunction(void* args){
	HunterParamsType* p = (HunterParamsType*) args;
	HunterType* hunter;
	// init needs to be protected as well\

	if(createHunter(&hunter) == C_FALSE) pthread_exit(NULL);
	initHunter(p->name, hunter, p->house);
	// action loop
	int exit = C_FALSE; // exit flag
	while(C_OK){
		usleep(HUNTER_WAIT); //delay for simulation
		exit = doHunterAction(hunter); //action loop
		if (exit == C_TRUE) break; //exit if scared to death or bored to death
	}
	// cleanup, exit
	return (void*)hunter;
}