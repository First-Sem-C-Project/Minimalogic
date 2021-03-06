#pragma once
#include "program.h"

typedef struct
{
    unsigned char sendIndex, receiver, receiveIndex;
} Connection;

typedef struct
{
    unsigned char index, conNo;
    Component deletedCompo;
    Connection connections[255];
} Delete;

typedef struct
{
    unsigned char sender;
    Connection connection;
} Wiring;
typedef struct
{
    Component component;
} Place;

typedef struct
{
    unsigned char index;
    Pair before, after;
} Move;

typedef struct
{
    char act;
    union
    {
        Delete deleted;
        Wiring wired;
        Place placed;
        Move moved;
    } Action;
} Actions;

void ToggleSimulation(bool *, unsigned char *);
void ToggleDropDown(bool *, char *);

void DeleteComponent(int *, Pair);
void InsertComponent(int *, Selection, int, int);
void UpdateWindowTitle(char *);

char WireIsValid(int *, Pair, int, int, int, int);
void ChangeNumofInputs(bool, Selection *);

void ChooseFile(int *, bool saving);
void SaveToFile(int *, char *);
void ReadFromFile(int *, char *);
void NewProject(int *grid, bool *updated);
void Undo(int *grid, int *currentUndoLevel, int totalUndoLevel);
void Redo(int *grid, int *currentUndoLevel, int totalUndoLevel);
void ShiftUndoBuffer(int *currentUndoLevel, int *totalUndoLevel);
void ClearUndoBuffer(int *currentUndoLevel, int *totalUndoLevel);
void UpdateChildCount(int, bool);
