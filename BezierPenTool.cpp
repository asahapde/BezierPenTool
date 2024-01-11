#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <math.h>
#include <cfloat>

struct Point {
	float x;
	float y;
	
	// Allow operators with the point
	Point operator+(const Point& other) const {
        return { x + other.x, y + other.y };
    }

    Point operator-(const Point& other) const {
        return { x - other.x, y - other.y };
    }

    Point operator*(float other) const {
        return { x * other, y * other};
    }

    Point operator/(float other) const {
        return { x / other, y / other };
    }
    
};

Point operator*(float scalar, const Point& point) {
    return point * scalar;
}

struct Node : Point {
	bool hasHandle1, hasHandle2;
	Point handle1;
	Point handle2;
};

// Computes the distance between two points
float distance(Point p1, Point p2) {
    float dx = p2.x - p1.x;
    float dy = p2.y - p1.y;
    return sqrt(dx*dx + dy*dy);
}

// Subtracts two points and returns the result
Point subtract(Point p1, Point p2) {
    Point result;
    result.x = p1.x - p2.x;
    result.y = p1.y - p2.y;
    return result;
}

// Adds two points and returns the result
Point add(Point p1, Point p2) {
    Point result;
    result.x = p1.x + p2.x;
    result.y = p1.y + p2.y;
    return result;
}

// Scales a point by a scalar factor and returns the result
Point scale(Point p, float scalar) {
    Point result;
    result.x = p.x * scalar;
    result.y = p.y * scalar;
    return result;
}

// Function headers
void myMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void myMouseCursorPosCallback(GLFWwindow * window, double xpos, double ypos);
void keyPressedCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
void addHandle(Node* intermediateNode);

// Global Variables
std::vector<Node> nodes;
int screenHeight;
int screenWidth;
Node* selectedNode = nullptr;
Node* selectedControlPoint1 = nullptr;
Node* selectedControlPoint2 = nullptr;
float HANDLE_RADIUS = 10.0f;
float NODE_RADIUS = 10.0f;


int main(int argc, char** argv)
{
    GLFWwindow* window;
    
    // Get command line arguments
    screenWidth = std::atoi(argv[1]);
    screenHeight = std::atoi(argv[2]);

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(screenWidth, screenHeight, "A Spline Tool", NULL, NULL ) ;
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);
   
    
    /* glfw stuff */
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, screenWidth, 0, screenHeight, -1.0, 1.0);
	glViewport(0, 0, screenWidth, screenHeight);
	glfwWindowHint(GLFW_SAMPLES, 4);
	glEnable(GL_MULTISAMPLE);
	
	

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Poll for and process events */
        glfwPollEvents();
        
        /* Get Input Callbacks */
        glfwSetMouseButtonCallback(window, myMouseButtonCallback);
        glfwSetCursorPosCallback(window, myMouseCursorPosCallback);
        glfwSetKeyCallback(window, keyPressedCallback);
        
        /* Render here */
		glClearColor(1,1,1,0);
		glClear(GL_COLOR_BUFFER_BIT);
		glColor3f(0.0, 0.0, 0.0);

		// Set line width and enable line smoothing
		glLineWidth(2.0f);
		glEnable(GL_LINE_SMOOTH);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		if(nodes.size() > 0){
			// Render each piece of the spline as a polyline with 200 line segments
			for (int i = 0; i < nodes.size() - 1; i++) {
				// Get the four points that define the current cubic Bezier curve
				Point p0 = nodes[i];
				Point p3 = nodes[i + 1];
				Point p1, p2;
				if (nodes[i].hasHandle2) {
					p1 = nodes[i].handle2;
				} else {
					p1 = p0 + (p3 - p0) / 3.0f;
				}
				if (nodes[i + 1].hasHandle1) {
					p2 = nodes[i + 1].handle1;
				} else {
					p2 = p3 - (p3 - p0) / 3.0f;
				}

				// Render the polyline with 200 line segments
				glBegin(GL_LINE_STRIP);
				for (int j = 0; j <= 200; j++) {
					float t = (float)j / 200.0f;
					Point pt = pow(1.0f - t, 3) * p0 + 3.0f * pow(1.0f - t, 2) * t * p1 + 3.0f * (1.0f - t) * pow(t, 2) * p2 + pow(t, 3) * p3;
					glVertex2f(pt.x, pt.y);
				}
				glEnd();
			}

			// Render each node as a square point
			glColor3f(0.0, 0.0, 1.0f);
			glPointSize(NODE_RADIUS);
			glBegin(GL_POINTS);
			for (int i = 0; i < nodes.size(); i++) {
				glVertex2f(nodes[i].x, nodes[i].y);
			}
			glEnd();

			// Enable point smoothing and render each control point as a round point
			glColor3f(0.0, 0.0, 0.0);
			glPointSize(HANDLE_RADIUS);
			glEnable(GL_POINT_SMOOTH);
			glBegin(GL_POINTS);
			for (int i = 0; i < nodes.size(); i++) {
				if (nodes[i].hasHandle1) {
					glVertex2f(nodes[i].handle1.x, nodes[i].handle1.y);
				}
				if (nodes[i].hasHandle2) {
					glVertex2f(nodes[i].handle2.x, nodes[i].handle2.y);
				}
			}
			glEnd();
			glDisable(GL_POINT_SMOOTH);

			// Render a dotted line connecting each control point to its associated node
			glColor3f(0.0, 255.0, 255.0);
			glEnable(GL_LINE_STIPPLE);
			glLineStipple(4, 0xAAAA);
			glLineWidth(1.0f);
			glBegin(GL_LINES);
			for (int i = 0; i < nodes.size(); i++) {
				if (nodes[i].hasHandle1) {
					glVertex2f(nodes[i].x, nodes[i].y);
					glVertex2f(nodes[i].handle1.x, nodes[i].handle1.y);
				}
				if (nodes[i].hasHandle2) {
					glVertex2f(nodes[i].x, nodes[i].y);
					glVertex2f(nodes[i].handle2.x, nodes[i].handle2.y);
				}
			}
			glEnd();
			glDisable(GL_LINE_STIPPLE);
		}
		
		/* Swap front and back buffers */
        glfwSwapBuffers(window);

    }

    glfwTerminate();
    return 0;
}

void myMouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	// Check if left mouse button is pressed
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        double mouseX, mouseY;
        glfwGetCursorPos(window, &mouseX, &mouseY);
        
        int winHeight;
        int winWidth;
        glfwGetWindowSize(window, &winWidth, &winHeight);
        mouseY = winHeight - mouseY;
        

		// Get the selected node
        for (Node& node : nodes)
        {
            double distX = node.x - mouseX;
            double distY = node.y - mouseY;
            if (distX * distX + distY * distY <= NODE_RADIUS * NODE_RADIUS)
            {
                selectedNode = &node;
                break;
            }
        }
        
        // Get the correct control point (handle in this case)
        for (Node& node : nodes)
        {
             if (node.hasHandle1 && abs(node.handle1.x - mouseX) <= HANDLE_RADIUS && abs(node.handle1.y - mouseY) <= HANDLE_RADIUS){
				 selectedControlPoint1 = &node;
			 }
			 else if (node.hasHandle2 && abs(node.handle2.x - mouseX) <= HANDLE_RADIUS && abs(node.handle2.y - mouseY) <= HANDLE_RADIUS){
				 selectedControlPoint2 = &node;
			 }
        }
        
        
        // Check if mouse is on a node or a control point
        Point mousePos = { (float)mouseX, (float)mouseY };
        bool onNode = false;
        for (Node& node : nodes) {
            if (distance(node, mousePos) < NODE_RADIUS) {
                onNode = true;
                break;
            } else if (node.hasHandle1 && abs(node.handle1.x - mouseX) <= HANDLE_RADIUS && abs(node.handle1.y - mouseY) <= HANDLE_RADIUS){
				onNode = true;
                break;
			}
			else if (node.hasHandle2 && abs(node.handle2.x - mouseX) <= HANDLE_RADIUS && abs(node.handle2.y - mouseY) <= HANDLE_RADIUS){
				onNode = true;
                break;
			}
        }
        
        // If mouse is not on a control point
        if (!onNode) {
			// Create a new node
            Node newNode = { mousePos.x, mousePos.y, false, true, {mousePos.x, mousePos.y + 50.0f}, {mousePos.x, mousePos.y + 50.0f} };
            if (nodes.empty()) {
                nodes.push_back(newNode);
            } else {
				// New node to the front or back depending on the distance and add handle to the intermediateNode
                float distToFirst = distance(nodes.front(), newNode);
                float distToLast = distance(nodes.back(), newNode);
                if (distToFirst > distToLast) {
					newNode.hasHandle1 = true;
					newNode.hasHandle2 = false;
                    nodes.push_back(newNode);
                    Node* intermediateNode = &nodes.at(nodes.size()-2);
                    if(nodes.size()>2) addHandle(intermediateNode);
                } else {
                    newNode.hasHandle1 = false;
					newNode.hasHandle2 = true;
                    nodes.insert(nodes.begin(), newNode);
                    Node* intermediateNode = &nodes.at(1);
                    if(nodes.size()>2) addHandle(intermediateNode);
                }
            }
        }
        
    }
    
    // Check if left mouse button is released
    if(button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    {
		selectedNode = nullptr;
		selectedControlPoint1 = nullptr;
		selectedControlPoint2 = nullptr;
	}
}

