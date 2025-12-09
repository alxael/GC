#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include "loadShaders.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtc/type_ptr.hpp"

GLuint
    VaoId1,
    VaoId2,
    VboId1, VboId2,
    EboId1, EboId2,
    ProgramId,
    obsLocation, viewLocation, projLocation,
    materialEmissionLocation, materialAmbientLocation, materialDiffuseLocation,
    materialSpecularLocation, materialShininessValueLocation,
    lightPosLocation, lightAmbientLocation, lightDiffuseLocation, lightSpecularLocation, lightAttenuationLocation;

float const uMin = -glm::pi<float>() / 2, uMax = glm::pi<float>() / 2, vMin = 0, vMax = 2 * glm::pi<float>();
int const parallelCount = 50, meridianCount = 50;
int const vertexCount = (parallelCount + 1) * (meridianCount + 1);
float uStep = (uMax - uMin) / parallelCount, vStep = (vMax - vMin) / meridianCount;

float radius = 50;
int idx;

glm::vec3 observer = glm::vec3(115.f, 160.f, 70.f);
glm::vec3 referencePoint = glm::vec3(0.f, 0.f, 0.f);
glm::vec3 vertex = glm::vec3(0.f, 1.f, 1.f);

float alpha = 0.0f, beta = 0.0f, distance = 300.0f;
float alphaIncrementOne = 0.01f, alphaIncrementTwo = 0.01f;
float width = 800, height = 600, znear = 1, fov = 30;
GLint winWidth = 1000, winHeight = 600;

glm::mat4 view, projection;

struct MaterialProperties
{
    glm::vec3 emission;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    GLfloat shininessValue;
};

MaterialProperties material, opaque, shiny;

struct Light
{
    glm::vec4 position;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    glm::vec3 attenuation;
};
Light lightSource;

bool keyStates[256] = {0};
bool specialKeyStates[512] = {0};

void ProcessKeys()
{
    if (keyStates['-'] || keyStates['s'])
        distance += 5.0;
    if (keyStates['+'] || keyStates['w'])
        distance -= 5.0;
    if (specialKeyStates[GLUT_KEY_LEFT])
        beta -= 0.01f;
    if (specialKeyStates[GLUT_KEY_RIGHT])
        beta += 0.01f;
    if (specialKeyStates[GLUT_KEY_UP])
    {
        alpha += alphaIncrementOne;
    }
    if (specialKeyStates[GLUT_KEY_DOWN])
    {
        alpha -= alphaIncrementTwo;
    }

    if (keyStates['a'])
        lightSource.position.x -= 2.0f;
    if (keyStates['d'])
        lightSource.position.x += 2.0f;
    if (keyStates['q'])
        lightSource.position.y += 2.0f;
    if (keyStates['e'])
        lightSource.position.y -= 2.0f;
    if (keyStates['z'])
        lightSource.position.z -= 2.0f;
    if (keyStates['c'])
        lightSource.position.z += 2.0f;
}

void DrawObserverPosition()
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
    snprintf(observerStr, sizeof(observerStr), "Observer: (%.1f,%.1f,%.1f) W/S/UP/DOWN/LEFT/RIGHT", observer.x, observer.y, observer.z);
    int textX = 10;
    int textY = winHeight - 20;
    glRasterPos2i(textX, textY);
    for (const char *c = observerStr; *c != '\0'; c++)
    {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *c);
    }

    char lightStr[128];
    snprintf(lightStr, sizeof(lightStr), "Light: (%.1f,%.1f,%.1f) A/D/Q/E/Z/C",
             lightSource.position.x, lightSource.position.y, lightSource.position.z);
    textY -= 25;
    glRasterPos2i(textX, textY);
    for (const char *c = lightStr; *c != '\0'; c++)
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

void updateMaterial(MaterialProperties material)
{
    glUniform3fv(materialEmissionLocation, 1, glm::value_ptr(material.emission));
    glUniform3fv(materialAmbientLocation, 1, glm::value_ptr(material.ambient));
    glUniform3fv(materialDiffuseLocation, 1, glm::value_ptr(material.diffuse));
    glUniform3fv(materialSpecularLocation, 1, glm::value_ptr(material.specular));
    glUniform1f(materialShininessValueLocation, material.shininessValue);
}

void updateLight(Light light)
{
    glUniform4fv(lightPosLocation, 1, glm::value_ptr(light.position));
    glUniform3fv(lightAmbientLocation, 1, glm::value_ptr(light.ambient));
    glUniform3fv(lightDiffuseLocation, 1, glm::value_ptr(light.diffuse));
    glUniform3fv(lightSpecularLocation, 1, glm::value_ptr(light.specular));
    glUniform3fv(lightAttenuationLocation, 1, glm::value_ptr(light.attenuation));
}

