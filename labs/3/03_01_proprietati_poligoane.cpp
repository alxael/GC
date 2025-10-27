//
// =====================================================
// | Grafica pe calculator                             |
// =====================================================
// | Laboratorul III - 03_01_proprietati_poligoane.cpp |
// =====================================================
//
// 1. Program ce deseneaza:
//		(i) 2 triunghiuri din planul 2D, unul vazut din fata, iar altul din spate, folosidu-se tehnicile MODERN OpenGL;
//		Sunt ilustrate optiunile GL_FRONT, GL_BACK, etc. legate de fata/spatele poligoanelor si alte functii asociate (glCullFace(), glFrontFace());
//		(ii) un poligon concav, folosindu-se tehnicile MODERN OpenGL;
// 2. In program sunt folosite doua VBO (Vertex Buffer Object) intr-un singur VAO (Vertex Array Object).
// Un VBO contine datele pentru triunghiuri, iar celalalt pentru poligonul concav.
// 
//  Biblioteci

#include <stdlib.h>         //  Biblioteci necesare pentru citirea shaderelor;
#include <stdio.h>
#include <GL/glew.h>        //  Definește prototipurile functiilor OpenGL si constantele necesare pentru programarea OpenGL moderna; 
#include <GL/freeglut.h>    //	Include functii pentru: 
							//	- gestionarea ferestrelor si evenimentelor de tastatura si mouse, 
							//  - desenarea de primitive grafice precum dreptunghiuri, cercuri sau linii, 
							//  - crearea de meniuri si submeniuri;
#include "loadShaders.h"	//	Fisierul care face legatura intre program si shadere;

//  Identificatorii obiectelor de tip OpenGL;
GLuint
	VaoId,
	VboId1,
	VboId2,
	ColorBufferId1,
	ColorBufferId2,
	ProgramId,
	codColLocation;
//	Dimensiunea ferestrei de vizualizare;
GLint winWidth = 800, winHeight = 600;
//	Variabila ce determina schimbarea culorii pixelilor in shader;
int codCol;		

