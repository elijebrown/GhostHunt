#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

#define MAX_STR         64
#define MAX_RUNS        50
#define BOREDOM_MAX     100
#define C_TRUE          1
#define C_FALSE         0
#define C_OK            1
#define C_NOK           0
#define HUNTER_WAIT     5000 //5000
#define GHOST_WAIT      600   //600
#define NUM_HUNTERS     4
#define FEAR_MAX        10     //10

#define SUFFICIENT_EV	3	// num evidence needed for a hunter to quit when they review evidence

#define LOGGING         C_TRUE

typedef enum EvidenceType EvidenceType;
typedef enum GhostClass GhostClass;

enum EvidenceType { EMF, TEMPERATURE, FINGERPRINTS, SOUND, EV_COUNT, EV_UNKNOWN };
enum GhostClass { POLTERGEIST, BANSHEE, BULLIES, PHANTOM, GHOST_COUNT, GH_UNKNOWN };
enum LoggerDetails { LOG_FEAR, LOG_BORED, LOG_EVIDENCE, LOG_SUFFICIENT, LOG_INSUFFICIENT, LOG_UNKNOWN };

// declare structs here
typedef struct Ghost GhostType; 
typedef struct Room RoomType;
typedef struct EvidenceList EvidenceListType;
typedef struct EvidenceNode EvidenceNodeType;
typedef struct Hunter HunterType;
typedef struct RoomNode RoomNodeType;
typedef struct RoomList RoomListType;
typedef struct House HouseType;
typedef struct GhostParams GhostParamsType;
typedef struct HunterParams HunterParamsType;

// Structs
struct Ghost{
    GhostClass ghost;
    RoomType* room;
    int bored;
    sem_t sem;
};

struct Room{
    // keep list of connected rooms
    RoomListType* adj;
    EvidenceListType* evidence;
    char name[MAX_STR];
    // collection of hunters in room. SSet, each hunter has specific index based on equipment
    HunterType* hunters[NUM_HUNTERS];
    int numHunters;
    GhostType* ghost;
    sem_t sem;
};

struct RoomNode{
    RoomType* data;
    RoomNodeType* next;
};

struct RoomList{
    RoomNodeType *head;
    RoomNodeType *tail;
    int size;
};

struct EvidenceNode{
    EvidenceType data;
    EvidenceNodeType* next;
};

struct EvidenceList{
    EvidenceNodeType *head;
    EvidenceNodeType *tail;
    int size;
    sem_t sem;
};

struct Hunter {
    char name[MAX_STR];
    EvidenceType equipment;
    RoomType* room;
    EvidenceListType* evidence;
    int fear;
    int bored;
    sem_t sem;
};

struct House{
    RoomListType rooms;
    EvidenceListType evidence;
    int numHunters;
};

struct GhostParams{
    HouseType* house;
};

struct HunterParams{
    HouseType* house;
    char name[MAX_STR];
};

// Function Foward Declarations

// room.c -----------------------------------------------------------------

/* Signature: RoomType* createRoom(char* name);
  * Purpose: Initialize a RoomType object
  * Params:  In:    name  - the name of the ghost.
  * Returns: RoomType object, with initialized values. Malloc'ed to heap
*/
RoomType* createRoom(char* name);

/* Signature: void connectRooms(RoomType* r1, RoomType* r2);
  * Purpose: Connect two rooms together so that they are adjacent to each other. 
  * Params:  In/Out:    r1  - the first room to connect.
  *                  r2  - the second room to connect.
  * Returns: void
*/
void connectRooms(RoomType* r1, RoomType* r2);

/* Signature: void addRoom(RoomListType* list, RoomType* room);
  * Purpose: Add a room to a room list.
  * Params:  In/Out:    list  - the list to add the room to.
  *                  room  - the room to add to the list.
  * Returns: void
*/
void addRoom(RoomListType* list, RoomType* room);

/* Signature: void initRoomList(RoomListType* list);
  * Purpose: Initialize a room list.
  * Params:  In/Out:    list  - the list to initialize.
  * Returns: void
*/
void initRoomList(RoomListType* list);

/* Signature: void cleanupRoom(RoomType* room);
  * Purpose: Cleanup a room object, free fields from memory. 
  * Params:  In/Out:    room  - the room to cleanup.
  * Returns: void
*/
void cleanupRoom(RoomType* room);

/* Signature: void cleanupRoomList(RoomListType* list);
  * Purpose: Cleanup a room list, free fields from memory. 
  * Params:  In/Out:    list  - the list to cleanup.
  * Returns: void
*/
void cleanupRoomList(RoomListType* list);

/* Signature: void cleanupAdjacencyList(RoomListType* list);
  * Purpose: Cleanup a room list that is a field of a room object.
  * Params:  In/Out:    list  - the list to cleanup.
  * Returns: void
*/
void cleanupAdjacencyList(RoomListType* list);

// evidence.c -------------------------------------------------------------------

