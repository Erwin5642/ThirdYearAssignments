#include <GL/freeglut.h>
#include <vector>
#include <algorithm>
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

vector<Vec2> clippingPolygon;
vector<Vec2> clippingNormals;
Vec2 P0, P1;

float theta0 = 0.0f, omega0 = 0.7f;
float theta1 = 3.14f, omega1 = 0.71f;
float r = 2.5f;
constexpr float radius = 0.9f;
chrono::high_resolution_clock::time_point lastTime;

float dot(const Vec2 &a, const Vec2 &b) {
    return a.x * b.x + a.y * b.y;
}

Vec2 normal(const Vec2 &v) {
    return {-v.y, v.x};
}

void createClippingPolygon() {
    clippingPolygon = {
        {-0.5f, -0.4f},
        {-0.5f,  0.5f},
        {-0.3f,  0.5f},
        {0.5f,  0.4f},
        {0.45f,  -0.1f},
        {0.4f,  -0.2f},
        { 0.35f,  -0.3f},
        { -0.3f, -0.5f},
        {-0.4f, -0.5f}
    };

    const size_t n = clippingPolygon.size();
    for(int i = 0; i < n; i++) {
        Vec2 &a = clippingPolygon[i];
        Vec2 &b = clippingPolygon[(i + 1) % n];

        Vec2 edge = b - a;
        Vec2 normal_edge = normal(edge);
        clippingNormals.push_back(normal_edge);
    }
}

bool CyrusBeck(Vec2 &P0, Vec2 &P1) {
    const size_t n = clippingPolygon.size();
    float tIn = 0.0f, tOut = 1.0f;
    const Vec2 edge = P1 - P0;

    for(int i = 0; i < n; i++) {
        Vec2 &normal = clippingNormals[i];
        Vec2 &vi = clippingPolygon[(i + 1) % n];

        const float angle = dot(normal, edge);
        if(angle == 0.0f) {
            continue;
        }

        const float t = - dot(normal, P0 - vi) / angle;

        if(angle > 0) {
            tOut = min(t, tOut);
        }
        else {
            tIn = max(t, tIn);
        }

        if(tIn > tOut) {
            return false;
        }
    }

    const Vec2 newP0 = P0 + edge * tIn;
    const Vec2 newP1 = P0 + edge * tOut;

    P0 = newP0;
    P1 = newP1;
    return true;
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    // Draw clipping polygon in white
    glColor3f(1.0, 1.0, 1.0);
    glBegin(GL_LINE_LOOP);
        for (const auto&[x, y] : clippingPolygon) {
            glVertex2f(x, y);
        }
    glEnd();

    glColor3f(1.0, 0.0, 0.0);
    glBegin(GL_LINES);
        glVertex2f(P0.x, P0.y);
        glVertex2f(P1.x, P1.y);
    glEnd();

    if(CyrusBeck(P0, P1)) {
        glColor3f(0.0, 1.0, 0.0);
        glBegin(GL_LINES);
            glVertex2f(P0.x, P0.y);
            glVertex2f(P1.x, P1.y);
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

    P0 = Vec2(radius * cos(theta0), radius * sin(theta0));
    P1 = Vec2(radius * cos(theta1), radius * sin(theta1));

    lastTime = currentTime;

    glutPostRedisplay();
    glutTimerFunc(16, update, 0);
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
    // Initialize geometry
    createClippingPolygon();

    // Example line (change as needed)
    P0 = {-0.8f, -0.6f};
    P1 = { 0.8f,  0.9f};

    glutDisplayFunc(display);
    glutTimerFunc(0, update, 0);
    glutMainLoop();

    return 0;
}