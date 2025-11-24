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

#include "soil.h"

GLuint
    VaoId,
    VboId,
    EboId,
    ColorBufferId,
    ProgramId,
    colorCodeLocation,
    myMatrixLocation,
    textureLocation;

GLint winWidth = 800, winHeight = 800;

int colorCode, translationCode;

glm::mat4 myMatrix, resizeMatrix;
glm::mat4 translationMatrix, scaleMatrix, transformationMatrix;

float xMin = -500, xMax = 500, yMin = -500, yMax = 500;
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

struct TextureCoordinates
{
    GLfloat texCoord[2];

    TextureCoordinates(GLfloat x, GLfloat y)
    {
        texCoord[0] = x;
        texCoord[1] = y;
    }
};

struct Vertex
{
    Position position;
    Color color;
    TextureCoordinates textureCoordinates;
};

void LoadTexture(const char *texturePath)
{
    glGenTextures(1, &textureLocation);
    glBindTexture(GL_TEXTURE_2D, textureLocation);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    int width, height;
    unsigned char *image = SOIL_load_image(texturePath, &width, &height, 0, SOIL_LOAD_RGB);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);

    SOIL_free_image_data(image);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void CreateVBO(void)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(0.0f, 1.0f);

    std::vector<Vertex> vertices = {
        // axis lines
        {Position(xMin, 0.0f, 0.0f), Color(0.0f, 0.0f, 0.0f), TextureCoordinates(0.0f, 0.f)},
        {Position(xMax, 0.0f, 0.0f), Color(0.0f, 0.0f, 0.0f), TextureCoordinates(0.0f, 0.f)},
        {Position(0.0f, yMin, 0.0f), Color(0.0f, 0.0f, 0.0f), TextureCoordinates(0.0f, 0.f)},
        {Position(0.0f, yMax, 0.0f), Color(0.0f, 0.0f, 0.0f), TextureCoordinates(0.0f, 0.f)},
        // square
        {Position(0.f, 0.f, 0.0f), Color(dis(gen), dis(gen), dis(gen)), TextureCoordinates(0.0f, 0.f)},
        {Position(50.f, 0.f, 0.0f), Color(dis(gen), dis(gen), dis(gen)), TextureCoordinates(1.0f, 0.f)},
        {Position(50.f, 50.f, 0.0f), Color(dis(gen), dis(gen), dis(gen)), TextureCoordinates(1.0f, 1.f)},
        {Position(0.f, 50.f, 0.0f), Color(dis(gen), dis(gen), dis(gen)), TextureCoordinates(0.0f, 1.f)},
    };

    std::vector<GLuint> indices = {4, 5, 6, 6, 7, 4};

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

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(
        2,
        2,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Vertex),
        (void *)offsetof(Vertex, textureCoordinates));

    glGenBuffers(1, &EboId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EboId);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);
}

void DestroyVBO(void)
{
    glDisableVertexAttribArray(2);
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
    ProgramId = LoadShaders("../assignments/4/shader-3.vert", "../assignments/4/shader-3.frag");
    glUseProgram(ProgramId);
}

void DestroyShaders(void)
{
    glDeleteProgram(ProgramId);
}

void Initialize(void)
{
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    CreateVBO();
    LoadTexture("../assignments/4/text_smiley_face.png");
    CreateShaders();

    colorCodeLocation = glGetUniformLocation(ProgramId, "colorCode");
    myMatrixLocation = glGetUniformLocation(ProgramId, "myMatrix");

    resizeMatrix = glm::ortho(xMin, xMax, yMin, yMax);

    GLfloat angle = glm::radians(70.0f);

    scaleMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(2.0f, 0.5f, 1.0f));
    translationMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(100.f, 100.f, 0.0));
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

    glDrawElements(GL_TRIANGLES, 3 * 2, GL_UNSIGNED_INT, (void *)(0));

    myMatrix = resizeMatrix * translationMatrix * scaleMatrix;
    glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);
    glDrawElements(GL_TRIANGLES, 3 * 2, GL_UNSIGNED_INT, (void *)(0));

    colorCode = 2;
    glUniform1i(colorCodeLocation, colorCode);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureLocation);
    glUniform1i(glGetUniformLocation(ProgramId, "myTexture"), 0);

    myMatrix = resizeMatrix * scaleMatrix * translationMatrix;
    glUniformMatrix4fv(myMatrixLocation, 1, GL_FALSE, &myMatrix[0][0]);
    glDrawElements(GL_TRIANGLES, 3 * 2, GL_UNSIGNED_INT, (void *)(0));

    glutSwapBuffers();
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
    glutIdleFunc(RenderFunction);
    glutMouseFunc(UseMouse);
    glutCloseFunc(Cleanup);

    glutMainLoop();

    return 0;
}