void CreateShaders(void)
{
    ProgramId = LoadShaders("../assignments/7/shader-1.vert", "../assignments/7/shader-1.frag");
    glUseProgram(ProgramId);
}

void CreateVAO1(void)
{
    glm::vec4 vertices[vertexCount];
    glm::vec3 colors[vertexCount];
    glm::vec3 normals[vertexCount];
    GLushort indices[4 * vertexCount];
    float u, v, x, y, z;
    int indexOne, indexTwo, indexThree, indexFour;

    for (int meridianIndex = 0; meridianIndex < meridianCount + 1; meridianIndex++)
    {
        for (int parallelIndex = 0; parallelIndex < parallelCount + 1; parallelIndex++)
        {
            u = uMin + parallelIndex * uStep;
            v = vMin + meridianIndex * vStep;
            x = radius * cosf(u) * cosf(v);
            y = radius * cosf(u) * sinf(v);
            z = radius * sinf(u) + 0.05 * radius * rand() / RAND_MAX;

            idx = meridianIndex * (parallelCount + 1) + parallelIndex;
            vertices[idx] = glm::vec4(x, y, z, 1.0);
            colors[idx] = glm::vec3(0.1 * sinf(u), 0.08 * cosf(v), -0.05 * sinf(u));
            normals[idx] = glm::vec3(cosf(u) * cosf(v), cosf(u) * sinf(v), sinf(u));

            if ((parallelIndex + 1) % (parallelCount + 1) != 0)
            {
                indexOne = idx;
                indexTwo = idx + (parallelCount + 1);
                indexThree = indexTwo + 1;
                indexFour = idx + 1;
                if (meridianIndex == meridianCount)
                {
                    indexTwo = idx;
                    indexThree = idx;
                    indexFour = idx;
                }
                indices[4 * idx] = indexOne;
                indices[4 * idx + 1] = indexTwo;
                indices[4 * idx + 2] = indexThree;
                indices[4 * idx + 3] = indexFour;
            }
        }
    };

    glGenVertexArrays(1, &VaoId1);
    glBindVertexArray(VaoId1);
    glGenBuffers(1, &VboId1);
    glGenBuffers(1, &EboId1);

    glBindBuffer(GL_ARRAY_BUFFER, VboId1);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices) + sizeof(colors) + sizeof(normals), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices), sizeof(colors), colors);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices) + sizeof(colors), sizeof(normals), normals);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EboId1);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid *)(0));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid *)(sizeof(vertices)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid *)(sizeof(vertices) + sizeof(colors)));
}

void CreateVAO2(void)
{
    GLfloat vertices[] = {
        -20.0f, 130.0f, -10.0f, 1.0f,
        20.0f, 130.0f, -10.0f, 1.0f,
        20.0f, 170.0f, -10.0f, 1.0f,
        -20.0f, 170.0f, -10.0f, 1.0f,
        -20.0f, 130.0f, 30.0f, 1.0f,
        20.0f, 130.0f, 30.0f, 1.0f,
        20.0f, 170.0f, 30.0f, 1.0f,
        -20.0f, 170.0f, 30.0f, 1.0f};

    GLfloat colors[] = {
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f};

    GLfloat normals[] = {
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,
        -1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f};

    GLushort indices[] = {
        1, 0, 2, 2, 0, 3,
        2, 3, 6, 6, 7, 3,
        7, 3, 4, 4, 3, 0,
        5, 4, 0, 1, 5, 0,
        1, 2, 5, 5, 2, 6,
        4, 5, 6, 7, 4, 6};

    glGenVertexArrays(1, &VaoId2);
    glBindVertexArray(VaoId2);
    glGenBuffers(1, &VboId2);
    glGenBuffers(1, &EboId2);

    glBindBuffer(GL_ARRAY_BUFFER, VboId2);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices) + sizeof(colors) + sizeof(normals), NULL, GL_STATIC_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices), sizeof(colors), colors);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices) + sizeof(colors), sizeof(normals), normals);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EboId2);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (GLvoid *)(0));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid *)(sizeof(vertices)));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid *)(sizeof(vertices) + sizeof(colors)));
}

void ReshapeFunction(GLint newWidth, GLint newHeight)
{
    glViewport(0, 0, newWidth, newHeight);
    winWidth = newWidth;
    winHeight = newHeight;
    width = winWidth / 10, height = winHeight / 10;
}

void DestroyShaders(void)
{
    glDeleteProgram(ProgramId);
}

void DestroyVBO(void)
{
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &VboId1);
    glDeleteBuffers(1, &EboId1);
    glDeleteBuffers(1, &VboId2);
    glDeleteBuffers(1, &EboId2);
}

