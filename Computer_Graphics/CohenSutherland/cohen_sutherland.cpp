//
// Created by jvgam on 04/10/2025.
//
#include <GL/freeglut.h>
#include <chrono>
#include <cmath>
using namespace std;

chrono::high_resolution_clock::time_point lastTime;

// Clipping window boundaries
const float xmin = -0.5, xmax = 0.5, ymin = -0.5, ymax = 0.5;

// Region codes
constexpr int INSIDE = 0;
constexpr int LEFT = 1;
constexpr int RIGHT = 2;
constexpr int BOTTOM = 4;
constexpr int TOP = 8;

// Line endpoints (original)
float x_0 = -0.7f, y_0 = -0.4f;
float x_1 = 0.7f, y_1 = 0.8f;

float theta0, theta1;
constexpr float radius = 0.9;

// Compute region code
int getMask(const float x, const float y) {
int mask = INSIDE;

if (x < xmin) mask |= LEFT;
else if (x > xmax) mask |= RIGHT;
if (y < ymin) mask |= BOTTOM;
else if (y > ymax) mask |= TOP;

return mask;
}

// Cohen-Sutherland clipping algorithm
bool CohenSutherland(float &x0, float &y0, float &x1, float &y1) {
int mask0 = getMask(x0, y0);
int mask1 = getMask(x1, y1);
bool accept = false;

while (true) {
if ((mask0 | mask1) == 0) {
accept = true;
break;
}
if ((mask0 & mask1) != 0) {
break;
}
int maskOut = mask0 ? mask0 : mask1;
float x, y;

if (maskOut & TOP) {
y = ymax;
x = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
}
else if (maskOut & BOTTOM) {
y = ymin;
x = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
}
else if (maskOut & RIGHT) {
x = xmax;
y = y0 + (y1 - y0) * (x - x0) / (x1 - x0);
}
else { // LEFT
x = xmin;
y = y0 + (y1 - y0) * (x - x0) / (x1 - x0);
}

if (maskOut == mask0) {
x0 = x;
y0 = y;
mask0 = getMask(x0, y0);
}
else {
x1 = x;
y1 = y;
        mask1 = getMask(x1, y1);
        }
    }

return accept;
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw clipping window
    glColor3f(1.0f, 1.0f, 1.0f); // white
    glBegin(GL_LINE_LOOP);
        glVertex2f(xmin, ymin);
        glVertex2f(xmax, ymin);
        glVertex2f(xmax, ymax);
        glVertex2f(xmin, ymax);
    glEnd();

    // Draw original line (red)
    glColor3f(1.0f, 0.0f, 0.0f);
    glBegin(GL_LINES);
        glVertex2f(x_0, y_0);
        glVertex2f(x_1, y_1);
    glEnd();

    // Draw clipped line (green)
    if (CohenSutherland(x_0, y_0, x_1, y_1)) {
        glColor3f(0.0f, 1.0f, 0.0f);
        glBegin(GL_LINES);
            glVertex2f(x_0, y_0);
            glVertex2f(x_1, y_1);
        glEnd();
    }

    glFlush();
}

void update(int value) {
    const auto currentTime = chrono::high_resolution_clock::now();
    const chrono::duration<float> elapsed = currentTime - lastTime;
    const float dt = elapsed.count();

    theta0 += cos(theta1) * dt;
    theta1 += cos(3 * sin(theta0)) * dt;

    x_0 = radius * cos(theta0);
    y_0 = radius * sin(theta0);
    x_1 = radius * cos(theta1);
    y_1 = radius * sin(theta1);

    lastTime = currentTime;

    glutPostRedisplay();
    glutTimerFunc(16, update, value);
}


int main(int argc, char *argv[]){
    glutInit(&argc, argv);
    glutInitWindowSize(800,600);
    glutInitWindowPosition(100, 100);
    glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE); // no depth needed
    glutCreateWindow("Cohen-Sutherland Algorithm");

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // black background
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1, 1, -1, 1); // 2D orthographic projection
    lastTime = chrono::high_resolution_clock::now();

    glutDisplayFunc(display);
    glutTimerFunc(0, update, 0);
    glutMainLoop();

    return 0;
}