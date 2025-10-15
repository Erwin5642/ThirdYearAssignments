#include <vector>
#include <Gl/glut.h>
using namespace std;

struct Vec2 {
    float x, y;

    Vec2 operator-(const Vec2 &other) const {
        return {x - other.x, y - other.y};
    }

    Vec2 operator+(const Vec2 &other) const {
        return {x + other.x, y + other.y};
    }

    Vec2 operator*(const float scalar) const {
        return {x * scalar, y * scalar};
    }
};

Vec2 p;

void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    glColor3f(0.0, 1.0, 0.0);
    glBegin(GL_POINTS);
        glVertex2f(p.x, p.y);
    glEnd();

    glFlush();
}

void update(int value) {


    glutPostRedisplay();
    glutTimerFunc(16, update, value);
}

int main(int argc, char ** argv) {
    glutInit(&argc, argv);
    glutInitWindowSize(800, 600);
    glutInitWindowPosition(100, 100);
    glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
    glutCreateWindow("Animation Functions");

    glClearColor(0.0, 0.0, 0.0, 1.0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, 800, 0, 600, -1, 1);

    glutDisplayFunc(display);
    glutTimerFunc(0, update, 0);
    glutMainLoop();

    return 0;
}

