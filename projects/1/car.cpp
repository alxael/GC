#include <stdlib.h>
#include <stdio.h>

#include <iostream>
#include <vector>
#include <deque>
#include <algorithm>
#include <cmath>
#include <string>
#include <random>
#include <cstring>

#include <GL/glew.h>
#include <GL/freeglut.h>

#include "loadShaders.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include "soil.h"
#include "stb_image_aug.h"

// data structures
struct Car
{
    float x, y;
    float speed;
};
struct Reward
{
    float x, y;
    bool collected;
};

static int winW = 1280, winH = 720;

// player
float playerX = 0.0f, playerY = 0.0f;
float playerSpeed = 0.0f, playerAcc = 0.0001f;
float rotSmooth = 0.0f;
float drift = 0.0f;
float carWidth = 0.2f, carHeight = 0.4f;

bool keyStates[256] = {0};
bool specialKeyStates[512] = {0};
bool gameOver = false;

// trail
std::deque<std::pair<float, float>> trail;
const int TRAIL_MAX = 14;
const float TRAIL_MIN_DIST = 0.03f;
float trail_last_x = 0.0f, trail_last_y = 0.0f;

// lanes
float laneWidth = 0.6f;
std::vector<float> laneCenters;
int laneNumLeft = 12, laneNumRight = 12;

// lines
float lineDashOffset = 0.0f;
std::vector<float> lineOffsets; // individual offset for each line
const float dashSpeedFactor = 1.5f;

// rewards
std::vector<Reward> rewards;
int score = 0;

std::vector<Car> aiCars;
#define NUM_AI_CARS 50
#define AI_MIN_Y 0.5f
#define AI_MAX_Y 8.0f
#define AI_SPEED 0.008f
#define AI_SPAWN_AHEAD_MIN 2.0f
#define AI_SPAWN_AHEAD_MAX 4.0f

std::random_device rd;
std::mt19937 gen(rd());

inline float randomFloat(float a, float b)
{
    std::uniform_real_distribution<float> dis(a, b);
    return dis(gen);
}

inline int randomInt(int a, int b)
{
    if (a > b)
        std::swap(a, b);
    std::uniform_int_distribution<int> dis(a, b);
    return dis(gen);
}

// textures
GLuint carTexture;
GLuint rewardTexture;
GLuint loadTexture(const char *filename)
{
    int width = 0, height = 0, channels = 0;
    unsigned char *data = stbi_load(filename, &width, &height, &channels, 4);
    if (!data)
    {
        std::cerr << "Failed to load texture: " << filename << std::endl;
        return 0;
    }
    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);
    return tex;
}

// GL
GLuint ProgramId;
GLint codColLocation;
GLint codColVertLoc;
GLint uProjLoc;
GLint uTexLoc;
GLint uColorLoc;

GLuint quadVAO;
GLuint quadVBO;
GLuint lineVAO;
GLuint lineVBO;

static const float quadData[] = {
    -0.5f, -0.5f, 0.0f, 0.0f,
    0.5f, -0.5f, 1.0f, 0.0f,
    0.5f, 0.5f, 1.0f, 1.0f,
    -0.5f, -0.5f, 0.0f, 0.0f,
    0.5f, 0.5f, 1.0f, 1.0f,
    -0.5f, 0.5f, 0.0f, 1.0f};

void createQuad()
{
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadData), quadData, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void *)(2 * sizeof(float)));
    glBindVertexArray(0);
}

void createLineBuffer()
{
    glGenVertexArrays(1, &lineVAO);
    glGenBuffers(1, &lineVBO);
    glBindVertexArray(lineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
    glBufferData(GL_ARRAY_BUFFER, 32768 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void *)0);
    glBindVertexArray(0);
}

glm::mat4 mat_identity()
{
    return glm::mat4(1.0f);
}
glm::mat4 mat_ortho(float l, float r, float b, float t)
{
    return glm::ortho(l, r, b, t);
}

glm::mat4 mat_translate(float x, float y)
{
    return glm::translate(glm::mat4(1.0f), glm::vec3(x, y, 0.0f));
}

glm::mat4 mat_rotateZ(float deg)
{
    return glm::rotate(glm::mat4(1.0f), glm::radians(deg), glm::vec3(0, 0, 1));
}