/* Signature: void initEvidenceList(EvidenceListType* list);
  * Purpose: Initialize an evidence list.
  * Params:  In/Out:    list  - the list to initialize.
  * Returns: void
*/
void initEvidenceList(EvidenceListType*);

/* Signature: void cleanupEvidenceList(EvidenceListType* list);
  * Purpose: Cleanup an evidence list, free fields from memory. 
  * Params:  In/Out:    list  - the list to cleanup.
  * Returns: void
*/
void cleanupEvidenceList(EvidenceListType*);

/* Signature: EvidenceType chooseEvidence(GhostClass ghost);
  * Purpose: Choose a random evidence type based on the ghost type.
  * Params:  In:    ghost  - the ghost type to choose evidence for.
  * Returns: EvidenceType enum
*/
EvidenceType chooseEvidence(GhostClass);

/* Signature: void addEvidence(RoomType* room, EvidenceType evidence);
  * Purpose: Add evidence to a room's evidence list field.
  * Params:  In/Out:    room  - the room to add evidence to.
  *              In:    evidence  - the evidence to add to the room.
  * Returns: void
*/
void addEvidence(RoomType*, EvidenceType);
/* Signature: void removeEvidence(EvidenceListType* list, EvidenceType evidence);
  * Purpose: Remove evidence from a room's evidence list field.
  * Params:  In/Out:    list  - the list to remove evidence from.
  *              In:    evidence  - the evidence to remove from the list.
  * Returns: void
*/
void removeEvidence(EvidenceListType*, EvidenceType);
/*  Signature: void addEvidenceHouse(EvidenceListType* list, EvidenceType evidence);
  * Purpose: Add evidence to a house's evidence list field.
  * Params:  In/Out:    list  - the list to add evidence to.
  *              In:    evidence  - the evidence to add to the list.
  * Returns: void
*/
void addEvidenceHouse(EvidenceListType*, EvidenceType);
/* Signature: EvidenceNodeType* findEvidence(EvidenceListType* list, EvidenceType ev);
  * Purpose: Find evidences in a list and returns the first node that matches. 
  * Params:  In/Out:    list  - the list to search.
  *              In:    ev  - the evidence (enumtype) to search for.
  * Returns: EvidenceNodeType* pointer to the evidence node if found, NULL otherwise.
*/
EvidenceNodeType* findEvidence(EvidenceListType *list, EvidenceType ev);
/* Signature: GhostClass convictSpirit(int emf, int fing, int temp, int sound);
  * Purpose: Determine the ghost type based on the evidence collected.
  * Params:  In:    emf  - the number of EMF evidence collected.
  *              In:    fing  - the number of fingerprint evidence collected.
  *              In:    temp  - the number of temperature evidence collected.
  *              In:    sound  - the number of sound evidence collected.
  * Returns: GhostClass enum (GH_UNKNOWN if not enough evidence)
*/
GhostClass convictSpirit(int emf, int fing, int temp, int sound);

// utils.c -------------------------------------------------------------------
int checkMalloc(void* ptr);     // returns C_TRUE or C_FALSE depending if malloc is successful for the given input ptr.
void getInput(char* input);     // generic input function. allows spaces in input. in/out: char ptr to store input. 
void printResults(HouseType* house, HunterType* hunters[], GhostType* ghost); // prints the end results of the simulation in a user friendly manner. 

// house.c -------------------------------------------------------------------
/*
    Initialize the fields of a HouseType structure.
        in: HouseType*  ptr for the house to be initialized.
*/
void initHouse(HouseType* house);

/*
    Populates rooms in a house with sample data from the assignment spec.
        in: HouseType*  ptr to the house to have data populated.
*/
void populateRooms(HouseType* house);

/*
    Cleans up all associated memory and frees that are associated with a house, including rooms and evidence.
        in: HouseType*  ptr to the house to be cleaned and free'd.
*/
void cleanupHouse(HouseType* house);


// ghost.c -------------------------------------------------------------------
/* Signature: initGhost(GhostType* ghost, HouseType* house);
  * Purpose: Initialize a ghost object. Spawns ghost into a random room in house. Random GhostClass type. 
  * Params:  In/Out:    ghost  - the ghost to initialize.
  *              In:    house  - the house the ghost is in.
  * Returns: void
*/
void initGhost(GhostType* ghost, HouseType* house);
/* Signature: void moveGhost(GhostType* ghost);
  * Purpose: Move a ghost to a random adjacent room.
  * Params:  In/Out:    ghost  - the ghost to move.
  * Returns: void
*/
void moveGhost(GhostType* ghost);
/* Signature: void leaveEvidence(GhostType* ghost);
  * Purpose: Leave evidence in the ghost's current room.
  * Params:  In/Out:    ghost  - the ghost that will leave evidence in it's given room.
  * Returns: void
*/
void leaveEvidence(GhostType* ghost);
/* Signature: void createGhost(GhostType** ghost);
  * Purpose: Allocate memory for a new GhostType object.
  * Params:  In/Out:    ghost  - the ghost to initialize.
  * Returns: C_OK or C_NOK depending on if the operation was successful.

*/
int createGhost(GhostType **ghost);
/* Signature: int doGhostAction(GhostType* ghost);
  * Purpose: Execute a ghost's action. Randomly chooses between moving, leaving evidence, or nothing.
  * Params:  In/Out:    ghost  - the ghost to execute an action for.
  * Returns: C_OK or C_NOK depending on if the ghost will remain in the simulation or is exiting (C_TRUE)
*/
int doGhostAction(GhostType *ghost);
/* Signature: void cleanupGhost(GhostType* ghost);
  * Purpose: Cleanup a ghost object, free fields from memory. 
  * Params:  In/Out:    ghost  - the ghost to cleanup.
  * Returns: void
*/
void cleanupGhost(GhostType* ghost);
/* Signature: void* ghostFunction(void* args);
  * Purpose: Executes all tasks for an individual ghost thread.
  * Params:  In:    args  - the parameters for the ghost thread. Includes the house the ghost is in.
  * Returns: void
*/
void* ghostFunction(void* args);


