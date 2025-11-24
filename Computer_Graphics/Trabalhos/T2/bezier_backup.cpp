#include <GL/freeglut.h>
#include <cmath>
#include <iostream>
#include <string_view>
#include <vector>

#define BUFFER_SIZE 512

double windowWidth = 960, windowHeight = 540;
int selectedPoint = -1;

struct Vec3 {
    double x, y, z;
};

void drawText(const double x, const double y, const char text[]) {
    std::string str = text;
    glRasterPos2d(x, y);
    for(auto const &c : str) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, c);
    }
}

class BezierCurve {
public:
    std::vector<Vec3> controlPoints;
    std::vector<Vec3> curvePoints;

    static std::size_t binomial(const std::size_t n, std::size_t k) {
        if (k > n) return 0;
        std::size_t res = 1;
        if (k > n - k) k = n - k;
        for (std::size_t i = 0; i < k; ++i)
            res = res * (n - i) / (i + 1);
        return res;
    }

    Vec3 computePoint(const double t) const {
        Vec3 p{};
        const std::size_t n = controlPoints.size();
        if (n == 0) return p;
        for (std::size_t i = 0; i < n; ++i) {
            const double coefficient = static_cast<double>(binomial(n - 1, i))
                * std::pow(1.0 - t, static_cast<double>(n - 1 - i))
                * std::pow(t, static_cast<double>(i));
            p.x += coefficient * controlPoints[i].x;
            p.y += coefficient * controlPoints[i].y;
            p.z += coefficient * controlPoints[i].z;
        }
        return p;
    }

    void generate(const std::size_t segments = 100) {
        curvePoints.clear();
        if (segments == 0) return;
        curvePoints.reserve(segments + 1);
        for (std::size_t i = 0; i <= segments; ++i) {
            const double t = static_cast<double>(i) / static_cast<double>(segments);
            curvePoints.push_back(computePoint(t));
        }
    }

    void draw() const {
        glColor3f(0.0, 0.75, 1.0);
        glLineWidth(5.0);
        glBegin(GL_LINE_STRIP);
        for (auto const& v : curvePoints)
            glVertex3d(v.x, v.y, v.z);
        glEnd();

        glColor3f(1.0, 0.0, 0.75);
        glBegin(GL_LINE_STRIP);
        for (auto const& v : controlPoints)
            glVertex3d(v.x, v.y, v.z);
        glEnd();

        glLineWidth(1.0);
    }

    void drawControlPoints(const GLenum mode = GL_RENDER) const
    {
        if (mode == GL_SELECT) {
            for (int i = 0; i < static_cast<int>(controlPoints.size()); i++) {
                glLoadName(i + 1);
                glBegin(GL_POINTS);
                const Vec3& p = controlPoints[i];
                glVertex3d(p.x, p.y, p.z);
                glEnd();
            }
            return;
        }

        for (int i = 0; i < static_cast<int>(controlPoints.size()); i++) {
            constexpr double pointRadius = 0.15;
            const Vec3& p = controlPoints[i];

            if (i == selectedPoint) glColor3f(0.75, 1.0, 0.0);
            else glColor3f(0.365, 0.5, 0.0);

            glPushMatrix();
            glTranslated(p.x, p.y, p.z);
            glutSolidSphere(pointRadius, 20, 20);
            glPopMatrix();
        }
    }
} bezier;

class Camera{
public:
    double originDistance = 10.0;
    double phi = 0.0;
    double theta = -2.0 * M_PI;

    void adjustDistance(const double delta) {
        originDistance += delta;
        if(originDistance < 0.1) originDistance = 0.1;
        if(originDistance > 90.0) originDistance = 90.0;
    }

    void rotate(const double dx, const double dy) {
        phi += dx;
        theta -= dy;

        if(theta < 0.1) theta = 0.1;
        if(theta > M_PI - 0.1) theta = M_PI - 0.1;
    }

    void applyView() const {
        const double x = originDistance * sin(theta) * cos(phi);
        const double y = originDistance * cos(theta);
        const double z = originDistance * sin(theta) * sin(phi);

        gluLookAt(x, y, z, 0, 0, 0, 0, 1, 0);
    }
} camera;

struct MouseState{
    double lastX = 0.0;
    double lastY = 0.0;
    bool leftButtonDown = false;
    bool rightButtonDown = false;
} mouse;

