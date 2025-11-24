#include <GL/freeglut.h>
#include <cmath>
#include <iostream>
#include <vector>

#define BUFFER_SIZE 512

double windowWidth = 960, windowHeight = 540;
int selectedCurve = 0;
int selectedPoint = -1;
constexpr int baseNameFrame = 1000;
bool showPolygon = true;

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

    void draw(bool highlight = false) const {
        if (!curvePoints.empty()) {
            if (highlight) glColor3f(1.0, 0.55, 0.0);
            else glColor3f(0.0, 0.75, 1.0);
            glLineWidth(5.0);
            glBegin(GL_LINE_STRIP);
            for (auto const& v : curvePoints)
                glVertex3d(v.x, v.y, v.z);
            glEnd();
        }

        // Poligono de controle
        if(showPolygon) {
            glColor3f(1.0, 0.0, 0.75);
            glBegin(GL_LINE_STRIP);
            for (auto const& v : controlPoints)
                glVertex3d(v.x, v.y, v.z);
            glEnd();
        }

        glLineWidth(1.0);
    }

    void selectControlPoints(int baseNameOffset = 0) const
    {
        for (int i = 0; i < static_cast<int>(controlPoints.size()); i++) {
            // Codifica nome com base no frame da curva
            glLoadName(baseNameOffset + (i + 1));
            glBegin(GL_POINTS);
            const Vec3& p = controlPoints[i];
            glVertex3d(p.x, p.y, p.z);
            glEnd();
        }

    }
};

std::vector<BezierCurve> curves;

class Camera{
public:
    double originDistance = 10.0;
    double phi = M_PI / 2;
    double theta = 0.1;
    double x = originDistance * sin(theta) * cos(phi);
    double y = originDistance * cos(theta);
    double z = originDistance * sin(theta) * sin(phi);
    double targetX = 0.0;
    double targetY = 0.0;
    double targetZ = 0.0;

    void adjustDistance(const double delta) {
        originDistance += delta;
        if(originDistance < 0.1) originDistance = 0.1;
        if(originDistance > 90.0) originDistance = 90.0;
    }

    void translate(const double forward, const double right) {
        const double fx = sin(phi);
        const double fz = cos(phi);

        const double rx = cos(phi);
        const double rz = -sin(phi);

        targetX += right * fx + forward * rx;
        targetZ += right * fz + forward * rz;
    }


    void rotate(const double dx, const double dy) {
        phi += dx;
        theta -= dy;

        if(theta < 0.1) theta = 0.1;
        if(theta > M_PI - 0.1) theta = M_PI - 0.1;
    }

    void applyView() {
        x = targetX + originDistance * sin(theta) * cos(phi);
        y = targetY + originDistance * cos(theta);
        z = targetZ + originDistance * sin(theta) * sin(phi);

        gluLookAt(x, y, z, targetX, targetY, targetZ, 0, 1, 0);
    }
} camera;

struct MouseState{
    double lastX = 0.0;
    double lastY = 0.0;
    bool leftButtonDown = false;
    bool rightButtonDown = false;
} mouse;

// Os nomes são codificados para que cada curva tenha um intervalo de baseNameFrame pontos
void decodePickName(GLuint name, int &curveIndex, int &pointIndex) {
    if (name == 0) { curveIndex = -1; pointIndex = -1; return; }
    curveIndex = static_cast<int>(name / baseNameFrame);
    pointIndex = static_cast<int>((name % baseNameFrame - 1));
}

int findNearestHit(const GLuint selectBuffer[], const int hits, int &outCurve, int &outPoint) {
    outCurve = -1;
    outPoint = -1;
    if (hits <= 0) return -1;

    const GLuint *ptr = selectBuffer;
    GLuint nearestName = 0;
    GLuint nearestZMin = 0xFFFFFFFF;

    for (int i = 0; i < hits; i++) {
        const GLuint nameCount = *ptr++;
        const GLuint zMin = *ptr++;
        ptr++; // zMax

        if (zMin < nearestZMin) {
            nearestZMin = zMin;
            nearestName = ptr[0]; 
        }

        ptr += nameCount;
    }

    if (nearestName == 0) return -1;

    decodePickName(nearestName, outCurve, outPoint);
    return 0;
}

void ensureAtLeastOneCurve() {
    if (curves.empty()) {
        curves.emplace_back();
    }
}

// Insere um novo ponto de controle na curva atual
void insertControlPointXZ(const int x, const int y)
{
    ensureAtLeastOneCurve();

    GLdouble modelView[16], projection[16];
    GLint viewport[4];

    glGetDoublev(GL_MODELVIEW_MATRIX, modelView);
    glGetDoublev(GL_PROJECTION_MATRIX, projection);
    glGetIntegerv(GL_VIEWPORT, viewport);

    GLdouble nearPoint[3], farPoint[3];

    gluUnProject(x, viewport[3]-y, 0.0, modelView, projection, viewport, &nearPoint[0], &nearPoint[1], &nearPoint[2]);
    gluUnProject(x, viewport[3]-y, 1.0, modelView, projection, viewport, &farPoint[0], &farPoint[1], &farPoint[2]);

    // Intersecção com o plano z=0 para posicionar novos pontos de controle
    const double denominator = (farPoint[1] - nearPoint[1]);
    if (std::abs(denominator) < 1e-8) return;
    const double t = -nearPoint[1] / denominator;
    if(t < 0 || t > 1) return;
    const double posX = nearPoint[0] + t * (farPoint[0] - nearPoint[0]);
    const double posZ = nearPoint[2] + t * (farPoint[2] - nearPoint[2]);
    constexpr double posY = 0.0;

    curves[selectedCurve].controlPoints.push_back({posX, posY, posZ});
    curves[selectedCurve].generate();
}

// Remove um ponto de controle
void removeControlPoint(const int curveIdx, const int index) {
    if (curveIdx < 0 || curveIdx >= static_cast<int>(curves.size())) return;
    if(curves[curveIdx].controlPoints.empty()) return;
    if(index < 0 || index >= static_cast<int>(curves[curveIdx].controlPoints.size())) return;

    curves[curveIdx].controlPoints.erase(curves[curveIdx].controlPoints.begin() + index);
    curves[curveIdx].generate();

    if (selectedPoint == index) selectedPoint = -1;
    if (selectedPoint > index) selectedPoint--;
}

