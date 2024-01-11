#include "defs.h"

int main()
{
    // Initialize the random number generator
    srand(time(NULL));

    // init house
    HouseType house;
    initHouse(&house);
    populateRooms(&house);

    // threads
    pthread_t ghost, hunters[NUM_HUNTERS];

    // instantiate parameter structs
    GhostParamsType ghostParam = {&house};
    HunterParamsType hunterParam[NUM_HUNTERS];
    for (int i = 0; i < NUM_HUNTERS; ++i) {
        char name[MAX_STR];
        printf("Enter hunter %d name: ", i+1);
        getInput(name);
        strcpy(hunterParam[i].name, name);
        hunterParam[i].house = &house;
    }

    // create threads
    for (int i = 0; i < NUM_HUNTERS; ++i) {
        pthread_create(&hunters[i], NULL, hunterFunction, &hunterParam[i]);
    }
    pthread_create(&ghost, NULL, ghostFunction, &ghostParam);

    void* ret_hunters[NUM_HUNTERS];
    void* ret_ghost;

    HunterType* h[NUM_HUNTERS];
    GhostType* g;

    // join threads
    for (int i = 0; i < NUM_HUNTERS; i++) {
        pthread_join(hunters[i], &ret_hunters[i]);
        h[i] = (HunterType*)ret_hunters[i];
    }
    pthread_join(ghost, &ret_ghost);
    g = (GhostType*)ret_ghost;

    // print final status message
     printResults(&house, h, g);

    //free memory
    cleanupGhost(ret_ghost);
    for(int k = 0; k < NUM_HUNTERS; k++ ) cleanupHunter(ret_hunters[k]);
    // print size of evidence list for house
    cleanupHouse(&house);
    return 0;
}

void printResults(HouseType* house, HunterType* hunters[], GhostType* ghost){

    printf("--- === FINAL RESULTS === --- \n\n");

    int flagAll = 0;
    for(int i = 0; i < NUM_HUNTERS; i++){
        if(hunters[i]->fear >= FEAR_MAX){
            printf("%s was scared to death!\n", hunters[i]->name);
            flagAll++;
        }
        else if (hunters[i]->bored >= BOREDOM_MAX){
            printf("%s was bored to death!\n", hunters[i]->name);
            flagAll++;
        }
    }
    if(flagAll == NUM_HUNTERS){
        printf("All hunters died from boredom or fear! The ghost won!!\n");
    }
    int emf=0, fing=0, temp=0, sound=0;
    char str[MAX_STR];

    EvidenceNodeType* tmp_node = (house->evidence).head;
    
    while( tmp_node != NULL) {
		switch(tmp_node->data){
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
        evidenceToString(tmp_node->data, str);
        printf("Evidence found: %s\n", str);
        tmp_node = tmp_node->next;
    }
    
    ghostToString(ghost->ghost, str);

    if( convictSpirit(emf,fing,temp,sound) != GH_UNKNOWN ){
        
        printf("The hunters determined that the ghost was a %s!\n", str);
    }
    else{
        printf("The ghost could not be identified by the hunters! (it was a %s!)\n",str);
    }
}

