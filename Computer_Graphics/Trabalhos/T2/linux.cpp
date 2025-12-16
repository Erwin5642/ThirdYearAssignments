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

    for(int i = 0; i < 484; i++)
        curves.emplace_back();

    // Curve 0
curves[0].controlPoints = {
    {0.389, 0.000, -28.194},
    {0.114, 0.000, -28.195},
    {-0.174, 0.000, -28.183},
    {-0.473, 0.000, -28.159}
};

// Curve 1
curves[1].controlPoints = {
    {-0.473, 0.000, -28.159},
    {-8.045, 0.000, -27.550},
    {-6.037, 0.000, -19.550},
    {-6.149, 0.000, -16.871}
};

// Curve 2
curves[2].controlPoints = {
    {-6.149, 0.000, -16.871},
    {-6.288, 0.000, -14.912},
    {-6.685, 0.000, -13.369},
    {-8.032, 0.000, -11.453}
};

// Curve 3
curves[3].controlPoints = {
    {-8.032, 0.000, -11.453},
    {-9.615, 0.000, -9.571},
    {-11.844, 0.000, -6.525},
    {-12.899, 0.000, -3.353}
};

// Curve 4
curves[4].controlPoints = {
    {-12.899, 0.000, -3.353},
    {-13.398, 0.000, -1.857},
    {-13.635, 0.000, -0.331},
    {-13.416, 0.000, 1.113}
};

// Curve 5
curves[5].controlPoints = {
    {-13.416, 0.000, 1.113},
    {-13.484, 0.000, 1.174},
    {-13.550, 0.000, 1.239},
    {-13.612, 0.000, 1.305}
};

// Curve 6
curves[6].controlPoints = {
    {-13.612, 0.000, 1.305},
    {-14.076, 0.000, 1.801},
    {-14.420, 0.000, 2.402},
    {-14.802, 0.000, 2.807}
};

// Curve 7
curves[7].controlPoints = {
    {-14.802, 0.000, 2.807},
    {-15.160, 0.000, 3.164},
    {-15.669, 0.000, 3.299},
    {-16.229, 0.000, 3.500}
};

// Curve 8
curves[8].controlPoints = {
    {-16.229, 0.000, 3.500},
    {-16.789, 0.000, 3.701},
    {-17.403, 0.000, 3.997},
    {-17.777, 0.000, 4.713}
};

// Curve 9
curves[9].controlPoints = {
    {-17.777, 0.000, 4.713},
    {-17.777, 0.000, 4.713},
    {-17.777, 0.000, 4.713}
};

// Curve 10
curves[10].controlPoints = {
    {-17.777, 0.000, 4.713},
    {-17.777, 0.000, 4.715},
    {-17.778, 0.000, 4.716},
    {-17.779, 0.000, 4.717}
};

// Curve 11
curves[11].controlPoints = {
    {-17.779, 0.000, 4.717},
    {-18.116, 0.000, 5.348},
    {-18.016, 0.000, 6.074},
    {-17.913, 0.000, 6.747}
};

// Curve 12
curves[12].controlPoints = {
    {-17.913, 0.000, 6.747},
    {-17.810, 0.000, 7.421},
    {-17.705, 0.000, 8.057},
    {-17.844, 0.000, 8.488}
};

// Curve 13
curves[13].controlPoints = {
    {-17.844, 0.000, 8.488},
    {-18.287, 0.000, 9.699},
    {-18.344, 0.000, 10.537},
    {-18.032, 0.000, 11.145}
};

// Curve 14
curves[14].controlPoints = {
    {-18.032, 0.000, 11.145},
    {-17.719, 0.000, 11.754},
    {-17.076, 0.000, 12.023},
    {-16.350, 0.000, 12.174}
};

// Curve 15
curves[15].controlPoints = {
    {-16.350, 0.000, 12.174},
    {-14.897, 0.000, 12.478},
    {-12.929, 0.000, 12.402},
    {-11.379, 0.000, 13.224}
};

// Curve 16
curves[16].controlPoints = {
    {-11.379, 0.000, 13.224},
    {-11.334, 0.000, 13.140},
    {-11.290, 0.000, 13.057},
    {-11.246, 0.000, 12.973}
};

// Curve 17
curves[17].controlPoints = {
    {-11.246, 0.000, 12.973},
    {-11.289, 0.000, 13.057},
    {-11.333, 0.000, 13.141},
    {-11.377, 0.000, 13.225}
};

// Curve 18
curves[18].controlPoints = {
    {-11.377, 0.000, 13.225},
    {-9.717, 0.000, 14.093},
    {-8.034, 0.000, 14.401},
    {-6.692, 0.000, 14.094}
};

// Curve 19
curves[19].controlPoints = {
    {-6.692, 0.000, 14.094},
    {-5.718, 0.000, 13.872},
    {-4.928, 0.000, 13.292},
    {-4.522, 0.000, 12.400}
};

// Curve 20
curves[20].controlPoints = {
    {-4.522, 0.000, 12.400},
    {-3.472, 0.000, 12.395},
    {-2.319, 0.000, 11.951},
    {-0.473, 0.000, 11.849}
};

// Curve 21
curves[21].controlPoints = {
    {-0.473, 0.000, 11.849},
    {0.779, 0.000, 11.748},
    {2.343, 0.000, 12.294},
    {4.143, 0.000, 12.194}
};

// Curve 22
curves[22].controlPoints = {
    {4.143, 0.000, 12.194},
    {4.190, 0.000, 12.389},
    {4.258, 0.000, 12.577},
    {4.351, 0.000, 12.755}
};

// Curve 23
curves[23].controlPoints = {
    {4.351, 0.000, 12.755},
    {4.352, 0.000, 12.757},
    {4.353, 0.000, 12.759},
    {4.354, 0.000, 12.761}
};

// Curve 24
curves[24].controlPoints = {
    {4.354, 0.000, 12.761},
    {5.052, 0.000, 14.155},
    {6.348, 0.000, 14.794},
    {7.729, 0.000, 14.685}
};

// Curve 25
curves[25].controlPoints = {
    {7.729, 0.000, 14.685},
    {9.112, 0.000, 14.575},
    {10.583, 0.000, 13.760},
    {11.771, 0.000, 12.345}
};

// Curve 26
curves[26].controlPoints = {
    {11.771, 0.000, 12.345},
    {11.699, 0.000, 12.284},
    {11.626, 0.000, 12.223},
    {11.553, 0.000, 12.162}
};

// Curve 27
curves[27].controlPoints = {
    {11.553, 0.000, 12.162},
    {11.627, 0.000, 12.222},
    {11.700, 0.000, 12.283},
    {11.773, 0.000, 12.343}
};

// Curve 28
curves[28].controlPoints = {
    {11.773, 0.000, 12.343},
    {12.906, 0.000, 10.969},
    {14.787, 0.000, 10.400},
    {16.034, 0.000, 9.648}
};

// Curve 29
curves[29].controlPoints = {
    {16.034, 0.000, 9.648},
    {16.657, 0.000, 9.272},
    {17.163, 0.000, 8.801},
    {17.202, 0.000, 8.117}
};

// Curve 30
curves[30].controlPoints = {
    {17.202, 0.000, 8.117},
    {17.241, 0.000, 7.433},
    {16.840, 0.000, 6.667},
    {15.917, 0.000, 5.643}
};

// Curve 31
curves[31].controlPoints = {
    {15.917, 0.000, 5.643},
    {15.917, 0.000, 5.642}
};

// Curve 32
curves[32].controlPoints = {
    {15.917, 0.000, 5.642},
    {15.916, 0.000, 5.642},
    {15.916, 0.000, 5.642},
    {15.916, 0.000, 5.641}
};

// Curve 33
curves[33].controlPoints = {
    {15.916, 0.000, 5.641},
    {15.612, 0.000, 5.299},
    {15.468, 0.000, 4.663},
    {15.312, 0.000, 3.987}
};

// Curve 34
curves[34].controlPoints = {
    {15.312, 0.000, 3.987},
    {15.157, 0.000, 3.311},
    {14.984, 0.000, 2.582},
    {14.429, 0.000, 2.110}
};

// Curve 35
curves[35].controlPoints = {
    {14.429, 0.000, 2.110},
    {14.428, 0.000, 2.109},
    {14.427, 0.000, 2.108},
    {14.425, 0.000, 2.107}
};

// Curve 36
curves[36].controlPoints = {
    {14.425, 0.000, 2.107},
    {14.205, 0.000, 1.914},
    {13.975, 0.000, 1.784},
    {13.744, 0.000, 1.696}
};

// Curve 37
curves[37].controlPoints = {
    {13.744, 0.000, 1.696},
    {14.515, 0.000, -0.592},
    {14.213, 0.000, -2.870},
    {13.434, 0.000, -4.929}
};

// Curve 38
curves[38].controlPoints = {
    {13.434, 0.000, -4.929},
    {12.478, 0.000, -7.456},
    {10.810, 0.000, -9.657},
    {9.535, 0.000, -11.163}
};

// Curve 39
curves[39].controlPoints = {
    {9.535, 0.000, -11.163},
    {8.109, 0.000, -12.963},
    {6.714, 0.000, -14.671},
    {6.741, 0.000, -17.194}
};

// Curve 40
curves[40].controlPoints = {
    {6.741, 0.000, -17.194},
    {6.784, 0.000, -21.044},
    {7.165, 0.000, -28.185},
    {0.389, 0.000, -28.194}
};

// Curve 41
curves[41].controlPoints = {
    {0.389, 0.000, -28.194},
    {0.389, 0.000, -28.194},
    {0.389, 0.000, -28.194},
    {0.389, 0.000, -28.194}
};

// Curve 42
curves[42].controlPoints = {
    {1.306, 0.000, -22.262},
    {1.691, 0.000, -22.262},
    {2.019, 0.000, -22.149},
    {2.357, 0.000, -21.905}
};

// Curve 43
curves[43].controlPoints = {
    {2.357, 0.000, -21.905},
    {2.699, 0.000, -21.656},
    {2.946, 0.000, -21.345},
    {3.145, 0.000, -20.910}
};

// Curve 44
curves[44].controlPoints = {
    {3.145, 0.000, -20.910},
    {3.340, 0.000, -20.486},
    {3.434, 0.000, -20.071},
    {3.444, 0.000, -19.580}
};

// Curve 45
curves[45].controlPoints = {
    {3.444, 0.000, -19.580},
    {3.444, 0.000, -19.567},
    {3.444, 0.000, -19.556},
    {3.447, 0.000, -19.543}
};

// Curve 46
curves[46].controlPoints = {
    {3.447, 0.000, -19.543},
    {3.451, 0.000, -19.038},
    {3.364, 0.000, -18.609},
    {3.173, 0.000, -18.170}
};

// Curve 47
curves[47].controlPoints = {
    {3.173, 0.000, -18.170},
    {3.063, 0.000, -17.920},
    {2.938, 0.000, -17.710},
    {2.788, 0.000, -17.528}
};

// Curve 48
curves[48].controlPoints = {
    {2.788, 0.000, -17.528},
    {2.737, 0.000, -17.553},
    {2.684, 0.000, -17.576},
    {2.629, 0.000, -17.599}
};

// Curve 49
curves[49].controlPoints = {
    {2.629, 0.000, -17.599},
    {2.248, 0.000, -17.762},
    {1.956, 0.000, -17.866},
    {1.712, 0.000, -17.950}
};

// Curve 50
curves[50].controlPoints = {
    {1.712, 0.000, -17.950},
    {1.800, 0.000, -18.056},
    {1.874, 0.000, -18.182},
    {1.938, 0.000, -18.340}
};

// Curve 51
curves[51].controlPoints = {
    {1.938, 0.000, -18.340},
    {2.036, 0.000, -18.577},
    {2.084, 0.000, -18.809},
    {2.093, 0.000, -19.086}
};

// Curve 52
curves[52].controlPoints = {
    {2.093, 0.000, -19.086},
    {2.093, 0.000, -19.097},
    {2.097, 0.000, -19.106},
    {2.097, 0.000, -19.119}
};

// Curve 53
curves[53].controlPoints = {
    {2.097, 0.000, -19.119},
    {2.102, 0.000, -19.384},
    {2.067, 0.000, -19.611},
    {1.990, 0.000, -19.843}
};

// Curve 54
curves[54].controlPoints = {
    {1.990, 0.000, -19.843},
    {1.909, 0.000, -20.086},
    {1.806, 0.000, -20.261},
    {1.657, 0.000, -20.407}
};

// Curve 55
curves[55].controlPoints = {
    {1.657, 0.000, -20.407},
    {1.507, 0.000, -20.552},
    {1.358, 0.000, -20.619},
    {1.179, 0.000, -20.624}
};

// Curve 56
curves[56].controlPoints = {
    {1.179, 0.000, -20.624},
    {1.171, 0.000, -20.625},
    {1.163, 0.000, -20.625},
    {1.154, 0.000, -20.625}
};

// Curve 57
curves[57].controlPoints = {
    {1.154, 0.000, -20.625},
    {0.986, 0.000, -20.624},
    {0.840, 0.000, -20.566},
    {0.689, 0.000, -20.440}
};

// Curve 58
curves[58].controlPoints = {
    {0.689, 0.000, -20.440},
    {0.531, 0.000, -20.307},
    {0.413, 0.000, -20.138},
    {0.315, 0.000, -19.902}
};

// Curve 59
curves[59].controlPoints = {
    {0.315, 0.000, -19.902},
    {0.218, 0.000, -19.666},
    {0.170, 0.000, -19.432},
    {0.161, 0.000, -19.154}
};

