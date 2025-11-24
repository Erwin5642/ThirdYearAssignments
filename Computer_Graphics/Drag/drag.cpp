#include <GL/glut.h>
#include <GL/glu.h>
#include <GL/gl.h>
#include <iostream>

using namespace std;

#define HEIGHT 480
#define WIDTH 640

int keyboard_event_x = 0;
int keyboard_event_y = 0;
double objx = 0, objy=0, objz=0;
double angle_x = 0, angle_y = 0;
double move_x = 0;
double move_y = 0;
double move_z = 0;
int move_cube = 0;
/*THE FUNCTION TO DRAW THE STUFF ON THE SCREEN*/
void display( )
{
    glClear( GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT );
    int offset = 10;
    //The big green floor like polygon
    //The big green floor like polygon
    //The big green floor like polygon
    if (keyboard_event_x == 1){
        glRotatef(angle_x, 0.0, 1.0, 0.0);

    }
    else if (keyboard_event_y == 1){
        //glRotatef(angle_x, 0.0, 1.0, 0.0);
        glRotatef(angle_y, 1.0, 0.0, 0.0);
    }
    //glRotatef(angle_y, 1.0, 0.0, 0.0);
    glPushMatrix();
    glBegin( GL_LINES );
        glColor3f(0, 0, 1);
        glVertex3f( 0, 0,  100  );
        glVertex3f( 0, 0, -100  );
    glEnd( );
    glBegin( GL_LINES );
        glColor3f(1, 0, 0);
        glVertex3f( -80, 0,  0  );
        glVertex3f( 80, 0,  0  );
    glEnd( );
    glColor3f(0.8, 0.8, 0.8);
    for (int i = 0; i < 170 ; i+=10){
        glBegin( GL_LINES );
        glVertex3f( -80+i, 0,  100  );
        glVertex3f( -80+i, 0, -100  );
        glEnd( );
    }
    //glColor3f(0, 1, 0);
    for (int i = 0; i < 220 ; i+= 20){
        glBegin( GL_LINES );
        glVertex3f( -80, 0,  100-i  );
        glVertex3f(  80, 0,  100-i  );
        glEnd( );
    }

    //The red cube to be drawn at clicked position
    glColor3f(0.8, 0.8, 0.8);
    glPushMatrix( );
        glColor3f( 1, 0, 0 );
        glTranslatef( objx, objy, objz );
        glTranslatef(move_x, move_y, move_z);
        glBegin( GL_LINES );
        glColor3f(0, 0, 1);
        glVertex3f( 0, 0,  0  );
        glVertex3f( 0, 0, 40  );
        glEnd( );
        glBegin( GL_LINES );
        glColor3f(1, 0, 0);
        glVertex3f( 0, 0,  0  );
        glVertex3f( 40, 0,  0  );
        glEnd( );
        glBegin( GL_LINES );
        glColor3f(0, 1, 0);
        glVertex3f( 0, 0,  0  );
        glVertex3f( 0, 40,  0  );
        glEnd( );
        glutSolidCube( 17 );
    glPopMatrix( );


    glPopMatrix();
    glFlush( );
    glutSwapBuffers( );
}

void mouse( int button, int state, int x, int y)
{
    double modelview[16], projection[16];
    int viewport[4];
    float z = 1 - 0.0001;
    //float z = 1 - 0.001;

    if (button == GLUT_LEFT_BUTTON) {
        if (state == GLUT_DOWN) {

            /*Read the projection, modelview and viewport matrices
            using the glGet functions.*/
            glGetDoublev( GL_PROJECTION_MATRIX, projection );
            glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
            glGetIntegerv( GL_VIEWPORT, viewport );

            //Read the window z value from the z-buffer
            //glReadPixels( x, viewport[3]-y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &z );
            //if (z == 1) z -= 0.00001;
            //Use the gluUnProject to get the world co-ordinates of
            //the point the user clicked and save in objx, objy, objz.
            gluUnProject( x, viewport[3]-y, z, modelview, projection, viewport, &objx, &objy, &objz );
            cout << objx <<"," << objy <<"," << objz <<"," << z << endl;
            keyboard_event_y = 0;
            keyboard_event_x = 0;
            move_x = 0;
            move_y = 0;
            move_z = 0;
        }
    }
    glutPostRedisplay();
}
void keyboard( int key, int x, int y )
{


    if (key == GLUT_KEY_RIGHT){
        keyboard_event_x = 1;
        keyboard_event_y = 0;
        angle_x=2.0;
        glutPostRedisplay();
        //cout << "aqui" << endl;
    }

    else if(key == GLUT_KEY_LEFT){
        keyboard_event_x = 1;
        keyboard_event_y = 0;
        angle_x=-2.0;
        glutPostRedisplay();
        //cout << "aqui-1" << endl;
    }

    else if(key == GLUT_KEY_UP){
        keyboard_event_y = 1;
        keyboard_event_x = 0;
        angle_y=2.0;
        glutPostRedisplay();
        //cout << "aqui-3" << endl;
    }

    else if (key ==  GLUT_KEY_DOWN) {
        keyboard_event_y = 1;
        keyboard_event_x = 0;
        angle_y=-2.0;
        glutPostRedisplay();
        //cout << "aqui-4" << endl;
    }

}

void keyInput(unsigned char key, int x, int y)
{
    keyboard_event_y = keyboard_event_x = 0;
    switch (key) {
        case 'x':
            move_x+=1;
            cout << "dentro" << endl;
            break;
        case 'X':
            move_x-=1;
            break;
        case 'y':
            move_y+=1;
            break;
        case 'Y':
            move_y-=1;
            break;
        case 'z':
            move_z+=1;
            break;
        case 'Z':
            move_z-=1;
            break;

  }
  glutPostRedisplay();
}

void init( int width, int height )
{
        glClearColor( 0.3, 0.3, 0.3, 1 );
        glViewport( 0, 0, width, height );
        glMatrixMode( GL_PROJECTION );
        glLoadIdentity( );
        gluPerspective( 45, 1.33, 0.1, 400 );
        glMatrixMode( GL_MODELVIEW );
        glLoadIdentity( );
        gluLookAt( -50, 100, 250, 0, 0, 0, 0, 1, 0 );
}

int main( int argc, char **argv )
{
        glutInit( &argc, argv );
        //The most important part specify the things your
        //glut window should provide
        glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH );
        glutInitWindowSize( 1920, 1200 );
        glutCreateWindow("Teste usando gluUnProject");

        //enable z buffer
        glEnable( GL_DEPTH_TEST );
        //set the value in z-buffer as 1.0
        glClearDepth( 1.0 );
        init( 1920, 1200 );
        glutDisplayFunc( display );
        glutReshapeFunc( init );
        //glutIdleFunc( display );
        glutMouseFunc( mouse );
        glutKeyboardFunc(keyInput);
        glutSpecialFunc( keyboard );
        glutMainLoop( );
}