glm::mat4 mat_scale(float sx, float sy)
{
    return glm::scale(glm::mat4(1.0f), glm::vec3(sx, sy, 1.0f));
}

glm::mat4 mat_mul(const glm::mat4 &a, const glm::mat4 &b)
{
    return a * b;
}

void initLanes(int numLeft = 12, int numRight = 12, float width = 0.6f)
{
    laneCenters.clear();
    laneWidth = width;
    laneNumLeft = numLeft;
    laneNumRight = numRight;

    for (int i = -numLeft; i <= numRight; ++i)
    {
        float center = i * laneWidth + laneWidth * 0.5f;
        laneCenters.push_back(center);
    }

    lineOffsets.clear();
    lineOffsets.reserve(laneCenters.size());
    const float dashLen = 0.7f;
    const float gapLen = 0.5f;
    float patternLen = dashLen + gapLen;

    for (size_t i = 0; i < laneCenters.size(); ++i)
    {
        lineOffsets.push_back(randomFloat(0.0f, patternLen));
    }
}

void spawnReward()
{
    if (laneCenters.empty())
        return;
    Reward r;
    r.x = laneCenters[randomInt(0, (int)laneCenters.size() - 1)];
    r.y = playerY + randomFloat(2.0f, 5.0f);
    r.collected = false;
    rewards.push_back(r);
}

void resetGame()
{
    playerX = 0.0f;
    playerY = 0.0f;
    playerSpeed = 0.0f;
    drift = 0.0f;
    rotSmooth = 0.0f;
    gameOver = false;
    trail.clear();
    aiCars.clear();
    rewards.clear();
    score = 0;
    if (laneCenters.empty())
        initLanes(laneNumLeft, laneNumRight, laneWidth);
    for (int i = 0; i < NUM_AI_CARS; ++i)
    {
        Car c;
        c.x = laneCenters[randomInt(0, (int)laneCenters.size() - 1)];
        const float safeAhead = 1.0f;
        c.y = playerY + safeAhead + randomFloat(AI_MIN_Y, AI_MAX_Y);
        c.speed = AI_SPEED * randomFloat(0.9f, 1.4f);
        aiCars.push_back(c);
    }
    for (int i = 0; i < 8; ++i)
        spawnReward();
}

