#include "program.h"

int main(int argc, char **argv)
{
    int grid[GRID_ROW * GRID_COL];
    //Initialize the window, char maps, grid, font, UI etc
    InitEverything(grid, argc, argv);
    //The main program loop. User input, drawing, output, simulation everything happens here
    MainProgramLoop(grid);
    //Destroying textures, window, renderer and closing fonts happens here
    CloseEverything();
    return 0;
}
