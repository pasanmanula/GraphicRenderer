#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include "Renderer.h"
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"
#include "IndexBuffer.h"
#include "VertexArray.h"
#include "Shader.h"
#include "Texture.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw_gl3.h"

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    //Setting up the openGL profile to Core (To support Vertex array object)
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); //VERSION 3.3
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //Core Profile


    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(960, 540, "Hello World", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    glfwSwapInterval(1); //Sync with monitors refresh rate

    if (glewInit() != GLEW_OK) {
        std::cout << "Error!" << std::endl;
    }

    std::cout << glGetString(GL_VERSION) << std::endl;
    /* 
    -0.5f, -0.5f, 0.0f, 0.0f,//Vertx 0
        0.5f, -0.5f, 1.0f, 0.0f,//Vertx 1
        0.5f, 0.5f, 1.0f, 1.0f,//Vertx 2
        -0.5f, 0.5f, 0.0f, 1.0f //Vertx 3
    */
    float positions[] = {
         100.0f, 100.0f, 0.0f, 0.0f,//Vertx 0
         200.0f, 100.0f, 1.0f, 0.0f,//Vertx 1
         200.0f, 200.0f, 1.0f, 1.0f,//Vertx 2
         100.0f, 200.0f, 0.0f, 1.0f //Vertx 3
    };

    unsigned int indices[] = {
        0,1,2,
        2,3,0
    };


    GLCall(glEnable(GL_BLEND));
    GLCall(glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA));

    //Vertex array
    VertexArray va;
    //Vertex buffer- Copy positions array to vRAM of the GPU AND select that buffer. (Remember this is a state machine)
    VertexBuffer vb(positions, 4 * 4 * sizeof(float));

    //Vertex Attributes - Telling our Layout (Here positions)
    VertexBufferLayout layout;
    layout.Push<float>(2);
    layout.Push<float>(2);
    va.AddBuffer(vb, layout);

    //Index Buffer - To remove duplicate vertices. The index buffer MUST be unsigned int not signed int
    IndexBuffer ib(indices,6);
   
    //Shader - block of code that run on the GPU
    //Vertex shader - This shader will call for every vertex (3 times in this case). Primary purpose :- Where the vertex should place in the screen.
    //Fragment/Pixel shader - Runs once for each pixel that needs to get rasterized(drawn on the screen). Primary purpose :- Decide which color that pixel supposed to be.
    glm::mat4 proj = glm::ortho(0.0f,960.0f,0.0f,540.0f,-1.0f,1.0f);
    glm::mat4 view = glm::translate(glm::mat4(1.0f),glm::vec3(-100,0,0));

    Shader shader("resources/shaders/Basic.shader");
    shader.Bind();
    shader.setUniform4f("u_Color", 0.2f, 0.3f, 0.8f, 1.0f);
    

    Texture texture("resources/textures/howdy.png");
    texture.Bind();
    shader.setUniform1i("u_Texture",0);
    
    //Unbinding everything
    va.Unbind();
    vb.Unbind();
    ib.Unbind();
    shader.Unbind();

    Renderer renderer;

    ImGui::CreateContext();
    ImGui_ImplGlfwGL3_Init(window, true);
    ImGui::StyleColorsDark();

    glm::vec3 translation(200, 200, 0);

    float redChannel = 0.0f;
    float increment = 0.05f;
    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplGlfwGL3_NewFrame();

        glm::mat4 model = glm::translate(glm::mat4(1.0f), translation);
        glm::mat4 mvp = proj * view * model;

        //=================Way the we draw things========================
        //binding the shader
        shader.Bind();
        //Setup the uniforms 
        shader.setUniform4f("u_Color", redChannel, 0.3f, 0.8f, 1.0f);
        shader.setUniformMat4f("u_MVP", mvp);
        //Draw call
        renderer.Draw(va,ib,shader);
        GLCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));
        //=================================================================================

        if (redChannel > 1.0f)
            increment -= 0.2f;
        else if (redChannel < 0.0f)
            increment += 0.2f;
        redChannel += increment;

        {
                                 
            ImGui::SliderFloat3("Translation", &translation.x, 0.0f, 960.0f);            
            

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        }

        ImGui::Render();
        ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());

         /* Swap front and back buffers */
        glfwSwapBuffers(window);
        /* Poll for and process events */
        glfwPollEvents();
    }

    ImGui_ImplGlfwGL3_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();
    return 0;
}