// Arrasta pontos de Controle
void dragSelectedPoint(const int x, const int y) {
    if(selectedPoint < 0) return;
    if (selectedCurve < 0 || selectedCurve >= static_cast<int>(curves.size())) return;

    GLdouble modelView[16], projection[16];
    GLint viewport[4];
    GLdouble posX, posY, posZ;
    GLdouble winX, winY, winZ;

    glGetDoublev(GL_MODELVIEW_MATRIX, modelView);
    glGetDoublev(GL_PROJECTION_MATRIX, projection);
    glGetIntegerv(GL_VIEWPORT, viewport);

    const Vec3 &p = curves[selectedCurve].controlPoints[selectedPoint];

    gluProject(p.x, p.y, p.z, modelView, projection, viewport, &winX, &winY, &winZ);

    gluUnProject(x, viewport[3] - y, winZ, modelView, projection, viewport, &posX, &posY, &posZ);

    curves[selectedCurve].controlPoints[selectedPoint].x = posX;
    curves[selectedCurve].controlPoints[selectedPoint].y = posY;
    curves[selectedCurve].controlPoints[selectedPoint].z = posZ;

    curves[selectedCurve].generate();
}

void pickPoints(const int x, const int y)
{
    ensureAtLeastOneCurve();

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

    // Seleção dos pontos de controle
    for (int ci = 0; ci < static_cast<int>(curves.size()); ++ci) {
        const int baseName = ci * baseNameFrame;
        curves[ci].selectControlPoints(baseName);
    }

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();

    const GLint hits = glRenderMode(GL_RENDER);

    int hitCurve = -1, hitPoint = -1;
    findNearestHit(selectBuffer, hits, hitCurve, hitPoint);

    if (hitCurve >= 0 && hitCurve < static_cast<int>(curves.size())
        && hitPoint >= 0 && hitPoint < static_cast<int>(curves[hitCurve].controlPoints.size())) {
        selectedCurve = hitCurve;
        selectedPoint = hitPoint;
    } else {
        selectedPoint = -1;
    }
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

    double offsetX = windowWidth * 0.75, offsetY = windowHeight - 20;

    glColor3f(1.0, 1.0, 1.0);
    drawText(offsetX, offsetY, "Simulador de Curvas de Bezier");
    offsetY -= 20;
    glColor3f(1.0, 0.8, 0.0);
    drawText(offsetX, offsetY, "Comandos:");
    offsetY -= 20;
    drawText(offsetX, offsetY, "N = nova curva");
    offsetY -= 20;
    drawText(offsetX, offsetY, "Botao direito = incluir/excluir pontos");
    offsetY -= 20;
    drawText(offsetX, offsetY, "Space = proxima curva");
    offsetY -= 20;
    drawText(offsetX, offsetY, "Backspace = deletar ponto de controle");
    offsetY -= 20;
    drawText(offsetX, offsetY, "Del = deletar curva");
    offsetY -= 20;
    drawText(offsetX, offsetY, "H = ocultar/desocultar poligonos e pontos de controle");
    offsetY -= 20;
    for(size_t ci = 0; ci < curves.size(); ci++) {
        char buffer[80];
        if(static_cast<int>(ci) == selectedCurve) {
            glColor3f(0.0, 1.0, 0.0);
            snprintf(buffer, sizeof(buffer), "> Curva %zu (selecionada) - %zu pontos", ci, curves[ci].controlPoints.size());
        }
        else {
            glColor3f(1.0, 1.0, 1.0);
            snprintf(buffer, sizeof(buffer), "  Curva %zu - %zu pontos", ci, curves[ci].controlPoints.size());
        }
        drawText(offsetX, offsetY, buffer);
        offsetY -= 18;
    }
    glColor3f(1.0, 1.0, 1.0);
    offsetY -= 8;
    drawText(offsetX, offsetY, "Pontos de Controle da Curva Selecionada:");
    offsetY -= 18;

    for(size_t i = 0; i < curves[selectedCurve].controlPoints.size(); i++) {
        char buffer[80];
        const Vec3 &p = curves[selectedCurve].controlPoints[i];
        if(static_cast<int>(i) == selectedPoint)
            snprintf(buffer, sizeof(buffer), "* P%zu: (%.2f, %.2f, %.2f)", i, p.x, p.y, p.z);
        else
            snprintf(buffer, sizeof(buffer), "  P%zu: (%.2f, %.2f, %.2f)", i, p.x, p.y, p.z);
        drawText(offsetX, offsetY, buffer);
        offsetY -= 16;
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

    // Curvas com poligons de controle
    for (size_t ci = 0; ci < curves.size(); ++ci) {
        bool highlight = (static_cast<int>(ci) == selectedCurve);
        curves[ci].draw(highlight);
    }

    // Pontos de controle
    if(showPolygon) {
        for (int ci = 0; ci < static_cast<int>(curves.size()); ++ci) {
            const BezierCurve &c = curves[ci];
            for (int i = 0; i < static_cast<int>(c.controlPoints.size()); ++i) {
                constexpr double pointRadius = 0.15;
                const Vec3& p = c.controlPoints[i];

                if (ci == selectedCurve && i == selectedPoint) glColor3f(0.75, 1.0, 0.0);
                else if (ci == selectedCurve) glColor3f(0.365, 0.6, 0.0);
                else glColor3f(0.6, 0.6, 0.6);

                glPushMatrix();
                glTranslated(p.x, p.y, p.z);
                glutSolidSphere(pointRadius, 20, 20);
                glPopMatrix();
            }
        }
    }

    // grid
    glColor3f(0.5,0.5,0.5);
    glBegin(GL_LINES);
    for(int i=-50;i<=50;i++) {
        glVertex3f(static_cast<float>(i),0,-50); glVertex3f(static_cast<float>(i),0,50);
        glVertex3f(-50,0,static_cast<float>(i)); glVertex3f(50,0,static_cast<float>(i));
    }
    glEnd();


    glLineWidth(2.0);
    // Eixo z
    glColor3f(0.0, 0.0, 1.0);
    glBegin(GL_LINES);
    glVertex3f(0.0f, 0.0f, -50.0f);
    glVertex3f(0.0f, 0.0f, 50.0f);
    glEnd();

    // Eixo x
    glColor3f(1.0, 0.0, 0.0);
    glBegin(GL_LINES);
    glVertex3f(-50.0f, 0.0f, 0.0f);
    glVertex3f(50.0f, 0.0f, 0.0f);
    glEnd();

    glLineWidth(1.0);

    drawUI();

    glutSwapBuffers();
}

void reshape(const int w, const int h) {
    if(h == 0) return;
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

            if(selectedPoint != -1) {
                removeControlPoint(selectedCurve, selectedPoint);
            }
            else {
                insertControlPointXZ(x, y);
            }

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
        if (!curves.empty()) {
            int last = static_cast<int>(curves[selectedCurve].controlPoints.size()) - 1;
            if (last >= 0) removeControlPoint(selectedCurve, last);
        }
    }
    if (key == 'n' || key == 'N') {
        curves.emplace_back();
        selectedCurve = static_cast<int>(curves.size()) - 1;
        selectedPoint = -1;
    }
    if(key == 'h' || key == 'H') showPolygon = !showPolygon;
    if (key == 127) {
        if (!curves.empty() && selectedCurve >= 0 && selectedCurve < static_cast<int>(curves.size())) {

            curves.erase(curves.begin() + selectedCurve);

            if (curves.empty()) {
                curves.emplace_back();
                selectedCurve = 0;
            } else if (selectedCurve >= static_cast<int>(curves.size())) {
                selectedCurve = static_cast<int>(curves.size()) - 1;
            }

            selectedPoint = -1;
        }
    }
    if(key == 32) {
        if(!curves.empty()) {
            selectedCurve = (selectedCurve + 1) % static_cast<int>(curves.size());
            selectedPoint = -1;
        }
    }

    glutPostRedisplay();
}

