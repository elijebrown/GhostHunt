## GhostHunt
This program is for practicing essential low-level programming skills in C, such as pointer/memory allocation and threading. The program is a fun little simulation, wherein 4 hunters are trying to track down a ghost in the given house. The ghost is lackadaisically floating from room to room, occasionally leaving a trail of evidence behind for the hunters to find! Each hunter and ghost is a seperate thread in the simulation, and the rooms (and their fields) are locked dynamically with semaphores. 

## Included Files
This project includes the following files:
- main.c:		Contains the main control flow for the program.
- defs.h:		Contains constant #define definitions as well as typedef struct defs, as well as function signature declarations.
- utils.c:		Contains definitions for constants as well as function signatures.
- ghost.c:		Contains code pertaining to ghost-related operations in the program.
- room.c:		Contains code pertaining to room-related operations in the program.
- evidence.c:	Contains code pertaining to evidence-related operations in the program.
- logger.c:		Contains logger functions provided by course instructors to aid with auto-grading and debugging. Logging is toggleable via defs.h
- hunter.c:		Contains code pertaining to hunter-related operations in the program.
- building.c:	Contains code pertaininig to building-related operations in the program.
- makefile		A makefile for this project.
- README.md:    Contains this descriptive document with project details.

## Compiling + Execution
1. Navigate to the folder containing the sourcecode in a terminal.
2. run `make` in this folder
3. Use the command `./final` in the same folder containing the new .exe to run the program.

## Usage
1. Run the code as specified in the compiling and execution section. On initial run of the program, the program asks for four names for each hunter in the simulation. Enter one name, press enter, repeat as requested by the program.

## Other Notes
- run `make clean` to clean up the directory