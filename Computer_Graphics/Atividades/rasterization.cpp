#include <GL/glut.h>
#include <stdio.h>
#include <math.h>

int radius = 100; 

void display(void){    
    glBegin(GL_POINT);
        for(int i = 0; i < 5000; i++){
            glVertex2f(radius * cos(i), radius * sin(i))
        }
    glEnd();
    
    glFlush();
}

int main(int argc,char *argv[]){
    glutInit(&argc,argv);
    glutInitDisplayMode(GLUT_SINGLE|GLUT_RGB);
    glutInitWindowSize(600,600);
    glutInitWindowPosition(100,100);
    glutCreateWindow("Polygon Clipping!");
    glutDisplayFunc(display);
    glutMainLoop();
    return 0;
}