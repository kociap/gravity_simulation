# Gravity Simulation
This program simulates the behaviour of objects in a 2D gravitational field. The simulation uses the Verlet integration method to solve the equation of motion of objects in the gravitational field.

## Building and Running The Program
The program may be compiled for Windows using clang++ by simply running the CMake build command. Before the program may be run, the following must be done:
 - the shader files must be copied from `./shaders` to the directory in which the .exe file is located.
 - csv file named `sim.txt` with data must be present in the .exe directory. The format of the csv file is `position x, position y, velocity x, velocity y, mass`.

### Keybinds
There are a number of keybinds provided by the program:
- lmb (hold) - move the camera.
- scroll - zoom in and out.
- q - decrease simulation speed x2.
- w - increase simulation speed x2.
- r - toggle between run and single-step modes.
- s - step one simulation frame (if single-step mode is enabled).
- d - enable debug information logging.
- z - decrease the scale of rendered objects x2.
- x - increase the scale of rendered objects x2.
- t - toggle field rendering.
- 1, 2, 3, 4 - change field rendering method.

## System Requirements
The program requires OpenGL 4.5.
