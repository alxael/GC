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
    myMatrixLocation;

GLint winWidth = 800, winHeight = 800;

int colorCode, translationCode;

glm::mat4 myMatrix, resizeMatrix;

float xMin = -1000, xMax = 1000, yMin = -1000, yMax = 1000;
float deltaX = xMax - xMin, deltaY = yMax - yMin;
float centerX = (xMin + xMax) / 2, centerY = (yMin + yMax) / 2;

int vertexCount = 20;
float firstCircleRadius = 200.f, secondCircleRadius = 400.f;

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
        // axis lines
        {Position(xMin, 0.0f, 0.0f), Color(0.0f, 0.0f, 0.0f)},
        {Position(xMax, 0.0f, 0.0f), Color(0.0f, 0.0f, 0.0f)},
        {Position(0.0f, yMin, 0.0f), Color(0.0f, 0.0f, 0.0f)},
        {Position(0.0f, yMax, 0.0f), Color(0.0f, 0.0f, 0.0f)},
    };

    std::vector<GLuint> indices;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0.0f, 1.0f);

    int startIndex = vertices.size();
    for(int vertexIndex = 0; vertexIndex <= vertexCount; vertexIndex ++) {
        float angle = (2 * vertexIndex * glm::pi<float>()) / vertexCount;

        vertices.push_back({Position(firstCircleRadius * cos(angle), firstCircleRadius * sin(angle), 0.0f), Color(dis(gen), dis(gen), dis(gen))});
        vertices.push_back({Position(secondCircleRadius * cos(angle), secondCircleRadius * sin(angle), 0.0f), Color(dis(gen), dis(gen), dis(gen))});
    
        if(vertexIndex != 0) {
            // first triangle
            indices.emplace_back(startIndex);
            indices.emplace_back(startIndex + 1);
            indices.emplace_back(startIndex + 2);
            // second triangle
            indices.emplace_back(startIndex + 1);
            indices.emplace_back(startIndex + 2);
            indices.emplace_back(startIndex + 3);
            startIndex += 2;
        }
    }

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
    CreateVBO();
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

    // draw axis lines
    colorCode = 1;
    glUniform1i(colorCodeLocation, colorCode);
    glLineWidth(2.0);
    glDrawArrays(GL_LINES, 0, 4);

    glLineWidth(3.0);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDrawElements(GL_TRIANGLES, 3 * vertexCount * 2, GL_UNSIGNED_INT, (void*)(0));

    glFlush();
}

void Cleanup(void)
{
    DestroyShaders();
    DestroyVBO();
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
    glutCreateWindow("Lab 4");

    glewInit();

    Initialize();
    glutDisplayFunc(RenderFunction);
    glutMouseFunc(UseMouse);
    glutCloseFunc(Cleanup);

    glutMainLoop();

    return 0;
}