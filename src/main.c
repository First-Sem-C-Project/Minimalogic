#include <direct.h>
#include "program.h"

int main(int argc, char **argv)
{
    //Change directory to location of the executable so that the program can find fonts
    char *path = argv[0], i;
    for (i = SDL_strlen(path) - 1; path[i] != '\\'; i--);
    path[i] = '\0';
    _chdir(path);

    int grid[GRID_ROW * GRID_COL];
    //Initialize the window, char maps, grid, font, UI etc
    InitEverything(grid);
    //The main program loop. User input, drawing, output, simulation everything happens here
    ProgramMainLoop(grid);
    //Destroying textures, window, renderer and closing fonts happens here
    CloseEverything();
    return 0;
}