// Curve 60
curves[60].controlPoints = {
    {0.161, 0.000, -19.154},
    {0.159, 0.000, -19.143},
    {0.159, 0.000, -19.134},
    {0.159, 0.000, -19.123}
};

// Curve 61
curves[61].controlPoints = {
    {0.159, 0.000, -19.123},
    {0.156, 0.000, -18.970},
    {0.165, 0.000, -18.830},
    {0.189, 0.000, -18.694}
};

// Curve 62
curves[62].controlPoints = {
    {0.189, 0.000, -18.694},
    {-0.155, 0.000, -18.865},
    {-0.479, 0.000, -18.982},
    {-0.783, 0.000, -19.054}
};

// Curve 63
curves[63].controlPoints = {
    {-0.783, 0.000, -19.054},
    {-0.801, 0.000, -19.186},
    {-0.811, 0.000, -19.322},
    {-0.814, 0.000, -19.464}
};

// Curve 64
curves[64].controlPoints = {
    {-0.814, 0.000, -19.464},
    {-0.814, 0.000, -19.477},
    {-0.814, 0.000, -19.489},
    {-0.814, 0.000, -19.502}
};

// Curve 65
curves[65].controlPoints = {
    {-0.814, 0.000, -19.502},
    {-0.820, 0.000, -20.005},
    {-0.737, 0.000, -20.436},
    {-0.543, 0.000, -20.875}
};

// Curve 66
curves[66].controlPoints = {
    {-0.543, 0.000, -20.875},
    {-0.350, 0.000, -21.313},
    {-0.110, 0.000, -21.628},
    {0.227, 0.000, -21.884}
};

// Curve 67
curves[67].controlPoints = {
    {0.227, 0.000, -21.884},
    {0.564, 0.000, -22.141},
    {0.896, 0.000, -22.258},
    {1.288, 0.000, -22.262}
};

// Curve 68
curves[68].controlPoints = {
    {1.288, 0.000, -22.262},
    {1.294, 0.000, -22.262},
    {1.300, 0.000, -22.262},
    {1.306, 0.000, -22.262}
};

// Curve 69
curves[69].controlPoints = {
    {1.306, 0.000, -22.262},
    {1.306, 0.000, -22.262},
    {1.306, 0.000, -22.262},
    {1.306, 0.000, -22.262}
};

// Curve 70
curves[70].controlPoints = {
    {-3.956, 0.000, -21.845},
    {-3.701, 0.000, -21.845},
    {-3.473, 0.000, -21.760},
    {-3.237, 0.000, -21.571}
};

// Curve 71
curves[71].controlPoints = {
    {-3.237, 0.000, -21.571},
    {-2.981, 0.000, -21.367},
    {-2.787, 0.000, -21.105},
    {-2.625, 0.000, -20.737}
};

// Curve 72
curves[72].controlPoints = {
    {-2.625, 0.000, -20.737},
    {-2.463, 0.000, -20.368},
    {-2.376, 0.000, -20.000},
    {-2.352, 0.000, -19.565}
};

// Curve 73
curves[73].controlPoints = {
    {-2.352, 0.000, -19.565},
    {-2.352, 0.000, -19.564},
    {-2.352, 0.000, -19.562},
    {-2.352, 0.000, -19.561}
};

// Curve 74
curves[74].controlPoints = {
    {-2.352, 0.000, -19.561},
    {-2.341, 0.000, -19.379},
    {-2.342, 0.000, -19.207},
    {-2.357, 0.000, -19.040}
};

// Curve 75
curves[75].controlPoints = {
    {-2.357, 0.000, -19.040},
    {-2.407, 0.000, -19.026},
    {-2.456, 0.000, -19.010},
    {-2.505, 0.000, -18.994}
};

// Curve 76
curves[76].controlPoints = {
    {-2.505, 0.000, -18.994},
    {-2.780, 0.000, -18.899},
    {-3.022, 0.000, -18.770},
    {-3.234, 0.000, -18.630}
};

// Curve 77
curves[77].controlPoints = {
    {-3.234, 0.000, -18.630},
    {-3.213, 0.000, -18.776},
    {-3.210, 0.000, -18.924},
    {-3.226, 0.000, -19.090}
};

// Curve 78
curves[78].controlPoints = {
    {-3.226, 0.000, -19.090},
    {-3.228, 0.000, -19.099},
    {-3.228, 0.000, -19.106},
    {-3.228, 0.000, -19.115}
};

// Curve 79
curves[79].controlPoints = {
    {-3.228, 0.000, -19.115},
    {-3.250, 0.000, -19.335},
    {-3.296, 0.000, -19.519},
    {-3.373, 0.000, -19.705}
};

// Curve 80
curves[80].controlPoints = {
    {-3.373, 0.000, -19.705},
    {-3.456, 0.000, -19.898},
    {-3.548, 0.000, -20.035},
    {-3.670, 0.000, -20.140}
};

// Curve 81
curves[81].controlPoints = {
    {-3.670, 0.000, -20.140},
    {-3.780, 0.000, -20.235},
    {-3.884, 0.000, -20.279},
    {-3.999, 0.000, -20.278}
};

// Curve 82
curves[82].controlPoints = {
    {-3.999, 0.000, -20.278},
    {-4.010, 0.000, -20.278},
    {-4.022, 0.000, -20.277},
    {-4.034, 0.000, -20.276}
};

// Curve 83
curves[83].controlPoints = {
    {-4.034, 0.000, -20.276},
    {-4.163, 0.000, -20.265},
    {-4.270, 0.000, -20.202},
    {-4.372, 0.000, -20.079}
};

// Curve 84
curves[84].controlPoints = {
    {-4.372, 0.000, -20.079},
    {-4.473, 0.000, -19.955},
    {-4.539, 0.000, -19.803},
    {-4.587, 0.000, -19.600}
};

// Curve 85
curves[85].controlPoints = {
    {-4.587, 0.000, -19.600},
    {-4.635, 0.000, -19.397},
    {-4.648, 0.000, -19.198},
    {-4.628, 0.000, -18.970}
};

// Curve 86
curves[86].controlPoints = {
    {-4.628, 0.000, -18.970},
    {-4.628, 0.000, -18.961},
    {-4.626, 0.000, -18.953},
    {-4.626, 0.000, -18.944}
};

// Curve 87
curves[87].controlPoints = {
    {-4.626, 0.000, -18.944},
    {-4.604, 0.000, -18.723},
    {-4.560, 0.000, -18.539},
    {-4.480, 0.000, -18.353}
};

// Curve 88
curves[88].controlPoints = {
    {-4.480, 0.000, -18.353},
    {-4.399, 0.000, -18.161},
    {-4.305, 0.000, -18.025},
    {-4.184, 0.000, -17.920}
};

// Curve 89
curves[89].controlPoints = {
    {-4.184, 0.000, -17.920},
    {-4.163, 0.000, -17.902},
    {-4.143, 0.000, -17.886},
    {-4.123, 0.000, -17.872}
};

// Curve 90
curves[90].controlPoints = {
    {-4.123, 0.000, -17.872},
    {-4.250, 0.000, -17.775},
    {-4.310, 0.000, -17.730},
    {-4.414, 0.000, -17.654}
};

// Curve 91
curves[91].controlPoints = {
    {-4.414, 0.000, -17.654},
    {-4.480, 0.000, -17.605},
    {-4.559, 0.000, -17.547},
    {-4.651, 0.000, -17.479}
};

// Curve 92
curves[92].controlPoints = {
    {-4.651, 0.000, -17.479},
    {-4.852, 0.000, -17.667},
    {-5.009, 0.000, -17.903},
    {-5.145, 0.000, -18.214}
};

// Curve 93
curves[93].controlPoints = {
    {-5.145, 0.000, -18.214},
    {-5.308, 0.000, -18.583},
    {-5.394, 0.000, -18.951},
    {-5.420, 0.000, -19.386}
};

// Curve 94
curves[94].controlPoints = {
    {-5.420, 0.000, -19.386},
    {-5.420, 0.000, -19.387},
    {-5.420, 0.000, -19.389},
    {-5.420, 0.000, -19.390}
};

// Curve 95
curves[95].controlPoints = {
    {-5.420, 0.000, -19.390},
    {-5.444, 0.000, -19.825},
    {-5.401, 0.000, -20.199},
    {-5.282, 0.000, -20.586}
};

// Curve 96
curves[96].controlPoints = {
    {-5.282, 0.000, -20.586},
    {-5.162, 0.000, -20.972},
    {-5.002, 0.000, -21.252},
    {-4.770, 0.000, -21.483}
};

// Curve 97
curves[97].controlPoints = {
    {-4.770, 0.000, -21.483},
    {-4.537, 0.000, -21.713},
    {-4.303, 0.000, -21.829},
    {-4.022, 0.000, -21.844}
};

// Curve 98
curves[98].controlPoints = {
    {-4.022, 0.000, -21.844},
    {-4.000, 0.000, -21.845},
    {-3.978, 0.000, -21.846},
    {-3.956, 0.000, -21.845}
};

// Curve 99
curves[99].controlPoints = {
    {-3.956, 0.000, -21.845},
    {-3.956, 0.000, -21.845},
    {-3.956, 0.000, -21.845},
    {-3.956, 0.000, -21.845}
};

// Curve 100
curves[100].controlPoints = {
    {-1.563, 0.000, -18.829},
    {-0.970, 0.000, -18.831},
    {-0.256, 0.000, -18.637},
    {0.607, 0.000, -18.082}
};

// Curve 101
curves[101].controlPoints = {
    {0.607, 0.000, -18.082},
    {1.138, 0.000, -17.737},
    {1.550, 0.000, -17.708},
    {2.501, 0.000, -17.301}
};

// Curve 102
curves[102].controlPoints = {
    {2.501, 0.000, -17.301},
    {2.501, 0.000, -17.301},
    {2.502, 0.000, -17.300},
    {2.502, 0.000, -17.300}
};

// Curve 103
curves[103].controlPoints = {
    {2.502, 0.000, -17.300},
    {2.502, 0.000, -17.300},
    {2.503, 0.000, -17.300},
    {2.503, 0.000, -17.300}
};

// Curve 104
curves[104].controlPoints = {
    {2.503, 0.000, -17.300},
    {2.961, 0.000, -17.112},
    {3.229, 0.000, -16.868},
    {3.360, 0.000, -16.610}
};

// Curve 105
curves[105].controlPoints = {
    {3.360, 0.000, -16.610},
    {3.491, 0.000, -16.352},
    {3.495, 0.000, -16.073},
    {3.385, 0.000, -15.779}
};

// Curve 106
curves[106].controlPoints = {
    {3.385, 0.000, -15.779},
    {3.165, 0.000, -15.192},
    {2.464, 0.000, -14.573},
    {1.481, 0.000, -14.266}
};

// Curve 107
curves[107].controlPoints = {
    {1.481, 0.000, -14.266},
    {1.481, 0.000, -14.266},
    {1.481, 0.000, -14.266},
    {1.480, 0.000, -14.266}
};

// Curve 108
curves[108].controlPoints = {
    {1.480, 0.000, -14.266},
    {1.480, 0.000, -14.266},
    {1.480, 0.000, -14.266},
    {1.479, 0.000, -14.265}
};

// Curve 109
curves[109].controlPoints = {
    {1.479, 0.000, -14.265},
    {1.000, 0.000, -14.110},
    {0.582, 0.000, -13.766},
    {0.089, 0.000, -13.484}
};

// Curve 110
curves[110].controlPoints = {
    {0.089, 0.000, -13.484},
    {-0.404, 0.000, -13.202},
    {-0.962, 0.000, -12.975},
    {-1.720, 0.000, -13.018}
};

// Curve 111
curves[111].controlPoints = {
    {-1.720, 0.000, -13.018},
    {-2.366, 0.000, -13.056},
    {-2.752, 0.000, -13.275},
    {-3.101, 0.000, -13.556}
};

// Curve 112
curves[112].controlPoints = {
    {-3.101, 0.000, -13.556},
    {-3.450, 0.000, -13.836},
    {-3.754, 0.000, -14.189},
    {-4.199, 0.000, -14.450}
};

// Curve 113
curves[113].controlPoints = {
    {-4.199, 0.000, -14.450},
    {-4.200, 0.000, -14.450},
    {-4.200, 0.000, -14.450},
    {-4.200, 0.000, -14.451}
};

// Curve 114
curves[114].controlPoints = {
    {-4.200, 0.000, -14.451},
    {-4.201, 0.000, -14.451},
    {-4.201, 0.000, -14.451},
    {-4.201, 0.000, -14.451}
};

// Curve 115
curves[115].controlPoints = {
    {-4.201, 0.000, -14.451},
    {-4.919, 0.000, -14.857},
    {-5.310, 0.000, -15.326},
    {-5.433, 0.000, -15.732}
};

// Curve 116
curves[116].controlPoints = {
    {-5.433, 0.000, -15.732},
    {-5.556, 0.000, -16.139},
    {-5.441, 0.000, -16.486},
    {-5.085, 0.000, -16.752}
};

// Curve 117
curves[117].controlPoints = {
    {-5.085, 0.000, -16.752},
    {-4.685, 0.000, -17.052},
    {-4.407, 0.000, -17.256},
    {-4.222, 0.000, -17.392}
};

// Curve 118
curves[118].controlPoints = {
    {-4.222, 0.000, -17.392},
    {-4.038, 0.000, -17.527},
    {-3.961, 0.000, -17.578},
    {-3.903, 0.000, -17.634}
};

// Curve 119
curves[119].controlPoints = {
    {-3.903, 0.000, -17.634},
    {-3.902, 0.000, -17.634},
    {-3.902, 0.000, -17.635},
    {-3.902, 0.000, -17.635}
};

