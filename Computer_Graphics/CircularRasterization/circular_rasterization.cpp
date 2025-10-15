#include <vector>
#include <Gl/glut.h>
#include <cmath>

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

const Vec2 p0 = {400.0, 300.0};
const float radius = 100.0;
vector<Vec2> pixels;

void circularRasterization(const Vec2 &p0, const float radius) {
    pixels.clear();
    float x = 0;
    float y = radius;

    // F(x, y) = x^2 + y^2 - R^2 = 0
    // d_start = F(1, R - 1/2) = 1 + (R - 1/2)^2 - R^2 = 5/4 - R
    // dE = d_old + (2x + 3)
    // dSE = d_old + (2x - 2y + 5)

    float d = 5.0/4.0 - radius;

    pixels.push_back(Vec2(0, radius + p0.y));

    while(y > x) {
        if(d < 0) {
            d += 2.0 * x + 3.0;
        }
        else {
            d += 2.0 * (x - y) + 5.0;
            y--;
        }
        x++;

        pixels.push_back({p0.x + x, p0.y + y});
        pixels.push_back({p0.x - x, p0.y + y});
        pixels.push_back({p0.x + x, p0.y - y});
        pixels.push_back({p0.x - x, p0.y - y});
        pixels.push_back({p0.x + y, p0.y + x});
        pixels.push_back({p0.x - y, p0.y + x});
        pixels.push_back({p0.x + y, p0.y - x});
        pixels.push_back({p0.x - y, p0.y - x});
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
    glutCreateWindow("Circular Rasterization");

    glClearColor(0.0, 0.0, 0.0, 1.0);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, 800, 0, 600, -1, 1);

    circularRasterization(p0, radius);

    glutDisplayFunc(display);
    glutMainLoop();

    return 0;
}
