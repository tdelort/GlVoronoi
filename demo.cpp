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
#include <string>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 800

#define MAX_SEED_COUNT 128 // Change in shader too

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

    uniform int u_seedCount;
    uniform vec2 u_seedsPosition[128];
    uniform vec3 u_seedsColor[128];
    uniform int u_distanceFunction;

	out vec4 outColor;

    float distanceFunction(vec2 a, vec2 b)
    {
        switch (u_distanceFunction)
        {
            case 0:
                // euclidean
                return distance(a, b);
            case 1:
                // manhattan
                return abs(a.x - b.x) + abs(a.y - b.y);
            case 2:
                // chebyshev
                return max(abs(a.x - b.x), abs(a.y - b.y));
            case 3:
            default:
                return 0.0;
        }
    }

	void main()
	{
        vec2 uv = fragPos.xy * 0.5 + 0.5;
        int seedIndex = 0;
        //bool edge = false;
        float minDistance = distanceFunction(uv, u_seedsPosition[0]);
        for (int i = 0; i < u_seedCount; i++)
        {
            vec2 seedUV = u_seedsPosition[i];
            float newDistance = distanceFunction(uv, seedUV);
            if (newDistance < minDistance)
            {
                seedIndex = i;
                //edge = abs(minDistance - newDistance) < 0.002;
                minDistance = distanceFunction(uv, seedUV);
            }
        }
        //vec3 color = edge ? vec3(1.0) : minDistance < 0.002 ? vec3(1.0) : u_seedsColor[seedIndex];
        vec3 color = minDistance < 0.002 ? vec3(1.0) : u_seedsColor[seedIndex];
        outColor = vec4(color, 1.0);
	}
)glsl";

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

inline float frand(int lo, int hi)
{
    return lo + static_cast <float> (rand()) /( static_cast <float> (RAND_MAX/(hi-lo)));
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
    GLint distanceFunctionUniform = glGetUniformLocation(program, "u_distanceFunction");
    GLint seedCountUniform = glGetUniformLocation(program, "u_seedCount");
    GLint seedsPositionUniform = glGetUniformLocation(program, "u_seedsPosition");
    GLint seedsColorUniform = glGetUniformLocation(program, "u_seedsColor");
    // ----- plane created -----

    // Our state
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    int num_seeds = MAX_SEED_COUNT;

    int distanceFunctionChoice = 0;

    float seeds_position[MAX_SEED_COUNT * 2];
    float seeds_color[MAX_SEED_COUNT * 3];

    // Generate seeds
    std::cout << "Generating seeds..." << std::endl;
    for (int i = 0; i < MAX_SEED_COUNT; i++)
    {
        seeds_position[i * 2] = frand(0, 1);
        seeds_position[i * 2 + 1] = frand(0, 1);
        //std::cout << "[" << i << "] " << seeds_position[i * 2] << " " << seeds_position[i * 2 + 1] << std::endl;

        seeds_color[i * 3] = frand(0, 1);
        seeds_color[i * 3 + 1] = frand(0, 1);
        seeds_color[i * 3 + 2] = frand(0, 1);
        //std::cout << "[" << i << "] " << seeds_color[i * 3] << " " << seeds_color[i * 3 + 1] << " " << seeds_color[i * 3 + 2] << std::endl;
    }

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
            ImGui::Begin("Voronoi");
            ImGui::SliderInt("Distance function", &distanceFunctionChoice, 0, 3);
            if(ImGui::Button("Randomize Seeds"))
            {
                for (int i = 0; i < MAX_SEED_COUNT; i++)
                {
                    seeds_position[i * 2] = frand(0, 1);
                    seeds_position[i * 2 + 1] = frand(0, 1);

                    seeds_color[i * 3] = frand(0, 1);
                    seeds_color[i * 3 + 1] = frand(0, 1);
                    seeds_color[i * 3 + 2] = frand(0, 1);
                }
            }
            ImGui::End();
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
        glUniform1i(distanceFunctionUniform, distanceFunctionChoice);
        glUniform1i(seedCountUniform, num_seeds);
        glUniform2fv(seedsPositionUniform, num_seeds, seeds_position);
        glUniform3fv(seedsColorUniform, num_seeds, seeds_color); 
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // Show
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }


    return 0;
}