// Curve 120
curves[120].controlPoints = {
    {-3.902, 0.000, -17.635},
    {-3.902, 0.000, -17.635}
};

// Curve 121
curves[121].controlPoints = {
    {-3.902, 0.000, -17.635},
    {-3.601, 0.000, -17.920},
    {-3.122, 0.000, -18.438},
    {-2.399, 0.000, -18.687}
};

// Curve 122
curves[122].controlPoints = {
    {-2.399, 0.000, -18.687},
    {-2.151, 0.000, -18.773},
    {-1.873, 0.000, -18.828},
    {-1.563, 0.000, -18.829}
};

// Curve 123
curves[123].controlPoints = {
    {-1.563, 0.000, -18.829},
    {-1.563, 0.000, -18.829},
    {-1.563, 0.000, -18.829},
    {-1.563, 0.000, -18.829}
};

// Curve 124
curves[124].controlPoints = {
    {2.583, 0.000, -16.377},
    {2.327, 0.000, -16.362},
    {2.059, 0.000, -16.230},
    {1.747, 0.000, -16.056}
};

// Curve 125
curves[125].controlPoints = {
    {1.747, 0.000, -16.056},
    {1.435, 0.000, -15.882},
    {1.084, 0.000, -15.659},
    {0.704, 0.000, -15.440}
};

// Curve 126
curves[126].controlPoints = {
    {0.704, 0.000, -15.440},
    {-0.056, 0.000, -15.001},
    {-0.932, 0.000, -14.581},
    {-1.802, 0.000, -14.581}
};

// Curve 127
curves[127].controlPoints = {
    {-1.802, 0.000, -14.581},
    {-2.673, 0.000, -14.581},
    {-3.369, 0.000, -14.984},
    {-3.891, 0.000, -15.397}
};

// Curve 128
curves[128].controlPoints = {
    {-3.891, 0.000, -15.397},
    {-4.152, 0.000, -15.604},
    {-4.367, 0.000, -15.813},
    {-4.540, 0.000, -15.974}
};

// Curve 129
curves[129].controlPoints = {
    {-4.540, 0.000, -15.974},
    {-4.626, 0.000, -16.054},
    {-4.700, 0.000, -16.123},
    {-4.770, 0.000, -16.177}
};

// Curve 130
curves[130].controlPoints = {
    {-4.770, 0.000, -16.177},
    {-4.840, 0.000, -16.230},
    {-4.896, 0.000, -16.284},
    {-5.013, 0.000, -16.284}
};

// Curve 131
curves[131].controlPoints = {
    {-5.013, 0.000, -16.284},
    {-5.015, 0.000, -16.202},
    {-5.016, 0.000, -16.120},
    {-5.018, 0.000, -16.039}
};

// Curve 132
curves[132].controlPoints = {
    {-5.018, 0.000, -16.039},
    {-5.027, 0.000, -15.957},
    {-5.035, 0.000, -15.959},
    {-5.032, 0.000, -15.957}
};

// Curve 133
curves[133].controlPoints = {
    {-5.032, 0.000, -15.957},
    {-5.026, 0.000, -15.954},
    {-4.999, 0.000, -15.944},
    {-4.968, 0.000, -15.920}
};

// Curve 134
curves[134].controlPoints = {
    {-4.968, 0.000, -15.920},
    {-4.917, 0.000, -15.880},
    {-4.846, 0.000, -15.816},
    {-4.761, 0.000, -15.737}
};

// Curve 135
curves[135].controlPoints = {
    {-4.761, 0.000, -15.737},
    {-4.592, 0.000, -15.579},
    {-4.368, 0.000, -15.362},
    {-4.092, 0.000, -15.143}
};

// Curve 136
curves[136].controlPoints = {
    {-4.092, 0.000, -15.143},
    {-3.540, 0.000, -14.706},
    {-2.773, 0.000, -14.257},
    {-1.802, 0.000, -14.257}
};

// Curve 137
curves[137].controlPoints = {
    {-1.802, 0.000, -14.257},
    {-0.829, 0.000, -14.257},
    {0.091, 0.000, -14.712},
    {0.866, 0.000, -15.159}
};

// Curve 138
curves[138].controlPoints = {
    {0.866, 0.000, -15.159},
    {1.254, 0.000, -15.383},
    {1.606, 0.000, -15.606},
    {1.905, 0.000, -15.773}
};

// Curve 139
curves[139].controlPoints = {
    {1.905, 0.000, -15.773},
    {2.204, 0.000, -15.940},
    {2.456, 0.000, -16.045},
    {2.602, 0.000, -16.054}
};

// Curve 140
curves[140].controlPoints = {
    {2.602, 0.000, -16.054},
    {2.596, 0.000, -16.162},
    {2.589, 0.000, -16.269},
    {2.583, 0.000, -16.377}
};

// Curve 141
curves[141].controlPoints = {
    {3.447, 0.000, -15.214},
    {4.093, 0.000, -12.668},
    {5.595, 0.000, -8.990},
    {6.561, 0.000, -7.195}
};

// Curve 142
curves[142].controlPoints = {
    {6.561, 0.000, -7.195},
    {7.074, 0.000, -6.243},
    {8.095, 0.000, -4.220},
    {8.536, 0.000, -1.783}
};

// Curve 143
curves[143].controlPoints = {
    {8.536, 0.000, -1.783},
    {8.816, 0.000, -1.791},
    {9.124, 0.000, -1.751},
    {9.453, 0.000, -1.666}
};

// Curve 144
curves[144].controlPoints = {
    {9.453, 0.000, -1.666},
    {10.607, 0.000, -4.657},
    {8.475, 0.000, -7.877},
    {7.500, 0.000, -8.774}
};

// Curve 145
curves[145].controlPoints = {
    {7.500, 0.000, -8.774},
    {7.107, 0.000, -9.156},
    {7.088, 0.000, -9.327},
    {7.283, 0.000, -9.319}
};

// Curve 146
curves[146].controlPoints = {
    {7.283, 0.000, -9.319},
    {8.341, 0.000, -8.384},
    {9.729, 0.000, -6.503},
    {10.234, 0.000, -4.380}
};

// Curve 147
curves[147].controlPoints = {
    {10.234, 0.000, -4.380},
    {10.464, 0.000, -3.412},
    {10.513, 0.000, -2.394},
    {10.266, 0.000, -1.390}
};

// Curve 148
curves[148].controlPoints = {
    {10.266, 0.000, -1.390},
    {10.387, 0.000, -1.340},
    {10.510, 0.000, -1.285},
    {10.634, 0.000, -1.226}
};

// Curve 149
curves[149].controlPoints = {
    {10.634, 0.000, -1.226},
    {12.486, 0.000, -0.325},
    {13.170, 0.000, 0.459},
    {12.841, 0.000, 1.529}
};

// Curve 150
curves[150].controlPoints = {
    {12.841, 0.000, 1.529},
    {12.733, 0.000, 1.525},
    {12.626, 0.000, 1.526},
    {12.522, 0.000, 1.528}
};

// Curve 151
curves[151].controlPoints = {
    {12.522, 0.000, 1.528},
    {12.512, 0.000, 1.529},
    {12.503, 0.000, 1.529},
    {12.493, 0.000, 1.529}
};

// Curve 152
curves[152].controlPoints = {
    {12.493, 0.000, 1.529},
    {12.761, 0.000, 0.681},
    {12.167, 0.000, 0.055},
    {10.585, 0.000, -0.661}
};

// Curve 153
curves[153].controlPoints = {
    {10.585, 0.000, -0.661},
    {8.943, 0.000, -1.383},
    {7.635, 0.000, -1.311},
    {7.414, 0.000, 0.154}
};

// Curve 154
curves[154].controlPoints = {
    {7.414, 0.000, 0.154},
    {7.400, 0.000, 0.230},
    {7.388, 0.000, 0.309},
    {7.380, 0.000, 0.388}
};

// Curve 155
curves[155].controlPoints = {
    {7.380, 0.000, 0.388},
    {7.257, 0.000, 0.431},
    {7.134, 0.000, 0.485},
    {7.010, 0.000, 0.553}
};

// Curve 156
curves[156].controlPoints = {
    {7.010, 0.000, 0.553},
    {6.239, 0.000, 0.974},
    {5.819, 0.000, 1.740},
    {5.585, 0.000, 2.678}
};

// Curve 157
curves[157].controlPoints = {
    {5.585, 0.000, 2.678},
    {5.351, 0.000, 3.616},
    {5.284, 0.000, 4.750},
    {5.219, 0.000, 6.024}
};

// Curve 158
curves[158].controlPoints = {
    {5.219, 0.000, 6.024},
    {5.219, 0.000, 6.025},
    {5.219, 0.000, 6.025},
    {5.219, 0.000, 6.025}
};

// Curve 159
curves[159].controlPoints = {
    {5.219, 0.000, 6.025},
    {5.180, 0.000, 6.666},
    {4.916, 0.000, 7.533},
    {4.649, 0.000, 8.451}
};

// Curve 160
curves[160].controlPoints = {
    {4.649, 0.000, 8.451},
    {1.960, 0.000, 10.369},
    {-1.772, 0.000, 11.200},
    {-4.941, 0.000, 9.038}
};

// Curve 161
curves[161].controlPoints = {
    {-4.941, 0.000, 9.038},
    {-5.156, 0.000, 8.698},
    {-5.402, 0.000, 8.361},
    {-5.656, 0.000, 8.029}
};

// Curve 162
curves[162].controlPoints = {
    {-5.656, 0.000, 8.029},
    {-5.818, 0.000, 7.817},
    {-5.984, 0.000, 7.607},
    {-6.149, 0.000, 7.399}
};

// Curve 163
curves[163].controlPoints = {
    {-6.149, 0.000, 7.399},
    {-5.824, 0.000, 7.399},
    {-5.547, 0.000, 7.346},
    {-5.324, 0.000, 7.244}
};

// Curve 164
curves[164].controlPoints = {
    {-5.324, 0.000, 7.244},
    {-5.046, 0.000, 7.118},
    {-4.851, 0.000, 6.916},
    {-4.754, 0.000, 6.656}
};

// Curve 165
curves[165].controlPoints = {
    {-4.754, 0.000, 6.656},
    {-4.561, 0.000, 6.137},
    {-4.755, 0.000, 5.404},
    {-5.374, 0.000, 4.566}
};

// Curve 166
curves[166].controlPoints = {
    {-5.374, 0.000, 4.566},
    {-5.992, 0.000, 3.729},
    {-7.039, 0.000, 2.785},
    {-8.578, 0.000, 1.841}
};

// Curve 167
curves[167].controlPoints = {
    {-8.578, 0.000, 1.841},
    {-8.579, 0.000, 1.841},
    {-8.579, 0.000, 1.840}
};

// Curve 168
curves[168].controlPoints = {
    {-8.579, 0.000, 1.840},
    {-9.709, 0.000, 1.137},
    {-10.342, 0.000, 0.275},
    {-10.638, 0.000, -0.661}
};

// Curve 169
curves[169].controlPoints = {
    {-10.638, 0.000, -0.661},
    {-10.934, 0.000, -1.598},
    {-10.892, 0.000, -2.610},
    {-10.664, 0.000, -3.610}
};

// Curve 170
curves[170].controlPoints = {
    {-10.664, 0.000, -3.610},
    {-10.226, 0.000, -5.528},
    {-9.101, 0.000, -7.394},
    {-8.383, 0.000, -8.565}
};

// Curve 171
curves[171].controlPoints = {
    {-8.383, 0.000, -8.565},
    {-8.190, 0.000, -8.707},
    {-8.314, 0.000, -8.301},
    {-9.110, 0.000, -6.823}
};

// Curve 172
curves[172].controlPoints = {
    {-9.110, 0.000, -6.823},
    {-9.823, 0.000, -5.472},
    {-11.157, 0.000, -2.354},
    {-9.331, 0.000, 0.080}
};

// Curve 173
curves[173].controlPoints = {
    {-9.331, 0.000, 0.080},
    {-9.282, 0.000, -1.652},
    {-8.869, 0.000, -3.419},
    {-8.174, 0.000, -5.071}
};

// Curve 174
curves[174].controlPoints = {
    {-8.174, 0.000, -5.071},
    {-7.163, 0.000, -7.364},
    {-5.047, 0.000, -11.340},
    {-4.879, 0.000, -14.509}
};

// Curve 175
curves[175].controlPoints = {
    {-4.879, 0.000, -14.509},
    {-4.792, 0.000, -14.446},
    {-4.495, 0.000, -14.245},
    {-4.362, 0.000, -14.170}
};

// Curve 176
curves[176].controlPoints = {
    {-4.362, 0.000, -14.170},
    {-4.362, 0.000, -14.169},
    {-4.361, 0.000, -14.169},
    {-4.361, 0.000, -14.169}
};

// Curve 177
curves[177].controlPoints = {
    {-4.361, 0.000, -14.169},
    {-3.973, 0.000, -13.941},
    {-3.681, 0.000, -13.607},
    {-3.304, 0.000, -13.303}
};

// Curve 178
curves[178].controlPoints = {
    {-3.304, 0.000, -13.303},
    {-2.926, 0.000, -12.999},
    {-2.453, 0.000, -12.737},
    {-1.739, 0.000, -12.695}
};

// Curve 179
curves[179].controlPoints = {
    {-1.739, 0.000, -12.695},
    {-0.906, 0.000, -12.647},
    {-0.271, 0.000, -12.905},
    {0.250, 0.000, -13.203}
};

// Curve 180
curves[180].controlPoints = {
    {0.250, 0.000, -13.203},
    {0.769, 0.000, -13.500},
    {1.185, 0.000, -13.828},
    {1.578, 0.000, -13.957}
};

