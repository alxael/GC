#include <stdlib.h>
#include <stdio.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

#include "loadShaders.h"

GLuint
    VaoId,
    VboId,
    ColorBufferId,
    ProgramId,
    codColLocation;

GLint winWidth = 600, winHeight = 400;

int codCol;

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
        {Position(-0.2f, -0.2f, 0.0f), Color(1.0f, 0.0f, 0.0f)},
        {Position(0.0f, 0.2f, 0.0f), Color(0.0f, 1.0f, 0.0f)},
        {Position(0.2f, -0.2f, 0.0f), Color(0.0f, 0.0f, 1.0f)},
        {Position(-0.8f, -0.8f, 0.0f), Color(1.0f, 0.0f, 0.0f)},
        {Position(0.0f, 0.8f, 0.0f), Color(0.0f, 1.0f, 0.0f)},
        {Position(0.8f, -0.8f, 0.0f), Color(0.0f, 0.0f, 1.0f)}};

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
    ProgramId = LoadShaders("../assignments/2/shader.vert", "../assignments/2/shader.frag");
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

    codColLocation = glGetUniformLocation(ProgramId, "codColShader");
}

void RenderFunction(void)
{
    glClear(GL_COLOR_BUFFER_BIT);

    glUniform1i(codColLocation, codCol);
    glLineWidth(5.0);
    glDrawArrays(GL_LINE_STRIP, 0, 6);

    glUniform1i(codColLocation, codCol);
    glPointSize(20.0);
    glDrawArrays(GL_POINTS, 0, 6);
    glDisable(GL_POINT_SMOOTH);

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
            codCol = 1;
            glUniform1i(codColLocation, codCol);
        }
        break;
    case GLUT_RIGHT_BUTTON:
        if (state == GLUT_DOWN)
        {
            codCol = 0;
            glUniform1i(codColLocation, codCol);
        }
        break;
    default:
        break;
    }
    glutPostRedisplay();
}

int main(int argc, char *argv[])
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowPosition(100, 300);
    glutInitWindowSize(800, 800);
    glutCreateWindow("Varfuri triunghi");

    glewInit();

    Initialize();
    glutDisplayFunc(RenderFunction);
    glutMouseFunc(UseMouse);
    glutCloseFunc(Cleanup);

    glutMainLoop();

    return 0;
}