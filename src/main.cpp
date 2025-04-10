#include <iostream>
#include <SDL2/SDL.h>
#include <glad/glad.h>
#include <string>
#include <fstream>
#include <vector>
#include <chrono>

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "Camera.hpp"

// #define GLM_ENABLE_EXPERIMENTAL
// #include <glm/gtx/string_cast.hpp>

// Error checking
static void GLClearError()
{
    while (glGetError() != GL_NO_ERROR)
        ;
}
static bool GLCheckError(const char *function, int line)
{
    while (GLenum error = glGetError())
    {
        std::cout << "OpenGL Error: " << error << ", Line: " << line << ", Function: " << function << std::endl;
        return true;
    }
    return false;
}
#define GLCheck(x)  \
    GLClearError(); \
    x;              \
    GLCheckError(#x, __LINE__);

struct App
{
    int mScreenWidth = 640;
    int mScreenHeight = 480;
    SDL_Window *mGraphicsApplicationWindow = nullptr;
    SDL_GLContext mOpenGLContext = nullptr;
    GLuint mGraphicsPipelineShaderProgram = 0;
    bool mQuit = false;
    Camera mCamera;
};

struct Mesh3D
{
    // VAO, VBO, and IBO
    GLuint mVertexArrayObject = 0;
    GLuint mVertexBufferObject = 0;
    GLuint mIndexBufferObject = 0;
};

// Globals
App gApp;
Mesh3D gMesh;

float gSpinAngle = 0.0f;

const int gTargetFPS = 60;
const int gFrameDuration = 1000 / gTargetFPS;

// Function to load shader source code from file
std::string LoadShaderAsString(const std::string filename)
{
    std::string result = "";
    std::string line = "";

    std::ifstream myFile(filename);

    while (std::getline(myFile, line))
    {
        result += line + "\n";
    }
    myFile.close();

    return result;
}

void GetOpenGLVersionInfo()
{
    std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "Shading Language: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
}

// Function to set up vertex data and buffers
void VertexSpecification(Mesh3D *mesh)
{
    // Vertex Positions
    GLfloat vertexData[] = {
        // Triangle data for quad
        -0.5f, -0.5f, 0.0f, // Bottom Left
        1.0f, 0.0f, 0.0f,   // Red
        0.5f, -0.5f, 0.0f,  // Bottom Right
        0.0f, 1.0f, 0.0f,   // Green
        -0.5f, 0.5f, 0.0f,  // Top Left
        0.0f, 0.0f, 1.0f,   // Blue
        0.5f, 0.5f, 0.0f,   // Top Right
        0.5f, 0.5f, 0.5f,   // Gray
    };

    GLuint indexData[] = {
        0, 1, 2,
        2, 1, 3};

    // Setup VAO
    glGenVertexArrays(1, &mesh->mVertexArrayObject);
    glBindVertexArray(mesh->mVertexArrayObject);

    // Setup VBO
    glGenBuffers(1, &mesh->mVertexBufferObject);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->mVertexBufferObject);

    // Send vertex data to GPU
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);

    // Setup IBO
    glGenBuffers(1, &mesh->mIndexBufferObject);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->mIndexBufferObject);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indexData), indexData, GL_STATIC_DRAW);

    // Locations
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 6, (void *)0);
    glEnableVertexAttribArray(0);

    // Colors
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 6, (void *)(3 * sizeof(GLfloat)));
    glEnableVertexAttribArray(1);

    // Send to vertex shader
    glBindVertexArray(0);

    // Unbind VAO and VBO
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