int findNearestHit(const GLuint selectBuffer[], const int hits) {
    if (hits <= 0) return -1;

    const GLuint *ptr = selectBuffer;
    GLuint nearestName = 0;
    GLuint nearestZMin = 0xFFFFFFFF;

    for (int i = 0; i < hits; i++) {
        const GLuint nameCount = *ptr++;
        const GLuint zMin = *ptr++;
        ptr++;

        if (zMin < nearestZMin) {
            nearestZMin = zMin;
            nearestName = ptr[0];
        }

        ptr += nameCount;
    }

    return static_cast<int>(nearestName) - 1;
}

void insertControlPointXZ(const int x, const int y)
{
    GLdouble modelView[16], projection[16];
    GLint viewport[4];

    glGetDoublev(GL_MODELVIEW_MATRIX, modelView);
    glGetDoublev(GL_PROJECTION_MATRIX, projection);
    glGetIntegerv(GL_VIEWPORT, viewport);

    GLdouble nearPoint[3], farPoint[3];

    gluUnProject(x, viewport[3]-y, 0.0, modelView, projection, viewport, &nearPoint[0], &nearPoint[1], &nearPoint[2]);
    gluUnProject(x, viewport[3]-y, 1.0, modelView, projection, viewport, &farPoint[0], &farPoint[1], &farPoint[2]);

    // Indentificamos a posição no plano baseado na intersecção com a reta
    const double t = -nearPoint[1] / (farPoint[1] - nearPoint[1]);
    if(t < 0 || t > 1) return;
    const double posX = nearPoint[0] + t * (farPoint[0] - nearPoint[0]);
    const double posZ = nearPoint[2] + t * (farPoint[2] - nearPoint[2]);
    const double posY = 0.0;

    bezier.controlPoints.push_back({posX, posY, posZ});
    bezier.generate();
}

void removeControlPoint(const int index) {
    if(bezier.controlPoints.empty()) return;

    bezier.controlPoints.erase(bezier.controlPoints.begin() + index);
    bezier.generate();
}

void dragSelectedPoint(const int x, const int y) {
    if(selectedPoint < 0) return;

    GLdouble modelView[16], projection[16];
    GLint viewport[4];
    GLdouble posX, posY, posZ;
    GLdouble winX, winY, winZ;

    glGetDoublev(GL_MODELVIEW_MATRIX, modelView);
    glGetDoublev(GL_PROJECTION_MATRIX, projection);
    glGetIntegerv(GL_VIEWPORT, viewport);

    const Vec3 &p = bezier.controlPoints[selectedPoint];

    gluProject(p.x, p.y, p.z, modelView, projection, viewport, &winX, &winY, &winZ);

    gluUnProject(x, viewport[3] - y, winZ, modelView, projection, viewport, &posX, &posY, &posZ);

    bezier.controlPoints[selectedPoint].x = posX;
    bezier.controlPoints[selectedPoint].y = posY;
    bezier.controlPoints[selectedPoint].z = posZ;

    bezier.generate();
}

void pickPoints(const int x, const int y)
{
    GLuint selectBuffer[BUFFER_SIZE];
    GLint viewport[4];

    glGetIntegerv(GL_VIEWPORT, viewport);

    glSelectBuffer(BUFFER_SIZE, selectBuffer);
    glRenderMode(GL_SELECT);

    glInitNames();
    glPushName(0);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();

    gluPickMatrix(x, viewport[3] - y, 15.0, 15.0, viewport);
    gluPerspective(45.0, windowWidth / windowHeight, 0.1, 1000.0);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    camera.applyView();

    bezier.drawControlPoints(GL_SELECT);

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    const GLint hits = glRenderMode(GL_RENDER);

    selectedPoint = findNearestHit(selectBuffer, hits);
}

void drawUI() {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, windowWidth, 0, windowHeight);

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glDisable(GL_LIGHTING);

    glColor3f(1.0, 1.0, 1.0);
    double offsetX = windowWidth * 0.85, offsetY = windowHeight - 20;

    drawText(offsetX, offsetY, "Pontos de Controle:");
    for(size_t i = 0; i < bezier.controlPoints.size(); i++) {
        char buffer[50];
        const Vec3 &p = bezier.controlPoints[i];
        snprintf(buffer, sizeof(buffer), "P%lld: (%.2f, %.2f, %.2f)", i, p.x, p.y, p.z);
        offsetY -= 20;
        drawText(offsetX, offsetY, buffer);
    }

    glEnable(GL_LIGHTING);
    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
}

