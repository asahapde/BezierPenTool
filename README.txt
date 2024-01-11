BezierPenTool.cpp - This is the source code for the spline tool in opengl

a.out - The output after compiling the source code. You can see the output here.

How to Run?
 1. Run this command on terminal "g++ BezierPenTool.cpp -lOpenGL -lglfw"
 2. Run this command on terminal "./a.out <screenWidth> <screenHeight>"
 3. You will see the output on the screen
 
How it Works?
I setup the screen as per the assignment instructions. I render the nodes, splines and control points inside the while statement which swaps buffers every frame.

I use the callback functions to get the user inputs and change the positions on the screen.