GLuint CompileShader(GLuint type, const std::string &source)
{
    GLuint shaderObject;

    if (type == GL_VERTEX_SHADER)
    {
        shaderObject = glCreateShader(GL_VERTEX_SHADER);
    }
    else if (type == GL_FRAGMENT_SHADER)
    {
        shaderObject = glCreateShader(GL_FRAGMENT_SHADER);
    }
    else
    {
        std::cout << "Only Vertex shader and Fragment shader are supported" << std::endl;
        exit(1);
    }

    const char *charSource = source.c_str();
    glShaderSource(shaderObject, 1, &charSource, nullptr);
    glCompileShader(shaderObject);

    // Check for compilation errors, rewrite later
    GLint isCompiled = 0;
    glGetShaderiv(shaderObject, GL_COMPILE_STATUS, &isCompiled);
    if (isCompiled == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetShaderiv(shaderObject, GL_INFO_LOG_LENGTH, &maxLength);

        std::vector<char> infoLog(maxLength);
        glGetShaderInfoLog(shaderObject, maxLength, &maxLength, &infoLog[0]);

        glDeleteShader(shaderObject);

        std::cout << "Shader compilation error: " << std::string(infoLog.begin(), infoLog.end()) << std::endl;
        exit(1);
    }

    return shaderObject;
}

GLuint CreateShaderProgram(const std::string &vertexShaderSource, const std::string &fragmentShaderSource)
{
    GLuint programObject = glCreateProgram();

    GLuint myVertexShader = CompileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint myFragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    glAttachShader(programObject, myVertexShader);
    glAttachShader(programObject, myFragmentShader);
    glLinkProgram(programObject);

    // Check for linking errors, rewrite later
    GLint isLinked = 0;
    glGetProgramiv(programObject, GL_LINK_STATUS, &isLinked);
    if (isLinked == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetProgramiv(programObject, GL_INFO_LOG_LENGTH, &maxLength);

        std::vector<char> infoLog(maxLength);
        glGetProgramInfoLog(programObject, maxLength, &maxLength, &infoLog[0]);

        glDeleteProgram(programObject);
        glDeleteShader(myVertexShader);
        glDeleteShader(myFragmentShader);

        std::cout << "Program linking error: " << std::string(infoLog.begin(), infoLog.end()) << std::endl;
        exit(1);
    }

    // Detach and delete shaders after linking
    glDetachShader(programObject, myVertexShader);
    glDetachShader(programObject, myFragmentShader);
    glDeleteShader(myVertexShader);
    glDeleteShader(myFragmentShader);

    return programObject;
}

void CreateGraphicsPipeline()
{
    std::string vertexShaderSource = LoadShaderAsString("../shaders/vertex.glsl");
    std::string fragmentShaderSource = LoadShaderAsString("../shaders/fragment.glsl");

    gApp.mGraphicsPipelineShaderProgram = CreateShaderProgram(vertexShaderSource, fragmentShaderSource);
}

