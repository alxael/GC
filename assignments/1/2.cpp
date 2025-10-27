#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

GLuint
    VaoId,
    EboId,
    VboId,
    ColorBufferId,
    VertexShaderId,
    FragmentShaderId,
    ProgramId;

const GLchar *VertexShader =
    {
        "#version 400\n"
        "layout(location=0) in vec4 in_Position;\n"
        "layout(location=1) in vec4 in_Color;\n"
        "uniform mat4 mvpMatrix;\n"
        "out vec4 ex_Color;\n"
        "void main(void)\n"
        "{\n"
        "  gl_Position = mvpMatrix * in_Position;\n"
        "  ex_Color = in_Color;\n"
        "}\n"};

const GLchar *FragmentShader =
    {
        "#version 400\n"

        "in vec4 ex_Color;\n"
        "out vec4 out_Color;\n"

        "void main(void)\n"
        "{\n"
        "  out_Color = ex_Color;\n"
        "}\n"};

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

void multiplyMatrices(const float *a, const float *b, float *result)
{
    for (int i = 0; i < 16; ++i)
        result[i] = 0.0f;
    for (int row = 0; row < 4; ++row)
        for (int col = 0; col < 4; ++col)
            for (int k = 0; k < 4; ++k)
                result[col * 4 + row] += a[k * 4 + row] * b[col * 4 + k];
}

void createMVP(float *mvp)
{
    float fovy = 45.0f * 3.14159265f / 180.0f;
    float aspect = 800.0f / 500.0f;
    float near = 0.1f;
    float far = 100.0f;
    float f = 1.0f / tan(fovy / 2.0f);

    float proj[16] = {
        f / aspect, 0, 0, 0,
        0, f, 0, 0,
        0, 0, (far + near) / (near - far), -1,
        0, 0, (2 * far * near) / (near - far), 0};

    float eyeX = 0.0f, eyeY = 2.0f, eyeZ = 3.0f;
    float centerX = 0.0f, centerY = 0.0f, centerZ = 0.0f;
    float upX = 0.0f, upY = 1.0f, upZ = 0.0f;

    float fx = centerX - eyeX;
    float fy = centerY - eyeY;
    float fz = centerZ - eyeZ;
    float rlf = 1.0f / sqrtf(fx * fx + fy * fy + fz * fz);
    fx *= rlf;
    fy *= rlf;
    fz *= rlf;

    float sx = fy * upZ - fz * upY;
    float sy = fz * upX - fx * upZ;
    float sz = fx * upY - fy * upX;
    float rls = 1.0f / sqrtf(sx * sx + sy * sy + sz * sz);
    sx *= rls;
    sy *= rls;
    sz *= rls;

    float ux = sy * fz - sz * fy;
    float uy = sz * fx - sx * fz;
    float uz = sx * fy - sy * fx;

    float view[16] = {
        sx, ux, -fx, 0,
        sy, uy, -fy, 0,
        sz, uz, -fz, 0,
        0, 0, 0, 1};

    float translation[16] = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        -eyeX, -eyeY, -eyeZ, 1};

    float temp[16];
    multiplyMatrices(view, translation, temp);

    multiplyMatrices(proj, temp, mvp);
}

void CreateVBO(void)
{
    Vertex vertices[] = {
        {Position(-0.8f, -0.8f, -0.8f), Color(0.0f, 0.0f, 0.0f)}, // black - 0
        {Position(0.8f, -0.8f, -0.8f), Color(1.0f, 0.0f, 0.0f)},  // red - 1
        {Position(-0.8f, 0.8f, -0.8f), Color(0.0f, 1.0f, 0.0f)},  // green - 2
        {Position(0.8f, 0.8f, -0.8f), Color(1.0f, 1.0f, 0.0f)},   // yellow - 3
        {Position(-0.8f, -0.8f, 0.8f), Color(0.0f, 0.0f, 1.0f)},  // blue - 4
        {Position(0.8f, -0.8f, 0.8f), Color(1.0f, 0.0f, 1.0f)},   // magenta - 5
        {Position(-0.8f, 0.8f, 0.8f), Color(0.0f, 1.0f, 1.0f)},   // cyan - 6
        {Position(0.8f, 0.8f, 0.8f), Color(1.0f, 1.0f, 1.0f)},    // white - 7
    };

    GLuint indices[] = {
        0, 1, 3, 2, // bottom - back
        2, 0, 4, 6, // left - left
        7, 3, 1, 5, // right - right
        0, 1, 5, 4, // back - bottom
        6, 2, 3, 7, // front - top
        6, 7, 5, 4  // top - front
    };

    glGenVertexArrays(1, &VaoId);
    glBindVertexArray(VaoId);

    glGenBuffers(1, &VboId);
    glBindBuffer(GL_ARRAY_BUFFER, VboId);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &EboId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EboId);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

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

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glDeleteBuffers(1, &EboId);

    glBindVertexArray(0);
    glDeleteVertexArrays(1, &VaoId);
}

void CreateShaders(void)
{
    VertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(VertexShaderId, 1, &VertexShader, NULL);
    glCompileShader(VertexShaderId);

    FragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(FragmentShaderId, 1, &FragmentShader, NULL);
    glCompileShader(FragmentShaderId);

    ProgramId = glCreateProgram();
    glAttachShader(ProgramId, VertexShaderId);
    glAttachShader(ProgramId, FragmentShaderId);
    glLinkProgram(ProgramId);
    glUseProgram(ProgramId);

    float mvpMatrix[16];
    createMVP(mvpMatrix);
    GLuint mvpLoc = glGetUniformLocation(ProgramId, "mvpMatrix");
    glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, mvpMatrix);
}

void DestroyShaders(void)
{
    glUseProgram(0);

    glDetachShader(ProgramId, VertexShaderId);
    glDetachShader(ProgramId, FragmentShaderId);

    glDeleteShader(FragmentShaderId);
    glDeleteShader(VertexShaderId);

    glDeleteProgram(ProgramId);
}

void Initialize(void)
{
    glClearColor(0.5f, 0.5f, 0.5f, 0.0f);
    glEnable(GL_DEPTH_TEST);

    glDepthFunc(GL_LESS);
    CreateVBO();
    CreateShaders();
}

void RenderFunction(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPointSize(20.0);

    glDrawElements(GL_QUADS, 24, GL_UNSIGNED_INT, 0);

    glFlush();
}

void Cleanup(void)
{
    DestroyShaders();
    DestroyVBO();
}

int main(int argc, char *argv[])
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowPosition(100, 300);
    glutInitWindowSize(800, 500);
    glutCreateWindow("Cub magic");

    glewInit();

    Initialize();
    glutDisplayFunc(RenderFunction);
    glutCloseFunc(Cleanup);

    glutMainLoop();

    return 0;
}