void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    camera.applyView();

    bezier.draw();
    bezier.drawControlPoints();

    //glDisable(GL_LIGHTING);
    glColor3f(0.5,0.5,0.5);
    glBegin(GL_LINES);
    for(int i=-50;i<=50;i++) {
        glVertex3f(i,0,-50); glVertex3f(i,0,50);
        glVertex3f(-50,0,i); glVertex3f(50,0,i);
    }
    glEnd();
    //glEnable(GL_LIGHTING);

    drawUI();

    glutSwapBuffers();
}

void reshape(const int w, const int h) {
    glViewport(0, 0, w, h);

    windowWidth = w;
    windowHeight = h;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    gluPerspective(45.0, static_cast<float>(w) / static_cast<float>(h), 0.1, 1000.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void mouseButton(const int button, const int state, const int x, const int y) {
    if(state == GLUT_DOWN) {
        if(button == GLUT_LEFT_BUTTON) {
            pickPoints(x, y);

            mouse.leftButtonDown = true;
        }
        else if(button == GLUT_RIGHT_BUTTON) {
            pickPoints(x, y);

            mouse.rightButtonDown = true;

            if(selectedPoint != -1) removeControlPoint(selectedPoint);
            else insertControlPointXZ(x, y);

            selectedPoint = -1;
        }

        mouse.lastX = x;
        mouse.lastY = y;
    }
    else if(state == GLUT_UP) {
        mouse.leftButtonDown = false;
        mouse.rightButtonDown = false;
        selectedPoint = -1;
    }

    glutPostRedisplay();
}

void mouseWheel(int, const int direction, int, int) {
    camera.adjustDistance(direction * 0.5);

    glutPostRedisplay();
}

void mouseMotion(const int x, const int y) {
    if(mouse.leftButtonDown) {
        if(selectedPoint != -1) {
            dragSelectedPoint(x, y);
        }
        else {
            const double dx = x - mouse.lastX;
            const double dy = y - mouse.lastY;

            camera.rotate(dx * 0.005, dy * 0.005);
        }

        mouse.lastX = x;
        mouse.lastY = y;

        glutPostRedisplay();
    }
}

void keyboard(const unsigned char key, int, int){
    if(key == 27) {
        std::exit(0);
    }
    if(key == 8) {
        removeControlPoint(static_cast<int>(bezier.controlPoints.size()) - 1);
    }

    glutPostRedisplay();
}

int main(int argc, char **argv){
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(960,  540);
    glutInitWindowPosition (100, 100);
    glutCreateWindow(argv[0]);
    glEnable(GL_DEPTH_TEST);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    GLfloat lightPos[] = {5.0, 10.0, 5.0, 1.0};
    GLfloat lightAmbient[] = {0.2, 0.2, 0.2, 1.0};
    GLfloat lightDiffuse[] = {0.8, 0.8, 0.8, 1.0};
    GLfloat lightSpecular[] = {1.0, 1.0, 1.0, 1.0};

    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);

    glClearColor(0.1, 0.1, 0.18, 1.0);
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouseButton);
    glutMotionFunc(mouseMotion);
    glutMouseWheelFunc(mouseWheel);
    glutReshapeFunc(reshape);
    glutDisplayFunc(display);

    bezier.controlPoints = {
        {2.170, -1.100, 0.0},
        {2.169, -0.871, 0.0},
        {2.034, -0.662, 0.0},
        {1.825, -0.569, 0.0},
        {1.671, -0.224, 0.0},
        {1.220, -0.117, 0.0},
        {0.928, -0.355, 0.0},
        {0.922, -0.355, 0.0},
        {0.918, -0.355, 0.0},
        {0.913, -0.355, 0.0},
        {0.571, -0.355, 0.0},
        {0.300, -0.656, 0.0},
        {0.335, -0.995, 0.0},
        {0.128, -1.292, 0.0},
        {0.247, -1.713, 0.0},
        {0.578, -1.858, 0.0},
        {0.741, -2.179, 0.0},
        {1.166, -2.273, 0.0},
        {1.450, -2.053, 0.0},
        {1.465, -2.053, 0.0},
        {1.479, -2.053, 0.0},
        {1.493, -2.053, 0.0},
        {1.812, -2.054, 0.0},
        {2.074, -1.792, 0.0},
        {2.075, -1.474, 0.0},
        {2.075, -1.456, 0.0},
        {2.075, -1.439, 0.0},
        {2.072, -1.423, 0.0},
        {2.136, -1.328, 0.0},
        {2.170, -1.215, 0.0},
        {2.170, -1.100, 0.0}
    };

    bezier.generate();

    glutMainLoop();

    return 0;
}