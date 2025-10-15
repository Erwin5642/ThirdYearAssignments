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

Vec2 p0 = {100, 320}, p1 = {300, 420};
vector<Vec2> pixels;

void bresenham(const Vec2 &p0, const Vec2 &p1) {
    pixels.clear();
    float x_k = p0.x;
    float y_k = p0.y;

    const float dy = p1.y - p0.y;
    const float dx = p1.x - p0.x;

    // F(x, y) = ax + by + c = 0
    // d_start = F(x + 1, y + 1/2) = a(x + 1) + b(y + 1/2) + c = ax + by + c + ax + b/2 = ax + b/2
    // d_next = F(x + 2, y + 1/2) = a(x + 2) + b (y + 1/2) + c
    // d_next = F(x + 2, y + 3/2) = a(x + 2) + b (y + 3/2) + c
    // dE = d_next - d = ax + 2a + by + b/2 + c - (ax + a + by + b/2 + c) = a
    // dNE = d_next - d = ax + 2a + by + 3b/2 + c - (ax + a + by + b/2 + c) = a + b
    // a = dy, b = dx, c = B * dx

    float d = 2 * dy - dx;
    const float dE = 2 * dy;
    const float dNE = 2 * (dy - dx);

    pixels.push_back(p0);

    while(x_k < p1.x) {
        if(d <= 0) {
            d += dE;
            x_k += 1;
        }
        else {
            d += dNE;
            x_k += 1;
            y_k += 1;
        }
        pixels.push_back(Vec2(x_k, y_k));
    }

}

void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    glColor3f(0.0, 1.0, 0.0);
    glBegin(GL_POINTS);
    for (const auto &[x, y] : pixels) {
        glVertex2f(x, y);
    }
    glEnd();

    glFlush();
}

int main(int argc, char ** argv) {
    glutInit(&argc, argv);
    glutInitWindowSize(800, 600);
    glutInitWindowPosition(100, 100);
    glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);
    glutCreateWindow("Bresenham");

    glClearColor(0.0, 0.0, 0.0, 1.0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, 600, 0, 800, -1, 1);

    bresenham(p0, p1);

    glutDisplayFunc(display);
    glutMainLoop();

    return 0;
}