// Curve 181
curves[181].controlPoints = {
    {1.578, 0.000, -13.957},
    {1.578, 0.000, -13.957},
    {1.579, 0.000, -13.957},
    {1.580, 0.000, -13.957}
};

// Curve 182
curves[182].controlPoints = {
    {1.580, 0.000, -13.957},
    {2.411, 0.000, -14.217},
    {3.071, 0.000, -14.677},
    {3.447, 0.000, -15.214}
};

// Curve 183
curves[183].controlPoints = {
    {3.447, 0.000, -15.214},
    {3.447, 0.000, -15.214},
    {3.447, 0.000, -15.214},
    {3.447, 0.000, -15.214}
};

// Curve 184
curves[184].controlPoints = {
    {8.725, 0.000, -0.637},
    {9.062, 0.000, -0.639},
    {9.469, 0.000, -0.527},
    {9.916, 0.000, -0.327}
};

// Curve 185
curves[185].controlPoints = {
    {9.916, 0.000, -0.327},
    {11.117, 0.000, 0.227},
    {11.493, 0.000, 0.703},
    {11.165, 0.000, 1.401}
};

// Curve 186
curves[186].controlPoints = {
    {11.165, 0.000, 1.401},
    {10.889, 0.000, 1.928},
    {9.706, 0.000, 2.766},
    {8.895, 0.000, 2.545}
};

// Curve 187
curves[187].controlPoints = {
    {8.895, 0.000, 2.545},
    {8.068, 0.000, 2.331},
    {7.664, 0.000, 1.139},
    {7.799, 0.000, 0.238}
};

// Curve 188
curves[188].controlPoints = {
    {7.799, 0.000, 0.238},
    {7.871, 0.000, -0.373},
    {8.217, 0.000, -0.635},
    {8.725, 0.000, -0.637}
};

// Curve 189
curves[189].controlPoints = {
    {7.366, 0.000, 1.009},
    {7.434, 0.000, 2.111},
    {7.979, 0.000, 3.234},
    {8.943, 0.000, 3.477}
};

// Curve 190
curves[190].controlPoints = {
    {8.943, 0.000, 3.477},
    {9.998, 0.000, 3.755},
    {11.519, 0.000, 2.850},
    {12.161, 0.000, 2.112}
};

// Curve 191
curves[191].controlPoints = {
    {12.161, 0.000, 2.112},
    {12.289, 0.000, 2.107},
    {12.414, 0.000, 2.101},
    {12.536, 0.000, 2.098}
};

// Curve 192
curves[192].controlPoints = {
    {12.536, 0.000, 2.098},
    {13.099, 0.000, 2.084},
    {13.571, 0.000, 2.116},
    {14.054, 0.000, 2.538}
};

// Curve 193
curves[193].controlPoints = {
    {14.054, 0.000, 2.538},
    {14.055, 0.000, 2.539},
    {14.055, 0.000, 2.539},
    {14.056, 0.000, 2.540}
};

// Curve 194
curves[194].controlPoints = {
    {14.056, 0.000, 2.540},
    {14.056, 0.000, 2.540},
    {14.057, 0.000, 2.541},
    {14.058, 0.000, 2.541}
};

// Curve 195
curves[195].controlPoints = {
    {14.058, 0.000, 2.541},
    {14.429, 0.000, 2.855},
    {14.605, 0.000, 3.449},
    {14.758, 0.000, 4.115}
};

// Curve 196
curves[196].controlPoints = {
    {14.758, 0.000, 4.115},
    {14.911, 0.000, 4.780},
    {15.033, 0.000, 5.505},
    {15.492, 0.000, 6.021}
};

// Curve 197
curves[197].controlPoints = {
    {15.492, 0.000, 6.021},
    {15.492, 0.000, 6.021},
    {15.492, 0.000, 6.021},
    {15.492, 0.000, 6.022}
};

// Curve 198
curves[198].controlPoints = {
    {15.492, 0.000, 6.022},
    {15.492, 0.000, 6.022},
    {15.493, 0.000, 6.022},
    {15.493, 0.000, 6.022}
};

// Curve 199
curves[199].controlPoints = {
    {15.493, 0.000, 6.022},
    {16.375, 0.000, 7.001},
    {16.658, 0.000, 7.662},
    {16.634, 0.000, 8.084}
};

// Curve 200
curves[200].controlPoints = {
    {16.634, 0.000, 8.084},
    {16.610, 0.000, 8.506},
    {16.304, 0.000, 8.820},
    {15.740, 0.000, 9.160}
};

// Curve 201
curves[201].controlPoints = {
    {15.740, 0.000, 9.160},
    {14.612, 0.000, 9.841},
    {12.613, 0.000, 10.432},
    {11.336, 0.000, 11.979}
};

// Curve 202
curves[202].controlPoints = {
    {11.336, 0.000, 11.979},
    {10.227, 0.000, 13.298},
    {8.875, 0.000, 14.023},
    {7.684, 0.000, 14.117}
};

// Curve 203
curves[203].controlPoints = {
    {7.684, 0.000, 14.117},
    {6.494, 0.000, 14.211},
    {5.467, 0.000, 13.717},
    {4.861, 0.000, 12.501}
};

// Curve 204
curves[204].controlPoints = {
    {4.861, 0.000, 12.501},
    {4.860, 0.000, 12.500},
    {4.860, 0.000, 12.499},
    {4.860, 0.000, 12.498}
};

// Curve 205
curves[205].controlPoints = {
    {4.860, 0.000, 12.498},
    {4.859, 0.000, 12.498},
    {4.859, 0.000, 12.497},
    {4.858, 0.000, 12.496}
};

// Curve 206
curves[206].controlPoints = {
    {4.858, 0.000, 12.496},
    {4.482, 0.000, 11.781},
    {4.639, 0.000, 10.653},
    {4.955, 0.000, 9.463}
};

// Curve 207
curves[207].controlPoints = {
    {4.955, 0.000, 9.463},
    {5.272, 0.000, 8.274},
    {5.727, 0.000, 7.052},
    {5.787, 0.000, 6.059}
};

// Curve 208
curves[208].controlPoints = {
    {5.787, 0.000, 6.059},
    {5.787, 0.000, 6.059},
    {5.787, 0.000, 6.058},
    {5.787, 0.000, 6.058}
};

// Curve 209
curves[209].controlPoints = {
    {5.787, 0.000, 6.058},
    {5.787, 0.000, 6.057},
    {5.787, 0.000, 6.057},
    {5.787, 0.000, 6.056}
};

// Curve 210
curves[210].controlPoints = {
    {5.787, 0.000, 6.056},
    {5.852, 0.000, 4.784},
    {5.923, 0.000, 3.673},
    {6.137, 0.000, 2.816}
};

// Curve 211
curves[211].controlPoints = {
    {6.137, 0.000, 2.816},
    {6.350, 0.000, 1.959},
    {6.687, 0.000, 1.378},
    {7.283, 0.000, 1.052}
};

// Curve 212
curves[212].controlPoints = {
    {7.283, 0.000, 1.052},
    {7.311, 0.000, 1.037},
    {7.338, 0.000, 1.022},
    {7.366, 0.000, 1.009}
};

// Curve 213
curves[213].controlPoints = {
    {7.366, 0.000, 1.009},
    {7.366, 0.000, 1.009},
    {7.366, 0.000, 1.009},
    {7.366, 0.000, 1.009}
};

// Curve 214
curves[214].controlPoints = {
    {-11.995, 0.000, 1.092},
    {-11.906, 0.000, 1.092},
    {-11.812, 0.000, 1.100},
    {-11.712, 0.000, 1.115}
};

// Curve 215
curves[215].controlPoints = {
    {-11.712, 0.000, 1.115},
    {-11.037, 0.000, 1.217},
    {-10.448, 0.000, 1.689},
    {-9.881, 0.000, 2.458}
};

// Curve 216
curves[216].controlPoints = {
    {-9.881, 0.000, 2.458},
    {-9.314, 0.000, 3.227},
    {-8.787, 0.000, 4.277},
    {-8.244, 0.000, 5.442}
};

// Curve 217
curves[217].controlPoints = {
    {-8.244, 0.000, 5.442},
    {-8.244, 0.000, 5.442},
    {-8.244, 0.000, 5.443},
    {-8.244, 0.000, 5.443}
};

// Curve 218
curves[218].controlPoints = {
    {-8.244, 0.000, 5.443},
    {-8.243, 0.000, 5.444},
    {-8.243, 0.000, 5.444},
    {-8.243, 0.000, 5.444}
};

// Curve 219
curves[219].controlPoints = {
    {-8.243, 0.000, 5.444},
    {-7.807, 0.000, 6.354},
    {-6.887, 0.000, 7.354},
    {-6.108, 0.000, 8.375}
};

// Curve 220
curves[220].controlPoints = {
    {-6.108, 0.000, 8.375},
    {-5.329, 0.000, 9.395},
    {-4.725, 0.000, 10.419},
    {-4.804, 0.000, 11.203}
};

// Curve 221
curves[221].controlPoints = {
    {-4.804, 0.000, 11.203},
    {-4.804, 0.000, 11.204},
    {-4.804, 0.000, 11.206},
    {-4.804, 0.000, 11.207}
};

// Curve 222
curves[222].controlPoints = {
    {-4.804, 0.000, 11.207},
    {-4.804, 0.000, 11.208},
    {-4.805, 0.000, 11.209},
    {-4.805, 0.000, 11.210}
};

// Curve 223
curves[223].controlPoints = {
    {-4.805, 0.000, 11.210},
    {-4.906, 0.000, 12.547},
    {-5.660, 0.000, 13.275},
    {-6.818, 0.000, 13.539}
};

// Curve 224
curves[224].controlPoints = {
    {-6.818, 0.000, 13.539},
    {-7.976, 0.000, 13.804},
    {-9.545, 0.000, 13.540},
    {-11.112, 0.000, 12.721}
};

// Curve 225
curves[225].controlPoints = {
    {-11.112, 0.000, 12.721},
    {-11.113, 0.000, 12.721},
    {-11.113, 0.000, 12.721},
    {-11.113, 0.000, 12.721}
};

// Curve 226
curves[226].controlPoints = {
    {-11.113, 0.000, 12.721},
    {-12.847, 0.000, 11.802},
    {-14.910, 0.000, 11.893},
    {-16.233, 0.000, 11.617}
};

// Curve 227
curves[227].controlPoints = {
    {-16.233, 0.000, 11.617},
    {-16.895, 0.000, 11.479},
    {-17.327, 0.000, 11.271},
    {-17.525, 0.000, 10.885}
};

// Curve 228
curves[228].controlPoints = {
    {-17.525, 0.000, 10.885},
    {-17.723, 0.000, 10.499},
    {-17.728, 0.000, 9.825},
    {-17.307, 0.000, 8.677}
};

// Curve 229
curves[229].controlPoints = {
    {-17.307, 0.000, 8.677},
    {-17.306, 0.000, 8.675},
    {-17.305, 0.000, 8.673},
    {-17.305, 0.000, 8.672}
};

// Curve 230
curves[230].controlPoints = {
    {-17.305, 0.000, 8.672},
    {-17.304, 0.000, 8.670},
    {-17.304, 0.000, 8.668},
    {-17.303, 0.000, 8.667}
};

// Curve 231
curves[231].controlPoints = {
    {-17.303, 0.000, 8.667},
    {-17.095, 0.000, 8.024},
    {-17.249, 0.000, 7.321},
    {-17.350, 0.000, 6.661}
};

// Curve 232
curves[232].controlPoints = {
    {-17.350, 0.000, 6.661},
    {-17.451, 0.000, 6.001},
    {-17.501, 0.000, 5.401},
    {-17.275, 0.000, 4.983}
};

// Curve 233
curves[233].controlPoints = {
    {-17.275, 0.000, 4.983},
    {-17.275, 0.000, 4.982},
    {-17.275, 0.000, 4.982},
    {-17.274, 0.000, 4.981}
};

// Curve 234
curves[234].controlPoints = {
    {-17.274, 0.000, 4.981},
    {-17.274, 0.000, 4.980},
    {-17.274, 0.000, 4.980},
    {-17.273, 0.000, 4.979}
};

// Curve 235
curves[235].controlPoints = {
    {-17.273, 0.000, 4.979},
    {-16.985, 0.000, 4.423},
    {-16.562, 0.000, 4.224},
    {-16.036, 0.000, 4.036}
};

// Curve 236
curves[236].controlPoints = {
    {-16.036, 0.000, 4.036},
    {-15.511, 0.000, 3.847},
    {-14.889, 0.000, 3.700},
    {-14.397, 0.000, 3.207}
};

// Curve 237
curves[237].controlPoints = {
    {-14.397, 0.000, 3.207},
    {-14.396, 0.000, 3.206},
    {-14.395, 0.000, 3.205},
    {-14.394, 0.000, 3.204}
};

// Curve 238
curves[238].controlPoints = {
    {-14.394, 0.000, 3.204},
    {-14.393, 0.000, 3.203},
    {-14.393, 0.000, 3.202},
    {-14.392, 0.000, 3.201}
};

// Curve 239
curves[239].controlPoints = {
    {-14.392, 0.000, 3.201},
    {-13.937, 0.000, 2.722},
    {-13.596, 0.000, 2.121},
    {-13.196, 0.000, 1.694}
};

// Curve 240
curves[240].controlPoints = {
    {-13.196, 0.000, 1.694},
    {-12.859, 0.000, 1.334},
    {-12.522, 0.000, 1.096},
    {-12.013, 0.000, 1.092}
};

