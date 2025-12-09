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
    VbNormalId, // Added normal buffer
    VbModelMatrixId,
    EboId,
    ColorBufferId,
    ProgramId,
    colorCodeLocation,
    viewLocation,
    projectionLocation,
    obsLocation, // Added observer location
    materialEmissionLocation, materialAmbientLocation, materialDiffuseLocation,
    materialSpecularLocation, materialShininessValueLocation,
    lightPosLocation, lightAmbientLocation, lightDiffuseLocation, lightSpecularLocation, lightAttenuationLocation;

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

glm::vec3 getRandomColor(int seed)
{
    float xValue = (seed + 1) * 3453.342;
    float yValue = (seed + 1) * 8978.123;
    float zValue = (seed + 1) * 5472.234;

    return glm::vec3(xValue - (long)xValue, yValue - (long)yValue, zValue - (long)zValue);
}

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

void CreateVBO(void)
{
    const int sideVertices = coneSlices + 1; // Apex + ring
    const int baseVertices = coneSlices + 1; // Center + ring
    const int vertexCount = sideVertices + baseVertices;

    std::vector<glm::vec4> vertices(vertexCount);
    std::vector<glm::vec3> colors(vertexCount);
    std::vector<glm::vec3> normals(vertexCount); // Replaced texCoords with normals

    const int sideIndicesCount = 3 * coneSlices;
    const int baseIndicesCount = 3 * coneSlices;
    const int totalIndices = sideIndicesCount + baseIndicesCount;
    std::vector<GLuint> indices(totalIndices);

    glm::vec3 baseColor(0.6f, 0.65f, 0.8f);

    int v = 0;

    // Apex (top point)
    vertices[v] = glm::vec4(0.0f, coneHeight, 0.0f, 1.0f);
    colors[v] = baseColor;
    normals[v] = glm::vec3(0.0f, 1.0f, 0.0f); // Pointing straight up
    int apexIndex = v++;

    // Side surface ring (base circle)
    for (int i = 0; i < coneSlices; ++i)
    {
        float theta = 2.0f * glm::pi<float>() * float(i) / float(coneSlices);
        float x = coneRadius * cos(theta);
        float z = coneRadius * sin(theta);
        float y = 0.0f;

        vertices[v] = glm::vec4(x, y, z, 1.0f);
        colors[v] = baseColor;

        // Cone side normal: radial + upward tilt (matches surface slope)
        float slope = coneHeight / coneRadius;                           // Aspect ratio
        normals[v] = glm::normalize(glm::vec3(x, coneRadius * 0.2f, z)); // Slight upward tilt
        v++;
    }

    int baseStart = v;

    // Base center
    vertices[v] = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
    colors[v] = baseColor;
    normals[v] = glm::vec3(0.0f, -1.0f, 0.0f); // Pointing down for base
    int baseCenterIndex = v++;

    // Base ring (same positions as side ring, different normals)
    for (int i = 0; i < coneSlices; ++i)
    {
        float theta = 2.0f * glm::pi<float>() * float(i) / float(coneSlices);
        float x = coneRadius * cos(theta);
        float z = coneRadius * sin(theta);
        float y = 0.0f;

        vertices[v] = glm::vec4(x, y, z, 1.0f);
        colors[v] = baseColor;
        normals[v] = glm::vec3(0.0f, -1.0f, 0.0f); // Flat down for base
        v++;
    }

    // Side triangles (apex fan)
    int idx = 0;
    for (int i = 0; i < coneSlices; ++i)
    {
        indices[idx++] = apexIndex;
        indices[idx++] = 1 + i;
        indices[idx++] = 1 + (i + 1) % coneSlices;
    }

    // Base triangles (center fan)
    int baseFanStart = baseCenterIndex;
    for (int i = 0; i < coneSlices; ++i)
    {
        indices[idx++] = baseFanStart;
        indices[idx++] = baseStart + (i + 1) % coneSlices;
        indices[idx++] = baseStart + i;
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

    // Position (location 0)
    glGenBuffers(1, &VbPositionId);
    glBindBuffer(GL_ARRAY_BUFFER, VbPositionId);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec4), vertices.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(glm::vec4), (void *)0);

    // Color (location 1)
    glGenBuffers(1, &VbColorId);
    glBindBuffer(GL_ARRAY_BUFFER, VbColorId);
    glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(glm::vec3), colors.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void *)0);

    // Normal (location 2) - REPLACED texcoords
    glGenBuffers(1, &VbNormalId);
    glBindBuffer(GL_ARRAY_BUFFER, VbNormalId);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), normals.data(), GL_STATIC_DRAW);
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void *)0);

    // Model matrix (locations 3,4,5,6)
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
    glDisableVertexAttribArray(2); // Normal
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    glDeleteBuffers(1, &VbNormalId); // Added
    glDeleteBuffers(1, &VbPositionId);
    glDeleteBuffers(1, &VbColorId);
    glDeleteBuffers(1, &VbModelMatrixId);
    glDeleteBuffers(1, &EboId);

    glBindVertexArray(0);
    glDeleteVertexArrays(1, &VaoId);
}

void CreateShaders(void)
{
    ProgramId = LoadShaders("../assignments/7/shader-2.vert", "../assignments/7/shader-2.frag"); // Use your lighting shaders
    glUseProgram(ProgramId);
}

void DestroyShaders(void)
{
    glDeleteProgram(ProgramId);
}

void Initialize(void)
{
    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);

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

    CreateVBO();
    CreateShaders();

    // All uniform locations
    obsLocation = glGetUniformLocation(ProgramId, "obsShader");
    viewLocation = glGetUniformLocation(ProgramId, "viewShader");
    projectionLocation = glGetUniformLocation(ProgramId, "projectionShader");
    colorCodeLocation = glGetUniformLocation(ProgramId, "colorCodeShader");

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

    updateMaterial(shiny);
    updateLight(lightSource);
}

void ProcessKeys(void)
{
    if (keyStates['s'])
        distance -= 5.0f;
    if (keyStates['w'])
        distance += 5.0f;
    if (specialKeyStates[GLUT_KEY_LEFT])
        beta -= 0.01f;
    if (specialKeyStates[GLUT_KEY_RIGHT])
        beta += 0.01f;
    if (specialKeyStates[GLUT_KEY_UP])
        alpha += alphaIncrementOne;
    if (specialKeyStates[GLUT_KEY_DOWN])
        alpha -= alphaIncrementTwo;
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

void RenderFunction(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    ProcessKeys();

    observer = referencePoint + glm::vec3(distance * cos(alpha) * cos(beta),
                                          distance * cos(alpha) * sin(beta),
                                          distance * sin(alpha));

    glUniform3fv(obsLocation, 1, glm::value_ptr(observer)); // Pass observer to shader

    view = glm::lookAt(observer, referencePoint, vertex);
    projection = glm::perspective(fov, GLfloat(width) / GLfloat(height), distanceNear, 2000.f);

    glUseProgram(ProgramId);
    glUniformMatrix4fv(viewLocation, 1, GL_FALSE, &view[0][0]);
    glUniformMatrix4fv(projectionLocation, 1, GL_FALSE, glm::value_ptr(projection));

    updateLight(lightSource);
    updateMaterial(shiny);

    glBindVertexArray(VaoId);

    colorCode = 0;
    glUniform1i(colorCodeLocation, colorCode);

    int totalIndices = 3 * coneSlices * 2; // Side + base triangles
    glDrawElementsInstanced(GL_TRIANGLES, totalIndices, GL_UNSIGNED_INT, 0, instanceCount);

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
    glutCreateWindow("Tema 7 - 2");

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
