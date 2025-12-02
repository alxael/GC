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

#include "SOIL.h"

GLuint
    VaoId,
    VbPositionId,
    VbColorId,
    VbModelMatrixId,
    VbTexCoordId,
    EboId,
    ColorBufferId,
    ProgramId,
    colorCodeLocation,
    viewLocation,
    projectionLocation,
    texture;

GLint winWidth = 800, winHeight = 800;

int colorCode;
const int instanceCount = 200;

const int coneSlices = 40;
const int coneStacks = 1;
const float coneHeight = 100.0f;
const float coneRadius = 50.0f;

glm::vec3 observer = glm::vec3(200.f, 200.f, 200.f);
glm::vec3 referencePoint = glm::vec3(0.f, 0.f, 0.f);
glm::vec3 vertex = glm::vec3(0.f, 1.f, 1.f);

glm::mat4 view, projection;

float alpha = 0.f, beta = 0.f, distance = 1000.f, alphaIncrementOne = 0.01f, alphaIncrementTwo = 0.01f;
float xMin = -200.f, xMax = 200.f, yMin = -200.f, yMax = 200.f, zMin = -200.f, zMax = 200.f;
float distanceNear = 1.f, fov = 90.f * glm::pi<float>() / 180.f, width = 800, height = 800;

bool keyStates[256] = {0};
bool specialKeyStates[512] = {0};

void CreateVBO(void)
{
    const int sideVertices = coneSlices + 2;
    const int baseVertices = coneSlices + 2;
    const int vertexCount = sideVertices + baseVertices;

    std::vector<glm::vec4> vertices(vertexCount);
    std::vector<glm::vec3> colors(vertexCount);
    std::vector<glm::vec2> texCoords(vertexCount);

    const int sideIndicesCount = 3 * (sideVertices - 2);
    const int baseIndicesCount = 3 * (baseVertices - 2);
    const int totalIndices = sideIndicesCount + baseIndicesCount;
    std::vector<GLuint> indices(totalIndices);

    int v = 0;

    vertices[v] = glm::vec4(0.0f, coneHeight, 0.0f, 1.0f);
    colors[v] = glm::vec3(1.0f, 1.0f, 1.0f);
    texCoords[v] = glm::vec2(0.5f, 1.0f);
    int apexIndex = v;
    v++;

    for (int i = 0; i <= coneSlices; ++i)
    {
        float theta = 2.0f * glm::pi<float>() * float(i) / float(coneSlices);
        float x = coneRadius * cos(theta);
        float z = coneRadius * sin(theta);
        float y = 0.0f;

        vertices[v] = glm::vec4(x, y, z, 1.0f);
        colors[v] = glm::vec3(1.0f, 1.0f, 1.0f);
        texCoords[v] = glm::vec2(float(i) / float(coneSlices), 0.0f);
        v++;
    }

    int sideStart = 0;
    int baseStart = v;

    vertices[v] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    colors[v] = glm::vec3(1.0f, 1.0f, 1.0f);
    texCoords[v] = glm::vec2(0.5f, 0.5f);
    int baseCenterIndex = v;
    v++;

    for (int i = 0; i <= coneSlices; ++i)
    {
        float theta = 2.0f * glm::pi<float>() * float(i) / float(coneSlices);
        float x = coneRadius * cos(theta);
        float z = coneRadius * sin(theta);
        float y = 0.0f;

        vertices[v] = glm::vec4(x, y, z, 1.0f);
        colors[v] = glm::vec3(1.0f, 1.0f, 1.0f);
        texCoords[v] = glm::vec2(0.5f + 0.5f * cos(theta), 0.5f + 0.5f * sin(theta));
        v++;
    }

    // ---- Indices for side fan ----
    int idx = 0;
    for (int i = 1; i <= coneSlices; ++i)
    {
        indices[idx++] = apexIndex;
        indices[idx++] = i;
        indices[idx++] = i + 1;
    }

    int baseFanStart = baseCenterIndex;
    for (int i = 1; i <= coneSlices; ++i)
    {
        indices[idx++] = baseFanStart;
        indices[idx++] = baseFanStart + i + 1;
        indices[idx++] = baseFanStart + i;
    }

    std::vector<glm::mat4> modelMatrices;
    modelMatrices.reserve(instanceCount);

    for (int instanceId = 0; instanceId < instanceCount; instanceId++)
    {
        modelMatrices.push_back(
            glm::translate(
                glm::mat4(1.0f),
                glm::vec3(300.f * cos(instanceId),
                          300.f * sin(instanceId),
                          50.f * (instanceId - instanceCount / 2))) *
            glm::rotate(glm::mat4(1.0f),
                        (instanceId + 1) * glm::pi<float>() / 10,
                        glm::vec3(1, 0, 0)) *
            glm::rotate(glm::mat4(1.0f),
                        instanceId * glm::pi<float>() / 6,
                        glm::vec3(0, 1, 0)) *
            glm::rotate(glm::mat4(1.0f),
                        3 * instanceId * glm::pi<float>() / 2,
                        glm::vec3(0, 0, 1)) *
            glm::scale(glm::mat4(1.0f),
                       glm::vec3(1.0f, 1.0f, 1.0f)));
    }

    glGenVertexArrays(1, &VaoId);
    glBindVertexArray(VaoId);

    glGenBuffers(1, &VbPositionId);
    glBindBuffer(GL_ARRAY_BUFFER, VbPositionId);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec4), vertices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void *)0);

    glGenBuffers(1, &VbColorId);
    glBindBuffer(GL_ARRAY_BUFFER, VbColorId);
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(glm::vec3), colors.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void *)0);

    glGenBuffers(1, &VbTexCoordId);
    glBindBuffer(GL_ARRAY_BUFFER, VbTexCoordId);
    glBufferData(GL_ARRAY_BUFFER, texCoords.size() * sizeof(glm::vec2), texCoords.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (void *)0);

    glGenBuffers(1, &VbModelMatrixId);
    glBindBuffer(GL_ARRAY_BUFFER, VbModelMatrixId);
    glBufferData(GL_ARRAY_BUFFER, modelMatrices.size() * sizeof(glm::mat4), modelMatrices.data(), GL_STATIC_DRAW);
    for (int col = 0; col < 4; col++)
    {
        glEnableVertexAttribArray(3 + col);
        glVertexAttribPointer(3 + col, 4, GL_FLOAT, GL_FALSE,
                              sizeof(glm::mat4), (void *)(sizeof(glm::vec4) * col));
        glVertexAttribDivisor(3 + col, 1);
    }

    glGenBuffers(1, &EboId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EboId);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint), indices.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);
}

