#include <direct.h>
#include "program.h"

int main(int argc, char **argv)
{
    //Change directory to location of the executable so that the program can find fonts
    char *path, len;
    path = argv[0];
    for (int i = SDL_strlen(path) - 1; i >= 0; i--)
    {
        if (path[i] == '\\')
        {
            path[i + 1] = '\0';
            break;
        }
    }
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
