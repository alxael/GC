#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <random>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "loadShaders.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtc/type_ptr.hpp"

GLuint
	VaoId,
	VboId,
	EboId,
	ColorBufferId,
	ProgramId,
	colorCodeLocation,
	viewLocation,
	projectionLocation;

GLint winWidth = 800, winHeight = 800;

int colorCode;

glm::vec3 observer = glm::vec3(200.f, 200.f, 200.f);
glm::vec3 referencePoint = glm::vec3(0.f, 0.f, 0.f);
glm::vec3 vertex = glm::vec3(0.f, 1.f, 1.f);

glm::mat4 view, projection;

float xMin = -200.f, xMax = 200.f, yMin = -200.f, yMax = 200.f, zMin = -200.f, zMax = 200.f;
float distanceNear = 0.f, distanceFar = 1900.f;
float fov = 90.f * glm::pi<float>() / 180.f;

bool keyStates[256] = {0};
bool specialKeyStates[512] = {0};

struct Position
{
	GLfloat data[4];

	Position(GLfloat x, GLfloat y, GLfloat z)
	{
		data[0] = x;
		data[1] = y;
		data[2] = z;
		data[3] = 1.0f;
	}

	Position(GLfloat x, GLfloat y, GLfloat z, GLfloat t)
	{
		data[0] = x;
		data[1] = y;
		data[2] = z;
		data[3] = t;
	}
};

struct Color
{
	GLfloat data[4];

	Color(GLfloat r, GLfloat g, GLfloat b)
	{
		data[0] = r;
		data[1] = g;
		data[2] = b;
		data[3] = 1.0f;
	}

	Color(GLfloat r, GLfloat g, GLfloat b, GLfloat t)
	{
		data[0] = r;
		data[1] = g;
		data[2] = b;
		data[3] = t;
	}
};

struct Vertex
{
	Position position;
	Color color;
};

void CreateVBO(void)
{
	std::vector<Vertex> vertices = {
		{Position(50.f, 50.0f, 50.0f), Color(1.0f, 0.0f, 0.0f)},
		{Position(50.f, -50.f, -50.f), Color(0.0f, 1.0f, 0.0f)},
		{Position(-50.f, 50.f, -50.f), Color(0.0f, 0.0f, 1.0f)},
		{Position(-50.f, -50.f, 50.f), Color(1.0f, 1.0f, 0.0f)},
		{Position(xMin, 0.0f, 0.0f), Color(0.0f, 0.0f, 0.0f)},
		{Position(xMax, 0.0f, 0.0f), Color(0.0f, 0.0f, 0.0f)},
		{Position(0.0f, yMin, 0.0f), Color(0.0f, 0.0f, 0.0f)},
		{Position(0.0f, yMax, 0.0f), Color(0.0f, 0.0f, 0.0f)},
		{Position(0.0f, 0.0f, zMin), Color(0.0f, 0.0f, 0.0f)},
		{Position(0.0f, 0.0f, zMax), Color(0.0f, 0.0f, 0.0f)},
	};

	std::vector<GLuint> indices = {
		0,
		1,
		2, // first face
		0,
		1,
		3, // second face
		0,
		2,
		3, // third face
		1,
		2,
		3, // fourth face
	};

	glGenVertexArrays(1, &VaoId);
	glBindVertexArray(VaoId);

	glGenBuffers(1, &VboId);
	glBindBuffer(GL_ARRAY_BUFFER, VboId);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
		0,
		4,
		GL_FLOAT,
		GL_FALSE,
		sizeof(Vertex),
		(void *)offsetof(Vertex, position));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(
		1,
		4,
		GL_FLOAT,
		GL_FALSE,
		sizeof(Vertex),
		(void *)offsetof(Vertex, color));

	glGenBuffers(1, &EboId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EboId);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);
}

void DestroyVBO(void)
{
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDeleteBuffers(1, &ColorBufferId);
	glDeleteBuffers(1, &VboId);
	glDeleteBuffers(1, &EboId);

	glBindVertexArray(0);
	glDeleteVertexArrays(1, &VaoId);
}

void CreateShaders(void)
{
	ProgramId = LoadShaders("../assignments/5/shader.vert", "../assignments/5/shader.frag");
	glUseProgram(ProgramId);
}

