#include "defs.h"

void initEvidenceList(EvidenceListType* list){

    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
	sem_init(&list->sem, 0, 1);
    return;
}

void cleanupEvidenceList(EvidenceListType* list){
    
	EvidenceNodeType *next;

	while (list->head != NULL ) {
		next = list->head->next;
		free(list->head);
		list->head = next;
	}
	sem_destroy(&list->sem);
	list->size = 0;
	list->head = NULL;
	list->tail = NULL;
    return;
}

EvidenceType chooseEvidence(GhostClass type){
	// check for valid type
	if (type != POLTERGEIST && type != BANSHEE && type != PHANTOM && type != BULLIES) return EV_UNKNOWN;
	int rand = randInt(0, 3);
	switch(type){
		case POLTERGEIST:
			switch(rand){
				case 0:
					return EMF;
				case 1:
					return FINGERPRINTS;
				case 2:
					return TEMPERATURE;
				default:
					return EV_UNKNOWN;
			}
		case BANSHEE:
			switch(rand){
				case 0:
					return EMF;
				case 1:
					return TEMPERATURE;
				case 2:
					return SOUND;
				default:
					return EV_UNKNOWN;
			}
		case PHANTOM:
			switch(rand){
				case 0:
					return FINGERPRINTS;
				case 1:
					return TEMPERATURE;
				case 2:
					return SOUND;
				default:
					return EV_UNKNOWN;
			}
		case BULLIES:
			switch(rand){
				case 0:
					return EMF;
				case 1:
					return FINGERPRINTS;
				case 2:
					return SOUND;
				default:
					return EV_UNKNOWN;
			}
		default:
			return EV_UNKNOWN;
	}
}

void addEvidence(RoomType* room, EvidenceType evidence){
	
	// create new node
	EvidenceNodeType* newNode = malloc(sizeof(EvidenceNodeType));
	if (!checkMalloc(newNode)) return;
	newNode->data = evidence;
	newNode->next = NULL;
	// add to list
	if (room->evidence->head == NULL){
		room->evidence->head = newNode;
		room->evidence->tail = newNode;
	} else {
		room->evidence->tail->next = newNode;
		room->evidence->tail = newNode;
	}
	room->evidence->size++;
	return;
}

void addEvidenceHouse(EvidenceListType* list, EvidenceType evidence){
	// only add evidence to house if it doesn't already exist
	if (findEvidence(list, evidence) != NULL) return;
	
	// create new node
	EvidenceNodeType* newNode = malloc(sizeof(EvidenceNodeType));
	if (!checkMalloc(newNode)) return;
	newNode->data = evidence;
	newNode->next = NULL;
	// add to list
	if (list->head == NULL){
		list->head = newNode;
		list->tail = newNode;
	} else {
		list->tail->next = newNode;
		list->tail = newNode;
	}
	list->size++;
	return;
}

void removeEvidence(EvidenceListType* list, EvidenceType ev){
    sem_wait(&list->sem); // Lock the list semaphore

    if(list->size == 0){
        sem_post(&list->sem); // Unlock the list semaphore
        return;
    }

    EvidenceNodeType* temp = list->head;
    EvidenceNodeType* prev = NULL;

    while(temp != NULL){
        if(temp->data == ev){
            if(prev == NULL){
                list->head = temp->next;
                if(temp == list->tail){
                    list->tail = NULL; // Update tail if head is also tail
                }
            } else {
                prev->next = temp->next;
                if(temp == list->tail){
                    list->tail = prev; // Update tail if removing tail
                }
            }

            free(temp);
            list->size--;
            sem_post(&list->sem); // Unlock the list semaphore
            return;
        }
        prev = temp;
        temp = temp->next;
    }

    sem_post(&list->sem); // Unlock the list semaphore
}


EvidenceNodeType* findEvidence(EvidenceListType *list, EvidenceType ev){
	EvidenceNodeType* temp = list->head;
	
	while ( temp != NULL ) {
		if ( temp->data == ev ){
			return temp;
		}
		temp = temp->next;
	}

	return NULL;
}

GhostClass convictSpirit(int emf, int fing, int temp, int sound){
	if(emf == 1 && fing == 1 && temp == 1) return POLTERGEIST;
	else if(emf == 1 && temp == 1 && sound == 1) return BANSHEE;
	else if(fing == 1 && temp == 1 && sound == 1) return PHANTOM;
	else if(emf == 1 && fing == 1 && sound == 1) return BULLIES;
	else return GH_UNKNOWN;
}
