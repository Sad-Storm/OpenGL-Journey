#include <iostream>
#include <SDL2/SDL.h>
#include <glad/glad.h>
#include <vector>
#include <string>
#include <fstream>

// Globals
int gScreenWidth = 640;
int gScreenHeight = 480;
SDL_Window* gGraphicsApplicationWindow = nullptr;
SDL_GLContext gOpenGLContext = nullptr;

// VAO
GLuint gVertexArrayObject = 0;
// VBO
GLuint gVertexBufferObject = 0;
GLuint gVertexBufferObject2 = 0;

// Program Object (for shader)
GLuint gGraphicsPipelineShaderProgram = 0;

bool gQuit = false;

std::string LoadShaderAsString(const std::string& filename){
    
    std::string result = "";

    std::string line = "";
    std::ifstream myFile(filename.c_str());

    if (myFile.is_open()){
        while (std::getline(myFile, line)){
            result+=line+"\n";
        }
        myFile.close();
    }

    return result;
}

void GetOpenGLVersionInfo(){
    std::cout << "Vendor: " << glGetString(GL_VENDOR) << std::endl;
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
    std::cout << "Version: " << glGetString(GL_VERSION) << std::endl;
    std::cout << "Shading Language: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
}

void VertexSpecification(){
    // On the CPU
    // Vertex Positions
    const std::vector<GLfloat> vertexPosition = {
        -0.8f,-0.8f,0.0f, // vertex 1
        0.8f,-0.8f,0.0f, // vertex 2
        0.0f,0.8f,0.0f // vertex 3
    };

    // Vertex Colors
    const std::vector<GLfloat> vertexColors = {
        1.0f,0.0f,0.0f, // color 1
        0.0f,1.0f,0.0f, // color 2
        0.0f,0.0f,1.0f, // color 3
    };

    // Start setting up things on the GPU
    glGenVertexArrays(1,&gVertexArrayObject);
    glBindVertexArray(gVertexArrayObject);

    // Generate VBO, positions
    glGenBuffers(1, &gVertexBufferObject);
    glBindBuffer(GL_ARRAY_BUFFER, gVertexBufferObject);
    glBufferData(GL_ARRAY_BUFFER, vertexPosition.size() * sizeof(GLfloat), vertexPosition.data(), GL_STATIC_DRAW);

    // Generate VBO2, colors
    glGenBuffers(1, &gVertexBufferObject2);
    glBindBuffer(GL_ARRAY_BUFFER, gVertexBufferObject2);
    glBufferData(GL_ARRAY_BUFFER, vertexColors.size() * sizeof(GLfloat), vertexColors.data(), GL_STATIC_DRAW);

    // Linking attributes in VAO
    // Enable VAO 1
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    
    glBindVertexArray(0);
    // Disable VAO 0
    glDisableVertexAttribArray(0);
    // Disable VAO 1
    glDisableVertexAttribArray(1);
}

GLuint CompileShader(GLuint type, const std::string& source){
    GLuint shaderObject;

    if(type==GL_VERTEX_SHADER){
        shaderObject = glCreateShader(GL_VERTEX_SHADER);
    }else if(type==GL_FRAGMENT_SHADER){
        shaderObject = glCreateShader(GL_FRAGMENT_SHADER);
    } else {
        std::cout << "Only Vertex shader and Fragment shader are supported" << std::endl;
        exit(1);
    }

    const char* charSource = source.c_str();
    glShaderSource(shaderObject, 1, &charSource, nullptr);
    glCompileShader(shaderObject);

    return shaderObject;
}

GLuint CreateShaderProgram(const std::string& vertexShaderSource, const std::string& fragmentShaderSource){

    GLuint programObject = glCreateProgram();

    GLuint myVertexShader = CompileShader(GL_VERTEX_SHADER, vertexShaderSource);
    GLuint myFragmentShader = CompileShader(GL_FRAGMENT_SHADER, fragmentShaderSource);

    glAttachShader(programObject, myVertexShader);
    glAttachShader(programObject, myFragmentShader);
    glLinkProgram(programObject);

    // Validate our program
    glValidateProgram(programObject);
    // TODO: glDetachShader, glDeleteShader

    return programObject;
}

void CreateGraphicsPipeline(){
    std::string vertexShaderSource = LoadShaderAsString("./shaders/vertex.glsl");
    std::string fragmentShaderSource = LoadShaderAsString("./shaders/fragment.glsl");

    gGraphicsPipelineShaderProgram = CreateShaderProgram(vertexShaderSource, fragmentShaderSource);
}

void InitializeProgram(){
    if(SDL_Init(SDL_INIT_VIDEO)<0){
        std::cout << "SDL2 could not initialize video subsystem" << std::endl;
        exit(1);
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    gGraphicsApplicationWindow = SDL_CreateWindow("SDL game", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, gScreenWidth, gScreenHeight, SDL_WINDOW_OPENGL);

    if(gGraphicsApplicationWindow==nullptr){
        std::cout << "SDL_Window was not able to be created" << std::endl;
        exit(1);
    }

    gOpenGLContext = SDL_GL_CreateContext(gGraphicsApplicationWindow);

    if(gOpenGLContext==nullptr){
        std::cout << "OpenGL context not available" << std::endl;
        exit(1);
    }

    // Initialize Glad Library
    if(!gladLoadGLLoader(SDL_GL_GetProcAddress)){
        std::cout << "Glad failed to initialize" << std::endl;
        exit(1);
    }

    GetOpenGLVersionInfo();
}

void Input(){
    SDL_Event e;

    while(SDL_PollEvent(&e) != 0){
        if (e.type==SDL_QUIT){
            std::cout << "Goodbye!" << std::endl;
            gQuit = true;
        }
    }
}

void PreDraw(){
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    glViewport(0,0,gScreenWidth,gScreenHeight);
    glClearColor(.2f,.0f,.1f,1.f);

    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    glUseProgram(gGraphicsPipelineShaderProgram);


}

void Draw(){

    glBindVertexArray(gVertexArrayObject);
    glBindBuffer(GL_ARRAY_BUFFER,gVertexBufferObject);

    glDrawArrays(GL_TRIANGLES,0,3);
}

void MainLoop(){
    while(!gQuit){
        Input();

        PreDraw();

        Draw();

        // Update screen
        SDL_GL_SwapWindow(gGraphicsApplicationWindow);
    }
}

void CleanUp(){

    // Destroy the window
    SDL_DestroyWindow(gGraphicsApplicationWindow);

    // Quit SDL subsystems
    SDL_Quit();
}

int main(int argc, char* argv[]){

    // Setup graphics program
    InitializeProgram();

    // Set up geometry, VAO, and VBO
    VertexSpecification();

    // Create graphics pipeline
    // At the moment we set up vertext and fragment shaders
    CreateGraphicsPipeline();

    // Call main loop
    MainLoop();

    // When program terminates clean up
    CleanUp();

    return 0;
}