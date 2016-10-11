No makefile yet. This is how I am building currently:

gcc main.c -o main -lglfw3 -lGL -lm -ldl -lXinerama -lXrandr -lXi -lXcursor -lX11 -lXxf86vm -lpthread
-lvulkan -Wall -Wextra -Wpedantic -g