void drawTexturedQuad(float x, float y, float sx, float sy, float angleDeg, GLuint texture)
{
    glm::mat4 T = glm::translate(glm::mat4(1.0f), glm::vec3(x, y, 0.0f));
    glm::mat4 R = glm::rotate(glm::mat4(1.0f), glm::radians(angleDeg), glm::vec3(0.0f, 0.0f, 1.0f));
    glm::mat4 S = glm::scale(glm::mat4(1.0f), glm::vec3(sx, sy, 1.0f));
    glm::mat4 M = T * R * S;
    if (codColVertLoc >= 0)
        glUniformMatrix4fv(codColVertLoc, 1, GL_FALSE, glm::value_ptr(M));
    if (codColLocation >= 0)
        glUniform1i(codColLocation, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    if (uTexLoc >= 0)
        glUniform1i(uTexLoc, 0);
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void drawColoredQuad(float x, float y, float sx, float sy, float angleDeg, const float color[4])
{
    glm::mat4 T = mat_translate(x, y);
    glm::mat4 R = mat_rotateZ(angleDeg);
    glm::mat4 S = mat_scale(sx, sy);
    glm::mat4 M = mat_mul(T, mat_mul(R, S));
    if (codColVertLoc >= 0)
        glUniformMatrix4fv(codColVertLoc, 1, GL_FALSE, glm::value_ptr(M));
    if (codColLocation >= 0)
        glUniform1i(codColLocation, 1); // color mode
    if (uColorLoc >= 0)
        glUniform4fv(uColorLoc, 1, color);
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void drawLines(const std::vector<float> &verts, const std::vector<float> &offsets, const float color[4], float lineWidth = 3.0f)
{
    if (verts.empty() || offsets.empty())
        return;

    const float dashLen = 1.50f;
    const float gapLen = 0.5f;
    float patternLen = dashLen + gapLen;

    std::vector<float> dashed;
    dashed.reserve(verts.size() * 2);

    for (size_t i = 0, lineIdx = 0; i + 3 < verts.size(); i += 4, ++lineIdx)
    {
        float x1 = verts[i + 0], y1 = verts[i + 1];
        float x2 = verts[i + 2], y2 = verts[i + 3];

        float dx = x2 - x1, dy = y2 - y1;
        float len = sqrtf(dx * dx + dy * dy);
        if (len <= 0.0001f)
            continue;
        float ux = dx / len, uy = dy / len;

        float localOffset = offsets[lineIdx % offsets.size()];
        float pos = -localOffset;

        while (pos < len)
        {
            float segStart = std::max(0.0f, pos);
            float segEnd = std::min(len, pos + dashLen);
            if (segEnd > segStart + 1e-6f)
            {
                float sx = x1 + ux * segStart;
                float sy = y1 + uy * segStart;
                float ex = x1 + ux * segEnd;
                float ey = y1 + uy * segEnd;
                dashed.push_back(sx);
                dashed.push_back(sy);
                dashed.push_back(ex);
                dashed.push_back(ey);
            }
            pos += patternLen;
        }
    }

    if (dashed.empty())
        return;

    glm::mat4 I = mat_identity();
    if (codColVertLoc >= 0)
        glUniformMatrix4fv(codColVertLoc, 1, GL_FALSE, glm::value_ptr(I));
    if (codColLocation >= 0)
        glUniform1i(codColLocation, 1); // color mode
    if (uColorLoc >= 0)
        glUniform4fv(uColorLoc, 1, color);

    glBindVertexArray(lineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
    size_t maxFloats = 32768;
    size_t uploadCount = std::min(dashed.size(), maxFloats);
    if (uploadCount > 0)
    {
        glBufferSubData(GL_ARRAY_BUFFER, 0, uploadCount * sizeof(float), dashed.data());
        glLineWidth(lineWidth);
        int cnt = (int)(uploadCount / 2);
        if (cnt > 0)
            glDrawArrays(GL_LINES, 0, cnt);
    }
    glBindVertexArray(0);
}

void update()
{
    if (gameOver)
    {
        glutPostRedisplay();
        return;
    }

    const float maxSpeed = 0.02f;
    const float minSpeed = -0.02f;
    if (specialKeyStates[GLUT_KEY_UP] || keyStates['w'] || keyStates['W'])
    {
        playerSpeed += playerAcc;
        if (playerSpeed > maxSpeed)
            playerSpeed = maxSpeed;
    }
    else if (specialKeyStates[GLUT_KEY_DOWN] || keyStates['s'] || keyStates['S'])
    {
        playerSpeed -= playerAcc;
        if (playerSpeed < minSpeed)
            playerSpeed = minSpeed;
    }
    else
    {
        playerSpeed = 0.0f;
    }

    if (specialKeyStates[GLUT_KEY_LEFT] || keyStates['a'] || keyStates['A'])
    {
        playerX -= 0.009f;
        drift += 0.05f;
        if (drift > 10.0f)
            drift = 10.0f;
    }
    else if (specialKeyStates[GLUT_KEY_RIGHT] || keyStates['d'] || keyStates['D'])
    {
        playerX += 0.009f;
        drift -= 0.05f;
        if (drift < -10.0f)
            drift = -10.0f;
    }
    else
        drift *= 0.9f;

    rotSmooth = rotSmooth * 0.9f + drift * 0.1f;

    float leftLimit = -laneNumLeft * laneWidth + carWidth / 2.0f;
    float rightLimit = laneNumRight * laneWidth - carWidth / 2.0f;
    if (playerX < leftLimit)
    {
        playerX = leftLimit;
        drift = 0.0f;
        rotSmooth = 0.0f;
    }
    if (playerX > rightLimit)
    {
        playerX = rightLimit;
        drift = 0.0f;
        rotSmooth = 0.0f;
    }

    playerY += playerSpeed;

    float minScroll = 0.008f;
    float lineSpeed = minScroll + playerSpeed * 0.3f;
    lineDashOffset += lineSpeed * dashSpeedFactor;
    for (size_t i = 0; i < lineOffsets.size(); ++i)
    {
        lineOffsets[i] += lineSpeed * dashSpeedFactor;
    }

    for (auto &c : aiCars)
    {
        c.y -= c.speed;
        if (c.y < playerY - 2.0f)
        {
            bool overlap = false;
            int attempts = 0, MAX_ATT = 12;
            do
            {
                overlap = false;
                if (!laneCenters.empty())
                    c.x = laneCenters[randomInt(0, (int)laneCenters.size() - 1)];
                c.y = playerY + randomFloat(AI_SPAWN_AHEAD_MIN, AI_SPAWN_AHEAD_MAX);
                for (auto &o : aiCars)
                {
                    if (&o == &c)
                        continue;
                    if (fabsf(o.x - c.x) < carWidth * 1.05f && fabsf(o.y - c.y) < carHeight * 1.2f)
                    {
                        overlap = true;
                        break;
                    }
                }
                ++attempts;
            } while (overlap && attempts < MAX_ATT);
        }
        if (!gameOver && fabsf(playerX - c.x) < carWidth && fabsf(playerY - c.y) < carHeight)
        {
            gameOver = true;
            std::cout << "GAME OVER!" << std::endl;
        }
    }

    const float REWARD_SPEED = 0.008f;
    for (auto &r : rewards)
    {
        r.y -= REWARD_SPEED;
        if (!r.collected && fabsf(playerX - r.x) < carWidth / 2.0f && fabsf(playerY - r.y) < carHeight / 2.0f)
        {
            r.collected = true;
            score += 1;
            std::cout << "+1 Score! Total: " << score << std::endl;
        }
    }

    rewards.erase(std::remove_if(rewards.begin(), rewards.end(), [](Reward &r)
                                 { return r.collected || r.y < playerY - 5.0f; }),
                  rewards.end());

    const int TARGET_REWARDS = 12;
    while (rewards.size() < (size_t)TARGET_REWARDS)
        spawnReward();

    float spawnProbBase = 0.002f;
    float spawnProbSpeedScale = playerSpeed * 6.0f;
    float spawnProb = spawnProbBase + spawnProbSpeedScale;
    if (spawnProb > 0.15f)
        spawnProb = 0.15f;
    if (randomFloat(0.0f, 1.0f) < spawnProb)
    {
        spawnReward();
    }

    float tx = playerX;
    float ty = playerY - carHeight * 0.35f;
    float dx = tx - trail_last_x, dy = ty - trail_last_y;
    if (trail.empty() || (dx * dx + dy * dy) >= (TRAIL_MIN_DIST * TRAIL_MIN_DIST))
    {
        trail.emplace_back(tx, ty);
        trail_last_x = tx;
        trail_last_y = ty;
        if ((int)trail.size() > TRAIL_MAX)
            trail.pop_front();
    }

    glutPostRedisplay();
}

void drawHUD(const glm::mat4 &proj)
{
    char buf[64];
    snprintf(buf, sizeof(buf), "Score: %d", score);

    int px = 10;
    int py = winH - 24;

    glUseProgram(0);
    glWindowPos2i(px, py);
    for (char *p = buf; *p; ++p)
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *p);

    if (gameOver)
    {
        const char msg[] = "GAME OVER! Press R to restart";
        int msgw = (int)strlen(msg) * 9;
        int cx = (winW / 2) - (msgw / 2);
        int cy = (winH / 2) + 10;
        glWindowPos2i(cx, cy);
        for (const char *p = msg; *p; ++p)
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *p);
    }
    glUseProgram(ProgramId);
}

void renderScene()
{
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(ProgramId);

    float aspect = (float)winW / (float)winH;
    float zoom = 2.0f;
    glm::mat4 proj = glm::ortho(-zoom * aspect, zoom * aspect, -zoom, zoom);
    if (uProjLoc >= 0)
        glUniformMatrix4fv(uProjLoc, 1, GL_FALSE, glm::value_ptr(proj));

    static float camX = 0.0f, camY = 0.0f;
    camX = camX * 0.9f + playerX * 0.1f;
    camY = camY * 0.9f + playerY * 0.1f;

    if (!laneCenters.empty())
    {
        std::vector<float> verts;
        verts.reserve((laneNumLeft + laneNumRight + 1) * 4);
        float startY = camY - 4.0f;
        float endY = camY + 12.0f;
        int drawLeft = laneNumLeft;
        int drawRight = laneNumRight;
        for (int i = -drawLeft; i <= drawRight; ++i)
        {
            float x = i * laneWidth;
            verts.push_back(x - camX);
            verts.push_back(startY - camY);
            verts.push_back(x - camX);
            verts.push_back(endY - camY);
        }
        float laneColor[4] = {1.0f, 0.85f, 0.0f, 1.0f};
        drawLines(verts, lineOffsets, laneColor, 5.0f);
    }

    for (auto &r : rewards)
    {
        if (r.collected)
            continue;
        drawTexturedQuad(r.x - camX, r.y - camY, 0.2f, 0.2f, 0.0f, rewardTexture);
    }

    for (auto &c : aiCars)
    {
        drawTexturedQuad(c.x - camX, c.y - camY, carWidth, carHeight, 0.0f, carTexture);
    }

    if (!trail.empty())
    {
        int n = (int)trail.size(), i = 0;
        for (auto &p : trail)
        {
            float t = (float)i / std::max(1, n - 1);
            float alpha = 0.3f + 0.7f * t;
            float scale = 0.55f + 0.7f * t;
            float px = p.first - camX, py = p.second - camY;
            float w = (carWidth * 0.3f) * scale;
            float h = (carHeight * 0.25f) * scale;
            float col[4] = {0.15f, 0.15f, 0.15f, alpha};
            drawColoredQuad(px, py, w, h, 0.0f, col);
            ++i;
        }
    }

    drawTexturedQuad(playerX - camX, playerY - camY, carWidth, carHeight, -rotSmooth, carTexture);

    drawHUD(proj);

    glUseProgram(0);
    glutSwapBuffers();
}

void handleKeyDown(unsigned char key, int, int)
{
    keyStates[key] = true;
    if (gameOver && (key == 'r' || key == 'R'))
        resetGame();
    if (key == 27) // escape key
        exit(0);
}
void handleKeyUp(unsigned char key, int, int) { keyStates[key] = false; }
void handleSpecialDown(int key, int, int) { specialKeyStates[key] = true; }
void handleSpecialUp(int key, int, int) { specialKeyStates[key] = false; }
void reshape(int w, int h)
{
    winW = w;
    winH = h;
    glViewport(0, 0, w, h);
}

void initGL()
{
    glewExperimental = GL_TRUE;
    GLenum er = glewInit();
    if (er != GLEW_OK)
    {
        std::cerr << "GLEW init error: " << glewGetErrorString(er) << std::endl;
        exit(1);
    }

    glClearColor(0.6f, 0.6f, 0.6f, 1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    createQuad();
    createLineBuffer();

    ProgramId = LoadShaders("../projects/1/shader.vert", "../projects/1/shader.frag");
    glUseProgram(ProgramId);

    codColLocation = glGetUniformLocation(ProgramId, "codColShader");
    codColVertLoc = glGetUniformLocation(ProgramId, "codColVert");
    uProjLoc = glGetUniformLocation(ProgramId, "uProj");
    uTexLoc = glGetUniformLocation(ProgramId, "uTex");
    uColorLoc = glGetUniformLocation(ProgramId, "uColor");

    if (codColLocation < 0)
        std::cerr << "Warning: codColShader uniform not found\n";
    if (codColVertLoc < 0)
        std::cerr << "Warning: codColVert uniform not found\n";

    carTexture = loadTexture("../projects/1/car.png");
    if (carTexture == 0)
    {
        std::cerr << "Failed to load car.png. Adjust path.\n";
        exit(1);
    }
    rewardTexture = loadTexture("../projects/1/coin.png");
    if (rewardTexture == 0)
    {
        std::cerr << "Failed to load coin.png. Adjust path.\n";
        exit(1);
    }

    initLanes(18, 18, 0.6f);
    resetGame();
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(winW, winH);
    glutCreateWindow("Infinite Lanes");

    GLenum e = glewInit();
    if (e != GLEW_OK)
    {
        std::cerr << "glewInit failed: " << glewGetErrorString(e) << std::endl;
        return -1;
    }
    std::cout << "GL version: " << (const char *)glGetString(GL_VERSION) << std::endl;

    initGL();

    glutDisplayFunc(renderScene);
    glutIdleFunc(update);
    glutKeyboardFunc(handleKeyDown);
    glutKeyboardUpFunc(handleKeyUp);
    glutSpecialFunc(handleSpecialDown);
    glutSpecialUpFunc(handleSpecialUp);
    glutReshapeFunc(reshape);

    glutMainLoop();
    return 0;
}