//  Se initializeaza un Vertex Buffer Object (VBO) pentru transferul datelor spre memoria placii grafice (spre shadere);
//  In acesta se stocheaza date despre varfuri (coordonate, culori, indici, texturare etc.);
void CreateVBO(void)
{
	//  Coordonatele varfurilor pentru cele doua triunghiuri;
	GLfloat Vertices1[] = {
		//	Triunghi "mic" - vazut din spate (sens ceasornic);
		-0.2f, -0.2f, 0.0f, 1.0f,	// Jos stanga;
		 0.0f,  0.2f, 0.0f, 1.0f,	// Sus;
		 0.2f, -0.2f, 0.0f, 1.0f,   // Jos dreapta;
		//	Triunghi "mare" - vazut din fata (sens trigonometric);
		-0.6f, -0.5f, 0.0f, 1.0f,   // Jos stanga;
		 0.6f, -0.5f, 0.0f, 1.0f,   // Jos dreapta;
		 0.0f,  0.6f, 0.0f, 1.0f	// Sus;
	};

	//  Culorile varfurilor pentru cele doua triunghiuri;
	GLfloat Colors1[] = {
		//	Triunghi "mic"
		1.0f, 0.0f, 0.0f, 1.0f,
		0.0f, 1.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 1.0f,
		//	Triunghi "mare"
		1.0f, 0.0f, 1.0f, 1.0f,
		1.0f, 1.0f, 0.0f, 1.0f,
		0.0f, 1.0f, 1.0f, 1.0f
	};

	// Coordonatele varfurilor pentru poligonul concav;
	GLfloat Vertices2[] = {
		//	Cele 4 varfuri ale poligonului
		 0.78f,  0.22f, 0.0f, 1.0f, // A
		 0.5f,   0.1f,  0.0f, 1.0f, // B
		 0.9f,   0.1f,  0.0f, 1.0f, // C
		 0.9f,   0.5f,  0.0f, 1.0f, // D
	};

	//  Culorile varfurilor pentru poligonul concav;
	GLfloat Colors2[] = {
		1.0f, 0.0f, 1.0f, 1.0f,
		0.8f, 1.0f, 0.2f, 1.0f,
		1.0f, 0.0f, 1.0f, 1.0f,
		0.0f, 1.0f, 1.0f, 1.0f,
	};
	//  Transmiterea datelor prin buffere;

	//  Se creeaza / se leaga un VAO (Vertex Array Object) - util cand se utilizeaza mai multe VBO;
	glGenVertexArrays(1, &VaoId);                                                   //  Generarea VAO si indexarea acestuia catre variabila VaoId;
	glBindVertexArray(VaoId);

	//  Se creeaza un buffer pentru COORDONATE (VARFURILE TRIUNGHIURILOR);
	glGenBuffers(1, &VboId1);                                                        //  Generarea bufferului si indexarea acestuia catre variabila VboId;
	glBindBuffer(GL_ARRAY_BUFFER, VboId1);                                           //  Setarea tipului de buffer - atributele varfurilor;
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices1), Vertices1, GL_STATIC_DRAW);     //  Punctele sunt "copiate" in bufferul curent;
	//  Se asociaza atributul (0 = coordonate) pentru shader;
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

	//  Se creeaza un buffer pentru CULORI (VARFURILE TRIUNGHIURILOR);
	glGenBuffers(1, &ColorBufferId1);
	glBindBuffer(GL_ARRAY_BUFFER, ColorBufferId1);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Colors1), Colors1, GL_STATIC_DRAW);
	//  Se asociaza atributul (1 =  culoare) pentru shader;
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);

	//  Se creeaza un buffer pentru COORDONATE (VARFURILE POLIGONULUI);
	glGenBuffers(1, &VboId2);                                                        //  Generarea bufferului si indexarea acestuia catre variabila VboId;
	glBindBuffer(GL_ARRAY_BUFFER, VboId2);                                           //  Setarea tipului de buffer - atributele varfurilor;
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices2), Vertices2, GL_STATIC_DRAW);      //  Punctele sunt "copiate" in bufferul curent;
	//  Se asociaza atributul (0 = coordonate) pentru shader;
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
	//  Se creeaza un buffer pentru CULORI (VARFURILE POLIGONULUI);
	glGenBuffers(1, &ColorBufferId2);
	glBindBuffer(GL_ARRAY_BUFFER, ColorBufferId2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Colors2), Colors2, GL_STATIC_DRAW);
	//  Se asociaza atributul (1 =  culoare) pentru shader;
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);
}

//  Eliminarea obiectelor de tip VBO dupa rulare;
void DestroyVBO(void)
{
	//  Eliberarea atributelor din shadere (pozitie, culoare, texturare etc.);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);

	//  Stergerea bufferelor pentru varfuri, culori;
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &ColorBufferId1);
	glDeleteBuffers(1, &VboId1);
	glDeleteBuffers(1, &ColorBufferId2);
	glDeleteBuffers(1, &VboId2);

	//  Eliberaea obiectelor de tip VAO;
	glBindVertexArray(0);
	glDeleteVertexArrays(1, &VaoId);
}

//  Crearea si compilarea obiectelor de tip shader;
//	Trebuie sa fie in acelasi director cu proiectul actual;
//  Shaderul de varfuri / Vertex shader - afecteaza geometria scenei;
//  Shaderul de fragment / Fragment shader - afecteaza culoarea pixelilor;
void CreateShaders(void)
{
	ProgramId = LoadShaders("../labs/3/03_01_Shader.vert", "../labs/3/03_01_Shader.frag");
	glUseProgram(ProgramId);
}

//  Elimina obiectele de tip shader dupa rulare;
void DestroyShaders(void)
{
	glDeleteProgram(ProgramId);
}

//  Setarea parametrilor necesari pentru fereastra de vizualizare;
void Initialize(void)
{
	glClearColor(1.0f, 1.0f, 1.0f, 0.0f);   //  Culoarea de fond a ecranului;
	CreateVBO();                            //  Trecerea datelor de randare spre bufferul folosit de shadere;
	CreateShaders();                        //  Initializarea shaderelor;
	codColLocation = glGetUniformLocation(ProgramId, "codColShader"); //	Instantierea variabilei uniforme;
}