// Curve 241
curves[241].controlPoints = {
    {-12.013, 0.000, 1.092},
    {-12.007, 0.000, 1.092},
    {-12.001, 0.000, 1.092},
    {-11.995, 0.000, 1.092}
};

// Curve 242
curves[242].controlPoints = {
    {-11.995, 0.000, 1.092},
    {-11.995, 0.000, 1.092},
    {-11.995, 0.000, 1.092},
    {-11.995, 0.000, 1.092}
};

// Curve 243
curves[243].controlPoints = {
    {-18.198, 0.000, 21.498},
    {-18.208, 0.000, 21.508},
    {-18.218, 0.000, 21.518},
    {-18.228, 0.000, 21.528}
};

// Curve 244
curves[244].controlPoints = {
    {-18.228, 0.000, 21.528},
    {-18.228, 0.000, 21.627},
    {-18.228, 0.000, 21.725},
    {-18.228, 0.000, 21.823}
};

// Curve 245
curves[245].controlPoints = {
    {-18.228, 0.000, 21.823},
    {-18.228, 0.000, 21.904},
    {-18.188, 0.000, 21.944},
    {-18.108, 0.000, 21.944}
};

// Curve 246
curves[246].controlPoints = {
    {-18.108, 0.000, 21.944},
    {-17.969, 0.000, 21.954},
    {-17.831, 0.000, 21.964},
    {-17.692, 0.000, 21.973}
};

// Curve 247
curves[247].controlPoints = {
    {-17.692, 0.000, 21.973},
    {-17.390, 0.000, 21.999},
    {-17.176, 0.000, 22.059},
    {-17.050, 0.000, 22.155}
};

// Curve 248
curves[248].controlPoints = {
    {-17.050, 0.000, 22.155},
    {-16.919, 0.000, 22.245},
    {-16.855, 0.000, 22.402},
    {-16.855, 0.000, 22.623}
};

// Curve 249
curves[249].controlPoints = {
    {-16.855, 0.000, 22.623},
    {-16.855, 0.000, 25.214},
    {-16.855, 0.000, 27.804},
    {-16.855, 0.000, 30.394}
};

// Curve 250
curves[250].controlPoints = {
    {-16.855, 0.000, 30.394},
    {-16.855, 0.000, 30.656},
    {-16.904, 0.000, 30.827},
    {-17.005, 0.000, 30.908}
};

// Curve 251
curves[251].controlPoints = {
    {-17.005, 0.000, 30.908},
    {-17.105, 0.000, 30.983},
    {-17.317, 0.000, 31.034},
    {-17.639, 0.000, 31.059}
};

// Curve 252
curves[252].controlPoints = {
    {-17.639, 0.000, 31.059},
    {-17.795, 0.000, 31.069},
    {-17.952, 0.000, 31.079},
    {-18.108, 0.000, 31.089}
};

// Curve 253
curves[253].controlPoints = {
    {-18.108, 0.000, 31.089},
    {-18.188, 0.000, 31.089},
    {-18.228, 0.000, 31.129},
    {-18.228, 0.000, 31.209}
};

// Curve 254
curves[254].controlPoints = {
    {-18.228, 0.000, 31.209},
    {-18.228, 0.000, 31.308},
    {-18.228, 0.000, 31.406},
    {-18.228, 0.000, 31.505}
};

// Curve 255
curves[255].controlPoints = {
    {-18.228, 0.000, 31.505},
    {-18.218, 0.000, 31.515},
    {-18.208, 0.000, 31.524},
    {-18.198, 0.000, 31.534}
};

// Curve 256
curves[256].controlPoints = {
    {-18.198, 0.000, 31.534},
    {-17.232, 0.000, 31.514},
    {-16.577, 0.000, 31.505},
    {-16.234, 0.000, 31.505}
};

// Curve 257
curves[257].controlPoints = {
    {-16.234, 0.000, 31.505},
    {-15.132, 0.000, 31.505},
    {-14.029, 0.000, 31.505},
    {-12.927, 0.000, 31.505}
};

// Curve 258
curves[258].controlPoints = {
    {-12.927, 0.000, 31.505},
    {-12.650, 0.000, 31.505},
    {-12.036, 0.000, 31.514},
    {-11.084, 0.000, 31.534}
};

// Curve 259
curves[259].controlPoints = {
    {-11.084, 0.000, 31.534},
    {-11.034, 0.000, 30.809},
    {-10.901, 0.000, 29.948},
    {-10.684, 0.000, 28.952}
};

// Curve 260
curves[260].controlPoints = {
    {-10.684, 0.000, 28.952},
    {-10.838, 0.000, 28.927},
    {-10.992, 0.000, 28.902},
    {-11.145, 0.000, 28.877}
};

// Curve 261
curves[261].controlPoints = {
    {-11.145, 0.000, 28.877},
    {-11.231, 0.000, 29.184},
    {-11.323, 0.000, 29.447},
    {-11.423, 0.000, 29.669}
};

// Curve 262
curves[262].controlPoints = {
    {-11.423, 0.000, 29.669},
    {-11.519, 0.000, 29.890},
    {-11.643, 0.000, 30.102},
    {-11.794, 0.000, 30.303}
};

// Curve 263
curves[263].controlPoints = {
    {-11.794, 0.000, 30.303},
    {-11.940, 0.000, 30.499},
    {-12.124, 0.000, 30.648},
    {-12.345, 0.000, 30.748}
};

// Curve 264
curves[264].controlPoints = {
    {-12.345, 0.000, 30.748},
    {-12.562, 0.000, 30.849},
    {-12.812, 0.000, 30.900},
    {-13.094, 0.000, 30.900}
};

// Curve 265
curves[265].controlPoints = {
    {-13.094, 0.000, 30.900},
    {-13.688, 0.000, 30.900},
    {-14.281, 0.000, 30.900},
    {-14.875, 0.000, 30.900}
};

// Curve 266
curves[266].controlPoints = {
    {-14.875, 0.000, 30.900},
    {-15.097, 0.000, 30.900},
    {-15.274, 0.000, 30.833},
    {-15.405, 0.000, 30.697}
};

// Curve 267
curves[267].controlPoints = {
    {-15.405, 0.000, 30.697},
    {-15.536, 0.000, 30.556},
    {-15.600, 0.000, 30.366},
    {-15.600, 0.000, 30.130}
};

// Curve 268
curves[268].controlPoints = {
    {-15.600, 0.000, 30.130},
    {-15.600, 0.000, 27.628},
    {-15.600, 0.000, 25.126},
    {-15.600, 0.000, 22.623}
};

// Curve 269
curves[269].controlPoints = {
    {-15.600, 0.000, 22.623},
    {-15.600, 0.000, 22.407},
    {-15.537, 0.000, 22.250},
    {-15.411, 0.000, 22.155}
};

// Curve 270
curves[270].controlPoints = {
    {-15.411, 0.000, 22.155},
    {-15.285, 0.000, 22.054},
    {-15.064, 0.000, 21.994},
    {-14.747, 0.000, 21.973}
};

// Curve 271
curves[271].controlPoints = {
    {-14.747, 0.000, 21.973},
    {-14.614, 0.000, 21.964},
    {-14.480, 0.000, 21.954},
    {-14.347, 0.000, 21.944}
};

// Curve 272
curves[272].controlPoints = {
    {-14.347, 0.000, 21.944},
    {-14.266, 0.000, 21.944},
    {-14.227, 0.000, 21.904},
    {-14.227, 0.000, 21.823}
};

// Curve 273
curves[273].controlPoints = {
    {-14.227, 0.000, 21.823},
    {-14.227, 0.000, 21.725},
    {-14.227, 0.000, 21.627},
    {-14.227, 0.000, 21.528}
};

// Curve 274
curves[274].controlPoints = {
    {-14.227, 0.000, 21.528},
    {-14.236, 0.000, 21.518},
    {-14.246, 0.000, 21.508},
    {-14.256, 0.000, 21.498}
};

// Curve 275
curves[275].controlPoints = {
    {-14.256, 0.000, 21.498},
    {-15.223, 0.000, 21.519},
    {-15.882, 0.000, 21.528},
    {-16.234, 0.000, 21.528}
};

// Curve 276
curves[276].controlPoints = {
    {-16.234, 0.000, 21.528},
    {-16.577, 0.000, 21.528},
    {-17.232, 0.000, 21.519},
    {-18.198, 0.000, 21.498}
};

// Curve 277
curves[277].controlPoints = {
    {-8.298, 0.000, 21.944},
    {-8.500, 0.000, 21.944},
    {-8.673, 0.000, 22.019},
    {-8.819, 0.000, 22.170}
};

// Curve 278
curves[278].controlPoints = {
    {-8.819, 0.000, 22.170},
    {-8.965, 0.000, 22.316},
    {-9.038, 0.000, 22.489},
    {-9.037, 0.000, 22.691}
};

// Curve 279
curves[279].controlPoints = {
    {-9.037, 0.000, 22.691},
    {-9.038, 0.000, 22.892},
    {-8.965, 0.000, 23.067},
    {-8.819, 0.000, 23.213}
};

// Curve 280
curves[280].controlPoints = {
    {-8.819, 0.000, 23.213},
    {-8.673, 0.000, 23.359},
    {-8.500, 0.000, 23.431},
    {-8.298, 0.000, 23.431}
};

// Curve 281
curves[281].controlPoints = {
    {-8.298, 0.000, 23.431},
    {-8.097, 0.000, 23.431},
    {-7.923, 0.000, 23.359},
    {-7.777, 0.000, 23.213}
};

// Curve 282
curves[282].controlPoints = {
    {-7.777, 0.000, 23.213},
    {-7.626, 0.000, 23.067},
    {-7.550, 0.000, 22.892},
    {-7.550, 0.000, 22.691}
};

// Curve 283
curves[283].controlPoints = {
    {-7.550, 0.000, 22.691},
    {-7.550, 0.000, 22.489},
    {-7.626, 0.000, 22.316},
    {-7.777, 0.000, 22.170}
};

// Curve 284
curves[284].controlPoints = {
    {-7.777, 0.000, 22.170},
    {-7.923, 0.000, 22.019},
    {-8.097, 0.000, 21.944},
    {-8.298, 0.000, 21.944}
};

// Curve 285
curves[285].controlPoints = {
    {-7.648, 0.000, 24.670},
    {-8.006, 0.000, 24.816},
    {-8.731, 0.000, 24.974},
    {-9.823, 0.000, 25.145}
};

// Curve 286
curves[286].controlPoints = {
    {-9.823, 0.000, 25.145},
    {-9.814, 0.000, 25.269},
    {-9.804, 0.000, 25.392},
    {-9.794, 0.000, 25.516}
};

// Curve 287
curves[287].controlPoints = {
    {-9.794, 0.000, 25.516},
    {-9.572, 0.000, 25.538},
    {-9.350, 0.000, 25.560},
    {-9.128, 0.000, 25.583}
};

// Curve 288
curves[288].controlPoints = {
    {-9.128, 0.000, 25.583},
    {-9.007, 0.000, 25.598},
    {-8.922, 0.000, 25.672},
    {-8.872, 0.000, 25.803}
};

// Curve 289
curves[289].controlPoints = {
    {-8.872, 0.000, 25.803},
    {-8.816, 0.000, 25.934},
    {-8.789, 0.000, 26.180},
    {-8.789, 0.000, 26.542}
};

// Curve 290
curves[290].controlPoints = {
    {-8.789, 0.000, 26.542},
    {-8.789, 0.000, 27.826},
    {-8.789, 0.000, 29.110},
    {-8.789, 0.000, 30.394}
};

// Curve 291
curves[291].controlPoints = {
    {-8.789, 0.000, 30.394},
    {-8.789, 0.000, 30.620},
    {-8.834, 0.000, 30.782},
    {-8.925, 0.000, 30.878}
};

// Curve 292
curves[292].controlPoints = {
    {-8.925, 0.000, 30.878},
    {-9.016, 0.000, 30.969},
    {-9.191, 0.000, 31.029},
    {-9.453, 0.000, 31.059}
};

// Curve 293
curves[293].controlPoints = {
    {-9.453, 0.000, 31.059},
    {-9.552, 0.000, 31.069},
    {-9.650, 0.000, 31.079},
    {-9.748, 0.000, 31.089}
};

// Curve 294
curves[294].controlPoints = {
    {-9.748, 0.000, 31.089},
    {-9.829, 0.000, 31.104},
    {-9.869, 0.000, 31.144},
    {-9.869, 0.000, 31.209}
};

// Curve 295
curves[295].controlPoints = {
    {-9.869, 0.000, 31.209},
    {-9.869, 0.000, 31.308},
    {-9.869, 0.000, 31.406},
    {-9.869, 0.000, 31.505}
};

// Curve 296
curves[296].controlPoints = {
    {-9.869, 0.000, 31.505},
    {-9.859, 0.000, 31.515},
    {-9.849, 0.000, 31.524},
    {-9.839, 0.000, 31.534}
};

// Curve 297
curves[297].controlPoints = {
    {-9.839, 0.000, 31.534},
    {-9.013, 0.000, 31.514},
    {-8.476, 0.000, 31.505},
    {-8.230, 0.000, 31.505}
};

// Curve 298
curves[298].controlPoints = {
    {-8.230, 0.000, 31.505},
    {-7.953, 0.000, 31.505},
    {-7.402, 0.000, 31.514},
    {-6.577, 0.000, 31.534}
};

// Curve 299
curves[299].controlPoints = {
    {-6.577, 0.000, 31.534},
    {-6.566, 0.000, 31.524},
    {-6.556, 0.000, 31.515},
    {-6.545, 0.000, 31.505}
};

// Curve 300
curves[300].controlPoints = {
    {-6.545, 0.000, 31.505},
    {-6.545, 0.000, 31.406},
    {-6.545, 0.000, 31.308},
    {-6.545, 0.000, 31.209}
};