void DestroyVBO(void)
{
    glDisableVertexAttribArray(6);
    glDisableVertexAttribArray(5);
    glDisableVertexAttribArray(4);
    glDisableVertexAttribArray(3);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glDeleteBuffers(1, &VbTexCoordId);
    glDeleteBuffers(1, &VbPositionId);
    glDeleteBuffers(1, &VbColorId);
    glDeleteBuffers(1, &VbModelMatrixId);
    glDeleteBuffers(1, &EboId);

    glBindVertexArray(0);
    glDeleteVertexArrays(1, &VaoId);
}

void CreateShaders(void)
{
    ProgramId = LoadShaders("../assignments/6/shader-1.vert", "../assignments/6/shader-1.frag");
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
    if (keyStates['s'])
    {
        distance -= 5.0f;
    }
    if (keyStates['w'])
    {
        distance += 5.0f;
    }
    if (specialKeyStates[GLUT_KEY_LEFT])
    {
        beta -= 0.01f;
    }
    if (specialKeyStates[GLUT_KEY_RIGHT])
    {
        beta += 0.01f;
    }
    if (specialKeyStates[GLUT_KEY_UP])
    {
        alpha += alphaIncrementOne;
    }
    if (specialKeyStates[GLUT_KEY_DOWN])
    {
        alpha -= alphaIncrementTwo;
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

    observer = referencePoint + glm::vec3(distance * cos(alpha) * cos(beta),
                                          distance * cos(alpha) * sin(beta),
                                          distance * sin(alpha));
    view = glm::lookAt(observer, referencePoint, vertex);
    projection = glm::perspective(fov, GLfloat(width) / GLfloat(height), distanceNear, 2000.f);

    glUseProgram(ProgramId);
    glUniformMatrix4fv(viewLocation, 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projection));

    glBindVertexArray(VaoId);

    colorCode = 0;
    glUniform1i(colorCodeLocation, colorCode);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1i(glGetUniformLocation(ProgramId, "myTexture"), 0);

    int sideVertices = coneSlices + 2;
    int baseVertices = coneSlices + 2;
    int totalIndices = 3 * (sideVertices - 2) + 3 * (baseVertices - 2);

    glDrawElementsInstanced(GL_TRIANGLES, totalIndices,
                            GL_UNSIGNED_INT, 0, instanceCount);

    glBindVertexArray(0);
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
    glutCreateWindow("Lab 6 - 2");

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