// hunter.c -------------------------------------------------------------------

/* Signature: int doHunterAction(HunterType* hunter);
  * Purpose: Execute a hunter's action. Randomly chooses between moving, collecting evidence, or reviewing evidence.
  * Params:  In/Out:    hunter  - the hunter to execute an action for.
  * Returns: C_OK or C_NOK depending on if the hunter is exiting (C_TRUE) or not (C_FALSE)
*/
int doHunterAction(HunterType*);

/*  
    Determine if a hunter is able to collect evidence at a given time or not.
        in: HunterType* ptr to hunter, which has a reference to the room it's in and the evidence list of the room.
    Returns: boolean C_TRUE or C_FALSE of if a hunter has evidence in its room that corresponds to its equipment type
*/
int canCollect(HunterType*);

/*
	Allocate memory for a new HunterType.
	in/out:	*HunterType	pointer to a hunter pointer, to be assigned by the alloc.
    Returns: C_OK or C_NOK depending on if the operation was successful.
*/
int createHunter(HunterType**);

/*
	add a hunter to a room's array of hunters. Assumes there's one hunter per equipment type.
	in:		HunterType* ptr to a hunter to add to the room
	out:	RoomType*       ptr to room for hunter to be added to
*/
void addHunter(HunterType*, RoomType*);

/*
	remove a hunter from a room. Assumes there is only one hunter per equipment type.
	in:		HunterType*	ptr to a hunter to add to the room
	out:	*room	ptr to room for hunter to be added to
*/
void removeHunter(HunterType*, RoomType*);

/*
	Clean up and free a hunter's memory
	in:	HunterType*	ptr to hunter memory to be freed
*/
void cleanupHunter(HunterType*);

/*
    Executes all tasks for an individual hunter thread.
    In: *args   Function params to be called for the thread. Params include name, house, for the hunter.
*/
void* hunterFunction(void*);

/*
	Initialize a hunter with input param fields.
		in:	char*		string name for hunter.
		in:	HunterType*	the hunter to be init'd
		in:	HouseType*	RoomType, the room that the hunter starts in.
*/
void initHunter(char*, HunterType*, HouseType*);

/*
	Move a hunter to a random choice from any of the adjacent rooms to it's current room.
		in/out:	HunterType*	a HunterType to be moved.
*/
void moveHunter(HunterType*);

/*
    Review action for a hunter. Checks to see if there is sufficient evidence collected by all hunters in the house.
    Sufficient evidence defined as a constant number of pieces of evidence, #def'd at top of this file.
        in: HunterType* The hunter reviewing evidence. 
    Returns: GhostClass enumtype of the corresponding ghost if there is sufficient evidence, GH_UNKNOWN otherwise.
*/
GhostClass reviewEvidence(HunterType*);

/*
    Function for when a hunter decides to collect evidence. Collects evidence from their current room if possible.
    Adds it to the hunter's house's list of evidence if evidence is collected.
        in: HunterType* The hunter who is collecting evidence.
*/
void collectEvidence(HunterType*);


// Helper Utilies
int randInt(int,int);        // Pseudo-random number generator function
float randFloat(float, float);  // Pseudo-random float generator function
enum GhostClass randomGhost();  // Return a randomly selected a ghost type
void ghostToString(enum GhostClass, char*); // Convert a ghost type to a string, stored in output paremeter
void evidenceToString(enum EvidenceType, char*); // Convert an evidence type to a string, stored in output parameter

// Logging Utilities (provided by assignment spec)
void l_hunterInit(char* name, enum EvidenceType equipment);
void l_hunterMove(char* name, char* room);
void l_hunterReview(char* name, enum LoggerDetails reviewResult);
void l_hunterCollect(char* name, enum EvidenceType evidence, char* room);
void l_hunterExit(char* name, enum LoggerDetails reason);
void l_ghostInit(enum GhostClass type, char* room);
void l_ghostMove(char* room);
void l_ghostEvidence(enum EvidenceType evidence, char* room);
void l_ghostExit(enum LoggerDetails reason);