// Curve 301
curves[301].controlPoints = {
    {-6.545, 0.000, 31.209},
    {-6.545, 0.000, 31.149},
    {-6.593, 0.000, 31.109},
    {-6.689, 0.000, 31.089}
};

// Curve 302
curves[302].controlPoints = {
    {-6.689, 0.000, 31.089},
    {-6.785, 0.000, 31.079},
    {-6.881, 0.000, 31.069},
    {-6.977, 0.000, 31.059}
};

// Curve 303
curves[303].controlPoints = {
    {-6.977, 0.000, 31.059},
    {-7.218, 0.000, 31.034},
    {-7.388, 0.000, 30.974},
    {-7.489, 0.000, 30.878}
};

// Curve 304
curves[304].controlPoints = {
    {-7.489, 0.000, 30.878},
    {-7.585, 0.000, 30.777},
    {-7.633, 0.000, 30.615},
    {-7.633, 0.000, 30.394}
};

// Curve 305
curves[305].controlPoints = {
    {-7.633, 0.000, 30.394},
    {-7.633, 0.000, 29.110},
    {-7.633, 0.000, 27.826},
    {-7.633, 0.000, 26.542}
};

// Curve 306
curves[306].controlPoints = {
    {-7.633, 0.000, 26.542},
    {-7.633, 0.000, 26.029},
    {-7.615, 0.000, 25.420},
    {-7.580, 0.000, 24.716}
};

// Curve 307
curves[307].controlPoints = {
    {-7.580, 0.000, 24.716},
    {-7.580, 0.000, 24.685},
    {-7.603, 0.000, 24.670},
    {-7.648, 0.000, 24.670}
};

// Curve 308
curves[308].controlPoints = {
    {-3.722, 0.000, 24.670},
    {-4.084, 0.000, 24.816},
    {-4.796, 0.000, 24.974},
    {-5.858, 0.000, 25.145}
};

// Curve 309
curves[309].controlPoints = {
    {-5.858, 0.000, 25.145},
    {-5.848, 0.000, 25.269},
    {-5.838, 0.000, 25.392},
    {-5.828, 0.000, 25.516}
};

// Curve 310
curves[310].controlPoints = {
    {-5.828, 0.000, 25.516},
    {-5.607, 0.000, 25.538},
    {-5.385, 0.000, 25.560},
    {-5.164, 0.000, 25.583}
};

// Curve 311
curves[311].controlPoints = {
    {-5.164, 0.000, 25.583},
    {-5.043, 0.000, 25.598},
    {-4.957, 0.000, 25.672},
    {-4.906, 0.000, 25.803}
};

// Curve 312
curves[312].controlPoints = {
    {-4.906, 0.000, 25.803},
    {-4.851, 0.000, 25.934},
    {-4.823, 0.000, 26.180},
    {-4.823, 0.000, 26.542}
};

// Curve 313
curves[313].controlPoints = {
    {-4.823, 0.000, 26.542},
    {-4.823, 0.000, 27.826},
    {-4.823, 0.000, 29.110},
    {-4.823, 0.000, 30.394}
};

// Curve 314
curves[314].controlPoints = {
    {-4.823, 0.000, 30.394},
    {-4.823, 0.000, 30.620},
    {-4.872, 0.000, 30.784},
    {-4.967, 0.000, 30.884}
};

// Curve 315
curves[315].controlPoints = {
    {-4.967, 0.000, 30.884},
    {-5.063, 0.000, 30.980},
    {-5.237, 0.000, 31.039},
    {-5.489, 0.000, 31.059}
};

// Curve 316
curves[316].controlPoints = {
    {-5.489, 0.000, 31.059},
    {-5.587, 0.000, 31.069},
    {-5.685, 0.000, 31.079},
    {-5.783, 0.000, 31.089}
};

// Curve 317
curves[317].controlPoints = {
    {-5.783, 0.000, 31.089},
    {-5.863, 0.000, 31.104},
    {-5.903, 0.000, 31.144},
    {-5.903, 0.000, 31.209}
};

// Curve 318
curves[318].controlPoints = {
    {-5.903, 0.000, 31.209},
    {-5.903, 0.000, 31.308},
    {-5.903, 0.000, 31.406},
    {-5.903, 0.000, 31.505}
};

// Curve 319
curves[319].controlPoints = {
    {-5.903, 0.000, 31.505},
    {-5.893, 0.000, 31.515},
    {-5.883, 0.000, 31.524},
    {-5.873, 0.000, 31.534}
};

// Curve 320
curves[320].controlPoints = {
    {-5.873, 0.000, 31.534},
    {-5.048, 0.000, 31.514},
    {-4.512, 0.000, 31.505},
    {-4.266, 0.000, 31.505}
};

// Curve 321
curves[321].controlPoints = {
    {-4.266, 0.000, 31.505},
    {-4.039, 0.000, 31.505},
    {-3.513, 0.000, 31.514},
    {-2.688, 0.000, 31.534}
};

// Curve 322
curves[322].controlPoints = {
    {-2.688, 0.000, 31.534},
    {-2.677, 0.000, 31.524},
    {-2.667, 0.000, 31.515},
    {-2.656, 0.000, 31.505}
};

// Curve 323
curves[323].controlPoints = {
    {-2.656, 0.000, 31.505},
    {-2.656, 0.000, 31.406},
    {-2.656, 0.000, 31.308},
    {-2.656, 0.000, 31.209}
};

// Curve 324
curves[324].controlPoints = {
    {-2.656, 0.000, 31.209},
    {-2.656, 0.000, 31.144},
    {-2.698, 0.000, 31.104},
    {-2.778, 0.000, 31.089}
};

// Curve 325
curves[325].controlPoints = {
    {-2.778, 0.000, 31.089},
    {-2.851, 0.000, 31.079},
    {-2.924, 0.000, 31.069},
    {-2.997, 0.000, 31.059}
};

// Curve 326
curves[326].controlPoints = {
    {-2.997, 0.000, 31.059},
    {-3.254, 0.000, 31.019},
    {-3.429, 0.000, 30.953},
    {-3.525, 0.000, 30.863}
};

// Curve 327
curves[327].controlPoints = {
    {-3.525, 0.000, 30.863},
    {-3.616, 0.000, 30.772},
    {-3.661, 0.000, 30.615},
    {-3.661, 0.000, 30.394}
};

// Curve 328
curves[328].controlPoints = {
    {-3.661, 0.000, 30.394},
    {-3.661, 0.000, 29.145},
    {-3.661, 0.000, 27.897},
    {-3.661, 0.000, 26.648}
};

// Curve 329
curves[329].controlPoints = {
    {-3.661, 0.000, 26.648},
    {-3.283, 0.000, 26.205},
    {-2.936, 0.000, 25.895},
    {-2.619, 0.000, 25.719}
};

// Curve 330
curves[330].controlPoints = {
    {-2.619, 0.000, 25.719},
    {-2.302, 0.000, 25.543},
    {-1.964, 0.000, 25.455},
    {-1.606, 0.000, 25.455}
};

// Curve 331
curves[331].controlPoints = {
    {-1.606, 0.000, 25.455},
    {-1.214, 0.000, 25.455},
    {-0.950, 0.000, 25.578},
    {-0.814, 0.000, 25.825}
};

// Curve 332
curves[332].controlPoints = {
    {-0.814, 0.000, 25.825},
    {-0.673, 0.000, 26.067},
    {-0.603, 0.000, 26.475},
    {-0.603, 0.000, 27.048}
};

// Curve 333
curves[333].controlPoints = {
    {-0.603, 0.000, 27.048},
    {-0.603, 0.000, 28.164},
    {-0.603, 0.000, 29.279},
    {-0.603, 0.000, 30.394}
};

// Curve 334
curves[334].controlPoints = {
    {-0.603, 0.000, 30.394},
    {-0.603, 0.000, 30.615},
    {-0.650, 0.000, 30.772},
    {-0.745, 0.000, 30.863}
};

// Curve 335
curves[335].controlPoints = {
    {-0.745, 0.000, 30.863},
    {-0.841, 0.000, 30.953},
    {-1.015, 0.000, 31.019},
    {-1.267, 0.000, 31.059}
};

// Curve 336
curves[336].controlPoints = {
    {-1.267, 0.000, 31.059},
    {-1.337, 0.000, 31.069},
    {-1.408, 0.000, 31.079},
    {-1.478, 0.000, 31.089}
};

// Curve 337
curves[337].controlPoints = {
    {-1.478, 0.000, 31.089},
    {-1.564, 0.000, 31.104},
    {-1.606, 0.000, 31.144},
    {-1.606, 0.000, 31.209}
};

// Curve 338
curves[338].controlPoints = {
    {-1.606, 0.000, 31.209},
    {-1.606, 0.000, 31.308},
    {-1.606, 0.000, 31.406},
    {-1.606, 0.000, 31.505}
};

// Curve 339
curves[339].controlPoints = {
    {-1.606, 0.000, 31.505},
    {-1.596, 0.000, 31.515},
    {-1.586, 0.000, 31.524},
    {-1.577, 0.000, 31.534}
};

// Curve 340
curves[340].controlPoints = {
    {-1.577, 0.000, 31.534},
    {-0.751, 0.000, 31.514},
    {-0.240, 0.000, 31.505},
    {-0.044, 0.000, 31.505}
};

// Curve 341
curves[341].controlPoints = {
    {-0.044, 0.000, 31.505},
    {0.233, 0.000, 31.505},
    {0.785, 0.000, 31.514},
    {1.611, 0.000, 31.534}
};

// Curve 342
curves[342].controlPoints = {
    {1.611, 0.000, 31.534},
    {1.621, 0.000, 31.524},
    {1.631, 0.000, 31.515},
    {1.641, 0.000, 31.505}
};

// Curve 343
curves[343].controlPoints = {
    {1.641, 0.000, 31.505},
    {1.641, 0.000, 31.406},
    {1.641, 0.000, 31.308},
    {1.641, 0.000, 31.209}
};

// Curve 344
curves[344].controlPoints = {
    {1.641, 0.000, 31.209},
    {1.641, 0.000, 31.149},
    {1.595, 0.000, 31.109},
    {1.505, 0.000, 31.089}
};

// Curve 345
curves[345].controlPoints = {
    {1.505, 0.000, 31.089},
    {1.406, 0.000, 31.079},
    {1.308, 0.000, 31.069},
    {1.209, 0.000, 31.059}
};

// Curve 346
curves[346].controlPoints = {
    {1.209, 0.000, 31.059},
    {0.973, 0.000, 31.034},
    {0.805, 0.000, 30.974},
    {0.705, 0.000, 30.878}
};

// Curve 347
curves[347].controlPoints = {
    {0.705, 0.000, 30.878},
    {0.609, 0.000, 30.777},
    {0.561, 0.000, 30.615},
    {0.561, 0.000, 30.394}
};

// Curve 348
curves[348].controlPoints = {
    {0.561, 0.000, 30.394},
    {0.561, 0.000, 29.317},
    {0.561, 0.000, 28.240},
    {0.561, 0.000, 27.163}
};

// Curve 349
curves[349].controlPoints = {
    {0.561, 0.000, 27.163},
    {0.561, 0.000, 26.871},
    {0.546, 0.000, 26.608},
    {0.516, 0.000, 26.377}
};

// Curve 350
curves[350].controlPoints = {
    {0.516, 0.000, 26.377},
    {0.490, 0.000, 26.145},
    {0.440, 0.000, 25.924},
    {0.364, 0.000, 25.713}
};

// Curve 351
curves[351].controlPoints = {
    {0.364, 0.000, 25.713},
    {0.289, 0.000, 25.496},
    {0.188, 0.000, 25.318},
    {0.062, 0.000, 25.177}
};

// Curve 352
curves[352].controlPoints = {
    {0.062, 0.000, 25.177},
    {-0.063, 0.000, 25.036},
    {-0.227, 0.000, 24.924},
    {-0.428, 0.000, 24.844}
};

// Curve 353
curves[353].controlPoints = {
    {-0.428, 0.000, 24.844},
    {-0.630, 0.000, 24.758},
    {-0.867, 0.000, 24.716},
    {-1.139, 0.000, 24.716}
};

// Curve 354
curves[354].controlPoints = {
    {-1.139, 0.000, 24.716},
    {-1.582, 0.000, 24.716},
    {-1.999, 0.000, 24.810},
    {-2.392, 0.000, 25.002}
};

// Curve 355
curves[355].controlPoints = {
    {-2.392, 0.000, 25.002},
    {-2.780, 0.000, 25.188},
    {-3.203, 0.000, 25.550},
    {-3.661, 0.000, 26.089}
};

// Curve 356
curves[356].controlPoints = {
    {-3.661, 0.000, 26.089},
    {-3.676, 0.000, 26.089},
    {-3.691, 0.000, 26.089},
    {-3.706, 0.000, 26.089}
};

// Curve 357
curves[357].controlPoints = {
    {-3.706, 0.000, 26.089},
    {-3.691, 0.000, 25.631},
    {-3.676, 0.000, 25.173},
    {-3.661, 0.000, 24.716}
};

// Curve 358
curves[358].controlPoints = {
    {-3.661, 0.000, 24.716},
    {-3.661, 0.000, 24.685},
    {-3.682, 0.000, 24.670},
    {-3.722, 0.000, 24.670}
};

// Curve 359
curves[359].controlPoints = {
    {2.297, 0.000, 24.836},
    {2.282, 0.000, 24.846},
    {2.267, 0.000, 24.856},
    {2.252, 0.000, 24.866}
};

