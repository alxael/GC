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
    VaoHouseId,
    VaoTreeId,
    VboHouseId,
    VboTreeId,
    EboHouseId,
    EboTreeId,
    ProgramId,
    colorCodeLocation,
    myMatrixLocation;

GLint winWidth = 800, winHeight = 800;

int colorCode;

glm::mat4 myMatrix, resizeMatrix;

float xMin = -1000, xMax = 1000, yMin = -1000, yMax = 1000;

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
};

struct Vertex
{
    Position position;
    Color color;
};

void CreateVBOs(void)
{
    // House vertices (simple rectangle + triangle roof)
    std::vector<Vertex> houseVertices = {
        // House base (rectangle)
        {Position(-400.f, -400.0f, 0.0f), Color(0.65f, 0.30f, 0.16f)}, // bottom left
        {Position(-100.f, -400.0f, 0.0f), Color(0.65f, 0.30f, 0.16f)}, // bottom right
        {Position(-100.0f, 0.0f, 0.0f), Color(0.65f, 0.30f, 0.16f)},   // top right
        {Position(-400.0f, 0.0f, 0.0f), Color(0.65f, 0.30f, 0.16f)},   // top left
        // Roof (triangle)
        {Position(-450.f, 0.0f, 0.0f), Color(0.8f, 0.1f, 0.1f)},       // left roof
        {Position(-50.f, 0.0f, 0.0f), Color(0.8f, 0.1f, 0.1f)},        // right roof
        {Position(-250.f, 200.f, 0.0f), Color(0.8f, 0.1f, 0.1f)},      // top roof
    };

    std::vector<GLuint> houseIndices = {
        // Base rectangle
        0, 1, 2,
        2, 3, 0,
        // Roof triangle
        4, 5, 6
    };

    glGenVertexArrays(1, &VaoHouseId);
    glBindVertexArray(VaoHouseId);

    glGenBuffers(1, &VboHouseId);
    glBindBuffer(GL_ARRAY_BUFFER, VboHouseId);
    glBufferData(GL_ARRAY_BUFFER, houseVertices.size() * sizeof(Vertex), houseVertices.data(), GL_STATIC_DRAW);

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

    glGenBuffers(1, &EboHouseId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EboHouseId);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, houseIndices.size() * sizeof(GLuint), houseIndices.data(), GL_STATIC_DRAW);

    // Tree vertices (simple rectangle trunk + triangle foliage)
    std::vector<Vertex> treeVertices = {
        // Trunk (rectangle)
        {Position(200.f, -400.0f, 0.0f), Color(0.4f, 0.2f, 0.0f)},
        {Position(300.f, -400.0f, 0.0f), Color(0.4f, 0.2f, 0.0f)},
        {Position(300.f, -100.0f, 0.0f), Color(0.4f, 0.2f, 0.0f)},
        {Position(200.f, -100.0f, 0.0f), Color(0.4f, 0.2f, 0.0f)},
        // Foliage (triangle)
        {Position(175.f, -100.0f, 0.0f), Color(0.0f, 0.8f, 0.0f)},
        {Position(325.f, -100.0f, 0.0f), Color(0.0f, 0.8f, 0.0f)},
        {Position(250.f, 150.f, 0.0f), Color(0.0f, 0.8f, 0.0f)},
    };

    std::vector<GLuint> treeIndices = {
        0, 1, 2,
        2, 3, 0,
        4, 5, 6
    };

    glGenVertexArrays(1, &VaoTreeId);
    glBindVertexArray(VaoTreeId);

    glGenBuffers(1, &VboTreeId);
    glBindBuffer(GL_ARRAY_BUFFER, VboTreeId);
    glBufferData(GL_ARRAY_BUFFER, treeVertices.size() * sizeof(Vertex), treeVertices.data(), GL_STATIC_DRAW);

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

    glGenBuffers(1, &EboTreeId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EboTreeId);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, treeIndices.size() * sizeof(GLuint), treeIndices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);
}

void DestroyVBOs(void)
{
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &VboHouseId);
    glDeleteBuffers(1, &EboHouseId);
    glDeleteBuffers(1, &VboTreeId);
    glDeleteBuffers(1, &EboTreeId);

    glBindVertexArray(0);
    glDeleteVertexArrays(1, &VaoHouseId);
    glDeleteVertexArrays(1, &VaoTreeId);
}

void CreateShaders(void)
{
    ProgramId = LoadShaders("../assignments/4/shader.vert", "../assignments/4/shader.frag");
    glUseProgram(ProgramId);
}

void DestroyShaders(void)
{
    glDeleteProgram(ProgramId);
}

void Initialize(void)
{
    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
    CreateVBOs();
    CreateShaders();

    colorCodeLocation = glGetUniformLocation(ProgramId, "colorCodeShader");
    myMatrixLocation = glGetUniformLocation(ProgramId, "myMatrix");

    resizeMatrix = glm::ortho(xMin, xMax, yMin, yMax);
}

void RenderFunction(void)
{
    glClear(GL_COLOR_BUFFER_BIT);

    myMatrix = resizeMatrix;
    glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);

    colorCode = 1;
    glUniform1i(colorCodeLocation, colorCode);
    glLineWidth(2.0);

    // Draw the house
    glBindVertexArray(VaoHouseId);
    glDrawElements(GL_TRIANGLES, 9, GL_UNSIGNED_INT, 0);

    // Draw the tree
    glBindVertexArray(VaoTreeId);
    glDrawElements(GL_TRIANGLES, 9, GL_UNSIGNED_INT, 0);

    glFlush();
}

void Cleanup(void)
{
    DestroyShaders();
    DestroyVBOs();
}

void UseMouse(int button, int state, int x, int y)
{
    glutPostRedisplay();
}

int main(int argc, char *argv[])
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(winWidth, winHeight);
    glutCreateWindow("House and Tree");

    glewInit();

    Initialize();
    glutDisplayFunc(RenderFunction);
    glutMouseFunc(UseMouse);
    glutCloseFunc(Cleanup);

    glutMainLoop();

    return 0;
}