void DestroyShaders(void)
{
	glDeleteProgram(ProgramId);
}

void Initialize(void)
{
	glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
	CreateVBO();
	CreateShaders();

	viewLocation = glGetUniformLocation(ProgramId, "view");
	projectionLocation = glGetUniformLocation(ProgramId, "projection");
	colorCodeLocation = glGetUniformLocation(ProgramId, "colorCodeShader");
}

void ProcessKeys(void)
{
	if (keyStates['w'])
	{
		observer += glm::vec3(0.f, 0.f, 10.f);
	}
	if (keyStates['s'])
	{
		observer += glm::vec3(0.f, 0.f, -10.f);
	}
	if (specialKeyStates[GLUT_KEY_LEFT])
	{
		observer += glm::vec3(-10.f, 0.f, 0.f);
	}
	if (specialKeyStates[GLUT_KEY_RIGHT])
	{
		observer += glm::vec3(10.f, 0.f, 0.f);
	}
	if (specialKeyStates[GLUT_KEY_UP])
	{
		observer += glm::vec3(0.f, 10.f, 0.f);
	}
	if (specialKeyStates[GLUT_KEY_DOWN])
	{
		observer += glm::vec3(0.f, -10.f, 0.f);
	}
	if (keyStates['a'])
	{
		vertex += (0.1f, 0.f, 0.f);
	}
	if (keyStates['d'])
	{
		vertex += (-0.1f, 0.f, 0.f);
	}
}

void DrawObserverPosition(void)
{
	glUseProgram(0);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0, winWidth, 0, winHeight);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();

	glDisable(GL_DEPTH_TEST);
	glColor3f(0.0f, 0.0f, 0.0f);

	char observerStr[128];
	snprintf(observerStr, sizeof(observerStr), "(%.1f, %.1f, %.1f)", observer.x, observer.y, observer.z);

	int textX = winWidth - (int)(8 * strlen(observerStr)) - 10;
	int textY = winHeight - 20;

	glRasterPos2i(textX, textY);

	for (const char *c = observerStr; *c != '\0'; c++)
	{
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
	}

	glEnable(GL_DEPTH_TEST);

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();

	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glUseProgram(ProgramId);
}

void RenderFunction(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glEnable(GL_DEPTH_TEST);

	ProcessKeys();

	glBindVertexArray(VaoId);
	glBindBuffer(GL_ARRAY_BUFFER, VboId);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EboId);

	view = glm::lookAt(observer, referencePoint, vertex);
	glUniformMatrix4fv(viewLocation, 1, GL_FALSE, &view[0][0]);

	projection = glm::ortho(xMin, xMax, yMin, yMax, distanceNear, distanceFar);
	glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, &projection[0][0]);

	colorCode = 1;
	glUniform1i(colorCodeLocation, colorCode);
	glLineWidth(2.0);
	glDrawArrays(GL_LINES, 4, 8);

	colorCode = 0;
	glUniform1i(colorCodeLocation, colorCode);
	glDrawElements(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0);

	colorCode = 1;
	glUniform1i(colorCodeLocation, colorCode);
	glDrawElements(GL_LINE_LOOP, 12, GL_UNSIGNED_INT, 0);

	DrawObserverPosition();

	glutSwapBuffers();
	glFlush();
}

void Cleanup(void)
{
	DestroyShaders();
	DestroyVBO();
}

// key functions

void handleKeyDown(unsigned char key, int, int)
{
	keyStates[key] = true;
	if (key == 27)
		exit(0);
}
void handleKeyUp(unsigned char key, int, int) { keyStates[key] = false; }
void handleSpecialDown(int key, int, int) { specialKeyStates[key] = true; }
void handleSpecialUp(int key, int, int) { specialKeyStates[key] = false; }

int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(winWidth, winHeight);
	glutCreateWindow("Lab 5 - 1");

	glewInit();

	Initialize();
	glutDisplayFunc(RenderFunction);
	glutIdleFunc(RenderFunction);

	glutKeyboardFunc(handleKeyDown);
	glutKeyboardUpFunc(handleKeyUp);
	glutSpecialFunc(handleSpecialDown);
	glutSpecialUpFunc(handleSpecialUp);

	glutCloseFunc(Cleanup);

	glutMainLoop();

	return 0;
}