// Function to initialize the SDL and OpenGL context
void InitializeProgram(App *app)
{
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        std::cout << "SDL2 could not initialize video subsystem" << std::endl;
        exit(1);
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    app->mGraphicsApplicationWindow = SDL_CreateWindow("SDL game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, app->mScreenWidth, app->mScreenHeight, SDL_WINDOW_OPENGL);

    if (app->mGraphicsApplicationWindow == nullptr)
    {
        std::cout << "SDL_Window was not able to be created" << std::endl;
        exit(1);
    }

    app->mOpenGLContext = SDL_GL_CreateContext(app->mGraphicsApplicationWindow);

    if (app->mOpenGLContext == nullptr)
    {
        std::cout << "OpenGL context not available" << std::endl;
        exit(1);
    }

    // Initialize Glad Library
    if (!gladLoadGLLoader(SDL_GL_GetProcAddress))
    {
        std::cout << "Glad failed to initialize" << std::endl;
        exit(1);
    }

    GetOpenGLVersionInfo();
}

// Function to handle input events
void Input()
{
    SDL_Event e;

    while (SDL_PollEvent(&e) != 0)
    {
        if (e.type == SDL_QUIT)
        {
            std::cout << "Goodbye!" << std::endl;
            gApp.mQuit = true;
        }
        else if (e.type == SDL_MOUSEMOTION)
        {
            if (SDL_GetRelativeMouseMode())
            {
                int deltaX = e.motion.xrel;
                int deltaY = e.motion.yrel;
                gApp.mCamera.mouseLook(deltaX, deltaY);
            }
        }
        else if (e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_ESCAPE)
        {
            if (SDL_GetRelativeMouseMode())
            {
                SDL_SetRelativeMouseMode(SDL_FALSE);
                SDL_ShowCursor(SDL_TRUE);
            }
            else
            {
                SDL_SetRelativeMouseMode(SDL_TRUE);
                SDL_ShowCursor(SDL_FALSE);
            }
        }
    }

    float speed = 0.1f;

    const Uint8 *state = SDL_GetKeyboardState(NULL);

    if (state[SDL_SCANCODE_W])
    {
        gApp.mCamera.moveForward(speed);
    }
    if (state[SDL_SCANCODE_S])
    {
        gApp.mCamera.moveBackward(speed);
    }
    if (state[SDL_SCANCODE_A])
    {
        gApp.mCamera.moveLeft(speed);
    }
    if (state[SDL_SCANCODE_D])
    {
        gApp.mCamera.moveRight(speed);
    }
}

void PreDraw()
{
    glEnable(GL_DEPTH_TEST);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    glViewport(0, 0, gApp.mScreenWidth, gApp.mScreenHeight);
    glClearColor(0.2f, 0.0f, 0.1f, 1.0f);

    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    // Use Shader Program
    glUseProgram(gApp.mGraphicsPipelineShaderProgram);

    gSpinAngle += 0.01f;

    // Create transformation matrices
    glm::mat4 globalTransform = glm::rotate(glm::mat4(1.0f), gSpinAngle, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 viewSpace = gApp.mCamera.getViewMatrix();
    glm::mat4 perspective = glm::perspective(glm::radians(45.0f), ((float)gApp.mScreenWidth) / ((float)gApp.mScreenHeight), 0.1f, 10.0f);

    glm::mat4 transforms = perspective * viewSpace * globalTransform;

    // Find uniform locations
    GLint uTransformLocation = glGetUniformLocation(gApp.mGraphicsPipelineShaderProgram, "uTransform");

    // Set uniform
    if (uTransformLocation >= 0)
    {
        glUniformMatrix4fv(uTransformLocation, 1, GL_FALSE, &transforms[0][0]);
    }
    else
    {
        std::cout << "Transform uniform not found, does name match?" << std::endl;
        exit(1);
    }
}

void Draw()
{
    glBindVertexArray(gMesh.mVertexArrayObject);

    // glDrawArrays(GL_TRIANGLES,0,6);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
}

void MainLoop()
{
    // Move mouse to middle of screen
    SDL_WarpMouseInWindow(gApp.mGraphicsApplicationWindow, gApp.mScreenWidth / 2, gApp.mScreenHeight / 2);
    SDL_SetRelativeMouseMode(SDL_TRUE);
    SDL_ShowCursor(SDL_FALSE);

    bool frameSafe = true;

    while (!gApp.mQuit)
    {
        // Start frame timer
        std::chrono::high_resolution_clock::time_point frameStart = std::chrono::high_resolution_clock::now();

        Input();
        PreDraw();
        Draw();

        // Update screen
        SDL_GL_SwapWindow(gApp.mGraphicsApplicationWindow);

        // Calculate frame duration
        std::chrono::high_resolution_clock::time_point frameEnd = std::chrono::high_resolution_clock::now();
        long long frameTime = std::chrono::duration_cast<std::chrono::milliseconds>(frameEnd - frameStart).count();

        // Delay to maintain target frame rate
        if (frameTime < gFrameDuration)
        {
            SDL_Delay(gFrameDuration - frameTime);
        }
        else if (frameSafe && frameTime > gFrameDuration)
        {
            std::cout << "Frame rate has dropped below " << gTargetFPS << " FPS" << std::endl;
            frameSafe = false;
        }
    }
}

void CleanUp()
{
    // Destroy the window
    SDL_DestroyWindow(gApp.mGraphicsApplicationWindow);

    // Quit SDL subsystems
    SDL_Quit();
}

int main(int argc, char *argv[])
{
    // Setup graphics program
    InitializeProgram(&gApp);

    // Set up geometry, VAO, and VBO
    VertexSpecification(&gMesh);

    // Create graphics pipeline
    // At the moment we set up the
    // vertex and fragment shaders
    CreateGraphicsPipeline();

    // Call main loop
    MainLoop();

    // When program terminates clean up
    CleanUp();

    return 0;
}