void addHandle(Node* intermediateNode)
{
	// Draw the other handle and maintain colinearity
	if (intermediateNode->hasHandle1)
	{
		intermediateNode->hasHandle2 = true;
		float dx = intermediateNode->x - intermediateNode->handle1.x;
		float dy = intermediateNode->y - intermediateNode->handle1.y;
		intermediateNode->handle2.x = intermediateNode->x + dx;
		intermediateNode->handle2.y = intermediateNode->y + dy;
	} else if (intermediateNode->hasHandle2){
		intermediateNode->hasHandle1 = true;
		float dx = intermediateNode->x - intermediateNode->handle2.x;
		float dy = intermediateNode->y - intermediateNode->handle2.y;
		intermediateNode->handle1.x = intermediateNode->x + dx;
		intermediateNode->handle1.y = intermediateNode->y + dy;
	}
	
}

void keyPressedCallback(GLFWwindow* window, int key, int scancode, int action, int mods){
	if (key == GLFW_KEY_E && action == GLFW_PRESS) {
		// Clear all nodes
        nodes.clear();
	}
}

void myMouseCursorPosCallback(GLFWwindow * window, double xpos, double ypos) {
	// If node is selected
	if(selectedNode){
		double mouseX, mouseY;
		glfwGetCursorPos(window, &mouseX, &mouseY);
		
		int winHeight;
        int winWidth;
        glfwGetWindowSize(window, &winWidth, &winHeight);
        mouseY = winHeight - mouseY;
        
        // Get the previous distance
        float xDist = mouseX - selectedNode->x;
        float yDist = mouseY - selectedNode->y;
        
        // Move the node
        selectedNode->x = mouseX;
        selectedNode->y = mouseY;
        
        // Move the control points
        if(selectedNode->hasHandle1){
			selectedNode->handle1.x += xDist;
			selectedNode->handle1.y += yDist;
		}
		if(selectedNode->hasHandle2){
			selectedNode->handle2.x += xDist;
			selectedNode->handle2.y += yDist;
		}
	}
	
	// If handle1 is selected
	if (selectedControlPoint1){
		double mouseX, mouseY;
		glfwGetCursorPos(window, &mouseX, &mouseY);
		
		int winHeight;
        int winWidth;
        glfwGetWindowSize(window, &winWidth, &winHeight);
        mouseY = winHeight - mouseY;
        
        // Move the current handle1
		selectedControlPoint1->handle1.x = mouseX;
		selectedControlPoint1->handle1.y = mouseY;
		
		// Maintain co-linearity and move the other handle
		if (selectedControlPoint1->hasHandle2)
		{
			float dx = selectedControlPoint1->x - selectedControlPoint1->handle1.x;
			float dy = selectedControlPoint1->y - selectedControlPoint1->handle1.y;
			selectedControlPoint1->handle2.x = selectedControlPoint1->x + dx;
			selectedControlPoint1->handle2.y = selectedControlPoint1->y + dy;
		}
	}
	
	// If handle2 is selected
	if (selectedControlPoint2){
		double mouseX, mouseY;
		glfwGetCursorPos(window, &mouseX, &mouseY);
		
		int winHeight;
        int winWidth;
        glfwGetWindowSize(window, &winWidth, &winHeight);
        mouseY = winHeight - mouseY;
        
        // Move the current handle2
		selectedControlPoint2->handle2.x = mouseX;
		selectedControlPoint2->handle2.y = mouseY;
		
		// Maintain co-linearity and move the other handle
		float dx = selectedControlPoint2->x - selectedControlPoint2->handle2.x;
		float dy = selectedControlPoint2->y - selectedControlPoint2->handle2.y;
		selectedControlPoint2->handle1.x = selectedControlPoint2->x + dx;
		selectedControlPoint2->handle1.y = selectedControlPoint2->y + dy;
	}
};