// Curve 360
curves[360].controlPoints = {
    {2.252, 0.000, 24.866},
    {2.252, 0.000, 24.997},
    {2.252, 0.000, 25.128},
    {2.252, 0.000, 25.259}
};

// Curve 361
curves[361].controlPoints = {
    {2.252, 0.000, 25.259},
    {2.267, 0.000, 25.269},
    {2.282, 0.000, 25.279},
    {2.297, 0.000, 25.289}
};

// Curve 362
curves[362].controlPoints = {
    {2.297, 0.000, 25.289},
    {2.380, 0.000, 25.299},
    {2.464, 0.000, 25.309},
    {2.547, 0.000, 25.319}
};

// Curve 363
curves[363].controlPoints = {
    {2.547, 0.000, 25.319},
    {2.819, 0.000, 25.349},
    {3.000, 0.000, 25.418},
    {3.091, 0.000, 25.523}
};

// Curve 364
curves[364].controlPoints = {
    {3.091, 0.000, 25.523},
    {3.181, 0.000, 25.624},
    {3.227, 0.000, 25.803},
    {3.227, 0.000, 26.059}
};

// Curve 365
curves[365].controlPoints = {
    {3.227, 0.000, 26.059},
    {3.227, 0.000, 27.225},
    {3.227, 0.000, 28.391},
    {3.227, 0.000, 29.556}
};

// Curve 366
curves[366].controlPoints = {
    {3.227, 0.000, 29.556},
    {3.227, 0.000, 29.843},
    {3.257, 0.000, 30.104},
    {3.317, 0.000, 30.341}
};

// Curve 367
curves[367].controlPoints = {
    {3.317, 0.000, 30.341},
    {3.378, 0.000, 30.572},
    {3.475, 0.000, 30.792},
    {3.611, 0.000, 30.998}
};

// Curve 368
curves[368].controlPoints = {
    {3.611, 0.000, 30.998},
    {3.752, 0.000, 31.205},
    {3.954, 0.000, 31.365},
    {4.216, 0.000, 31.481}
};

// Curve 369
curves[369].controlPoints = {
    {4.216, 0.000, 31.481},
    {4.477, 0.000, 31.597},
    {4.792, 0.000, 31.655},
    {5.159, 0.000, 31.655}
};

// Curve 370
curves[370].controlPoints = {
    {5.159, 0.000, 31.655},
    {5.512, 0.000, 31.655},
    {5.859, 0.000, 31.575},
    {6.202, 0.000, 31.414}
};

// Curve 371
curves[371].controlPoints = {
    {6.202, 0.000, 31.414},
    {6.549, 0.000, 31.248},
    {6.924, 0.000, 30.949},
    {7.327, 0.000, 30.516}
};

// Curve 372
curves[372].controlPoints = {
    {7.327, 0.000, 30.516},
    {7.337, 0.000, 30.516},
    {7.347, 0.000, 30.516},
    {7.358, 0.000, 30.516}
};

// Curve 373
curves[373].controlPoints = {
    {7.358, 0.000, 30.516},
    {7.347, 0.000, 30.908},
    {7.337, 0.000, 31.301},
    {7.327, 0.000, 31.694}
};

// Curve 374
curves[374].controlPoints = {
    {7.327, 0.000, 31.694},
    {7.445, 0.000, 31.694},
    {7.563, 0.000, 31.694},
    {7.681, 0.000, 31.694}
};

// Curve 375
curves[375].controlPoints = {
    {7.681, 0.000, 31.694},
    {7.983, 0.000, 31.447},
    {8.644, 0.000, 31.230},
    {9.661, 0.000, 31.044}
};

// Curve 376
curves[376].controlPoints = {
    {9.661, 0.000, 31.044},
    {9.646, 0.000, 30.923},
    {9.631, 0.000, 30.802},
    {9.616, 0.000, 30.681}
};

// Curve 377
curves[377].controlPoints = {
    {9.616, 0.000, 30.681},
    {9.369, 0.000, 30.656},
    {9.122, 0.000, 30.631},
    {8.875, 0.000, 30.606}
};

// Curve 378
curves[378].controlPoints = {
    {8.875, 0.000, 30.606},
    {8.709, 0.000, 30.591},
    {8.603, 0.000, 30.532},
    {8.558, 0.000, 30.431}
};

// Curve 379
curves[379].controlPoints = {
    {8.558, 0.000, 30.431},
    {8.512, 0.000, 30.326},
    {8.491, 0.000, 30.084},
    {8.491, 0.000, 29.706}
};

// Curve 380
curves[380].controlPoints = {
    {8.491, 0.000, 29.706},
    {8.491, 0.000, 28.594},
    {8.491, 0.000, 27.481},
    {8.491, 0.000, 26.369}
};

// Curve 381
curves[381].controlPoints = {
    {8.491, 0.000, 26.369},
    {8.491, 0.000, 26.062},
    {8.506, 0.000, 25.560},
    {8.536, 0.000, 24.866}
};

// Curve 382
curves[382].controlPoints = {
    {8.536, 0.000, 24.866},
    {8.536, 0.000, 24.845},
    {8.512, 0.000, 24.836},
    {8.467, 0.000, 24.836}
};

// Curve 383
curves[383].controlPoints = {
    {8.467, 0.000, 24.836},
    {8.366, 0.000, 24.856},
    {8.188, 0.000, 24.866},
    {7.931, 0.000, 24.866}
};

// Curve 384
curves[384].controlPoints = {
    {7.931, 0.000, 24.866},
    {7.674, 0.000, 24.866},
    {7.164, 0.000, 24.856},
    {6.398, 0.000, 24.836}
};

// Curve 385
curves[385].controlPoints = {
    {6.398, 0.000, 24.836},
    {6.383, 0.000, 24.846},
    {6.368, 0.000, 24.856},
    {6.353, 0.000, 24.866}
};

// Curve 386
curves[386].controlPoints = {
    {6.353, 0.000, 24.866},
    {6.353, 0.000, 24.997},
    {6.353, 0.000, 25.128},
    {6.353, 0.000, 25.259}
};

// Curve 387
curves[387].controlPoints = {
    {6.353, 0.000, 25.259},
    {6.368, 0.000, 25.269},
    {6.383, 0.000, 25.279},
    {6.398, 0.000, 25.289}
};

// Curve 388
curves[388].controlPoints = {
    {6.398, 0.000, 25.289},
    {6.481, 0.000, 25.299},
    {6.564, 0.000, 25.309},
    {6.647, 0.000, 25.319}
};

// Curve 389
curves[389].controlPoints = {
    {6.647, 0.000, 25.319},
    {6.914, 0.000, 25.349},
    {7.092, 0.000, 25.418},
    {7.183, 0.000, 25.523}
};

// Curve 390
curves[390].controlPoints = {
    {7.183, 0.000, 25.523},
    {7.278, 0.000, 25.624},
    {7.327, 0.000, 25.803},
    {7.327, 0.000, 26.059}
};

// Curve 391
curves[391].controlPoints = {
    {7.327, 0.000, 26.059},
    {7.327, 0.000, 27.338},
    {7.327, 0.000, 28.617},
    {7.327, 0.000, 29.895}
};

// Curve 392
curves[392].controlPoints = {
    {7.327, 0.000, 29.895},
    {7.105, 0.000, 30.157},
    {6.833, 0.000, 30.402},
    {6.511, 0.000, 30.628}
};

// Curve 393
curves[393].controlPoints = {
    {6.511, 0.000, 30.628},
    {6.194, 0.000, 30.850},
    {5.877, 0.000, 30.961},
    {5.559, 0.000, 30.961}
};

// Curve 394
curves[394].controlPoints = {
    {5.559, 0.000, 30.961},
    {5.423, 0.000, 30.961},
    {5.296, 0.000, 30.940},
    {5.175, 0.000, 30.900}
};

// Curve 395
curves[395].controlPoints = {
    {5.175, 0.000, 30.900},
    {5.054, 0.000, 30.855},
    {4.931, 0.000, 30.779},
    {4.805, 0.000, 30.673}
};

// Curve 396
curves[396].controlPoints = {
    {4.805, 0.000, 30.673},
    {4.679, 0.000, 30.563},
    {4.579, 0.000, 30.394},
    {4.503, 0.000, 30.167}
};

// Curve 397
curves[397].controlPoints = {
    {4.503, 0.000, 30.167},
    {4.428, 0.000, 29.936},
    {4.389, 0.000, 29.660},
    {4.389, 0.000, 29.338}
};

// Curve 398
curves[398].controlPoints = {
    {4.389, 0.000, 29.338},
    {4.389, 0.000, 28.348},
    {4.389, 0.000, 27.358},
    {4.389, 0.000, 26.369}
};

// Curve 399
curves[399].controlPoints = {
    {4.389, 0.000, 26.369},
    {4.389, 0.000, 26.062},
    {4.404, 0.000, 25.560},
    {4.434, 0.000, 24.866}
};

// Curve 400
curves[400].controlPoints = {
    {4.434, 0.000, 24.866},
    {4.434, 0.000, 24.845},
    {4.414, 0.000, 24.836},
    {4.373, 0.000, 24.836}
};

// Curve 401
curves[401].controlPoints = {
    {4.373, 0.000, 24.836},
    {4.273, 0.000, 24.856},
    {4.093, 0.000, 24.866},
    {3.831, 0.000, 24.866}
};

// Curve 402
curves[402].controlPoints = {
    {3.831, 0.000, 24.866},
    {3.574, 0.000, 24.866},
    {3.062, 0.000, 24.856},
    {2.297, 0.000, 24.836}
};

// Curve 403
curves[403].controlPoints = {
    {10.197, 0.000, 24.836},
    {10.187, 0.000, 24.851},
    {10.177, 0.000, 24.866},
    {10.167, 0.000, 24.881}
};

// Curve 404
curves[404].controlPoints = {
    {10.167, 0.000, 24.881},
    {10.167, 0.000, 24.980},
    {10.167, 0.000, 25.078},
    {10.167, 0.000, 25.177}
};

// Curve 405
curves[405].controlPoints = {
    {10.167, 0.000, 25.177},
    {10.167, 0.000, 25.232},
    {10.240, 0.000, 25.269},
    {10.386, 0.000, 25.289}
};

// Curve 406
curves[406].controlPoints = {
    {10.386, 0.000, 25.289},
    {10.633, 0.000, 25.309},
    {10.841, 0.000, 25.394},
    {11.013, 0.000, 25.545}
};

// Curve 407
curves[407].controlPoints = {
    {11.013, 0.000, 25.545},
    {11.189, 0.000, 25.696},
    {11.395, 0.000, 25.946},
    {11.631, 0.000, 26.294}
};

// Curve 408
curves[408].controlPoints = {
    {11.631, 0.000, 26.294},
    {12.082, 0.000, 26.948},
    {12.532, 0.000, 27.602},
    {12.983, 0.000, 28.256}
};

// Curve 409
curves[409].controlPoints = {
    {12.983, 0.000, 28.256},
    {13.013, 0.000, 28.302},
    {13.028, 0.000, 28.343},
    {13.028, 0.000, 28.378}
};

// Curve 410
curves[410].controlPoints = {
    {13.028, 0.000, 28.378},
    {13.028, 0.000, 28.413},
    {13.019, 0.000, 28.450},
    {12.998, 0.000, 28.491}
};

// Curve 411
curves[411].controlPoints = {
    {12.998, 0.000, 28.491},
    {12.566, 0.000, 29.047},
    {12.133, 0.000, 29.603},
    {11.700, 0.000, 30.159}
};

// Curve 412
curves[412].controlPoints = {
    {11.700, 0.000, 30.159},
    {11.499, 0.000, 30.421},
    {11.305, 0.000, 30.623},
    {11.119, 0.000, 30.764}
};

// Curve 413
curves[413].controlPoints = {
    {11.119, 0.000, 30.764},
    {10.938, 0.000, 30.905},
    {10.799, 0.000, 30.990},
    {10.703, 0.000, 31.020}
};

// Curve 414
curves[414].controlPoints = {
    {10.703, 0.000, 31.020},
    {10.607, 0.000, 31.045},
    {10.478, 0.000, 31.069},
    {10.317, 0.000, 31.089}
};

// Curve 415
curves[415].controlPoints = {
    {10.317, 0.000, 31.089},
    {10.186, 0.000, 31.104},
    {10.122, 0.000, 31.155},
    {10.122, 0.000, 31.241}
};

// Curve 416
curves[416].controlPoints = {
    {10.122, 0.000, 31.241},
    {10.122, 0.000, 31.329},
    {10.122, 0.000, 31.417},
    {10.122, 0.000, 31.505}
};

// Curve 417
curves[417].controlPoints = {
    {10.122, 0.000, 31.505},
    {10.132, 0.000, 31.515},
    {10.142, 0.000, 31.524},
    {10.152, 0.000, 31.534}
};

// Curve 418
curves[418].controlPoints = {
    {10.152, 0.000, 31.534},
    {10.917, 0.000, 31.514},
    {11.301, 0.000, 31.505},
    {11.306, 0.000, 31.505}
};

// Curve 419
curves[419].controlPoints = {
    {11.306, 0.000, 31.505},
    {11.407, 0.000, 31.505},
    {11.841, 0.000, 31.514},
    {12.606, 0.000, 31.534}
};

// Curve 420
curves[420].controlPoints = {
    {12.606, 0.000, 31.534},
    {12.619, 0.000, 31.524},
    {12.631, 0.000, 31.515},
    {12.644, 0.000, 31.505}
};

// Curve 421
curves[421].controlPoints = {
    {12.644, 0.000, 31.505},
    {12.644, 0.000, 31.406},
    {12.644, 0.000, 31.308},
    {12.644, 0.000, 31.209}
};

