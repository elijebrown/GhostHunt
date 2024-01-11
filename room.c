#include "defs.h"

RoomType* createRoom(char* name){

    RoomType* room = malloc(sizeof(RoomType));
    if (!checkMalloc(room)){
		return NULL;
	}
	room->adj = malloc(sizeof(RoomListType));
	if(!checkMalloc(room->adj)){
	free(room);
	return NULL;
	}
	room->evidence = malloc(sizeof(EvidenceListType));
	if(!checkMalloc(room->evidence)){
		free(room);
		free(room->adj);
		return NULL;
	}

	// set hunter array to null
	for(int i = 0; i < NUM_HUNTERS; i++){
		room->hunters[i] = NULL;
	}
	initRoomList(room->adj);
	initEvidenceList(room->evidence);

    strncpy(room->name, name, MAX_STR);
    room->numHunters = 0;
	room->ghost = NULL;

	sem_init(&room->sem, 0, 1);

    return room;
}

void connectRooms(RoomType* r1, RoomType* r2){
    addRoom(r1->adj, r2);
    addRoom(r2->adj, r1);
}

//appends to end
void addRoom(RoomListType* list, RoomType* room) {

	RoomNodeType* new_node = malloc(sizeof(RoomNodeType));
    if (!checkMalloc(new_node)) return;

	new_node->data = room;
	new_node->next = NULL;

	if ( list->head == NULL ) {
		// empty list
		list->head = new_node;
		list->tail = new_node;
		list->size++;
		return;
	}

	// add to end of list
	list->tail->next = new_node;
	list->tail = new_node;

	list->size++;
}

void initRoomList(RoomListType* list) {
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
    return;
}

void cleanupRoomList(RoomListType* list){
	RoomNodeType *next;
	while (list->head != NULL ) {
		next = list->head->next;
		cleanupRoom(list->head->data);
		free(list->head->data);
		free(list->head);
		list->head = next;
	}
}

void cleanupRoom(RoomType* room){
	// adj, list
    if (room == NULL) return;
    cleanupAdjacencyList(room->adj);
    cleanupEvidenceList(room->evidence);

	// set hunter array to null
	for(int i = 0; i < NUM_HUNTERS; i++){
		room->hunters[i] = NULL;
	}

	free(room->adj);
	free(room->evidence);

	// sem
	sem_destroy(&room->sem);
    return;
}

void cleanupAdjacencyList(RoomListType* list) {
	RoomNodeType *next;
	while (list->head != NULL ) {
		next = list->head->next;
		free(list->head);
		list->head = next;
	}
}