void Cleanup(void)
{
    DestroyShaders();
    DestroyVBO();
}

void Initialize(void)
{
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    shiny.emission = glm::vec3(0.0f, 0.0f, 0.0f);
    shiny.ambient = glm::vec3(0.96f, 0.71f, 0.1f);
    shiny.diffuse = glm::vec3(0.96f, 0.71f, 0.1f);
    shiny.specular = glm::vec3(1.0f, 1.0f, 1.0f);
    shiny.shininessValue = 100.0f;

    opaque.emission = glm::vec3(0.0f, 0.0f, 0.0f);
    opaque.ambient = glm::vec3(0.5f, 0.25f, 0.2f);
    opaque.diffuse = glm::vec3(1.0f, 0.5f, 0.4f);
    opaque.specular = glm::vec3(0.0f, 0.0f, 0.0f);
    opaque.shininessValue = 1.0f;

    lightSource.position = glm::vec4(30.0f, 90.f, 50.0f, 1.0f);
    lightSource.ambient = glm::vec3(0.2f, 0.2f, 0.2f);
    lightSource.diffuse = glm::vec3(1.8f, 1.8f, 1.8f);        // brighter
    lightSource.specular = glm::vec3(1.8f, 1.8f, 1.8f);       // brighter
    lightSource.attenuation = glm::vec3(1.0f, 0.02f, 0.001f); // mild falloff

    CreateVAO1();
    CreateVAO2();
    CreateShaders();

    obsLocation = glGetUniformLocation(ProgramId, "obsShader");
    viewLocation = glGetUniformLocation(ProgramId, "viewShader");
    projLocation = glGetUniformLocation(ProgramId, "projectionShader");
    materialEmissionLocation = glGetUniformLocation(ProgramId, "materialShader.emission");
    materialAmbientLocation = glGetUniformLocation(ProgramId, "materialShader.ambient");
    materialDiffuseLocation = glGetUniformLocation(ProgramId, "materialShader.diffuse");
    materialSpecularLocation = glGetUniformLocation(ProgramId, "materialShader.specular");
    materialShininessValueLocation = glGetUniformLocation(ProgramId, "materialShader.shininessValue");
    lightPosLocation = glGetUniformLocation(ProgramId, "lightShader.position");
    lightAmbientLocation = glGetUniformLocation(ProgramId, "lightShader.ambient");
    lightDiffuseLocation = glGetUniformLocation(ProgramId, "lightShader.diffuse");
    lightSpecularLocation = glGetUniformLocation(ProgramId, "lightShader.specular");
    lightAttenuationLocation = glGetUniformLocation(ProgramId, "lightShader.attenuation");

    updateLight(lightSource);
}

void SetMVP(void)
{
    observer.x = referencePoint.x + distance * cos(alpha) * cos(beta);
    observer.y = referencePoint.y + distance * cos(alpha) * sin(beta);
    observer.z = referencePoint.z + distance * sin(alpha);
    glUniform3fv(obsLocation, 1, glm::value_ptr(observer));

    view = glm::lookAt(observer, referencePoint, vertex);
    glUniformMatrix4fv(viewLocation, 1, GL_FALSE, &view[0][0]);

    projection = glm::infinitePerspective(fov, GLfloat(width) / GLfloat(height), znear);
    glUniformMatrix4fv(projLocation, 1, GL_FALSE, &projection[0][0]);
}

void RenderFunction(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    ProcessKeys();
    SetMVP();
    updateLight(lightSource);

    glBindVertexArray(VaoId1);
    updateMaterial(shiny);
    for (int squareVertexIndex = 0; squareVertexIndex < vertexCount; squareVertexIndex++)
    {
        if ((squareVertexIndex + 1) % (parallelCount + 1) != 0)
            glDrawElements(
                GL_QUADS,
                4,
                GL_UNSIGNED_SHORT,
                (GLvoid *)((4 * squareVertexIndex) * sizeof(GLushort)));
    }

    glBindVertexArray(VaoId2);
    updateMaterial(opaque);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, (GLvoid *)(0));

    DrawObserverPosition();
    glutSwapBuffers();
    glFlush();
}

void handleKeyDown(unsigned char key, int x, int y)
{
    keyStates[key] = true;
    if (key == 27)
        exit(0);
}

void handleKeyUp(unsigned char key, int x, int y)
{
    keyStates[key] = false;
}

void handleSpecialDown(int key, int x, int y)
{
    specialKeyStates[key] = true;
}

void handleSpecialUp(int key, int x, int y)
{
    specialKeyStates[key] = false;
}

int main(int argc, char *argv[])
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowSize(winWidth, winHeight);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("Tema 7 - 1");

    glewInit();

    Initialize();
    glutReshapeFunc(ReshapeFunction);
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