// Curve 422
curves[422].controlPoints = {
    {12.644, 0.000, 31.209},
    {12.644, 0.000, 31.144},
    {12.605, 0.000, 31.104},
    {12.530, 0.000, 31.089}
};

// Curve 423
curves[423].controlPoints = {
    {12.530, 0.000, 31.089},
    {12.459, 0.000, 31.079},
    {12.389, 0.000, 31.069},
    {12.319, 0.000, 31.059}
};

// Curve 424
curves[424].controlPoints = {
    {12.319, 0.000, 31.059},
    {12.218, 0.000, 31.044},
    {12.145, 0.000, 31.015},
    {12.100, 0.000, 30.975}
};

// Curve 425
curves[425].controlPoints = {
    {12.100, 0.000, 30.975},
    {12.060, 0.000, 30.935},
    {12.047, 0.000, 30.883},
    {12.062, 0.000, 30.817}
};

// Curve 426
curves[426].controlPoints = {
    {12.062, 0.000, 30.817},
    {12.078, 0.000, 30.747},
    {12.099, 0.000, 30.684},
    {12.130, 0.000, 30.628}
};

// Curve 427
curves[427].controlPoints = {
    {12.130, 0.000, 30.628},
    {12.160, 0.000, 30.568},
    {12.205, 0.000, 30.495},
    {12.266, 0.000, 30.409}
};

// Curve 428
curves[428].controlPoints = {
    {12.266, 0.000, 30.409},
    {12.591, 0.000, 29.939},
    {12.916, 0.000, 29.468},
    {13.241, 0.000, 28.997}
};

// Curve 429
curves[429].controlPoints = {
    {13.241, 0.000, 28.997},
    {13.301, 0.000, 28.906},
    {13.348, 0.000, 28.861},
    {13.383, 0.000, 28.861}
};

// Curve 430
curves[430].controlPoints = {
    {13.383, 0.000, 28.861},
    {13.418, 0.000, 28.861},
    {13.463, 0.000, 28.897},
    {13.519, 0.000, 28.967}
};

// Curve 431
curves[431].controlPoints = {
    {13.519, 0.000, 28.967},
    {13.819, 0.000, 29.380},
    {14.119, 0.000, 29.792},
    {14.419, 0.000, 30.205}
};

// Curve 432
curves[432].controlPoints = {
    {14.419, 0.000, 30.205},
    {14.524, 0.000, 30.341},
    {14.601, 0.000, 30.449},
    {14.652, 0.000, 30.530}
};

// Curve 433
curves[433].controlPoints = {
    {14.652, 0.000, 30.530},
    {14.702, 0.000, 30.610},
    {14.740, 0.000, 30.691},
    {14.766, 0.000, 30.772}
};

// Curve 434
curves[434].controlPoints = {
    {14.766, 0.000, 30.772},
    {14.791, 0.000, 30.852},
    {14.778, 0.000, 30.916},
    {14.728, 0.000, 30.961}
};

// Curve 435
curves[435].controlPoints = {
    {14.728, 0.000, 30.961},
    {14.683, 0.000, 31.006},
    {14.599, 0.000, 31.039},
    {14.478, 0.000, 31.059}
};

// Curve 436
curves[436].controlPoints = {
    {14.478, 0.000, 31.059},
    {14.418, 0.000, 31.069},
    {14.357, 0.000, 31.079},
    {14.297, 0.000, 31.089}
};

// Curve 437
curves[437].controlPoints = {
    {14.297, 0.000, 31.089},
    {14.221, 0.000, 31.104},
    {14.184, 0.000, 31.144},
    {14.184, 0.000, 31.209}
};

// Curve 438
curves[438].controlPoints = {
    {14.184, 0.000, 31.209},
    {14.184, 0.000, 31.308},
    {14.184, 0.000, 31.406},
    {14.184, 0.000, 31.505}
};

// Curve 439
curves[439].controlPoints = {
    {14.184, 0.000, 31.505},
    {14.194, 0.000, 31.515},
    {14.204, 0.000, 31.524},
    {14.214, 0.000, 31.534}
};

// Curve 440
curves[440].controlPoints = {
    {14.214, 0.000, 31.534},
    {14.979, 0.000, 31.514},
    {15.516, 0.000, 31.505},
    {15.823, 0.000, 31.505}
};

// Curve 441
curves[441].controlPoints = {
    {15.823, 0.000, 31.505},
    {15.959, 0.000, 31.505},
    {16.410, 0.000, 31.514},
    {17.175, 0.000, 31.534}
};

// Curve 442
curves[442].controlPoints = {
    {17.175, 0.000, 31.534},
    {17.185, 0.000, 31.524},
    {17.195, 0.000, 31.515},
    {17.205, 0.000, 31.505}
};

// Curve 443
curves[443].controlPoints = {
    {17.205, 0.000, 31.505},
    {17.205, 0.000, 31.406},
    {17.205, 0.000, 31.308},
    {17.205, 0.000, 31.209}
};

// Curve 444
curves[444].controlPoints = {
    {17.205, 0.000, 31.209},
    {17.205, 0.000, 31.179},
    {17.195, 0.000, 31.157},
    {17.175, 0.000, 31.142}
};

// Curve 445
curves[445].controlPoints = {
    {17.175, 0.000, 31.142},
    {17.160, 0.000, 31.127},
    {17.136, 0.000, 31.116},
    {17.106, 0.000, 31.111}
};

// Curve 446
curves[446].controlPoints = {
    {17.106, 0.000, 31.111},
    {17.081, 0.000, 31.101},
    {17.041, 0.000, 31.094},
    {16.986, 0.000, 31.089}
};

// Curve 447
curves[447].controlPoints = {
    {16.986, 0.000, 31.089},
    {16.709, 0.000, 31.064},
    {16.482, 0.000, 30.995},
    {16.306, 0.000, 30.884}
};

// Curve 448
curves[448].controlPoints = {
    {16.306, 0.000, 30.884},
    {16.135, 0.000, 30.769},
    {15.893, 0.000, 30.490},
    {15.581, 0.000, 30.047}
};

// Curve 449
curves[449].controlPoints = {
    {15.581, 0.000, 30.047},
    {15.116, 0.000, 29.392},
    {14.650, 0.000, 28.737},
    {14.184, 0.000, 28.083}
};

// Curve 450
curves[450].controlPoints = {
    {14.184, 0.000, 28.083},
    {14.154, 0.000, 28.038},
    {14.139, 0.000, 27.998},
    {14.139, 0.000, 27.963}
};

// Curve 451
curves[451].controlPoints = {
    {14.139, 0.000, 27.963},
    {14.139, 0.000, 27.937},
    {14.154, 0.000, 27.902},
    {14.184, 0.000, 27.856}
};

// Curve 452
curves[452].controlPoints = {
    {14.184, 0.000, 27.856},
    {14.617, 0.000, 27.305},
    {15.050, 0.000, 26.754},
    {15.483, 0.000, 26.203}
};

// Curve 453
curves[453].controlPoints = {
    {15.483, 0.000, 26.203},
    {15.765, 0.000, 25.846},
    {15.994, 0.000, 25.611},
    {16.170, 0.000, 25.500}
};

// Curve 454
curves[454].controlPoints = {
    {16.170, 0.000, 25.500},
    {16.347, 0.000, 25.384},
    {16.579, 0.000, 25.314},
    {16.866, 0.000, 25.289}
};

// Curve 455
curves[455].controlPoints = {
    {16.866, 0.000, 25.289},
    {16.986, 0.000, 25.289},
    {17.047, 0.000, 25.247},
    {17.047, 0.000, 25.161}
};

// Curve 456
curves[456].controlPoints = {
    {17.047, 0.000, 25.161},
    {17.047, 0.000, 25.063},
    {17.047, 0.000, 24.964},
    {17.047, 0.000, 24.866}
};

// Curve 457
curves[457].controlPoints = {
    {17.047, 0.000, 24.866},
    {17.042, 0.000, 24.856},
    {17.036, 0.000, 24.846},
    {17.031, 0.000, 24.836}
};

// Curve 458
curves[458].controlPoints = {
    {17.031, 0.000, 24.836},
    {16.518, 0.000, 24.856},
    {16.128, 0.000, 24.866},
    {15.861, 0.000, 24.866}
};

// Curve 459
curves[459].controlPoints = {
    {15.861, 0.000, 24.866},
    {15.760, 0.000, 24.866},
    {15.326, 0.000, 24.856},
    {14.561, 0.000, 24.836}
};

// Curve 460
curves[460].controlPoints = {
    {14.561, 0.000, 24.836},
    {14.548, 0.000, 24.851},
    {14.536, 0.000, 24.866},
    {14.523, 0.000, 24.881}
};

// Curve 461
curves[461].controlPoints = {
    {14.523, 0.000, 24.881},
    {14.523, 0.000, 24.980},
    {14.523, 0.000, 25.078},
    {14.523, 0.000, 25.177}
};

// Curve 462
curves[462].controlPoints = {
    {14.523, 0.000, 25.177},
    {14.523, 0.000, 25.252},
    {14.556, 0.000, 25.289},
    {14.622, 0.000, 25.289}
};

// Curve 463
curves[463].controlPoints = {
    {14.622, 0.000, 25.289},
    {14.705, 0.000, 25.299},
    {14.789, 0.000, 25.309},
    {14.872, 0.000, 25.319}
};

// Curve 464
curves[464].controlPoints = {
    {14.872, 0.000, 25.319},
    {14.993, 0.000, 25.334},
    {15.078, 0.000, 25.361},
    {15.128, 0.000, 25.402}
};

// Curve 465
curves[465].controlPoints = {
    {15.128, 0.000, 25.402},
    {15.183, 0.000, 25.437},
    {15.201, 0.000, 25.490},
    {15.181, 0.000, 25.561}
};

// Curve 466
curves[466].controlPoints = {
    {15.181, 0.000, 25.561},
    {15.166, 0.000, 25.626},
    {15.140, 0.000, 25.690},
    {15.105, 0.000, 25.750}
};

// Curve 467
curves[467].controlPoints = {
    {15.105, 0.000, 25.750},
    {15.069, 0.000, 25.805},
    {15.012, 0.000, 25.884},
    {14.931, 0.000, 25.984}
};

// Curve 468
curves[468].controlPoints = {
    {14.931, 0.000, 25.984},
    {14.596, 0.000, 26.453},
    {14.261, 0.000, 26.921},
    {13.927, 0.000, 27.389}
};

// Curve 469
curves[469].controlPoints = {
    {13.927, 0.000, 27.389},
    {13.866, 0.000, 27.465},
    {13.820, 0.000, 27.502},
    {13.784, 0.000, 27.502}
};

// Curve 470
curves[470].controlPoints = {
    {13.784, 0.000, 27.502},
    {13.734, 0.000, 27.502},
    {13.683, 0.000, 27.453},
    {13.633, 0.000, 27.358}
};

// Curve 471
curves[471].controlPoints = {
    {13.633, 0.000, 27.358},
    {13.318, 0.000, 26.895},
    {13.004, 0.000, 26.432},
    {12.689, 0.000, 25.969}
};

// Curve 472
curves[472].controlPoints = {
    {12.689, 0.000, 25.969},
    {12.619, 0.000, 25.863},
    {12.568, 0.000, 25.788},
    {12.538, 0.000, 25.742}
};

// Curve 473
curves[473].controlPoints = {
    {12.538, 0.000, 25.742},
    {12.512, 0.000, 25.692},
    {12.490, 0.000, 25.637},
    {12.470, 0.000, 25.577}
};

// Curve 474
curves[474].controlPoints = {
    {12.470, 0.000, 25.577},
    {12.450, 0.000, 25.516},
    {12.450, 0.000, 25.472},
    {12.470, 0.000, 25.447}
};

// Curve 475
curves[475].controlPoints = {
    {12.470, 0.000, 25.447},
    {12.495, 0.000, 25.422},
    {12.535, 0.000, 25.397},
    {12.591, 0.000, 25.372}
};

// Curve 476
curves[476].controlPoints = {
    {12.591, 0.000, 25.372},
    {12.646, 0.000, 25.347},
    {12.724, 0.000, 25.329},
    {12.825, 0.000, 25.319}
};

// Curve 477
curves[477].controlPoints = {
    {12.825, 0.000, 25.319},
    {12.903, 0.000, 25.309},
    {12.981, 0.000, 25.299},
    {13.059, 0.000, 25.289}
};

// Curve 478
curves[478].controlPoints = {
    {13.059, 0.000, 25.289},
    {13.140, 0.000, 25.269},
    {13.180, 0.000, 25.226},
    {13.180, 0.000, 25.161}
};

// Curve 479
curves[479].controlPoints = {
    {13.180, 0.000, 25.161},
    {13.180, 0.000, 25.068},
    {13.180, 0.000, 24.974},
    {13.180, 0.000, 24.881}
};

// Curve 480
curves[480].controlPoints = {
    {13.180, 0.000, 24.881},
    {13.165, 0.000, 24.866},
    {13.149, 0.000, 24.851},
    {13.134, 0.000, 24.836}
};

// Curve 481
curves[481].controlPoints = {
    {13.134, 0.000, 24.836},
    {12.369, 0.000, 24.856},
    {11.847, 0.000, 24.866},
    {11.570, 0.000, 24.866}
};

// Curve 482
curves[482].controlPoints = {
    {11.570, 0.000, 24.866},
    {11.419, 0.000, 24.866},
    {10.962, 0.000, 24.856},
    {10.197, 0.000, 24.836}
};




    for(auto &curve : curves) {
        curve.generate();
    }

    selectedCurve = 0;
    selectedPoint = -1;

    glutMainLoop();

    return 0;
}
