#define GLFW_INCLUDE_NONE

// IMGUI
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

// GLAD
#include <glad/glad.h>

// GLFW
#include <GLFW/glfw3.h> // Will drag system OpenGL headers

// STD LIBS
#include <iostream>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 800

#define MAX_SEED_COUNT 128

#define _DEBUG_SHADER

static const char* vertexSource = R"glsl(
    #version 150

	in vec3 position;

    out vec3 fragPos;

	void main()
	{
		gl_Position = vec4(position, 1.0);
        fragPos = vec3(position);
	}
)glsl";

static const char* fragmentSource = R"glsl(
    #version 150

    in vec3 fragPos;

    uniform vec2 u_resolution;

	out vec4 outColor;

	void main()
	{
        vec2 uv = fragPos.xy * 0.5 + 0.5;
        vec2 pixelCoord = uv * u_resolution;
		outColor = vec4(pixelCoord / u_resolution, 0.0, 1.0);
	}
)glsl";

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

GLFWwindow *InitGUI()
{
    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        exit(1);

    // Decide GL+GLSL versions
    // GL 3.0 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Dear ImGui GLFW+OpenGL3 example", NULL, NULL);
    if (window == NULL)
        exit(1);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Using GLAD loader
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize OpenGL context" << std::endl;
        exit(1);
    }

    return window;
}


void CleanupGUI(GLFWwindow *window)
{
    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
}

GLuint CreateShaderProgram()
{
    // Creating program (shaders)
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexSource, NULL);
    glCompileShader(vertexShader);
#ifdef _DEBUG_SHADER
    GLint status;
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &status);
    std::cout << "Vertex shader compile status : " << status << " (1 means success)" << std::endl;
    if(status == GL_FALSE)
    {
        GLint infoLogLength;
        glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &infoLogLength);
        GLchar *strInfoLog = new GLchar[infoLogLength + 1];
        glGetShaderInfoLog(vertexShader, infoLogLength, NULL, strInfoLog);
        std::cout << "Compile failure in vertex shader : " << std::endl;
        std::cout << strInfoLog << std::endl;
        delete[] strInfoLog;
    }
#endif

    // Creating Fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentSource, NULL);
    glCompileShader(fragmentShader);
#ifdef _DEBUG_SHADER
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &status);
    std::cout << "Fragment shader compile status : " << status << " (1 means success)" << std::endl;
    if(status == GL_FALSE)
    {
        GLint infoLogLength;
        glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &infoLogLength);
        GLchar *strInfoLog = new GLchar[infoLogLength + 1];
        glGetShaderInfoLog(fragmentShader, infoLogLength, NULL, strInfoLog);
        std::cout << "Compile failure in fragment shader : " << std::endl;
        std::cout << strInfoLog << std::endl;
        delete[] strInfoLog;
    }
#endif

    // Creating shader program and linking it
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    glUseProgram(program);
    return program;
}

int main(int, char**)
{
    GLFWwindow *window = InitGUI();

    GLuint program = CreateShaderProgram();

    // ----- creating a plane -----
    GLfloat planeVertex[] = {
        -1, 1, 0,
        1, 1, 0,
        -1, -1, 0,
        1, -1, 0
    };
    GLuint planeIndices[] = {
        0, 1, 2,
        1, 2, 3
    };
    GLuint planeVBO, planeVAO, planeEBO;
    glGenVertexArrays(1, &planeVAO);
    glBindVertexArray(planeVAO);

    glGenBuffers(1, &planeVBO);
    glBindBuffer(GL_ARRAY_BUFFER, planeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(planeVertex), planeVertex, GL_STATIC_DRAW);

    GLint positionAttrib = glGetAttribLocation(program, "position");
    glVertexAttribPointer(positionAttrib, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(positionAttrib);

    glGenBuffers(1, &planeEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, planeEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(planeIndices), planeIndices, GL_STATIC_DRAW);

    // Uniforms
    GLint screenSizeUniform = glGetUniformLocation(program, "u_resolution");
    //GLint seedCountUniform = glGetUniformLocation(program, "seedCount");
    //GLint seedsPositionUniform = glGetUniformLocation(program, "seedsPosition");
    //GLint seedsColorUniform = glGetUniformLocation(program, "seedsColor");
    // ----- plane created -----

    // Our state
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    int num_seeds = 0;

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        {
            // IMGUI WINDOW HERE
        }


        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        
        // Clear
        glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw
        glBindVertexArray(planeVAO);
        glUniform2f(screenSizeUniform, display_w, display_h);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // Show
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }


    return 0;
}