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
const int instanceCount = 8;
const int parallelsCount = 30, meridiansCount = 30;
const int vertexCount = (parallelsCount + 1) * (meridiansCount + 1);
const float uMin = -glm::pi<float>() / 2, uMax = glm::pi<float>() / 2;
const float vMin = 0, vMax = 2 * glm::pi<float>();
const float uStep = (uMax - uMin) / parallelsCount, vStep = (vMax - vMin) / meridiansCount;
const float radius = 50;

glm::vec3 observer = glm::vec3(200.f, 200.f, 200.f);
glm::vec3 referencePoint = glm::vec3(0.f, 0.f, 0.f);
glm::vec3 vertex = glm::vec3(0.f, 1.f, 1.f);

glm::mat4 view, projection;

float alpha = 0.f, beta = 0.f, distance = 1000.f, alphaIncrementOne = 0.01f, alphaIncrementTwo = 0.01f;
float xMin = -200.f, xMax = 200.f, yMin = -200.f, yMax = 200.f, zMin = -200.f, zMax = 200.f;
float distanceNear = 1.f, fov = 90.f * glm::pi<float>() / 180.f, width = 800, height = 800;

bool keyStates[256] = {0};
bool specialKeyStates[512] = {0};

void LoadTexture(const char *texturePath)
{
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

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
    std::vector<glm::vec4> vertices(vertexCount);
    std::vector<glm::vec3> colors(vertexCount);
    std::vector<glm::vec2> texCoords(vertexCount);

    std::vector<GLuint> indices(6 * parallelsCount * meridiansCount, 0); // Only valid quads

    float u, v, x_vf, y_vf, z_vf;
    int index, triangleIndex = 0;

    for (int merid = 0; merid < meridiansCount + 1; merid++)
    {
        for (int parr = 0; parr < parallelsCount + 1; parr++)
        {
            u = uMin + parr * uStep;
            v = vMin + merid * vStep;
            x_vf = radius * cosf(u) * cosf(v);
            y_vf = radius * cosf(u) * sinf(v);
            z_vf = radius * sinf(u);

            index = merid * (parallelsCount + 1) + parr;
            vertices[index] = glm::vec4(x_vf, y_vf, z_vf, 1.0f);
            colors[index] = glm::vec3(0.2f + sinf(u), 0.1f + cosf(v), 0.5f + 0.5f * sinf(u));
            texCoords[index] = glm::vec2((v - vMin) / (vMax - vMin), (u - uMin) / (uMax - uMin));
        }
    }

    for (int merid = 0; merid < meridiansCount; merid++)
    {
        for (int parr = 0; parr < parallelsCount; parr++)
        {
            int i1 = merid * (parallelsCount + 1) + parr;
            int i2 = i1 + 1;
            int i3 = i1 + (parallelsCount + 1);
            int i4 = i3 + 1;

            indices[triangleIndex++] = i1;
            indices[triangleIndex++] = i2;
            indices[triangleIndex++] = i4;

            indices[triangleIndex++] = i1;
            indices[triangleIndex++] = i4;
            indices[triangleIndex++] = i3;
        }
    }

    std::vector<glm::mat4> modelMatrices;
    modelMatrices.reserve(instanceCount);

    std::vector<std::string> planetNames = {"Mercury", "Venus", "Earth", "Mars", "Jupiter", "Saturn", "Uranus", "Neptune"};
    std::vector<float> orbitalRadii = {60.f, 90.f, 120.f, 160.f, 220.f, 280.f, 340.f, 400.f};
    std::vector<float> orbitalSpeeds = {0.04f, 0.03f, 0.025f, 0.02f, 0.015f, 0.012f, 0.01f, 0.008f};
    std::vector<float> planetSizes = {0.8f, 1.0f, 1.2f, 0.9f, 2.5f, 2.2f, 1.6f, 1.5f};

    for (int instanceId = 0; instanceId < instanceCount; instanceId++)
    {
        int planetId = instanceId % 8;
        float time = glutGet(GLUT_ELAPSED_TIME) * 0.001f;

        float orbitAngle = orbitalSpeeds[planetId] * time + instanceId * glm::pi<float>() / 4;
        float orbitX = orbitalRadii[planetId] * cos(orbitAngle) * (0.8f + 0.4f * sin(time * 0.1f));
        float orbitZ = orbitalRadii[planetId] * sin(orbitAngle) * (1.2f + 0.3f * cos(time * 0.1f));
        float orbitY = 10.f * sin(orbitAngle * 2 + time * 0.2f);

        modelMatrices.push_back(
            glm::translate(glm::mat4(1.0f),
                           glm::vec3(orbitX + 20.f * sin(instanceId), orbitY, orbitZ + 15.f * cos(instanceId))) *

            glm::rotate(glm::mat4(1.0f),
                        time * (0.5f + planetId * 0.1f),
                        glm::vec3(0.3f + 0.1f * planetId, 1.0f, 0.7f)) *

            glm::rotate(glm::mat4(1.0f),
                        planetId * glm::pi<float>() / 12,
                        glm::vec3(1, 0, 0)) *

            glm::scale(glm::mat4(1.0f),
                       glm::vec3(planetSizes[planetId], planetSizes[planetId], planetSizes[planetId])));
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
        glVertexAttribPointer(3 + col, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void *)(sizeof(glm::vec4) * col));
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

    LoadTexture("../assignments/6/earf.png");

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

    glDrawElementsInstanced(GL_TRIANGLES, 6 * parallelsCount * meridiansCount,
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
    glutCreateWindow("Lab 6 - 1");

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