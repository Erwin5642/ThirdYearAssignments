#include <vector>
#include <GL/glut.h>
#include <cmath>
#include <iostream>

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

float dot(const Vec2 &a, const Vec2 &b) {
    return a.x * b.x + a.y * b.y;
}

Vec2 normal(const Vec2 &v) {
    return {-v.y, v.x};
}

Vec2 p0 = {-250, -200}, p1 = {350, 100};

vector<Vec2> pixels;

void dda(const Vec2 &p0, const Vec2 &p1) {
    pixels.clear();

    pixels.push_back(p0);
    pixels.push_back(p1);

    float x_k = p0.x;
    float y_k = p0.y;
    const float m = (p1.y - p0.y) / (p1.x - p0.x);
    while(x_k <= p1.x) {
        pixels.push_back({x_k, round(y_k)});

        x_k += 1;
        y_k += m;
    }
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    dda(p0, p1);

    glColor3f(0.0, 1.0, 0.0);
    for(const auto &[x, y] : pixels) {
        glBegin(GL_POINTS);
            glVertex2f(x, y);
        glEnd();
    }

    glFlush();
}

void update(int value) {

    glutPostRedisplay();
    glutTimerFunc(16, update, value);
}

int main(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitWindowSize(800, 600);
    glutInitWindowPosition(100, 15);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutCreateWindow("DDA");

    glClearColor(0.0, 0.0, 0.0, 1.0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(-400, 400, -300, 300, -1.0, 1.0);
    glMatrixMode(GL_MODELVIEW);

    glutDisplayFunc(display);
    glutTimerFunc(0, update, 0);
    glutMainLoop();

    return 0;
}