void specialKeys(const int key, int, int) {
    const double step = 0.5;
    switch(key) {
        case GLUT_KEY_LEFT:  camera.translate(0, -step); break;
        case GLUT_KEY_RIGHT: camera.translate(0, step); break;
        case GLUT_KEY_UP:    camera.translate(step, 0); break;
        case GLUT_KEY_DOWN:  camera.translate(-step, 0); break;
        default: break;
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
    glutSpecialFunc(specialKeys);
    glutMouseFunc(mouseButton);
    glutMotionFunc(mouseMotion);
    glutMouseWheelFunc(mouseWheel);
    glutReshapeFunc(reshape);
    glutDisplayFunc(display);

    for(int i = 0; i < 164; i++)
        curves.emplace_back();

    // Curve 0
curves[0].controlPoints = {
    {2.500, 4.330, 0.000},
    {2.500, 4.330, 1.381},
    {1.381, 4.330, 2.500},
    {0.000, 4.330, 2.500}
};

// Curve 1
curves[1].controlPoints = {
    {0.000, 4.330, 2.500},
    {-1.381, 4.330, 2.500},
    {-2.500, 4.330, 1.381},
    {-2.500, 4.330, 0.000}
};

// Curve 2
curves[2].controlPoints = {
    {-2.500, 4.330, 0.000},
    {-2.500, 4.330, -1.381},
    {-1.381, 4.330, -2.500},
    {0.000, 4.330, -2.500}
};

// Curve 3
curves[3].controlPoints = {
    {0.000, 4.330, -2.500},
    {1.381, 4.330, -2.500},
    {2.500, 4.330, -1.381},
    {2.500, 4.330, 0.000}
};

// Curve 4
curves[4].controlPoints = {
    {4.330, 2.500, 0.000},
    {4.330, 2.500, 2.391},
    {2.391, 2.500, 4.330},
    {0.000, 2.500, 4.330}
};

// Curve 5
curves[5].controlPoints = {
    {0.000, 2.500, 4.330},
    {-2.391, 2.500, 4.330},
    {-4.330, 2.500, 2.391},
    {-4.330, 2.500, 0.000}
};

// Curve 6
curves[6].controlPoints = {
    {-4.330, 2.500, 0.000},
    {-4.330, 2.500, -2.391},
    {-2.391, 2.500, -4.330},
    {0.000, 2.500, -4.330}
};

// Curve 7
curves[7].controlPoints = {
    {0.000, 2.500, -4.330},
    {2.391, 2.500, -4.330},
    {4.330, 2.500, -2.391},
    {4.330, 2.500, 0.000}
};

// Curve 8
curves[8].controlPoints = {
    {5.000, 0.000, 0.000},
    {5.000, 0.000, 2.761},
    {2.761, 0.000, 5.000},
    {0.000, 0.000, 5.000}
};

// Curve 9
curves[9].controlPoints = {
    {0.000, 0.000, 5.000},
    {-2.761, 0.000, 5.000},
    {-5.000, 0.000, 2.761},
    {-5.000, 0.000, 0.000}
};

// Curve 10
curves[10].controlPoints = {
    {-5.000, 0.000, 0.000},
    {-5.000, 0.000, -2.761},
    {-2.761, 0.000, -5.000},
    {0.000, 0.000, -5.000}
};

// Curve 11
curves[11].controlPoints = {
    {0.000, 0.000, -5.000},
    {2.761, 0.000, -5.000},
    {5.000, 0.000, -2.761},
    {5.000, 0.000, 0.000}
};

// Curve 12
curves[12].controlPoints = {
    {4.330, -2.500, 0.000},
    {4.330, -2.500, 2.391},
    {2.391, -2.500, 4.330},
    {0.000, -2.500, 4.330}
};

// Curve 13
curves[13].controlPoints = {
    {0.000, -2.500, 4.330},
    {-2.391, -2.500, 4.330},
    {-4.330, -2.500, 2.391},
    {-4.330, -2.500, 0.000}
};

// Curve 14
curves[14].controlPoints = {
    {-4.330, -2.500, 0.000},
    {-4.330, -2.500, -2.391},
    {-2.391, -2.500, -4.330},
    {0.000, -2.500, -4.330}
};

// Curve 15
curves[15].controlPoints = {
    {0.000, -2.500, -4.330},
    {2.391, -2.500, -4.330},
    {4.330, -2.500, -2.391},
    {4.330, -2.500, 0.000}
};

// Curve 16
curves[16].controlPoints = {
    {2.500, -4.330, 0.000},
    {2.500, -4.330, 1.381},
    {1.381, -4.330, 2.500},
    {0.000, -4.330, 2.500}
};

// Curve 17
curves[17].controlPoints = {
    {0.000, -4.330, 2.500},
    {-1.381, -4.330, 2.500},
    {-2.500, -4.330, 1.381},
    {-2.500, -4.330, 0.000}
};

// Curve 18
curves[18].controlPoints = {
    {-2.500, -4.330, 0.000},
    {-2.500, -4.330, -1.381},
    {-1.381, -4.330, -2.500},
    {0.000, -4.330, -2.500}
};

// Curve 19
curves[19].controlPoints = {
    {0.000, -4.330, -2.500},
    {1.381, -4.330, -2.500},
    {2.500, -4.330, -1.381},
    {2.500, -4.330, 0.000}
};

// Curve 20
curves[20].controlPoints = {
    {0.000, 5.000, 0.000},
    {2.761, 5.000, 0.000},
    {5.000, 2.761, 0.000},
    {5.000, 0.000, 0.000}
};

// Curve 21
curves[21].controlPoints = {
    {5.000, 0.000, 0.000},
    {5.000, -2.761, 0.000},
    {2.761, -5.000, 0.000},
    {0.000, -5.000, 0.000}
};

// Curve 22
curves[22].controlPoints = {
    {0.000, 5.000, 0.000},
    {1.953, 5.000, 1.953},
    {3.536, 2.761, 3.536},
    {3.536, 0.000, 3.536}
};

// Curve 23
curves[23].controlPoints = {
    {3.536, 0.000, 3.536},
    {3.536, -2.761, 3.536},
    {1.953, -5.000, 1.953},
    {0.000, -5.000, 0.000}
};

// Curve 24
curves[24].controlPoints = {
    {0.000, 5.000, 0.000},
    {0.000, 5.000, 2.761},
    {0.000, 2.761, 5.000},
    {0.000, 0.000, 5.000}
};

// Curve 25
curves[25].controlPoints = {
    {0.000, 0.000, 5.000},
    {0.000, -2.761, 5.000},
    {0.000, -5.000, 2.761},
    {0.000, -5.000, 0.000}
};

// Curve 26
curves[26].controlPoints = {
    {0.000, 5.000, 0.000},
    {-1.953, 5.000, 1.953},
    {-3.536, 2.761, 3.536},
    {-3.536, 0.000, 3.536}
};

// Curve 27
curves[27].controlPoints = {
    {-3.536, 0.000, 3.536},
    {-3.536, -2.761, 3.536},
    {-1.953, -5.000, 1.953},
    {0.000, -5.000, 0.000}
};

// Curve 28
curves[28].controlPoints = {
    {0.000, 5.000, 0.000},
    {-2.761, 5.000, 0.000},
    {-5.000, 2.761, 0.000},
    {-5.000, 0.000, 0.000}
};

// Curve 29
curves[29].controlPoints = {
    {-5.000, 0.000, 0.000},
    {-5.000, -2.761, 0.000},
    {-2.761, -5.000, 0.000},
    {0.000, -5.000, 0.000}
};

// Curve 30
curves[30].controlPoints = {
    {0.000, 5.000, 0.000},
    {-1.953, 5.000, -1.953},
    {-3.536, 2.761, -3.536},
    {-3.536, 0.000, -3.536}
};

// Curve 31
curves[31].controlPoints = {
    {-3.536, 0.000, -3.536},
    {-3.536, -2.761, -3.536},
    {-1.953, -5.000, -1.953},
    {0.000, -5.000, 0.000}
};

// Curve 32
curves[32].controlPoints = {
    {0.000, 5.000, 0.000},
    {-0.000, 5.000, -2.761},
    {-0.000, 2.761, -5.000},
    {-0.000, 0.000, -5.000}
};

// Curve 33
curves[33].controlPoints = {
    {-0.000, 0.000, -5.000},
    {-0.000, -2.761, -5.000},
    {-0.000, -5.000, -2.761},
    {0.000, -5.000, 0.000}
};

// Curve 34
curves[34].controlPoints = {
    {0.000, 5.000, 0.000},
    {1.953, 5.000, -1.953},
    {3.536, 2.761, -3.536},
    {3.536, 0.000, -3.536}
};

// Curve 35
curves[35].controlPoints = {
    {3.536, 0.000, -3.536},
    {3.536, -2.761, -3.536},
    {1.953, -5.000, -1.953},
    {0.000, -5.000, 0.000}
};

// Curve 36
curves[36].controlPoints = {
    {15.000, 0.000, 0.000},
    {15.000, 1.657, 0.000},
    {13.657, 3.000, 0.000},
    {12.000, 3.000, 0.000}
};

// Curve 37
curves[37].controlPoints = {
    {12.000, 3.000, 0.000},
    {10.343, 3.000, 0.000},
    {9.000, 1.657, 0.000},
    {9.000, 0.000, 0.000}
};

// Curve 38
curves[38].controlPoints = {
    {9.000, 0.000, 0.000},
    {9.000, -1.657, 0.000},
    {10.343, -3.000, 0.000},
    {12.000, -3.000, 0.000}
};

// Curve 39
curves[39].controlPoints = {
    {12.000, -3.000, 0.000},
    {13.657, -3.000, 0.000},
    {15.000, -1.657, 0.000},
    {15.000, 0.000, 0.000}
};

// Curve 40
curves[40].controlPoints = {
    {13.858, 0.000, 5.740},
    {13.858, 1.657, 5.740},
    {12.617, 3.000, 5.226},
    {11.087, 3.000, 4.592}
};

// Curve 41
curves[41].controlPoints = {
    {11.087, 3.000, 4.592},
    {9.556, 3.000, 3.958},
    {8.315, 1.657, 3.444},
    {8.315, 0.000, 3.444}
};

// Curve 42
curves[42].controlPoints = {
    {8.315, 0.000, 3.444},
    {8.315, -1.657, 3.444},
    {9.556, -3.000, 3.958},
    {11.087, -3.000, 4.592}
};

// Curve 43
curves[43].controlPoints = {
    {11.087, -3.000, 4.592},
    {12.617, -3.000, 5.226},
    {13.858, -1.657, 5.740},
    {13.858, 0.000, 5.740}
};

// Curve 44
curves[44].controlPoints = {
    {10.607, 0.000, 10.607},
    {10.607, 1.657, 10.607},
    {9.657, 3.000, 9.657},
    {8.485, 3.000, 8.485}
};

// Curve 45
curves[45].controlPoints = {
    {8.485, 3.000, 8.485},
    {7.314, 3.000, 7.314},
    {6.364, 1.657, 6.364},
    {6.364, 0.000, 6.364}
};

// Curve 46
curves[46].controlPoints = {
    {6.364, 0.000, 6.364},
    {6.364, -1.657, 6.364},
    {7.314, -3.000, 7.314},
    {8.485, -3.000, 8.485}
};

// Curve 47
curves[47].controlPoints = {
    {8.485, -3.000, 8.485},
    {9.657, -3.000, 9.657},
    {10.607, -1.657, 10.607},
    {10.607, 0.000, 10.607}
};

// Curve 48
curves[48].controlPoints = {
    {5.740, 0.000, 13.858},
    {5.740, 1.657, 13.858},
    {5.226, 3.000, 12.617},
    {4.592, 3.000, 11.087}
};

// Curve 49
curves[49].controlPoints = {
    {4.592, 3.000, 11.087},
    {3.958, 3.000, 9.556},
    {3.444, 1.657, 8.315},
    {3.444, 0.000, 8.315}
};

// Curve 50
curves[50].controlPoints = {
    {3.444, 0.000, 8.315},
    {3.444, -1.657, 8.315},
    {3.958, -3.000, 9.556},
    {4.592, -3.000, 11.087}
};

// Curve 51
curves[51].controlPoints = {
    {4.592, -3.000, 11.087},
    {5.226, -3.000, 12.617},
    {5.740, -1.657, 13.858},
    {5.740, 0.000, 13.858}
};

// Curve 52
curves[52].controlPoints = {
    {0.000, 0.000, 15.000},
    {0.000, 1.657, 15.000},
    {0.000, 3.000, 13.657},
    {0.000, 3.000, 12.000}
};

// Curve 53
curves[53].controlPoints = {
    {0.000, 3.000, 12.000},
    {0.000, 3.000, 10.343},
    {0.000, 1.657, 9.000},
    {0.000, 0.000, 9.000}
};

// Curve 54
curves[54].controlPoints = {
    {0.000, 0.000, 9.000},
    {0.000, -1.657, 9.000},
    {0.000, -3.000, 10.343},
    {0.000, -3.000, 12.000}
};

// Curve 55
curves[55].controlPoints = {
    {0.000, -3.000, 12.000},
    {0.000, -3.000, 13.657},
    {0.000, -1.657, 15.000},
    {0.000, 0.000, 15.000}
};

// Curve 56
curves[56].controlPoints = {
    {-5.740, 0.000, 13.858},
    {-5.740, 1.657, 13.858},
    {-5.226, 3.000, 12.617},
    {-4.592, 3.000, 11.087}
};

// Curve 57
curves[57].controlPoints = {
    {-4.592, 3.000, 11.087},
    {-3.958, 3.000, 9.556},
    {-3.444, 1.657, 8.315},
    {-3.444, 0.000, 8.315}
};

// Curve 58
curves[58].controlPoints = {
    {-3.444, 0.000, 8.315},
    {-3.444, -1.657, 8.315},
    {-3.958, -3.000, 9.556},
    {-4.592, -3.000, 11.087}
};

// Curve 59
curves[59].controlPoints = {
    {-4.592, -3.000, 11.087},
    {-5.226, -3.000, 12.617},
    {-5.740, -1.657, 13.858},
    {-5.740, 0.000, 13.858}
};

// Curve 60
curves[60].controlPoints = {
    {-10.607, 0.000, 10.607},
    {-10.607, 1.657, 10.607},
    {-9.657, 3.000, 9.657},
    {-8.485, 3.000, 8.485}
};

// Curve 61
curves[61].controlPoints = {
    {-8.485, 3.000, 8.485},
    {-7.314, 3.000, 7.314},
    {-6.364, 1.657, 6.364},
    {-6.364, 0.000, 6.364}
};

// Curve 62
curves[62].controlPoints = {
    {-6.364, 0.000, 6.364},
    {-6.364, -1.657, 6.364},
    {-7.314, -3.000, 7.314},
    {-8.485, -3.000, 8.485}
};

// Curve 63
curves[63].controlPoints = {
    {-8.485, -3.000, 8.485},
    {-9.657, -3.000, 9.657},
    {-10.607, -1.657, 10.607},
    {-10.607, 0.000, 10.607}
};

// Curve 64
curves[64].controlPoints = {
    {-13.858, 0.000, 5.740},
    {-13.858, 1.657, 5.740},
    {-12.617, 3.000, 5.226},
    {-11.087, 3.000, 4.592}
};

// Curve 65
curves[65].controlPoints = {
    {-11.087, 3.000, 4.592},
    {-9.556, 3.000, 3.958},
    {-8.315, 1.657, 3.444},
    {-8.315, 0.000, 3.444}
};

// Curve 66
curves[66].controlPoints = {
    {-8.315, 0.000, 3.444},
    {-8.315, -1.657, 3.444},
    {-9.556, -3.000, 3.958},
    {-11.087, -3.000, 4.592}
};

// Curve 67
curves[67].controlPoints = {
    {-11.087, -3.000, 4.592},
    {-12.617, -3.000, 5.226},
    {-13.858, -1.657, 5.740},
    {-13.858, 0.000, 5.740}
};

// Curve 68
curves[68].controlPoints = {
    {-15.000, 0.000, 0.000},
    {-15.000, 1.657, 0.000},
    {-13.657, 3.000, 0.000},
    {-12.000, 3.000, 0.000}
};

// Curve 69
curves[69].controlPoints = {
    {-12.000, 3.000, 0.000},
    {-10.343, 3.000, 0.000},
    {-9.000, 1.657, 0.000},
    {-9.000, 0.000, 0.000}
};

// Curve 70
curves[70].controlPoints = {
    {-9.000, 0.000, 0.000},
    {-9.000, -1.657, 0.000},
    {-10.343, -3.000, 0.000},
    {-12.000, -3.000, 0.000}
};

// Curve 71
curves[71].controlPoints = {
    {-12.000, -3.000, 0.000},
    {-13.657, -3.000, 0.000},
    {-15.000, -1.657, 0.000},
    {-15.000, 0.000, 0.000}
};

// Curve 72
curves[72].controlPoints = {
    {-13.858, 0.000, -5.740},
    {-13.858, 1.657, -5.740},
    {-12.617, 3.000, -5.226},
    {-11.087, 3.000, -4.592}
};

// Curve 73
curves[73].controlPoints = {
    {-11.087, 3.000, -4.592},
    {-9.556, 3.000, -3.958},
    {-8.315, 1.657, -3.444},
    {-8.315, 0.000, -3.444}
};

// Curve 74
curves[74].controlPoints = {
    {-8.315, 0.000, -3.444},
    {-8.315, -1.657, -3.444},
    {-9.556, -3.000, -3.958},
    {-11.087, -3.000, -4.592}
};

// Curve 75
curves[75].controlPoints = {
    {-11.087, -3.000, -4.592},
    {-12.617, -3.000, -5.226},
    {-13.858, -1.657, -5.740},
    {-13.858, 0.000, -5.740}
};

// Curve 76
curves[76].controlPoints = {
    {-10.607, 0.000, -10.607},
    {-10.607, 1.657, -10.607},
    {-9.657, 3.000, -9.657},
    {-8.485, 3.000, -8.485}
};

// Curve 77
curves[77].controlPoints = {
    {-8.485, 3.000, -8.485},
    {-7.314, 3.000, -7.314},
    {-6.364, 1.657, -6.364},
    {-6.364, 0.000, -6.364}
};

// Curve 78
curves[78].controlPoints = {
    {-6.364, 0.000, -6.364},
    {-6.364, -1.657, -6.364},
    {-7.314, -3.000, -7.314},
    {-8.485, -3.000, -8.485}
};

// Curve 79
curves[79].controlPoints = {
    {-8.485, -3.000, -8.485},
    {-9.657, -3.000, -9.657},
    {-10.607, -1.657, -10.607},
    {-10.607, 0.000, -10.607}
};

// Curve 80
curves[80].controlPoints = {
    {-5.740, 0.000, -13.858},
    {-5.740, 1.657, -13.858},
    {-5.226, 3.000, -12.617},
    {-4.592, 3.000, -11.087}
};

// Curve 81
curves[81].controlPoints = {
    {-4.592, 3.000, -11.087},
    {-3.958, 3.000, -9.556},
    {-3.444, 1.657, -8.315},
    {-3.444, 0.000, -8.315}
};

// Curve 82
curves[82].controlPoints = {
    {-3.444, 0.000, -8.315},
    {-3.444, -1.657, -8.315},
    {-3.958, -3.000, -9.556},
    {-4.592, -3.000, -11.087}
};

// Curve 83
curves[83].controlPoints = {
    {-4.592, -3.000, -11.087},
    {-5.226, -3.000, -12.617},
    {-5.740, -1.657, -13.858},
    {-5.740, 0.000, -13.858}
};

// Curve 84
curves[84].controlPoints = {
    {-0.000, 0.000, -15.000},
    {-0.000, 1.657, -15.000},
    {-0.000, 3.000, -13.657},
    {-0.000, 3.000, -12.000}
};

// Curve 85
curves[85].controlPoints = {
    {-0.000, 3.000, -12.000},
    {-0.000, 3.000, -10.343},
    {-0.000, 1.657, -9.000},
    {-0.000, 0.000, -9.000}
};

// Curve 86
curves[86].controlPoints = {
    {-0.000, 0.000, -9.000},
    {-0.000, -1.657, -9.000},
    {-0.000, -3.000, -10.343},
    {-0.000, -3.000, -12.000}
};

// Curve 87
curves[87].controlPoints = {
    {-0.000, -3.000, -12.000},
    {-0.000, -3.000, -13.657},
    {-0.000, -1.657, -15.000},
    {-0.000, 0.000, -15.000}
};

// Curve 88
curves[88].controlPoints = {
    {5.740, 0.000, -13.858},
    {5.740, 1.657, -13.858},
    {5.226, 3.000, -12.617},
    {4.592, 3.000, -11.087}
};

// Curve 89
curves[89].controlPoints = {
    {4.592, 3.000, -11.087},
    {3.958, 3.000, -9.556},
    {3.444, 1.657, -8.315},
    {3.444, 0.000, -8.315}
};

// Curve 90
curves[90].controlPoints = {
    {3.444, 0.000, -8.315},
    {3.444, -1.657, -8.315},
    {3.958, -3.000, -9.556},
    {4.592, -3.000, -11.087}
};

// Curve 91
curves[91].controlPoints = {
    {4.592, -3.000, -11.087},
    {5.226, -3.000, -12.617},
    {5.740, -1.657, -13.858},
    {5.740, 0.000, -13.858}
};

// Curve 92
curves[92].controlPoints = {
    {10.607, 0.000, -10.607},
    {10.607, 1.657, -10.607},
    {9.657, 3.000, -9.657},
    {8.485, 3.000, -8.485}
};

// Curve 93
curves[93].controlPoints = {
    {8.485, 3.000, -8.485},
    {7.314, 3.000, -7.314},
    {6.364, 1.657, -6.364},
    {6.364, 0.000, -6.364}
};

// Curve 94
curves[94].controlPoints = {
    {6.364, 0.000, -6.364},
    {6.364, -1.657, -6.364},
    {7.314, -3.000, -7.314},
    {8.485, -3.000, -8.485}
};

// Curve 95
curves[95].controlPoints = {
    {8.485, -3.000, -8.485},
    {9.657, -3.000, -9.657},
    {10.607, -1.657, -10.607},
    {10.607, 0.000, -10.607}
};

// Curve 96
curves[96].controlPoints = {
    {13.858, 0.000, -5.740},
    {13.858, 1.657, -5.740},
    {12.617, 3.000, -5.226},
    {11.087, 3.000, -4.592}
};

// Curve 97
curves[97].controlPoints = {
    {11.087, 3.000, -4.592},
    {9.556, 3.000, -3.958},
    {8.315, 1.657, -3.444},
    {8.315, 0.000, -3.444}
};

// Curve 98
curves[98].controlPoints = {
    {8.315, 0.000, -3.444},
    {8.315, -1.657, -3.444},
    {9.556, -3.000, -3.958},
    {11.087, -3.000, -4.592}
};

// Curve 99
curves[99].controlPoints = {
    {11.087, -3.000, -4.592},
    {12.617, -3.000, -5.226},
    {13.858, -1.657, -5.740},
    {13.858, 0.000, -5.740}
};

// Curve 100
curves[100].controlPoints = {
    {15.000, 0.000, 0.000},
    {15.000, 0.000, 8.284},
    {8.284, 0.000, 15.000},
    {0.000, 0.000, 15.000}
};

// Curve 101
curves[101].controlPoints = {
    {0.000, 0.000, 15.000},
    {-8.284, 0.000, 15.000},
    {-15.000, 0.000, 8.284},
    {-15.000, 0.000, 0.000}
};

// Curve 102
curves[102].controlPoints = {
    {-15.000, 0.000, 0.000},
    {-15.000, 0.000, -8.284},
    {-8.284, 0.000, -15.000},
    {0.000, 0.000, -15.000}
};

// Curve 103
curves[103].controlPoints = {
    {0.000, 0.000, -15.000},
    {8.284, 0.000, -15.000},
    {15.000, 0.000, -8.284},
    {15.000, 0.000, 0.000}
};

// Curve 104
curves[104].controlPoints = {
    {14.772, 1.148, 0.000},
    {14.772, 1.148, 8.158},
    {8.158, 1.148, 14.772},
    {0.000, 1.148, 14.772}
};

// Curve 105
curves[105].controlPoints = {
    {0.000, 1.148, 14.772},
    {-8.158, 1.148, 14.772},
    {-14.772, 1.148, 8.158},
    {-14.772, 1.148, 0.000}
};

// Curve 106
curves[106].controlPoints = {
    {-14.772, 1.148, 0.000},
    {-14.772, 1.148, -8.158},
    {-8.158, 1.148, -14.772},
    {0.000, 1.148, -14.772}
};

// Curve 107
curves[107].controlPoints = {
    {0.000, 1.148, -14.772},
    {8.158, 1.148, -14.772},
    {14.772, 1.148, -8.158},
    {14.772, 1.148, 0.000}
};

// Curve 108
curves[108].controlPoints = {
    {14.121, 2.121, 0.000},
    {14.121, 2.121, 7.799},
    {7.799, 2.121, 14.121},
    {0.000, 2.121, 14.121}
};

// Curve 109
curves[109].controlPoints = {
    {0.000, 2.121, 14.121},
    {-7.799, 2.121, 14.121},
    {-14.121, 2.121, 7.799},
    {-14.121, 2.121, 0.000}
};

// Curve 110
curves[110].controlPoints = {
    {-14.121, 2.121, 0.000},
    {-14.121, 2.121, -7.799},
    {-7.799, 2.121, -14.121},
    {0.000, 2.121, -14.121}
};

// Curve 111
curves[111].controlPoints = {
    {0.000, 2.121, -14.121},
    {7.799, 2.121, -14.121},
    {14.121, 2.121, -7.799},
    {14.121, 2.121, 0.000}
};

// Curve 112
curves[112].controlPoints = {
    {13.148, 2.772, 0.000},
    {13.148, 2.772, 7.261},
    {7.261, 2.772, 13.148},
    {0.000, 2.772, 13.148}
};

// Curve 113
curves[113].controlPoints = {
    {0.000, 2.772, 13.148},
    {-7.261, 2.772, 13.148},
    {-13.148, 2.772, 7.261},
    {-13.148, 2.772, 0.000}
};

// Curve 114
curves[114].controlPoints = {
    {-13.148, 2.772, 0.000},
    {-13.148, 2.772, -7.261},
    {-7.261, 2.772, -13.148},
    {0.000, 2.772, -13.148}
};

// Curve 115
curves[115].controlPoints = {
    {0.000, 2.772, -13.148},
    {7.261, 2.772, -13.148},
    {13.148, 2.772, -7.261},
    {13.148, 2.772, 0.000}
};

// Curve 116
curves[116].controlPoints = {
    {12.000, 3.000, 0.000},
    {12.000, 3.000, 6.627},
    {6.627, 3.000, 12.000},
    {0.000, 3.000, 12.000}
};

// Curve 117
curves[117].controlPoints = {
    {0.000, 3.000, 12.000},
    {-6.627, 3.000, 12.000},
    {-12.000, 3.000, 6.627},
    {-12.000, 3.000, 0.000}
};

// Curve 118
curves[118].controlPoints = {
    {-12.000, 3.000, 0.000},
    {-12.000, 3.000, -6.627},
    {-6.627, 3.000, -12.000},
    {0.000, 3.000, -12.000}
};

// Curve 119
curves[119].controlPoints = {
    {0.000, 3.000, -12.000},
    {6.627, 3.000, -12.000},
    {12.000, 3.000, -6.627},
    {12.000, 3.000, 0.000}
};

// Curve 120
curves[120].controlPoints = {
    {10.852, 2.772, 0.000},
    {10.852, 2.772, 5.993},
    {5.993, 2.772, 10.852},
    {0.000, 2.772, 10.852}
};

// Curve 121
curves[121].controlPoints = {
    {0.000, 2.772, 10.852},
    {-5.993, 2.772, 10.852},
    {-10.852, 2.772, 5.993},
    {-10.852, 2.772, 0.000}
};

// Curve 122
curves[122].controlPoints = {
    {-10.852, 2.772, 0.000},
    {-10.852, 2.772, -5.993},
    {-5.993, 2.772, -10.852},
    {0.000, 2.772, -10.852}
};

// Curve 123
curves[123].controlPoints = {
    {0.000, 2.772, -10.852},
    {5.993, 2.772, -10.852},
    {10.852, 2.772, -5.993},
    {10.852, 2.772, 0.000}
};

// Curve 124
curves[124].controlPoints = {
    {9.879, 2.121, 0.000},
    {9.879, 2.121, 5.456},
    {5.456, 2.121, 9.879},
    {0.000, 2.121, 9.879}
};

// Curve 125
curves[125].controlPoints = {
    {0.000, 2.121, 9.879},
    {-5.456, 2.121, 9.879},
    {-9.879, 2.121, 5.456},
    {-9.879, 2.121, 0.000}
};

// Curve 126
curves[126].controlPoints = {
    {-9.879, 2.121, 0.000},
    {-9.879, 2.121, -5.456},
    {-5.456, 2.121, -9.879},
    {0.000, 2.121, -9.879}
};

// Curve 127
curves[127].controlPoints = {
    {0.000, 2.121, -9.879},
    {5.456, 2.121, -9.879},
    {9.879, 2.121, -5.456},
    {9.879, 2.121, 0.000}
};

// Curve 128
curves[128].controlPoints = {
    {9.228, 1.148, 0.000},
    {9.228, 1.148, 5.097},
    {5.097, 1.148, 9.228},
    {0.000, 1.148, 9.228}
};

// Curve 129
curves[129].controlPoints = {
    {0.000, 1.148, 9.228},
    {-5.097, 1.148, 9.228},
    {-9.228, 1.148, 5.097},
    {-9.228, 1.148, 0.000}
};

// Curve 130
curves[130].controlPoints = {
    {-9.228, 1.148, 0.000},
    {-9.228, 1.148, -5.097},
    {-5.097, 1.148, -9.228},
    {0.000, 1.148, -9.228}
};

// Curve 131
curves[131].controlPoints = {
    {0.000, 1.148, -9.228},
    {5.097, 1.148, -9.228},
    {9.228, 1.148, -5.097},
    {9.228, 1.148, 0.000}
};

// Curve 132
curves[132].controlPoints = {
    {9.000, 0.000, 0.000},
    {9.000, 0.000, 4.971},
    {4.971, 0.000, 9.000},
    {0.000, 0.000, 9.000}
};

// Curve 133
curves[133].controlPoints = {
    {0.000, 0.000, 9.000},
    {-4.971, 0.000, 9.000},
    {-9.000, 0.000, 4.971},
    {-9.000, 0.000, 0.000}
};

// Curve 134
curves[134].controlPoints = {
    {-9.000, 0.000, 0.000},
    {-9.000, 0.000, -4.971},
    {-4.971, 0.000, -9.000},
    {0.000, 0.000, -9.000}
};

// Curve 135
curves[135].controlPoints = {
    {0.000, 0.000, -9.000},
    {4.971, 0.000, -9.000},
    {9.000, 0.000, -4.971},
    {9.000, 0.000, 0.000}
};

// Curve 136
curves[136].controlPoints = {
    {9.228, -1.148, 0.000},
    {9.228, -1.148, 5.097},
    {5.097, -1.148, 9.228},
    {0.000, -1.148, 9.228}
};

// Curve 137
curves[137].controlPoints = {
    {0.000, -1.148, 9.228},
    {-5.097, -1.148, 9.228},
    {-9.228, -1.148, 5.097},
    {-9.228, -1.148, 0.000}
};

// Curve 138
curves[138].controlPoints = {
    {-9.228, -1.148, 0.000},
    {-9.228, -1.148, -5.097},
    {-5.097, -1.148, -9.228},
    {0.000, -1.148, -9.228}
};

// Curve 139
curves[139].controlPoints = {
    {0.000, -1.148, -9.228},
    {5.097, -1.148, -9.228},
    {9.228, -1.148, -5.097},
    {9.228, -1.148, 0.000}
};

// Curve 140
curves[140].controlPoints = {
    {9.879, -2.121, 0.000},
    {9.879, -2.121, 5.456},
    {5.456, -2.121, 9.879},
    {0.000, -2.121, 9.879}
};

// Curve 141
curves[141].controlPoints = {
    {0.000, -2.121, 9.879},
    {-5.456, -2.121, 9.879},
    {-9.879, -2.121, 5.456},
    {-9.879, -2.121, 0.000}
};

// Curve 142
curves[142].controlPoints = {
    {-9.879, -2.121, 0.000},
    {-9.879, -2.121, -5.456},
    {-5.456, -2.121, -9.879},
    {0.000, -2.121, -9.879}
};

// Curve 143
curves[143].controlPoints = {
    {0.000, -2.121, -9.879},
    {5.456, -2.121, -9.879},
    {9.879, -2.121, -5.456},
    {9.879, -2.121, 0.000}
};

// Curve 144
curves[144].controlPoints = {
    {10.852, -2.772, 0.000},
    {10.852, -2.772, 5.993},
    {5.993, -2.772, 10.852},
    {0.000, -2.772, 10.852}
};

// Curve 145
curves[145].controlPoints = {
    {0.000, -2.772, 10.852},
    {-5.993, -2.772, 10.852},
    {-10.852, -2.772, 5.993},
    {-10.852, -2.772, 0.000}
};

// Curve 146
curves[146].controlPoints = {
    {-10.852, -2.772, 0.000},
    {-10.852, -2.772, -5.993},
    {-5.993, -2.772, -10.852},
    {0.000, -2.772, -10.852}
};

// Curve 147
curves[147].controlPoints = {
    {0.000, -2.772, -10.852},
    {5.993, -2.772, -10.852},
    {10.852, -2.772, -5.993},
    {10.852, -2.772, 0.000}
};

// Curve 148
curves[148].controlPoints = {
    {12.000, -3.000, 0.000},
    {12.000, -3.000, 6.627},
    {6.627, -3.000, 12.000},
    {0.000, -3.000, 12.000}
};

// Curve 149
curves[149].controlPoints = {
    {0.000, -3.000, 12.000},
    {-6.627, -3.000, 12.000},
    {-12.000, -3.000, 6.627},
    {-12.000, -3.000, 0.000}
};

// Curve 150
curves[150].controlPoints = {
    {-12.000, -3.000, 0.000},
    {-12.000, -3.000, -6.627},
    {-6.627, -3.000, -12.000},
    {0.000, -3.000, -12.000}
};

// Curve 151
curves[151].controlPoints = {
    {0.000, -3.000, -12.000},
    {6.627, -3.000, -12.000},
    {12.000, -3.000, -6.627},
    {12.000, -3.000, 0.000}
};

// Curve 152
curves[152].controlPoints = {
    {13.148, -2.772, 0.000},
    {13.148, -2.772, 7.261},
    {7.261, -2.772, 13.148},
    {0.000, -2.772, 13.148}
};

// Curve 153
curves[153].controlPoints = {
    {0.000, -2.772, 13.148},
    {-7.261, -2.772, 13.148},
    {-13.148, -2.772, 7.261},
    {-13.148, -2.772, 0.000}
};

// Curve 154
curves[154].controlPoints = {
    {-13.148, -2.772, 0.000},
    {-13.148, -2.772, -7.261},
    {-7.261, -2.772, -13.148},
    {0.000, -2.772, -13.148}
};

// Curve 155
curves[155].controlPoints = {
    {0.000, -2.772, -13.148},
    {7.261, -2.772, -13.148},
    {13.148, -2.772, -7.261},
    {13.148, -2.772, 0.000}
};

// Curve 156
curves[156].controlPoints = {
    {14.121, -2.121, 0.000},
    {14.121, -2.121, 7.799},
    {7.799, -2.121, 14.121},
    {0.000, -2.121, 14.121}
};

// Curve 157
curves[157].controlPoints = {
    {0.000, -2.121, 14.121},
    {-7.799, -2.121, 14.121},
    {-14.121, -2.121, 7.799},
    {-14.121, -2.121, 0.000}
};

// Curve 158
curves[158].controlPoints = {
    {-14.121, -2.121, 0.000},
    {-14.121, -2.121, -7.799},
    {-7.799, -2.121, -14.121},
    {0.000, -2.121, -14.121}
};

// Curve 159
curves[159].controlPoints = {
    {0.000, -2.121, -14.121},
    {7.799, -2.121, -14.121},
    {14.121, -2.121, -7.799},
    {14.121, -2.121, 0.000}
};

// Curve 160
curves[160].controlPoints = {
    {14.772, -1.148, 0.000},
    {14.772, -1.148, 8.158},
    {8.158, -1.148, 14.772},
    {0.000, -1.148, 14.772}
};

// Curve 161
curves[161].controlPoints = {
    {0.000, -1.148, 14.772},
    {-8.158, -1.148, 14.772},
    {-14.772, -1.148, 8.158},
    {-14.772, -1.148, 0.000}
};

// Curve 162
curves[162].controlPoints = {
    {-14.772, -1.148, 0.000},
    {-14.772, -1.148, -8.158},
    {-8.158, -1.148, -14.772},
    {0.000, -1.148, -14.772}
};

// Curve 163
curves[163].controlPoints = {
    {0.000, -1.148, -14.772},
    {8.158, -1.148, -14.772},
    {14.772, -1.148, -8.158},
    {14.772, -1.148, 0.000}
};

    for(auto &curve : curves) {
        curve.generate();
    }

    selectedCurve = 0;
    selectedPoint = -1;

    glutMainLoop();

    return 0;
}
