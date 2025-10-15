#include <GL/glut.h>
#include <cmath>
#include <chrono>

using namespace std;

struct Vec2 {
    float x, y;

    Vec2 operator-(const Vec2 &other) const {
        return {x - other.x, y - other.y};
    }

    Vec2 operator+(const Vec2 &other) const {
        return {x + other.x, y + other.y};
    }

    Vec2 operator*(float scalar) const {
        return {x * scalar, y * scalar};
    }
};

Vec2 p0 = {0, 0}, p1 = {0, 0};

float theta0 = 0.0;
float theta1 = 0.0;
float radius = 0.9;

chrono::high_resolution_clock::time_point lastTime;

void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    glColor3f(0.0, 1.0, 0.0);

    glBegin(GL_LINES);
        glVertex2f(p0.x, p0.y);
        glVertex2f(p1.x, p1.y);
    glEnd();

    glFlush();
}

void update(int value) {
    const auto currentTime = chrono::high_resolution_clock::now();
    const chrono::duration<float> elapsed = currentTime - lastTime;
    const float dt = elapsed.count();

    theta0 += (cos(theta0) * cos(theta0) + sin(theta1) * sin(theta1) - radius * radius) * dt;
    theta1 += (cos(theta1) * cos(theta1) + sin(theta0) * sin(theta0) - radius * radius) * dt;
    radius += ((1 - sin(theta0)) * radius + sin(theta1) * radius) * dt;

    p0 = Vec2(radius * cosf(theta0), radius * sinf(theta0));
    p1 = Vec2(radius * cosf(theta1), radius * sinf(theta1));

    lastTime = currentTime;

    glutPostRedisplay();
    glutTimerFunc(16, update, value);
}

int main(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(800, 600);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Cyrus-Beck Line Clipping");

    glClearColor(0.0, 0.0, 0.0, 0.0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(-1, 1, -1, 1);

    lastTime = chrono::high_resolution_clock::now();

    glutDisplayFunc(display);
    glutTimerFunc(0, update, 0);
    glutMainLoop();

    return 0;
}