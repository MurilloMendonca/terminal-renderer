# A simple terminal renderer proof of concept

This project is just an experiment on using the terminal and ANSI escape codes to render animations or even simple games.

All code is written in C, and uses POSIX functions to interact with the terminal, so it works on WSL, but some work is needed to run on Windows.

The main.c file is the first version of the project, a simple animation, some utility functions and only using the ANSI codes to clean the terminal and reposition the cursor.
It launches 2 threads using pthreads, one for the logic and another for the rendering. This is not required, but was implemented to test the concept of synchronization between the logic and renderer using only one simple buffer.

The file mainV2.c is the second version, now using user-defined structures to represent objects on the screen, and instead of just chars, now is possible to handle styled chars with font color and background color, allowing for more complex animations.
This approach, however, leads to a lot of flickering, as the terminal is cleared and re-rendered every frame. 

To avoid the flickering, the mainV3.c file was created, now using a double buffer to store the screen state, and only rendering the changes between frames. This approach is much more efficient, but requires more memory to store the screen state.

The mainV4.c file is the final version for now, it has all the goods from the previous versions, now with the addition of a IO handler thread.
This thread is responsible for handling the user input, capturing the key presses via the termios.h library.
For now, the only real application for this is to stop the program with the key 'q' and to change the color of the square using space.