//  Functia de desenare;
void RenderFunction(void)
{
	glClear(GL_COLOR_BUFFER_BIT);			//  Se curata ecranul OpenGL pentru a fi desenat noul continut;

	// Cele doua triunghiuri sunt desenate folosind acelasi VBO, deci trebuie "legate" din nou bufferele 1
	glBindBuffer(GL_ARRAY_BUFFER, VboId1);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, ColorBufferId1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);

	// Setarea dimensiunii segmentelor;
	glLineWidth(8.0);

	//	Setarea parametrilor privind fata/spatele poligoanelor;
	glPolygonMode(GL_FRONT, GL_LINE);		//	Modul de desenare in functie de orientare - ex: cu fata => deseneaza conturul, invers altfel;
	//	Modificare 1: schimbare fata poligonului
	//	Modificare 2: executate randurile de mai jos; testate si GL_FRONT, GL_FRONT_AND_BACK
	//	glEnable (GL_CULL_FACE);			// cull face
    //	glCullFace (GL_BACK);
	//	Modificare 3: executat randul de mai jos, combinate modificarile 2 si 3
	//	glFrontFace(GL_CW);
	//  Modificare 4: de folosit variabile de tipul GLuint pentru alegerea fata/spate GLuint mode1=GL_FRONT, etc.;

	//	Desenarea triunghiului "mic";
	codCol = 0;
	glUniform1i(codColLocation, codCol); //	Schimbarea variabilei din shader cu valoarea codCol;
	glDrawArrays(GL_TRIANGLES, 0, 3);

	//	Desenarea triunghiului "mare";
	codCol = 1;  //	Schimbarea variabilei din shader cu valoarea codCol;
	glUniform1i(codColLocation, codCol);
	glDrawArrays(GL_TRIANGLES, 3, 3);
 

	// Desenarea poligonului concav;
	// Poligonul este desenat folosind al doilea VBO, deci trebuie "legate" din nou bufferele
	glBindBuffer(GL_ARRAY_BUFFER, VboId2);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, ColorBufferId2);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, 0);
	
	// Setarea unor parametri pentru desenarea poligonului;
	glLineWidth(3.0);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	codCol = 0;
	glUniform1i(codColLocation, codCol);
	// Functia de desenare propriu-zisa;
	glDrawArrays(GL_POLYGON, 0, 4);

	glFlush();       //  Asigura rularea tuturor comenzilor OpenGL apelate anterior;
}

//  Functia de eliberare a resurselor alocate de program;
void Cleanup(void)
{
	DestroyShaders();
	DestroyVBO();
}

//	Punctul de intrare in program, se ruleaza rutina OpenGL;
int main(int argc, char* argv[])
{
	//  Se initializeaza GLUT si contextul OpenGL si se configureaza fereastra si modul de afisare;

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);				//	Modul de afisare al ferestrei, se foloseste un singur buffer de afisare si culori RGB;
	glutInitWindowPosition(100, 100);							//  Pozitia initiala a ferestrei;
	glutInitWindowSize(winWidth, winHeight);							 //  Dimensiunea ferestrei;
	glutCreateWindow("Fata si spatele poligoanelor");   //	Creeaza fereastra de vizualizare, indicand numele acesteia;

	//	Se initializeaza GLEW si se verifica suportul de extensii OpenGL modern disponibile pe sistemul gazda;
	//  Trebuie initializat inainte de desenare;

	glewInit();

	Initialize();                       //  Setarea parametrilor necesari pentru afisare;
	glutDisplayFunc(RenderFunction);    //  Desenarea scenei in fereastra;
	glutCloseFunc(Cleanup);             //  Eliberarea resurselor alocate de program;

	//  Bucla principala de procesare a evenimentelor GLUT (functiile care incep cu glut: glutInit etc.) este pornita;
	//  Prelucreaza evenimentele si deseneaza fereastra OpenGL pana cand utilizatorul o inchide;

	glutMainLoop();

	return 0;
}
