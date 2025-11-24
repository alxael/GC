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
    VbPositionId,
    VbColorId,
    VbModelMatrixId,
    EboId,
    ColorBufferId,
    ProgramId,
    colorCodeLocation,
    viewLocation,
    projectionLocation;

GLint winWidth = 800, winHeight = 800;

int colorCode;
const int INSTANCE_COUNT = 200;

glm::vec3 observer = glm::vec3(200.f, 200.f, 200.f);
glm::vec3 referencePoint = glm::vec3(0.f, 0.f, 0.f);
glm::vec3 vertex = glm::vec3(0.f, 1.f, 1.f);

glm::mat4 view, projection;

float xMin = -200.f, xMax = 200.f, yMin = -200.f, yMax = 200.f, zMin = -200.f, zMax = 200.f;
float distanceNear = 1.f, fov = 90.f * glm::pi<float>() / 180.f, width = 800, height = 800;

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

void CreateVBO(void)
{
    std::vector<Position> positions = {
        Position(50.f, 50.0f, 50.0f),
        Position(50.f, -50.f, -50.f),
        Position(-50.f, 50.f, -50.f),
        Position(-50.f, -50.f, 50.f),
    };

    std::vector<glm::vec4> colors;
    for (int instanceId = 0; instanceId < INSTANCE_COUNT; instanceId++)
    {
        colors.push_back(glm::vec4(
            0.35f + 0.30f * (sinf(instanceId / 4.0f + 2.0f) + 1.0f),
            0.25f + 0.25f * (sinf(instanceId / 5.0f + 3.0f) + 1.0f),
            0.25f + 0.35f * (sinf(instanceId / 6.0f + 4.0f) + 1.0f),
            1.0f));
    }

    std::vector<glm::mat4> modelMatrices;
    for (int instanceId = 0; instanceId < INSTANCE_COUNT; instanceId++)
    {
        modelMatrices.push_back(
            glm::translate(
                glm::mat4(1.0f),
                glm::vec3(300.f * cos(instanceId), 300.f * sin(instanceId), 50.f * (instanceId - INSTANCE_COUNT / 2))) *
            glm::rotate(glm::mat4(1.0f), (instanceId + 1) * glm::pi<float>() / 10, glm::vec3(1, 0, 0)) *
            glm::rotate(glm::mat4(1.0f), instanceId * glm::pi<float>() / 6, glm::vec3(0, 1, 0)) *
            glm::rotate(glm::mat4(1.0f), 3 * instanceId * glm::pi<float>() / 2, glm::vec3(0, 0, 1)));
    }

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

    // positions
    glGenBuffers(1, &VbPositionId);
    glBindBuffer(GL_ARRAY_BUFFER, VbPositionId);
    glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(Position), positions.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,
        4,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Position),
        0);

    // colors
    glGenBuffers(1, &VbColorId);
    glBindBuffer(GL_ARRAY_BUFFER, VbColorId);
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(glm::vec4), colors.data(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1,
        4,
        GL_FLOAT,
        GL_FALSE,
        sizeof(glm::vec4),
        0);
    glVertexAttribDivisor(1, 1);

    // model matrices
    glGenBuffers(1, &VbModelMatrixId);
    glBindBuffer(GL_ARRAY_BUFFER, VbModelMatrixId);
    glBufferData(GL_ARRAY_BUFFER, modelMatrices.size() * sizeof(glm::mat4), modelMatrices.data(), GL_STATIC_DRAW);

    for (int index = 0; index < 4; index++)
    {
        glEnableVertexAttribArray(2 + index);
        glVertexAttribPointer(2 + index,
                              4, GL_FLOAT, GL_FALSE,
                              sizeof(glm::mat4),
                              (void *)(sizeof(glm::vec4) * index));
        glVertexAttribDivisor(2 + index, 1);
    }

    // elements
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
    glDeleteBuffers(1, &VbPositionId);
    glDeleteBuffers(1, &VbColorId);
    glDeleteBuffers(1, &VbModelMatrixId);
    glDeleteBuffers(1, &EboId);

    glBindVertexArray(0);
    glDeleteVertexArrays(1, &VaoId);
}

void CreateShaders(void)
{
    ProgramId = LoadShaders("../assignments/5/shader-2.vert", "../assignments/5/shader-2.frag");
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

    viewLocation = glGetUniformLocation(ProgramId, "viewMatrix");
    projectionLocation = glGetUniformLocation(ProgramId, "projectionMatrix");
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
    glBindBuffer(GL_ARRAY_BUFFER, VbPositionId);
    glBindBuffer(GL_ARRAY_BUFFER, VbColorId);
    glBindBuffer(GL_ARRAY_BUFFER, VbModelMatrixId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EboId);

    view = glm::lookAt(observer, referencePoint, vertex);
    glUniformMatrix4fv(viewLocation, 1, GL_FALSE, &view[0][0]);

    projection = glm::infinitePerspective(fov, GLfloat(width) / GLfloat(height), distanceNear);
    glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projection));

    colorCode = 1;
    glUniform1i(colorCodeLocation, colorCode);
    glLineWidth(2.0);
    glDrawArrays(GL_LINES, 4, 8);

    colorCode = 0;
    glUniform1i(colorCodeLocation, colorCode);
    glDrawElementsInstanced(GL_TRIANGLES, 12, GL_UNSIGNED_INT, 0, INSTANCE_COUNT);

    colorCode = 1;
    glUniform1i(colorCodeLocation, colorCode);
    for (int index = 0; index < 12; index += 3)
    {
        glDrawElementsInstanced(GL_LINE_LOOP, 3, GL_UNSIGNED_INT, (void *)(index * sizeof(GLuint)), INSTANCE_COUNT);
    }

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
    glutCreateWindow("Lab 5 - 2");

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