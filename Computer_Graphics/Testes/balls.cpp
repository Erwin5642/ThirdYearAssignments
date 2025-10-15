#include <GL/glut.h>

#define NUM_BALLS 100

void drawScene()
{
    for(int i = 0; i < NUM_BALLS; i++){
        glBegin(GL_LINE_LOOP);
        for(t = -10 * PI; t <= 10 * PI; t += PI/20.0) 
            glVertex3f(20 * cos(t), 20 * sin(t), t);
        glEnd();
    }
}

void setup()
{
    glClearColor(1.0, 1.0, 1.0, 0.0);
}

void resize(int w, int h)
{
    glViewport(0, 0, (GLsizei)w, (GLsizei)h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-5.0, 5.0, -5.0, 5.0, 5.0, 100.0);

    glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(500, 500);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("balls");
    setup();
    glutDisplayFunc(drawScene);
    glutReshapeFunc(resize);
    glutMainLoop();

    return 0;
}