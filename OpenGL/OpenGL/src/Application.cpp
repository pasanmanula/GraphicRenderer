#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

struct ShaderProgramSource
{
    std::string VertexSource;
    std::string FragmentSource;
};

static ShaderProgramSource ParseShader(const std::string& filepath)
{
    std::ifstream stream(filepath);

    enum class ShaderType
    {
        NONE = -1, VERTEX = 0, FRAGMENT = 1
    };
    std::string line;
    std::stringstream ss[2];
    ShaderType type = ShaderType::NONE;

    while (getline(stream, line))
    {
        if (line.find("#shader") != std::string::npos) 
        {
            if (line.find("vertex") != std::string::npos)
                type = ShaderType::VERTEX;
            else if (line.find("fragment") != std::string::npos)
                type = ShaderType::FRAGMENT;
        }
        else 
        {
            ss[(int)type] << line << '\n';
        }
    }
    return { ss[0].str(),ss[1].str() };
}

static unsigned int CompileShader(unsigned int type, const std::string& source)
{
    unsigned int id = glCreateShader(type);
    const char* src = source.c_str(); //&source[0]
    glShaderSource(id,1,&src,nullptr);
    glCompileShader(id); //Compile the shader

    //Error handelling
    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS,&result);
    if (result == GL_FALSE) {
        int length;
        glGetShaderiv(id,GL_INFO_LOG_LENGTH, &length);
        char* message = (char*)alloca(length * sizeof(char));
        glGetShaderInfoLog(id,length,&length,message);
        std::cout << "Failed to compile " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << std::endl;
        std::cout << message << std::endl; 
        glDeleteShader(id);
        return 0;
    }

    return id;
}

static unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader)
{
    unsigned int program = glCreateProgram();
    unsigned int vs = CompileShader(GL_VERTEX_SHADER,vertexShader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glValidateProgram(program);

    glDeleteShader(vs); //Delete the intermediate files
    glDeleteShader(fs);

    return program;
}

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK) {
        std::cout << "Error!" << std::endl;
    }

    std::cout << glGetString(GL_VERSION) << std::endl;

    ///   

    //Vertex butter- Copy positions array to vRAM of the GPU AND select that buffer. (Remember this is a state machine)
    float positions[] = {
        -0.5f,-0.5f, //Vertx 0
         0.5f, -0.5f,//Vertx 1
         0.5f, 0.5f,//Vertx 2
         -0.5f, 0.5f //Vertx 3
    };

    unsigned int indices[] = {
        0,1,2,
        2,3,0
    };

    unsigned int buffer;
    glGenBuffers(1,&buffer); //One buffer and pointer to a unsigned int--> Generating a buffer and giving us an ID to refer later.
    glBindBuffer(GL_ARRAY_BUFFER,buffer); //Selecting(glbind**) the above buffer
    glBufferData(GL_ARRAY_BUFFER, 6 * 2 * sizeof(float),positions,GL_STATIC_DRAW); //Put the data into the buffer

    //Vertex Attributes - Telling our Layout (Here positions)
    glEnableVertexAttribArray(0); //Enable the vertex attribute
    glVertexAttribPointer(0,2,GL_FLOAT,GL_FALSE,sizeof(float)*2,0); //Telling the layout

    //Index Buffer - To remove duplicate vertices. The index buffer MUST be unsigned int not signed int
    unsigned int ibo;
    glGenBuffers(1, &ibo); //One buffer and pointer to a unsigned int--> Generating a buffer and giving us an ID to refer later.
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo); //Selecting(glbind**) the above buffer
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned int), indices, GL_STATIC_DRAW); //Put the data into the buffer

    //Shader - block of code that run on the GPU
    //Vertex shader - This shader will call for every vertex (3 times in this case). Primary purpose :- Where the vertex should place in the screen.
    //Fragment/Pixel shader - Runs once for each pixel that needs to get rasterized(drawn on the screen). Primary purpose :- Decide which color that pixel supposed to be.
    
    
    //std::string vertexShader =
    //    "#version 330 core\n"
    //    "\n"
    //    "layout(location =0) in vec4 positions;\n"
    //    "\n"
    //    "void main()"
    //    "{\n"
    //    "   gl_Position = positions;\n"
    //    "}\n";

    //std::string fragmentShader =
    //    "#version 330 core\n"
    //    "\n"
    //    "layout(location =0) out vec4 color;\n"
    //    "\n"
    //    "void main()"
    //    "{\n"
    //    "   color = vec4(1.0,0.0,0.0,1.0);\n"
    //    "}\n";
    //unsigned int shader = CreateShader(vertexShader, fragmentShader);

    ShaderProgramSource source = ParseShader("resources/shaders/Basic.shader");
    unsigned int shader = CreateShader(source.VertexSource, source.FragmentSource);
    glUseProgram(shader);


    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window)) 
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);
        /*
        glBegin(GL_TRIANGLES);
        glVertex2f(-0.5f,-0.5f);
        glVertex2f(0.0f, 0.5f);
        glVertex2f(0.5f, -0.5f);
        glEnd();
        */
        glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_INT,nullptr);
        //glDrawArrays(GL_TRIANGLES,0,6); //Draw vertices sequencially.
 
        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }
    glDeleteProgram(shader);
    glfwTerminate();
    return 0;
}