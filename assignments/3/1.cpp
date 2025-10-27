#include <stdlib.h>
#include <stdio.h>
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
    ColorBufferId,
    ProgramId,
    colorCodeLocation,
    myMatrixLocation;

GLint winWidth = 800, winHeight = 800;

int colorCode, translationCode;

glm::mat4 myMatrix, resizeMatrix, rotationMatrix, referenceTranslationMatrix, reverseTranslationMatrix;

float xMin = -1000, xMax = 1000, yMin = -1000, yMax = 1000;
float deltaX = xMax - xMin, deltaY = yMax - yMin;
float centerX = (xMin + xMax) / 2, centerY = (yMin + yMax) / 2;

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
    Vertex vertices[] = {
        // corners
        {Position(xMin, yMin, 0.0f), Color(0.0f, 0.0f, 1.0f)},
        {Position(xMin, yMax, 0.0f), Color(0.0f, 1.0f, 0.0f)},
        {Position(xMax, yMax, 0.0f), Color(1.0f, 0.0f, 0.0f)},
        {Position(xMax, yMin, 0.0f), Color(1.0f, 1.0f, 1.0f)},
        // axis lines
        {Position(xMin, 0.0f, 0.0f), Color(0.0f, 0.0f, 0.0f)},
        {Position(xMax, 0.0f, 0.0f), Color(0.0f, 0.0f, 0.0f)},
        {Position(0.0f, yMin, 0.0f), Color(0.0f, 0.0f, 0.0f)},
        {Position(0.0f, yMax, 0.0f), Color(0.0f, 0.0f, 0.0f)},
        // convex polygon
        {Position(100.0f, 100.0f, 0.0f), Color(1.0f, 0.0f, 0.0f)},
        {Position(300.0f, 100.0f, 0.0f), Color(0.0f, 0.0f, 1.0f)},
        {Position(500.0f, 300.0f, 0.0f), Color(0.0f, 1.0f, 0.0f)},
        {Position(300.0f, 500.0f, 0.0f), Color(1.0f, 0.0f, 0.0f)},
        {Position(100.0f, 300.0f, 0.0f), Color(0.0f, 1.0f, 0.0f)},
        // concave polygon
        {Position(700.0f, 500.0f, 0.0f), Color(1.0f, 0.0f, 0.0f)},
        {Position(900.0f, 900.0f, 0.0f), Color(0.0f, 1.0f, 0.0f)},
        {Position(700.0f, 800.0f, 0.0f), Color(0.0f, 1.0f, 0.0f)},
        {Position(500.0f, 900.0f, 0.0f), Color(0.0f, 0.0f, 1.0f)},
    };

    glGenVertexArrays(1, &VaoId);
    glBindVertexArray(VaoId);

    glGenBuffers(1, &VboId);
    glBindBuffer(GL_ARRAY_BUFFER, VboId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

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
}

void DestroyVBO(void)
{
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &ColorBufferId);
    glDeleteBuffers(1, &VboId);

    glBindVertexArray(0);
    glDeleteVertexArrays(1, &VaoId);
}

void CreateShaders(void)
{
    ProgramId = LoadShaders("../assignments/3/shader.vert", "../assignments/3/shader.frag");
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

    colorCodeLocation = glGetUniformLocation(ProgramId, "colorCodeShader");
    myMatrixLocation = glGetUniformLocation(ProgramId, "myMatrix");

    resizeMatrix = glm::ortho(xMin, xMax, yMin, yMax);

    GLfloat angle = glm::radians(70.0f);

    rotationMatrix = glm::rotate(glm::mat4(1.0f), angle, glm::vec3(0.0, 0.0, 1.0));
    referenceTranslationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(-300.f, -300.f, 0.0));
    reverseTranslationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(100.f, 100.f, 0.0));
}

void RenderFunction(void)
{
    glClear(GL_COLOR_BUFFER_BIT);

    myMatrix = resizeMatrix;
    glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);

    // draw background
    colorCode = 0;
    glUniform1i(colorCodeLocation, colorCode);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    // draw axis lines
    colorCode = 1;
    glUniform1i(colorCodeLocation, colorCode);
    glLineWidth(2.0);
    glDrawArrays(GL_LINES, 4, 4);

    if (translationCode == 1)
    {
        myMatrix = resizeMatrix * reverseTranslationMatrix * rotationMatrix * referenceTranslationMatrix;
        glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);
    }

    // draw polygons
    colorCode = 0;
    glUniform1i(colorCodeLocation, colorCode);
    glDrawArrays(GL_TRIANGLE_FAN, 8, 5);
    glDrawArrays(GL_TRIANGLE_FAN, 13, 4);

    glFlush();
}

void Cleanup(void)
{
    DestroyShaders();
    DestroyVBO();
}

void UseMouse(int button, int state, int x, int y)
{
    switch (button)
    {
    case GLUT_LEFT_BUTTON:
        if (state == GLUT_DOWN)
        {
            colorCode = 1;
            translationCode = 0;
            glUniform1i(colorCodeLocation, colorCode);
        }
        break;
    case GLUT_RIGHT_BUTTON:
        if (state == GLUT_DOWN)
        {
            colorCode = 0;
            translationCode = 1;
            glUniform1i(colorCodeLocation, colorCode);
        }
        break;
    default:
        break;
    }
    glutPostRedisplay();
}

int main(int argc, char *argv[])
{
    translationCode = 0;

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(winWidth, winHeight);
    glutCreateWindow("Lab 3");

    glewInit();

    Initialize();
    glutDisplayFunc(RenderFunction);
    glutMouseFunc(UseMouse);
    glutCloseFunc(Cleanup);

    glutMainLoop();

    return 0;
}