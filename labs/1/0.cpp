#include <iostream>
#include <GL/glew.h>
#include <GLUT/glut.h>

int main(int argc, char **argv)
{
	glutInit (&argc, argv); 
	glutInitDisplayMode (GLUT_SINGLE | GLUT_RGB);   //	Modul de afisare al ferestrei, se foloseste un singur buffer de afisare si culori RGB;
	glutInitWindowPosition (1100, 100);				//	Pozitia initiala a ferestrei;
	glutInitWindowSize (600, 400);					//	Dimensiunea ferestrei;
	int winID = glutCreateWindow ("GL_VERSION");	//	Creeaza fereastra de vizualizare, indicand numele acesteia;
	glutDestroyWindow(winID);

	//	Se initializeaza GLEW si se verifica suportul de extensii OpenGL modern disponibile pe sistemul gazda;
	//  Trebuie initializat inainte de desenare;

	GLenum err = glewInit();
	if (err != GLEW_OK)
	{
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		exit(-1);
	}

	//	Afisarea versiunilor de OpenGL si GLSL in consola;

	printf("OpenGL version supported by this platform: (%s) \n", glGetString(GL_VERSION)); 
	printf("GLSL version supported by this platform: (%s) \n", glGetString(GL_SHADING_LANGUAGE_VERSION)); 
    return 0;
}