

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "shader.h"
#include "basic_camera.h"
#include "camera.h"

#include<iostream>
#include<vector>
#include<cmath>
#include "pointLight.h"
using namespace std;

#define M_PI 3.14159265358979323846



void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
void drawFan(unsigned int VAO, Shader ourShader, glm::mat4 matrix);
void drawFurnace(unsigned int VAO, Shader ourShader, glm::mat4 matrix);
void lightEffect(unsigned int VAO, Shader lightShader, glm::mat4 model, glm::vec3 color);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

// camera
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;


//cylinder
const int SEGMENTS = 36; // Number of segments for the circle
const float RADIUS = 0.5f;
const float HEIGHT = 2.0f;

// Vertices and indices arrays
float lampVertices[(SEGMENTS + 1) * 6 * 2 + 12]; // (Position + Normal) * 2 (top and bottom) + 2 center points
unsigned int lampIndices[SEGMENTS * 12];        // Indices for sides and caps

float eyeX = 5.0f, eyeY = 5.0f, eyeZ = 25.0f;
float lookAtX = 5.25f, lookAtY = 4.0f, lookAtZ = 5.0f;
glm::vec3 V = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 eye = glm::vec3(eyeX, eyeY, eyeZ);
glm::vec3 lookAt = glm::vec3(lookAtX, lookAtY, lookAtZ);
BasicCamera basic_camera(eyeX, eyeY, eyeZ, lookAtX, lookAtY, lookAtZ, V);
Camera camera(glm::vec3(eyeX, eyeY, eyeZ));

// timing
float deltaTime = 0.0f;    // time between current frame and last frame
float lastFrame = 0.0f;

float r = 0.0f;
bool fanOn = false;
float aspectRatio = 4.0f / 3.0f;

bool birdEyeView = false;
glm::vec3 birdEyePosition(5.25f, 10.0f, 10.0f); // Initial position (10 units above)
glm::vec3 birdEyeTarget(5.25f, 0.0f, 6.0f);   // Focus point
float birdEyeSpeed = 1.0f;                   // Speed of movement

float theta = 0.0f; // Angle around the Y-axis
float radius = 2.0f; // Distance from lookAt point

void drawChair(unsigned int VAO, Shader ourShader, glm::mat4 matrix);
void drawTable(unsigned int VAO, Shader ourShader, glm::mat4 matrix);
void drawCylinder(Shader shaderProgram, glm::vec4 color, unsigned int VAO, glm::mat4 parentTrans, float posX = 0.0, float posY = 0.0, float posZ = 0.0, float rotX = 0.0, float rotY = 0.0, float rotZ = 0.0, float scX = 1.0, float scY = 1.0, float scZ = 1.0);


void drawCylinder(unsigned int& VAO_C, Shader& lightingShader, glm::vec3 color, glm::mat4 model, std::vector<unsigned int>& indices);
void drawSphere(unsigned int& VAO_S, Shader& lightingShader, glm::vec3 color, glm::mat4 model, std::vector<unsigned int>& indices);
void generateCylinder(float radius, float height, int segments, std::vector<float>& vertices, std::vector<unsigned int>& indices);
void generateCone(float radius, float height, int segments, std::vector<float>& vertices, std::vector<unsigned int>& indices);
void generateSphere(float radius, int sectorCount, int stackCount, std::vector<float>& vertices, std::vector<unsigned int>& indices);


// positions of the point lights
glm::vec3 pointLightPositions[] = {
    glm::vec3(2.0f,  7.0f,  0.0f),
    glm::vec3(0.0f,  7.0f,  7.0f)
};

PointLight pointlight1(

    pointLightPositions[0].x, pointLightPositions[0].y, pointLightPositions[0].z,  // position
    0.05f, 0.05f, 0.05f,     // ambient
    1.0f, 1.0f, 1.0f,     // diffuse
    1.0f, 1.0f, 1.0f,        // specular
    1.0f,   //k_c
    0.09f,  //k_l
    0.032f, //k_q
    1       // light number
);
PointLight pointlight2(

    pointLightPositions[1].x, pointLightPositions[1].y, pointLightPositions[1].z,  // position
    0.05f, 0.05f, 0.05f,     // ambient
    1.0f, 1.0f, 1.0f,     // diffuse
    1.0f, 1.0f, 1.0f,        // specular
    1.0f,   //k_c
    0.09f,  //k_l
    0.032f, //k_q
    2       // light number
);

// light settings
bool onOffToggle = true;
bool ambientToggle = true;
bool diffuseToggle = true;
bool specularToggle = true;
bool dl = true;
bool spt = true;
bool point1 = true;
bool point2 = true;

glm::vec3 amb(0.2f, 0.2f, 0.2f);
glm::vec3 def(0.8f, 0.8f, 0.8f);
glm::vec3 spec(1.0f, 1.0f, 1.0f);

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "1907056: Assignment 2", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    //glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    // tell GLFW to capture our mouse
    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile our shader zprogram
    // ------------------------------------
    Shader ourShader("vertexShader.vs", "fragmentShader.fs");

    Shader constantShader("vertexShader.vs", "fragmentShaderV2.fs");

    Shader lightingShader("vertexShaderForGouraudShading.vs", "fragmentShaderForGouraudShading.fs");


    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    /*float cube_vertices[] = {
        0.0f, 0.0f, 0.0f, 0.647f, 0.408f, 0.294f,
        1.0f, 0.0f, 0.0f, 0.647f, 0.408f, 0.294f,
        1.0f, 1.0f, 0.0f, 0.647f, 0.408f, 0.294f,
        0.0f, 1.0f, 0.0f, 0.647f, 0.408f, 0.294f,

        0.0f, 0.0f, 1.0f, 0.647f, 0.408f, 0.294f,
        1.0f, 0.0f, 1.0f, 0.647f, 0.408f, 0.294f,
        1.0f, 1.0f, 1.0f, 0.647f, 0.408f, 0.294f,
        0.0f, 1.0f, 1.0f, 0.647f, 0.408f, 0.294f,

    };

    unsigned int cube_indices[] = {
        1, 2, 3,
        3, 0, 1,

        5, 6, 7,
        7, 4, 5,

        4, 7, 3,
        3, 0, 4,

        5, 6, 2,
        2, 1, 5,

        5, 1, 0,
        0, 4, 5,

        6, 2, 3,
        3, 7, 6,
    };*/


    float cube_vertices[] = {
        // positions      // normals
        0.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f,
        1.0f, 0.0f, 0.0f, 0.0f, 0.0f, -1.0f,
        1.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f,
        0.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f,

        1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f,

        0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,

        0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,

        1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,
        1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f,

        0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f,
        1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f,
        1.0f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f, -1.0f, 0.0f
    };
    unsigned int cube_indices[] = {
        0, 3, 2,
        2, 1, 0,

        4, 5, 7,
        7, 6, 4,

        8, 9, 10,
        10, 11, 8,

        12, 13, 14,
        14, 15, 12,

        16, 17, 18,
        18, 19, 16,

        20, 21, 22,
        22, 23, 20
    };


    //float cylinder_vertices[] = {
    //    // Top circle vertices (radius = 0.5, y = 0.5)
    //    0.0f,  0.5f,  0.0f, 1.0f, 0.0f, 0.0f, // Center of top circle (red)

    //    // Top circle vertices (more vertices for roundness)
    //    0.5f,  0.5f,  0.0f, 1.0f, 0.0f, 0.0f, // Right edge of top circle (red)
    //    0.353f,  0.5f,  0.353f, 1.0f, 0.0f, 0.0f, // Top-right edge (red)
    //    0.0f,  0.5f,  0.5f, 1.0f, 0.0f, 0.0f, // Top-center (red)
    //    -0.353f,  0.5f,  0.353f, 1.0f, 0.0f, 0.0f, // Left-top edge (red)
    //    -0.5f,  0.5f,  0.0f, 1.0f, 0.0f, 0.0f, // Left edge of top circle (red)

    //    // More vertices for top circle to make it rounder
    //    -0.353f,  0.5f,  -0.353f, 1.0f, 0.0f, 0.0f, // Left-bottom edge (red)
    //    0.0f,  0.5f,  -0.5f, 1.0f, 0.0f, 0.0f, // Bottom-center (red)
    //    0.353f,  0.5f,  -0.353f, 1.0f, 0.0f, 0.0f, // Right-bottom edge (red)

    //    // Bottom circle vertices (radius = 0.5, y = -0.5)
    //    0.0f, -0.5f,  0.0f, 0.0f, 1.0f, 0.0f, // Center of bottom circle (green)

    //    // Bottom circle vertices (more vertices for roundness)
    //    0.5f, -0.5f,  0.0f, 0.0f, 1.0f, 0.0f, // Right edge of bottom circle (green)
    //    0.353f, -0.5f,  0.353f, 0.0f, 1.0f, 0.0f, // Bottom-right edge (green)
    //    0.0f, -0.5f,  0.5f, 0.0f, 1.0f, 0.0f, // Bottom-center (green)
    //    -0.353f, -0.5f,  0.353f, 0.0f, 1.0f, 0.0f, // Left-bottom edge (green)
    //    -0.5f, -0.5f,  0.0f, 0.0f, 1.0f, 0.0f, // Left edge of bottom circle (green)

    //    // More vertices for bottom circle to make it rounder
    //    -0.353f, -0.5f, -0.353f, 0.0f, 1.0f, 0.0f, // Left-top edge (green)
    //    0.0f, -0.5f, -0.5f, 0.0f, 1.0f, 0.0f, // Bottom-center (green)
    //    0.353f, -0.5f, -0.353f, 0.0f, 1.0f, 0.0f  // Right-bottom edge (green)
    //};


    //unsigned int cylinder_indices[] = {
    //    // Top circle
    //    0,  1,  2,
    //    0,  2,  3,
    //    0,  3,  4,
    //    0,  4,  5,
    //    0,  5,  6,
    //    0,  6,  7,
    //    0,  7,  8,
    //    0,  8,  1,

    //    // Bottom circle
    //    9,  10,  11,
    //    9,  11,  12,
    //    9,  12,  13,
    //    9,  13,  14,
    //    9,  14,  15,
    //    9,  15,  16,
    //    9,  16,  17,
    //    9,  17,  10,

    //    // Side faces
    //    1,  10,  2,
    //    2,  10,  11,
    //    2,  11,  3,
    //    3,  11,  12,
    //    3,  12,  4,
    //    4,  12,  13,
    //    4,  13,  5,
    //    5,  13,  14,
    //    5,  14,  6,
    //    6,  14,  15,
    //    6,  15,  7,
    //    7,  15,  16,
    //    7,  16,  8,
    //    8,  16,  17,
    //    8,  17,  1
    //};


    // Generate cylinder vertices and indices
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    generateCylinder(0.25f, 0.5f, 36, vertices, indices);

    // Generate cone data
    std::vector<float> vertices_k;
    std::vector<unsigned int> indices_k;
    generateCone(1.0f, 2.0f, 36, vertices_k, indices_k);

    // Generate sphere data
    std::vector<float> vertices_s;
    std::vector<unsigned int> indices_s;
    generateSphere(1.0f, 36, 18, vertices_s, indices_s);

    // Create VAO_S, VBO_S, and EBO_S
    unsigned int VAO_S, VBO_S, EBO_S;
    glGenVertexArrays(1, &VAO_S);
    glGenBuffers(1, &VBO_S);
    glGenBuffers(1, &EBO_S);

    // Bind VAO
    glBindVertexArray(VAO_S);

    // Bind and set VBO
    glBindBuffer(GL_ARRAY_BUFFER, VBO_S);
    glBufferData(GL_ARRAY_BUFFER, vertices_s.size() * sizeof(float), vertices_s.data(), GL_STATIC_DRAW);

    // Bind and set EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_S);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_s.size() * sizeof(unsigned int), indices_s.data(), GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // Unbind VAO
    glBindVertexArray(0);


    // Create VAO, VBO, and EBO
    unsigned int VAO_K, VBO_K, EBO_K;
    glGenVertexArrays(1, &VAO_K);
    glGenBuffers(1, &VBO_K);
    glGenBuffers(1, &EBO_K);

    // Bind VAO
    glBindVertexArray(VAO_K);

    // Bind and set VBO
    glBindBuffer(GL_ARRAY_BUFFER, VBO_K);
    glBufferData(GL_ARRAY_BUFFER, vertices_k.size() * sizeof(float), vertices_k.data(), GL_STATIC_DRAW);

    // Bind and set EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_K);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_k.size() * sizeof(unsigned int), indices_k.data(), GL_STATIC_DRAW);



    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    //normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)12);
    glEnableVertexAttribArray(1);


    // Unbind VAO
    glBindVertexArray(0);



    unsigned int VBO_C, VAO_C, EBO_C;
    glGenVertexArrays(1, &VAO_C);
    glGenBuffers(1, &VBO_C);
    glGenBuffers(1, &EBO_C);

    glBindVertexArray(VAO_C);

    glBindBuffer(GL_ARRAY_BUFFER, VBO_C);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO_C);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)12);
    glEnableVertexAttribArray(1);

    glBindVertexArray(0); // Unbind VAO
    






    //for cube
    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(cube_indices), cube_indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    ////color attribute
    //glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)12);
    //glEnableVertexAttribArray(1);

    // vertex normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)12);
    glEnableVertexAttribArray(1);


    // second, configure the light's VAO (VBO stays the same; the vertices are the same for the light object which is also a 3D cube)
    unsigned int lightCubeVAO;
    glGenVertexArrays(1, &lightCubeVAO);
    glBindVertexArray(lightCubeVAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    // note that we update the lamp's position attribute's stride to reflect the updated buffer data
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);


    r = 0.0f;

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        // --------------------
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        //ourShader.use();
        


        // be sure to activate shader when setting uniforms/drawing objects
        lightingShader.use();
        lightingShader.setVec3("viewPos", camera.Position);

        // point light 1
        pointlight1.setUpPointLight(lightingShader);
        // point light 2
        pointlight2.setUpPointLight(lightingShader);

        lightingShader.setVec3("diectionalLight.directiaon", 0.0f, -4.0f, 0.0f);
        lightingShader.setVec3("diectionalLight.ambient", 0.2, 0.2, 0.2);
        lightingShader.setVec3("diectionalLight.diffuse", 0.8f, 0.8f, 0.8f);
        lightingShader.setVec3("diectionalLight.specular", 1.0f, 1.0f, 1.0f);

        lightingShader.setBool("dlighton", dl);

        lightingShader.setVec3("spotlight.position", 4.0, 7, 5);
        lightingShader.setVec3("spotlight.direction", 0, -1, 0);
        lightingShader.setVec3("spotlight.ambient", amb);
        lightingShader.setVec3("spotlight.diffuse", def);
        lightingShader.setVec3("spotlight.specular", spec);
        lightingShader.setFloat("spotlight.k_c", 1.0f);
        lightingShader.setFloat("spotlight.k_l", 0.09);
        lightingShader.setFloat("spotlight.k_q", 0.032);
        lightingShader.setFloat("cos_theta", glm::cos(glm::radians(5.5f)));
        lightingShader.setBool("spotlighton", spt);


        lightingShader.use();

        // pass projection matrix to shader (note that in this case it could change every frame)
        //glm::mat4 projection = glm::perspective(glm::radians(basic_camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        //glm::mat4 projection = glm::ortho(-2.0f, +2.0f, -1.5f, +1.5f, 0.1f, 100.0f);
        //ourShader.setMat4("projection", projection);
        lightingShader.setMat4("projection", projection);
        //constantShader.setMat4("projection", projection);

        // camera/view transformation
        //glm::mat4 view = basic_camera.createViewMatrix();
        //ourShader.setMat4("view", view);
        //constantShader.setMat4("view", view);

        glm::mat4 view;

        if (birdEyeView) {
            // Set camera position directly above the scene
            glm::vec3 up(0.0f, 1.0f, 0.0f); // Ensure the up vector points backward
            view = glm::lookAt(birdEyePosition, birdEyeTarget, up);
        }
        else {
            view = camera.GetViewMatrix();
        }

        //ourShader.setMat4("view", view);

        lightingShader.setMat4("view", view);
        // Modelling Transformation
        //glm::mat4 identityMatrix = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
        //drawCube(ourShader, VAO, identityMatrix, translate_X, translate_Y, translate_Z, rotateAngle_X, rotateAngle_Y, rotateAngle_Z, scale_X, scale_Y, scale_Z);
        /*
        glm::mat4 identityMatrix = glm::mat4(1.0f);
        glm::mat4 translateMatrix, rotateXMatrix, rotateYMatrix, rotateZMatrix, scaleMatrix, model;
        translateMatrix = glm::translate(identityMatrix, glm::vec3(translate_X, translate_Y, translate_Z));
        rotateXMatrix = glm::rotate(identityMatrix, glm::radians(rotateAngle_X), glm::vec3(1.0f, 0.0f, 0.0f));
        rotateYMatrix = glm::rotate(identityMatrix, glm::radians(rotateAngle_Y), glm::vec3(0.0f, 1.0f, 0.0f));
        rotateZMatrix = glm::rotate(identityMatrix, glm::radians(rotateAngle_Z), glm::vec3(0.0f, 0.0f, 1.0f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(scale_X, scale_Y, scale_Z));
        model = identityMatrix;
        */
        //model = glm::scale(rotateZMatrix, glm::vec3(scX, scY, scZ));
        //modelCentered = glm::translate(model, glm::vec3(-0.25, -0.25, -0.25));

        //shaderProgram.setMat4("model", modelCentered);

        glm::mat4 identityMatrix = glm::mat4(1.0f);
        glm::mat4 translateMatrix, rotateXMatrix, rotateYMatrix, rotateZMatrix, scaleMatrix, model, RotateTranslateMatrix, InvRotateTranslateMatrix;


        //draw a cylinder
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(1.3f, 1.3f, 1.3f));
        translateMatrix = glm::translate(identityMatrix, glm::vec3(1.4f, 2.81f, 0.4f));
        model = translateMatrix * scaleMatrix;
        drawCylinder(VAO_C, lightingShader, glm::vec3(1.0f, 0.85f, 0.3f), model, indices);

        // draw a cone on top of the cylinder
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.80f, 0.80f, 0.80f));
        translateMatrix = glm::translate(identityMatrix, glm::vec3(4.0f, 7.0f, 0.0f));
        //rotate the cone in z axis
        rotateYMatrix = glm::rotate(identityMatrix, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        model = translateMatrix * rotateYMatrix * scaleMatrix;
        drawCylinder(VAO_K, lightingShader, glm::vec3(1.0f, 0.0f, 0.0f), model, indices);

        // Draw a sphere next to the sofa
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(1.5f, 1.5f, 1.5f));
        translateMatrix = glm::translate(identityMatrix, glm::vec3(6.0f, 3.65f, 5.0f)); // Adjust the offset as needed
        model = translateMatrix * scaleMatrix;
        drawSphere(VAO_S, lightingShader, glm::vec3(1.0f, 0.0f, 0.0f), model, indices_s);



        //first wall section 1
        translateMatrix = glm::translate(identityMatrix, glm::vec3(0.0f, 0.0f, 0.0f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(-0.1f, 7.2f, 10.0f));
        model = translateMatrix * scaleMatrix;
        lightEffect(VAO, lightingShader, model, glm::vec3(1.0, 0.98f, 0.97f));
        /*ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(1.0, 0.98f, 0.97f, 1.0f));
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);*/

        //first wall section 2
        translateMatrix = glm::translate(identityMatrix, glm::vec3(0.0f, 7.2f, 0.0f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(-0.1f, 2.8f, 10.0f));
        model = translateMatrix * scaleMatrix;
        lightEffect(VAO, lightingShader, model, glm::vec3(0.88, 0.75f, 0.64f));
        /*ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(0.88, 0.75f, 0.64f, 1.0f));
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);*/

        //second wall section 1
        translateMatrix = glm::translate(identityMatrix, glm::vec3(0.0f, 0.0f, 0.0f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(10.5f, 7.2f, -0.1f));
        model = translateMatrix * scaleMatrix;
        ourShader.setMat4("model", model);
        lightEffect(VAO, lightingShader, model, glm::vec3(1.0, 0.98f, 0.97f));
        /*ourShader.setVec4("color", glm::vec4(1.0, 0.98f, 0.97f, 1.0f));
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);*/

        //second wall section 2
        translateMatrix = glm::translate(identityMatrix, glm::vec3(0.0f, 7.2f, 0.0f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(10.5f, 2.8f, -0.1f));
        model = translateMatrix * scaleMatrix;
        lightEffect(VAO, lightingShader, model, glm::vec3(0.88, 0.75f, 0.64f));
        /*ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(0.88, 0.75f, 0.64f, 1.0f));
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);*/

        //roof
        translateMatrix = glm::translate(identityMatrix, glm::vec3(-0.1f, 10.0f, -0.1f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(10.5f, 0.1f, 10.0f));
        model = translateMatrix * scaleMatrix;
        lightEffect(VAO, lightingShader, model, glm::vec3(0.678, 0.678, 0.788));
        /*ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(0.678, 0.678, 0.788, 1.0f));
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);*/


        //floor
        translateMatrix = glm::translate(identityMatrix, glm::vec3(-0.1f, 0.0f, -0.1f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(10.5f, -0.1f, 10.0f));
        model = translateMatrix * scaleMatrix;
        lightEffect(VAO, lightingShader, model, glm::vec3(0.85, 0.65f, 0.56f));
        /*ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(0.85, 0.65f, 0.56f, 1.0f));
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);*/

        //upper box 1
        translateMatrix = glm::translate(identityMatrix, glm::vec3(0.0f, 3.7f, 0.0f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.5f, 2.0f, 2.0f));
        model = translateMatrix * scaleMatrix;
        lightEffect(VAO, lightingShader, model, glm::vec3(0.97, 0.48f, 0.21f));
        /*ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(0.97, 0.48f, 0.21f, 1.0f));
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);*/

        //upper boxes
        float z = 2.01f;
        float z_thin = 2.0f;
        for (int i = 0; i < 5; i++) {
            //thin separator
            translateMatrix = glm::translate(identityMatrix, glm::vec3(0.51f, 3.7f, z_thin));
            scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.01f, 2.0f, 0.02f));
            model = translateMatrix * scaleMatrix;
            lightEffect(VAO, lightingShader, model, glm::vec3(0.0, 0.0f, 0.0f));
            /*ourShader.setMat4("model", model);
            ourShader.setVec4("color", glm::vec4(0.0, 0.0f, 0.0f, 1.0f));
            glBindVertexArray(VAO);
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);*/
            z_thin += 1.0f;


            translateMatrix = glm::translate(identityMatrix, glm::vec3(0.0f, 3.7f, z));
            scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.5f, 2.0f, 0.98f));
            model = translateMatrix * scaleMatrix;
            lightEffect(VAO, lightingShader, model, glm::vec3(0.97, 0.48f, 0.21f));
            /*ourShader.setMat4("model", model);
            ourShader.setVec4("color", glm::vec4(0.97, 0.48f, 0.21f, 1.0f));
            glBindVertexArray(VAO);
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);*/
            z += 1.0f;
        }
        
        
        //white surface 1
        translateMatrix = glm::translate(identityMatrix, glm::vec3(0.0f, 2.0f, 0.0f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(1.0f, 0.2f, 10.0f));
        model = translateMatrix * scaleMatrix;
        lightEffect(VAO, lightingShader, model, glm::vec3(1.0, 0.98f, 0.97f));
        /*ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(1.0, 0.98f, 0.97f, 1.0f));
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);*/


        //lamp base
        translateMatrix = glm::translate(identityMatrix, glm::vec3(1.30f, 2.2f, 0.30f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.2f, 0.1f, 0.2f));
        model = translateMatrix * scaleMatrix;
        lightEffect(VAO, lightingShader, model, glm::vec3(0.0, 0.0f, 0.0f));
        /*ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(0.0, 0.0f, 0.0f, 1.0f));
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);*/


        //lamp stick
        translateMatrix = glm::translate(identityMatrix, glm::vec3(1.35f, 2.21f, 0.35f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.1f, 0.6f, 0.1f));
        model = translateMatrix * scaleMatrix;
        lightEffect(VAO, lightingShader, model, glm::vec3(0.0, 0.0f, 0.0f));
        /*ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(0.0, 0.0f, 0.0f, 1.0f));
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);*/
        
        //lamp
        //drawCylinder(ourShader, glm::vec4(1.0f, 1.0f, 0.0f, 1.0f), cylinder_VAO, identityMatrix, 1.5f, 2.81f, 0.5f, 0, 0, 0, 0.4f, 0.5f, 0.4f);

        //oven structure
        translateMatrix = glm::translate(identityMatrix, glm::vec3(0.0f, 2.2f, 3.0f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.8f, 0.8f, 2.0f));
        model = translateMatrix * scaleMatrix;
        lightEffect(VAO, lightingShader, model, glm::vec3(0.52, 0.45f, 0.39f));
        /*ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(0.52, 0.45f, 0.39f, 1.0f));
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);*/

        //oven black left
        translateMatrix = glm::translate(identityMatrix, glm::vec3(0.81f, 2.25f, 3.5f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.01f, 0.7f, 1.4f));
        model = translateMatrix * scaleMatrix;
        lightEffect(VAO, lightingShader, model, glm::vec3(0.047, 0.031f, 0.031f));
        /*ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(0.047, 0.031f, 0.031f, 1.0f));
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);*/

        //oven screen
        translateMatrix = glm::translate(identityMatrix, glm::vec3(0.81f, 2.75f, 3.10f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.01f, 0.15f, 0.3f));
        model = translateMatrix * scaleMatrix;
        lightEffect(VAO, lightingShader, model, glm::vec3(0.047, 0.031f, 0.031f));
        /*ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(0.047, 0.031f, 0.031f, 1.0f));
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);*/

        //oven keypad
        translateMatrix = glm::translate(identityMatrix, glm::vec3(0.81f, 2.40f, 3.10f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.01f, 0.3f, 0.3f));
        model = translateMatrix * scaleMatrix;
        lightEffect(VAO, lightingShader, model, glm::vec3(0.87, 0.79f, 0.72f));
        /*ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(0.87, 0.79f, 0.72f, 1.0f));
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);*/

        //electric stove
        translateMatrix = glm::translate(identityMatrix, glm::vec3(0.1f, 2.21f, 6.0f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.9f, 0.01f, 0.8f));
        model = translateMatrix * scaleMatrix;
        lightEffect(VAO, lightingShader, model, glm::vec3(0.047, 0.031f, 0.031f));
        /*ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(0.047, 0.031f, 0.031f, 1.0f));
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);*/

        //stove furnace 1
        translateMatrix = glm::translate(identityMatrix, glm::vec3(0.18f, 2.25f, 6.05f));
        drawFurnace(VAO, lightingShader, translateMatrix);

        //stove furnace 2
        translateMatrix = glm::translate(identityMatrix, glm::vec3(0.58f, 2.25f, 6.05f));
        drawFurnace(VAO, lightingShader, translateMatrix);

        //stove furnace 3
        translateMatrix = glm::translate(identityMatrix, glm::vec3(0.18f, 2.25f, 6.45f));
        drawFurnace(VAO, lightingShader, translateMatrix);

        //stove furnace 4
        translateMatrix = glm::translate(identityMatrix, glm::vec3(0.58f, 2.25f, 6.45f));
        drawFurnace(VAO, lightingShader, translateMatrix);


        /*translateMatrix = glm::translate(identityMatrix, glm::vec3(0.15f, 2.25f, 6.05f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.35f, 0.01f, 0.35f));
        model = translateMatrix * scaleMatrix;
        ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(1.0, 1.0f, 1.0f, 1.0f));

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);*/

       


        //white surface 2
        translateMatrix = glm::translate(identityMatrix, glm::vec3(0.0f, 2.0f, 0.0f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(7.0f, 0.2f, 1.0f));
        model = translateMatrix * scaleMatrix;
        lightEffect(VAO, lightingShader, model, glm::vec3(1.0, 0.98f, 0.97f));
        /*ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(1.0, 0.98f, 0.97f, 1.0f));
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);*/


        //lower box 1
        translateMatrix = glm::translate(identityMatrix, glm::vec3(0.0f, 0.0f, 1.0f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(1.0f, 2.0f, 1.0f));
        model = translateMatrix * scaleMatrix;
        lightEffect(VAO, lightingShader, model, glm::vec3(0.97, 0.48f, 0.21f));
        /*ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(0.97, 0.48f, 0.21f, 1.0f));
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);*/


        //lower boxes (2-4)
        float z_lower = 2.01f;
        float z_thin_lower = 2.0f;
        for (int i = 0; i < 3; i++) {
            //thin separator
            translateMatrix = glm::translate(identityMatrix, glm::vec3(1.01f, 0.0f, z_thin_lower));
            scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.01f, 2.0f, 0.02f));
            model = translateMatrix * scaleMatrix;
            lightEffect(VAO, lightingShader, model, glm::vec3(0.0, 0.0f, 0.0f));
            /*ourShader.setMat4("model", model);
            ourShader.setVec4("color", glm::vec4(0.0, 0.0f, 0.0f, 1.0f));
            glBindVertexArray(VAO);
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);*/
            z_thin_lower += 1.0f;


            translateMatrix = glm::translate(identityMatrix, glm::vec3(0.0f, 0.0f, z_lower));
            scaleMatrix = glm::scale(identityMatrix, glm::vec3(1.0f, 2.0f, 0.98f));
            model = translateMatrix * scaleMatrix;
            lightEffect(VAO, lightingShader, model, glm::vec3(0.97, 0.48f, 0.21f));
            /*ourShader.setMat4("model", model);
            ourShader.setVec4("color", glm::vec4(0.97, 0.48f, 0.21f, 1.0f));
            glBindVertexArray(VAO);
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);*/
            z_lower += 1.0f;
        }

        
        //store bottom
        translateMatrix = glm::translate(identityMatrix, glm::vec3(0.0f, 0.0f, 5.0f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(1.0f, 0.4f, 2.0f));
        model = translateMatrix * scaleMatrix;
        lightEffect(VAO, lightingShader, model, glm::vec3(0.64, 0.58f, 0.49f));
        /*ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(0.64, 0.58f, 0.49f, 1.0f));
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);*/

        //store top
        translateMatrix = glm::translate(identityMatrix, glm::vec3(0.0f, 1.6f, 5.0f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(1.0f, 0.4f, 2.0f));
        model = translateMatrix * scaleMatrix;
        lightEffect(VAO, lightingShader, model, glm::vec3(0.64, 0.58f, 0.49f));
        /*ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(0.64, 0.58f, 0.49f, 1.0f));
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);*/

        //store mid
        translateMatrix = glm::translate(identityMatrix, glm::vec3(0.0f, 0.4f, 5.0f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(1.0f, 1.2f, 2.0f));
        model = translateMatrix * scaleMatrix;
        lightEffect(VAO, lightingShader, model, glm::vec3(0.0, 0.0f, 0.0f));
        /*ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(0.0, 0.0f, 0.0f, 1.0f));
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);*/

        //store handle
        translateMatrix = glm::translate(identityMatrix, glm::vec3(1.01f, 1.2f, 5.2f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.01f, 0.05f, 1.65f));
        model = translateMatrix * scaleMatrix;
        lightEffect(VAO, lightingShader, model, glm::vec3(1.0, 1.0f, 1.0f));
        /*ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(1.0, 1.0f, 1.0f, 1.0f));
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);*/


        //lower boxes (5-7)
        float z_lower_left = 7.02f;
        float z_thin_lower_left = 7.0f;
        for (int i = 0; i < 3; i++) {
            //thin separator
            translateMatrix = glm::translate(identityMatrix, glm::vec3(1.01f, 0.0f, z_thin_lower_left));
            scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.01f, 2.0f, 0.02f));
            model = translateMatrix * scaleMatrix;
            lightEffect(VAO, lightingShader, model, glm::vec3(0.0, 0.0f, 0.0f));
            /*ourShader.setMat4("model", model);
            ourShader.setVec4("color", glm::vec4(0.0, 0.0f, 0.0f, 1.0f));
            glBindVertexArray(VAO);
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);*/
            z_thin_lower_left += 1.0f;


            translateMatrix = glm::translate(identityMatrix, glm::vec3(0.0f, 0.0f, z_lower_left));
            scaleMatrix = glm::scale(identityMatrix, glm::vec3(1.0f, 2.0f, 0.98f));
            model = translateMatrix * scaleMatrix;
            lightEffect(VAO, lightingShader, model, glm::vec3(0.97, 0.48f, 0.21f));
            /*ourShader.setMat4("model", model);
            ourShader.setVec4("color", glm::vec4(0.97, 0.48f, 0.21f, 1.0f));
            glBindVertexArray(VAO);
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);*/
            z_lower_left += 1.0f;
        }



        //lower boxes right (1-6)
        float x_lower_right = 1.12f;
        float x_thin_lower_right = 1.1f;
        for (int i = 0; i < 6; i++) {
            //thin separator
            translateMatrix = glm::translate(identityMatrix, glm::vec3(x_thin_lower_right, 0.0f,1.01f));
            scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.02f, 2.0f, 0.01f));
            model = translateMatrix * scaleMatrix;
            lightEffect(VAO, lightingShader, model, glm::vec3(0.0, 0.0f, 0.0f));
            /*ourShader.setMat4("model", model);
            ourShader.setVec4("color", glm::vec4(0.0, 0.0f, 0.0f, 1.0f));
            glBindVertexArray(VAO);
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);*/
            x_thin_lower_right += 1.0f;


            translateMatrix = glm::translate(identityMatrix, glm::vec3(x_lower_right, 0.0f, 0.0f));
            scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.98f, 2.0f, 1.0f));
            model = translateMatrix * scaleMatrix;
            lightEffect(VAO, lightingShader, model, glm::vec3(0.97, 0.48f, 0.21f));
            /*ourShader.setMat4("model", model);
            ourShader.setVec4("color", glm::vec4(0.97, 0.48f, 0.21f, 1.0f));
            glBindVertexArray(VAO);
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);*/
            x_lower_right += 1.0f;
        }

        //upper box right 1
        translateMatrix = glm::translate(identityMatrix, glm::vec3(0.0f, 3.7f, 0.0f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(2.0f, 2.0f, 1.0f));
        model = translateMatrix * scaleMatrix;
        lightEffect(VAO, lightingShader, model, glm::vec3(0.97, 0.48f, 0.21f));
        /*ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(0.97, 0.48f, 0.21f, 1.0f));
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);*/


        //upper box right 1 white front
        translateMatrix = glm::translate(identityMatrix, glm::vec3(0.9f, 3.9f, 1.02f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.9f, 1.6f, 0.02f));
        model = translateMatrix * scaleMatrix;
        lightEffect(VAO, lightingShader, model, glm::vec3(0.85, 0.65f, 0.56f));
        /*ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(0.85, 0.65f, 0.56f, 1.0f));
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);*/



        //window mid left
        translateMatrix = glm::translate(identityMatrix, glm::vec3(3.0f, 3.7f, 0.0f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.15f, 2.0f, 0.2f));
        model = translateMatrix * scaleMatrix;
        lightEffect(VAO, lightingShader, model, glm::vec3(0.97, 0.48f, 0.21f));
        /*ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(0.97, 0.48f, 0.21f, 1.0f));
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);*/

        //window mid 
        translateMatrix = glm::translate(identityMatrix, glm::vec3(4.5f, 3.7f, 0.0f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.15f, 2.0f, 0.2f));
        model = translateMatrix * scaleMatrix;
        lightEffect(VAO, lightingShader, model, glm::vec3(0.97, 0.48f, 0.21f));
        /*ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(0.97, 0.48f, 0.21f, 1.0f));
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);*/


        //window mid right
        translateMatrix = glm::translate(identityMatrix, glm::vec3(6.05f, 3.7f, 0.0f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.15f, 2.0f, 0.2f));
        model = translateMatrix * scaleMatrix;
        lightEffect(VAO, lightingShader, model, glm::vec3(0.97, 0.48f, 0.21f));
        /*ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(0.97, 0.48f, 0.21f, 1.0f));
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);*/

        //window top left 
        translateMatrix = glm::translate(identityMatrix, glm::vec3(3.0f, 5.7f, 0.0f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(1.7f, 0.15f, 0.2f));
        model = translateMatrix * scaleMatrix;
        lightEffect(VAO, lightingShader, model, glm::vec3(0.97, 0.48f, 0.21f));
        /*ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(0.97, 0.48f, 0.21f, 1.0f));
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);*/

        //window top right 
        translateMatrix = glm::translate(identityMatrix, glm::vec3(4.5f, 5.7f, 0.0f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(1.7f, 0.15f, 0.2f));
        model = translateMatrix * scaleMatrix;
        lightEffect(VAO, lightingShader, model, glm::vec3(0.97, 0.48f, 0.21f));
        /*ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(0.97, 0.48f, 0.21f, 1.0f));
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);*/

        //window bottom left 
        translateMatrix = glm::translate(identityMatrix, glm::vec3(3.0f, 3.7f, 0.0f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(1.7f, 0.15f, 0.2f));
        model = translateMatrix * scaleMatrix;
        lightEffect(VAO, lightingShader, model, glm::vec3(0.97, 0.48f, 0.21f));
        /*ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(0.97, 0.48f, 0.21f, 1.0f));
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);*/

        //window bottom right 
        translateMatrix = glm::translate(identityMatrix, glm::vec3(4.5f, 3.7f, 0.0f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(1.7f, 0.15f, 0.2f));
        model = translateMatrix * scaleMatrix;
        lightEffect(VAO, lightingShader, model, glm::vec3(0.97, 0.48f, 0.21f));
        /*ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(0.97, 0.48f, 0.21f, 1.0f));
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);*/


        //fridge back
        translateMatrix = glm::translate(identityMatrix, glm::vec3(7.1f, 0.0f, 0.0f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(3.0f, 5.0f, 1.0f));
        model = translateMatrix * scaleMatrix;
        lightEffect(VAO, lightingShader, model, glm::vec3(0.65, 0.57f, 0.47f));
        /*ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(0.65, 0.57f, 0.47f, 1.0f));
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);*/


        //fridge front left
        translateMatrix = glm::translate(identityMatrix, glm::vec3(7.1f, 0.0f, 1.0f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(1.2f, 5.0f, 0.2f));
        model = translateMatrix * scaleMatrix;
        lightEffect(VAO, lightingShader, model, glm::vec3(0.65, 0.57f, 0.47f));
        /*ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(0.65, 0.57f, 0.47f, 1.0f));
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);*/

        //fridge separator line
        translateMatrix = glm::translate(identityMatrix, glm::vec3(8.3f, 0.0f, 1.0f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.02f, 5.0f, 0.2f));
        model = translateMatrix * scaleMatrix;
        lightEffect(VAO, lightingShader, model, glm::vec3(0.0, 0.0f, 0.0f));
        /*ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(0.0, 0.0f, 0.0f, 1.0f));
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);*/

        //fridge front right
        translateMatrix = glm::translate(identityMatrix, glm::vec3(8.32f, 0.0f, 1.0f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(1.78f, 5.0f, 0.2f));
        model = translateMatrix * scaleMatrix;
        lightEffect(VAO, lightingShader, model, glm::vec3(0.65, 0.57f, 0.47f));
        /*ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(0.65, 0.57f, 0.47f, 1.0f));
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);*/

        //fridge left handle
        translateMatrix = glm::translate(identityMatrix, glm::vec3(8.0f, 1.9f, 1.21f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.1f, 2.5f, 0.1f));
        model = translateMatrix * scaleMatrix;
        lightEffect(VAO, lightingShader, model, glm::vec3(0.0, 0.0f, 0.0f));
        /*ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(0.0, 0.0f, 0.0f, 1.0f));
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);*/

        //fridge right handle
        translateMatrix = glm::translate(identityMatrix, glm::vec3(8.5f, 1.9f, 1.21f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.1f, 2.5f, 0.1f));
        model = translateMatrix * scaleMatrix;
        lightEffect(VAO, lightingShader, model, glm::vec3(0.0, 0.0f, 0.0f));
        /*ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(0.0, 0.0f, 0.0f, 1.0f));
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);*/

        //fridge password
        translateMatrix = glm::translate(identityMatrix, glm::vec3(9.0f, 2.5f, 1.21f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.6f, 0.4f, 0.01f));
        model = translateMatrix * scaleMatrix;
        lightEffect(VAO, lightingShader, model, glm::vec3(0.87, 0.79f, 0.72f));
        /*ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(0.87, 0.79f, 0.72f, 1.0f));
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);*/

        //sink
        translateMatrix = glm::translate(identityMatrix, glm::vec3(2.5f, 2.21f, 0.05f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(2.0f, 0.01f, 0.9f));
        model = translateMatrix * scaleMatrix;
        lightEffect(VAO, lightingShader, model, glm::vec3(0.0, 0.0f, 0.0f));
        /*ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(0.0, 0.0f, 0.0f, 1.0f));
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);*/

        //sink lines
        float sink_z = 0.1f;
        for (int i = 0; i < 10; i++) {
            translateMatrix = glm::translate(identityMatrix, glm::vec3(2.55f, 2.22f, sink_z));
            scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.8f, 0.01f, 0.02f));
            model = translateMatrix * scaleMatrix;
            lightEffect(VAO, lightingShader, model, glm::vec3(1.0, 1.0f, 1.0f));
            /*ourShader.setMat4("model", model);
            ourShader.setVec4("color", glm::vec4(1.0, 1.0f, 1.0f, 1.0f));
            glBindVertexArray(VAO);
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);*/
            sink_z += 0.1;
        }
        


        

        //tap 1
        translateMatrix = glm::translate(identityMatrix, glm::vec3(4.0f, 2.21f, 0.2f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.05f, 1.0f, 0.05f));
        model = translateMatrix * scaleMatrix;
        lightEffect(VAO, lightingShader, model, glm::vec3(0.5, 0.5f, 0.5f));
        /*ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(0.5, 0.5f, 0.5f, 1.0f));
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);*/

        //tap 2
        translateMatrix = glm::translate(identityMatrix, glm::vec3(4.0f, 3.21f, 0.2f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.05f, 0.05f, 0.5f));
        model = translateMatrix * scaleMatrix;
        lightEffect(VAO, lightingShader, model, glm::vec3(0.5, 0.5f, 0.5f));
        /*ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(0.5, 0.5f, 0.5f, 1.0f));
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);*/

        //tap 3
        translateMatrix = glm::translate(identityMatrix, glm::vec3(4.0f, 3.26f, 0.7f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.05f, -0.5f, 0.05f));
        model = translateMatrix * scaleMatrix;
        lightEffect(VAO, lightingShader, model, glm::vec3(0.5, 0.5f, 0.5f));
        /*ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(0.5, 0.5f, 0.5f, 1.0f));
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);*/



        //draw table
        translateMatrix = glm::translate(identityMatrix, glm::vec3(4.0f, 2.0f, 5.0f));
        drawTable(VAO, lightingShader, translateMatrix);

        //draw chair 1
        translateMatrix = glm::translate(identityMatrix, glm::vec3(4.5f, 1.5f, 7.5f));
        drawChair(VAO, lightingShader, translateMatrix);

        //draw chair 2
        translateMatrix = glm::translate(identityMatrix, glm::vec3(6.3f, 1.5f, 7.5f));
        drawChair(VAO, lightingShader, translateMatrix);

        //draw chair 3
        rotateYMatrix = glm::rotate(identityMatrix, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        translateMatrix = glm::translate(identityMatrix, glm::vec3(5.7f, 1.5f, 4.5f));
        drawChair(VAO, lightingShader, translateMatrix* rotateYMatrix);

        //draw chair 4
        rotateYMatrix = glm::rotate(identityMatrix, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        translateMatrix = glm::translate(identityMatrix, glm::vec3(7.4f, 1.5f, 4.5f));
        drawChair(VAO, lightingShader, translateMatrix* rotateYMatrix);


        //draw fan with rotations
        //translateMatrix = glm::translate(identityMatrix, glm::vec3(6.0f, 3.0f, 7.0f));
        drawFan(VAO, lightingShader, identityMatrix);


        ourShader.use();
        ourShader.setMat4("view", view);
        ourShader.setMat4("projection", projection);

        glBindVertexArray(lightCubeVAO);
        for (unsigned int i = 0; i < 2; i++)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, pointLightPositions[i]);
            model = glm::scale(model, glm::vec3(0.2f)); // Make it a smaller cube
            ourShader.setMat4("model", model);
            ourShader.setVec4("color", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
            //glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

void drawFan(unsigned int VAO, Shader ourShader, glm::mat4 matrix) {
    glm::mat4 identityMatrix = glm::mat4(1.0f);
    glm::mat4 translateMatrix, rotateXMatrix, rotateYMatrix, rotateZMatrix, scaleMatrix, model, RotateTranslateMatrix, InvRotateTranslateMatrix;

    if (fanOn) {
        //fan rod
        translateMatrix = glm::translate(identityMatrix, glm::vec3(5.15f, 10.0f, 4.9f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.2f, -1.5f, 0.2f));
        model = matrix * translateMatrix * scaleMatrix;
        ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        //fan middle
        rotateYMatrix = glm::rotate(identityMatrix, glm::radians(r), glm::vec3(0.0f, 1.0f, 0.0f));
        translateMatrix = glm::translate(identityMatrix, glm::vec3(4.85f, 8.5f, 4.6f));
        RotateTranslateMatrix = glm::translate(identityMatrix, glm::vec3(-0.4f, 0.0f, -0.4f));
        InvRotateTranslateMatrix = glm::translate(identityMatrix, glm::vec3(0.4f, 0.0f, 0.4f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.8f, -0.2f, 0.8f));
        model = matrix * translateMatrix * InvRotateTranslateMatrix * rotateYMatrix * RotateTranslateMatrix * scaleMatrix;
        ourShader.setMat4("model", model);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        //fan propelars left
        translateMatrix = glm::translate(identityMatrix, glm::vec3(4.85f, 8.5f, 4.8f));
        RotateTranslateMatrix = glm::translate(identityMatrix, glm::vec3(-0.4f, 0.0f, -0.2f));
        InvRotateTranslateMatrix = glm::translate(identityMatrix, glm::vec3(0.4f, 0.0f, 0.2f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(-1.5f, -0.2f, 0.4f));
        model = matrix * translateMatrix * InvRotateTranslateMatrix * rotateYMatrix * RotateTranslateMatrix * scaleMatrix;
        ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        //fan propelars right
        translateMatrix = glm::translate(identityMatrix, glm::vec3(5.65f, 8.5f, 4.8f));
        RotateTranslateMatrix = glm::translate(identityMatrix, glm::vec3(0.4f, 0.0f, -0.2f));
        InvRotateTranslateMatrix = glm::translate(identityMatrix, glm::vec3(-0.4f, 0.0f, 0.2f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(1.5f, -0.2f, 0.4f));
        model = matrix * translateMatrix * InvRotateTranslateMatrix * rotateYMatrix * RotateTranslateMatrix * scaleMatrix;
        ourShader.setMat4("model", model);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        //fan propelars up
        translateMatrix = glm::translate(identityMatrix, glm::vec3(5.05f, 8.5f, 4.6f));
        RotateTranslateMatrix = glm::translate(identityMatrix, glm::vec3(-0.2f, 0.0f, -0.4f));
        InvRotateTranslateMatrix = glm::translate(identityMatrix, glm::vec3(0.2f, 0.0f, 0.4f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.4f, -0.2f, -1.5f));
        model = matrix * translateMatrix * InvRotateTranslateMatrix * rotateYMatrix * RotateTranslateMatrix * scaleMatrix;
        ourShader.setMat4("model", model);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        //fan propelars down
        translateMatrix = glm::translate(identityMatrix, glm::vec3(5.05f, 8.5f, 5.4f));
        RotateTranslateMatrix = glm::translate(identityMatrix, glm::vec3(-0.2f, 0.0f, 0.4f));
        InvRotateTranslateMatrix = glm::translate(identityMatrix, glm::vec3(0.2f, 0.0f, -0.4f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.4f, -0.2f, 1.5f));
        model = matrix * translateMatrix * InvRotateTranslateMatrix * rotateYMatrix * RotateTranslateMatrix * scaleMatrix;
        ourShader.setMat4("model", model);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        r += 0.75f;
    }

    else {
        //fan rod
        translateMatrix = glm::translate(identityMatrix, glm::vec3(5.15f, 10.0f, 4.9f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.2f, -1.5f, 0.2f));
        model = matrix * translateMatrix * scaleMatrix;
        ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        //fan middle
        translateMatrix = glm::translate(identityMatrix, glm::vec3(4.85f, 8.5f, 4.6f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.8f, -0.2f, 0.8f));
        model = matrix * translateMatrix * scaleMatrix;
        ourShader.setMat4("model", model);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        //fan propelars left
        translateMatrix = glm::translate(identityMatrix, glm::vec3(4.85f, 8.5f, 4.8f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(-1.5f, -0.2f, 0.4f));
        model = matrix * translateMatrix * scaleMatrix;
        ourShader.setMat4("model", model);
        ourShader.setVec4("color", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        //fan propelars right
        translateMatrix = glm::translate(identityMatrix, glm::vec3(5.65f, 8.5f, 4.8f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(1.5f, -0.2f, 0.4f));
        model = matrix * translateMatrix * scaleMatrix;
        ourShader.setMat4("model", model);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        //fan propelars up
        translateMatrix = glm::translate(identityMatrix, glm::vec3(5.05f, 8.5f, 4.6f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.4f, -0.2f, -1.5f));
        model = matrix * translateMatrix * scaleMatrix;
        ourShader.setMat4("model", model);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

        //fan propelars down
        translateMatrix = glm::translate(identityMatrix, glm::vec3(5.05f, 8.5f, 5.4f));
        scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.4f, -0.2f, 1.5f));
        model = matrix * translateMatrix * scaleMatrix;
        ourShader.setMat4("model", model);

        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    }
}
void drawChair(unsigned int VAO, Shader ourShader, glm::mat4 matrix) {
    //chair sit
    glm::mat4 identityMatrix = glm::mat4(1.0f);
    glm::mat4 translateMatrix, rotateXMatrix, rotateYMatrix, rotateZMatrix, scaleMatrix, model;
    scaleMatrix = glm::scale(identityMatrix, glm::vec3(1.2f, 0.2f, 1.2f));
    model = matrix * scaleMatrix;
    ourShader.setMat4("model", model);
    ourShader.setVec4("color", glm::vec4(0.09f, 0.08f, 0.07f, 1.0f));

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    //chair left back leg
    translateMatrix = glm::translate(identityMatrix, glm::vec3(0.0f, 0.0f, 1.0f));
    scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.2f, -1.5f, 0.2f));
    model = matrix * translateMatrix * scaleMatrix;
    ourShader.setMat4("model", model);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);


    //chair right back leg
    translateMatrix = glm::translate(identityMatrix, glm::vec3(1.0f, 0.0f, 1.0f));
    scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.2f, -1.5f, 0.2f));
    model = matrix * translateMatrix * scaleMatrix;
    ourShader.setMat4("model", model);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    //chair left front leg
    translateMatrix = glm::translate(identityMatrix, glm::vec3(0.0f, 0.0f, 0.0f));
    scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.2f, -1.5f, 0.2f));
    model = matrix * translateMatrix * scaleMatrix;
    ourShader.setMat4("model", model);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);


    //chair right front leg
    translateMatrix = glm::translate(identityMatrix, glm::vec3(1.0f, 0.0f, 0.0f));
    scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.2f, -1.5f, 0.2f));
    model = matrix * translateMatrix * scaleMatrix;
    ourShader.setMat4("model", model);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    //chair left hand
    translateMatrix = glm::translate(identityMatrix, glm::vec3(0.0f, 0.0f, 1.0f));
    scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.2f, 1.8f, 0.2f));
    model = matrix * translateMatrix * scaleMatrix;
    ourShader.setMat4("model", model);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);


    //chair right hand
    translateMatrix = glm::translate(identityMatrix, glm::vec3(1.0f, 0.0f, 1.0f));
    scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.2f, 1.8f, 0.2f));
    model = matrix * translateMatrix * scaleMatrix;
    ourShader.setMat4("model", model);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    //chair top hand
    translateMatrix = glm::translate(identityMatrix, glm::vec3(0.0f, 1.8f, 1.0f));
    scaleMatrix = glm::scale(identityMatrix, glm::vec3(1.2f, 0.2f, 0.2f));
    model = matrix * translateMatrix * scaleMatrix;
    ourShader.setMat4("model", model);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    //chair mid hand
    translateMatrix = glm::translate(identityMatrix, glm::vec3(0.0f, 0.8f, 1.0f));
    scaleMatrix = glm::scale(identityMatrix, glm::vec3(1.2f, 0.2f, 0.2f));
    model = matrix * translateMatrix * scaleMatrix;
    ourShader.setMat4("model", model);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    
}
void drawTable(unsigned int VAO, Shader ourShader, glm::mat4 matrix) {
    //table top
    glm::mat4 identityMatrix = glm::mat4(1.0f);
    glm::mat4 translateMatrix, rotateXMatrix, rotateYMatrix, rotateZMatrix, scaleMatrix, model;
    scaleMatrix = glm::scale(identityMatrix, glm::vec3(4.0f, 0.2f, 2.0f));
    model = matrix * scaleMatrix;
    ourShader.setMat4("model", model);
    ourShader.setVec4("color", glm::vec4(0.09f, 0.08f, 0.07f, 1.0f));

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);


    //table front leg left
    translateMatrix = glm::translate(identityMatrix, glm::vec3(0.0f, 0.0f, 1.8f));
    scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.2f, -2.0f, 0.2f));
    model = matrix * translateMatrix * scaleMatrix;
    ourShader.setMat4("model", model);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    //table front leg right
    translateMatrix = glm::translate(identityMatrix, glm::vec3(3.8f, 0.0f, 1.8f));
    scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.2f, -2.0f, 0.2f));
    model = matrix * translateMatrix * scaleMatrix;
    ourShader.setMat4("model", model);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    //table back leg left
    translateMatrix = glm::translate(identityMatrix, glm::vec3(0.0f, 0.0f, 0.0f));
    scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.2f, -2.0f, 0.2f));
    model = matrix * translateMatrix * scaleMatrix;
    ourShader.setMat4("model", model);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

    //table back leg right
    translateMatrix = glm::translate(identityMatrix, glm::vec3(3.8f, 0.0f, 0.0f));
    scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.2f, -2.0f, 0.2f));
    model = matrix * translateMatrix * scaleMatrix;
    ourShader.setMat4("model", model);

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);

 
}

void drawCylinder(unsigned int& VAO_C, Shader& lightingShader, glm::vec3 color, glm::mat4 model, std::vector<unsigned int>& indices)
{
    lightingShader.use();

    // setting up materialistic property
    lightingShader.setVec3("material.ambient", color);
    lightingShader.setVec3("material.diffuse", color);
    lightingShader.setVec3("material.specular", color);
    lightingShader.setFloat("material.shininess", 32.0f);
    float emissiveIntensity = 0.05f; // Adjust this value to increase or decrease the intensity
    glm::vec3 emissiveColor = glm::vec3(1.0f, 1.0f, 0.0f) * emissiveIntensity;

    //lightingShader.setVec3("material.emissive", emissiveColor);


    lightingShader.setMat4("model", model);

    glBindVertexArray(VAO_C);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}


void drawSphere(unsigned int& VAO_S, Shader& lightingShader, glm::vec3 color, glm::mat4 model, std::vector<unsigned int>& indices)
{
    lightingShader.use();

    // Setting up materialistic property
    lightingShader.setVec3("material.ambient", color);
    lightingShader.setVec3("material.diffuse", color);
    lightingShader.setVec3("material.specular", color);
    lightingShader.setFloat("material.shininess", 32.0f);
    float emissiveIntensity = 0.05f; // Adjust this value to increase or decrease the intensity
    glm::vec3 emissiveColor = glm::vec3(1.0f, 0.0f, 0.0f) * emissiveIntensity;

    // lightingShader.setVec3("material.emissive", emissiveColor);

    lightingShader.setMat4("model", model);

    glBindVertexArray(VAO_S);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}


void generateCylinder(float radius, float height, int segments, std::vector<float>& vertices, std::vector<unsigned int>& indices) {
    float angleStep = 2.0f * M_PI / segments;

    // Generate vertices and normals
    for (int i = 0; i <= segments; ++i) {
        float angle = i * angleStep;
        float x = radius * cos(angle);
        float z = radius * sin(angle);

        // Bottom circle vertex
        vertices.push_back(x);
        vertices.push_back(0.0f);
        vertices.push_back(z);

        // Bottom circle normal (pointing down the y-axis)
        vertices.push_back(0.0f);
        vertices.push_back(-1.0f);
        vertices.push_back(0.0f);

        // Top circle vertex
        vertices.push_back(x);
        vertices.push_back(height);
        vertices.push_back(z);

        // Top circle normal (pointing up the y-axis)
        vertices.push_back(0.0f);
        vertices.push_back(1.0f);
        vertices.push_back(0.0f);

        // Side vertex (bottom)
        vertices.push_back(x);
        vertices.push_back(0.0f);
        vertices.push_back(z);

        // Side normal
        vertices.push_back(x);
        vertices.push_back(0.0f);
        vertices.push_back(z);

        // Side vertex (top)
        vertices.push_back(x);
        vertices.push_back(height);
        vertices.push_back(z);

        // Side normal
        vertices.push_back(x);
        vertices.push_back(0.0f);
        vertices.push_back(z);
    }

    // Generate indices for the sides
    for (int i = 0; i < segments; ++i) {
        int bottom1 = 4 * i + 2;
        int top1 = bottom1 + 1;
        int bottom2 = 4 * (i + 1) + 2;
        int top2 = bottom2 + 1;

        // Side triangles
        indices.push_back(bottom1);
        indices.push_back(top1);
        indices.push_back(bottom2);

        indices.push_back(top1);
        indices.push_back(top2);
        indices.push_back(bottom2);
    }

    // Center vertex for the top circle
    int topCenterIndex = vertices.size() / 6;
    vertices.push_back(0.0f);
    vertices.push_back(height);
    vertices.push_back(0.0f);

    // Top center normal (pointing up the y-axis)
    vertices.push_back(0.0f);
    vertices.push_back(1.0f);
    vertices.push_back(0.0f);

    // Generate indices for the top circle
    for (int i = 0; i < segments; ++i) {
        int top1 = 4 * i + 1;
        int top2 = 4 * ((i + 1) % segments) + 1;

        indices.push_back(topCenterIndex);
        indices.push_back(top1);
        indices.push_back(top2);
    }

    // Center vertex for the bottom circle
    int bottomCenterIndex = vertices.size() / 6;
    vertices.push_back(0.0f);
    vertices.push_back(0.0f);
    vertices.push_back(0.0f);

    // Bottom center normal (pointing down the y-axis)
    vertices.push_back(0.0f);
    vertices.push_back(-1.0f);
    vertices.push_back(0.0f);

    // Generate indices for the bottom circle
    for (int i = 0; i < segments; ++i) {
        int bottom1 = 4 * i;
        int bottom2 = 4 * ((i + 1) % segments);

        indices.push_back(bottomCenterIndex);
        indices.push_back(bottom2);
        indices.push_back(bottom1);
    }
}




void generateCone(float radius, float height, int segments, std::vector<float>& vertices, std::vector<unsigned int>& indices) {
    float angleStep = 2.0f * M_PI / segments;

    // Generate vertices and normals for the base circle
    for (int i = 0; i < segments; ++i) {
        float angle = i * angleStep;
        float x = radius * cos(angle);
        float z = radius * sin(angle);

        // Base circle vertex
        vertices.push_back(x);
        vertices.push_back(0.0f); // y = 0 for the base
        vertices.push_back(z);

        // Base circle normal (pointing downwards)
        vertices.push_back(0.0f);
        vertices.push_back(-1.0f);
        vertices.push_back(0.0f);
    }

    // Add the tip vertex of the cone
    int tipIndex = vertices.size() / 6; // Index of the tip
    vertices.push_back(0.0f);
    vertices.push_back(height); // Tip is at y = height
    vertices.push_back(0.0f);

    // Tip normal (pointing upwards)
    vertices.push_back(0.0f);
    vertices.push_back(1.0f);
    vertices.push_back(0.0f);

    // Add the center vertex of the base circle
    int baseCenterIndex = vertices.size() / 6; // Index of the base center
    vertices.push_back(0.0f);
    vertices.push_back(0.0f); // Center is at y = 0
    vertices.push_back(0.0f);

    // Base center normal (pointing downwards)
    vertices.push_back(0.0f);
    vertices.push_back(-1.0f);
    vertices.push_back(0.0f);

    // Generate indices and normals for the sides
    for (int i = 0; i < segments; ++i) {
        int nextIndex = (i + 1) % segments;

        // Side triangle
        indices.push_back(i);
        indices.push_back(nextIndex);
        indices.push_back(tipIndex);

        // Calculate normals for the side vertices
        glm::vec3 v1(vertices[6 * i], vertices[6 * i + 1], vertices[6 * i + 2]);
        glm::vec3 v2(vertices[6 * nextIndex], vertices[6 * nextIndex + 1], vertices[6 * nextIndex + 2]);
        glm::vec3 normal = glm::normalize(glm::cross(v2 - v1, glm::vec3(0.0f, height, 0.0f) - v1));

        // Add normals for the side vertices
        vertices.push_back(v1.x);
        vertices.push_back(v1.y);
        vertices.push_back(v1.z);
        vertices.push_back(normal.x);
        vertices.push_back(normal.y);
        vertices.push_back(normal.z);

        vertices.push_back(v2.x);
        vertices.push_back(v2.y);
        vertices.push_back(v2.z);
        vertices.push_back(normal.x);
        vertices.push_back(normal.y);
        vertices.push_back(normal.z);
    }

    // Generate indices for the base circle
    for (int i = 0; i < segments; ++i) {
        int nextIndex = (i + 1) % segments;

        indices.push_back(baseCenterIndex);
        indices.push_back(i);
        indices.push_back(nextIndex);
    }
}




void generateSphere(float radius, int sectorCount, int stackCount, std::vector<float>& vertices, std::vector<unsigned int>& indices) {
    float x, y, z, xy;                              // vertex position
    float nx, ny, nz, lengthInv = 1.0f / radius;    // vertex normal, lenginv is the inverse of the radius


    float sectorStep = 2 * M_PI / sectorCount;
    float stackStep = M_PI / stackCount;
    float sectorAngle, stackAngle;

    for (int i = 0; i <= stackCount; ++i) {
        stackAngle = M_PI / 2 - i * stackStep;        // starting from pi/2 to -pi/2
        xy = radius * cosf(stackAngle);             // r * cos(u)
        z = radius * sinf(stackAngle);              // r * sin(u)

        for (int j = 0; j <= sectorCount; ++j) {
            sectorAngle = j * sectorStep;           // starting from 0 to 2pi

            // vertex position (x, y, z)
            x = xy * cosf(sectorAngle);             // r * cos(u) * cos(v)
            y = xy * sinf(sectorAngle);             // r * cos(u) * sin(v)
            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);

            // normalized vertex normal (nx, ny, nz)
            nx = x * lengthInv;
            ny = y * lengthInv;
            nz = z * lengthInv;
            vertices.push_back(nx);
            vertices.push_back(ny);
            vertices.push_back(nz);
        }
    }

    // generate indices
    int k1, k2;
    for (int i = 0; i < stackCount; ++i) {
        k1 = i * (sectorCount + 1);     // beginning of current stack
        k2 = k1 + sectorCount + 1;      // beginning of next stack

        for (int j = 0; j < sectorCount; ++j, ++k1, ++k2) {
            // 2 triangles per sector excluding first and last stacks
            if (i != 0) {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }

            if (i != (stackCount - 1)) {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }
}



void drawFurnace(unsigned int VAO, Shader ourShader, glm::mat4 matrix) {
    
    //z1
    glm::mat4 identityMatrix = glm::mat4(1.0f);
    glm::mat4 translateMatrix, rotateXMatrix, rotateYMatrix, rotateZMatrix, scaleMatrix, model;
    scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.02f, 0.01f, 0.3f));
    model = matrix * scaleMatrix;
    lightEffect(VAO, ourShader, model, glm::vec3(1.0f, 1.0f, 1.0f));
    /*ourShader.setMat4("model", model);
    ourShader.setVec4("color", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);*/


    //z2
    translateMatrix = glm::translate(identityMatrix, glm::vec3(0.3f, 0.0f,0.0f));
    scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.02f, 0.01f, 0.3f));
    model = matrix * translateMatrix * scaleMatrix;
    lightEffect(VAO, ourShader, model, glm::vec3(1.0f, 1.0f, 1.0f));
    /*ourShader.setMat4("model", model);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);*/

    //x1
    translateMatrix = glm::translate(identityMatrix, glm::vec3(0.0f, 0.0f, 0.0f));
    scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.3f, 0.01f, 0.02f));
    model = matrix * translateMatrix * scaleMatrix;
    lightEffect(VAO, ourShader, model, glm::vec3(1.0f, 1.0f, 1.0f));
    /*ourShader.setMat4("model", model);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);*/

    //x2
    translateMatrix = glm::translate(identityMatrix, glm::vec3(0.0f, 0.0f, 0.3f));
    scaleMatrix = glm::scale(identityMatrix, glm::vec3(0.35f, 0.01f, 0.02f));
    model = matrix * translateMatrix * scaleMatrix;
    lightEffect(VAO, ourShader, model, glm::vec3(1.0f, 1.0f, 1.0f));
    /*ourShader.setMat4("model", model);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);*/

    


}


void lightEffect(unsigned int VAO, Shader lightShader, glm::mat4 model, glm::vec3 color)
{
    lightShader.use();
    lightShader.setVec3("material.ambient", color);
    lightShader.setVec3("material.diffuse", color);
    lightShader.setVec3("material.specular", glm::vec3(0.5f, 0.5f, 0.5f));
    lightShader.setFloat("material.shininess", 32.0f);

    lightShader.setMat4("model", model);
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        camera.ProcessKeyboard(FORWARD, deltaTime);
    }

    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    }

    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        camera.ProcessKeyboard(LEFT, deltaTime);
    }

    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
        camera.ProcessKeyboard(RIGHT, deltaTime);
    }

    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
        camera.ProcessKeyboard(UP, deltaTime);
    }

    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
        camera.ProcessKeyboard(DOWN, deltaTime);
    }

    if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS) {
        camera.ProcessKeyboard(P_UP, deltaTime);
    }

    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
        camera.ProcessKeyboard(P_DOWN, deltaTime);
    }

    if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS) {
        camera.ProcessKeyboard(Y_LEFT, deltaTime);
    }

    if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
        camera.ProcessKeyboard(Y_RIGHT, deltaTime);
    }

    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS) {
        camera.ProcessKeyboard(R_LEFT, deltaTime);
    }

    if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS) {
        camera.ProcessKeyboard(R_RIGHT, deltaTime);
    }

    if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
    {
        if (fanOn) {
            fanOn = false;
        }
        else {
            fanOn = true;
        }
    }

    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) {
        birdEyeView = !birdEyeView;
    }

    if (birdEyeView) {
        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
            birdEyePosition.z -= birdEyeSpeed * deltaTime; // Move forward along Z
            birdEyeTarget.z -= birdEyeSpeed * deltaTime;
            if (birdEyePosition.z <= 4.0) {
                birdEyePosition.z = 4.0;
            }
            if (birdEyeTarget.z <= 0.0) {
                birdEyeTarget.z = 0.0;
            }
        }
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
            birdEyePosition.z += birdEyeSpeed * deltaTime; // Move backward along Z
            birdEyeTarget.z += birdEyeSpeed * deltaTime;
            if (birdEyePosition.z >= 10.0) {
                birdEyePosition.z = 10.0;
            }
            if (birdEyeTarget.z >= 6.0) {
                birdEyeTarget.z = 6.0;
            }
        }
    }

    if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
    {
        if (!birdEyeView) {
            theta += 0.01f; // Increment angle to rotate
            // Calculate new eye position using spherical coordinates
            camera.Position.x = lookAtX + radius * sin(theta);
            camera.Position.y = lookAtY;
            camera.Position.z = lookAtZ + radius * cos(theta);
        }
    }

    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
    {
        if (dl)
            dl = false;
        else
            dl = true;

    }
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
    {
        if (point1)
        {
            point1 = false;
            pointlight1.turnOff();
        }
        else
        {
            point1 = true;
            pointlight1.turnOn();
        }

    }
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS)
    {
        if (point2)
        {
            point2 = false;
            pointlight2.turnOff();
        }
        else
        {
            point2 = true;
            pointlight2.turnOn();
        }

    }
    if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS)
    {
        if (spt)
        {
            spt = false;
        }
        else
            spt = true;

    }

    if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS)
    {
        if (ambientToggle)
        {
            pointlight1.turnAmbientOff();
            pointlight2.turnAmbientOff();
            ambientToggle = false;
        }
        else
        {
            pointlight1.turnAmbientOn();
            pointlight2.turnAmbientOn();
            ambientToggle = true;
        }

    }

    if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS)
    {
        if (diffuseToggle)
        {
            pointlight1.turnDiffuseOff();
            pointlight2.turnDiffuseOff();
            //d_def_on = 0.0f;

            diffuseToggle = false;
        }
        else
        {
            pointlight1.turnDiffuseOn();
            pointlight2.turnDiffuseOn();

            //d_def_on = 1.0f;
            diffuseToggle = true;
        }

    }

    if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS)
    {
        if (specularToggle)
        {
            pointlight1.turnSpecularOff();
            pointlight2.turnSpecularOff();
            //d_def_on = 0.0f;

            specularToggle = false;
        }
        else
        {
            pointlight1.turnSpecularOn();
            pointlight2.turnSpecularOn();

            //d_def_on = 1.0f;
            specularToggle = true;
        }

    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    float aspectRatio = 4.0f / 3.0f;
    int viewWidth, viewHeight;

    if (width / (float)height > aspectRatio) {
        // Window is too wide, fit height and adjust width
        viewHeight = height;
        viewWidth = (int)(height * aspectRatio);
    }
    else {
        // Window is too tall, fit width and adjust height
        viewWidth = width;
        viewHeight = (int)(width / aspectRatio);
    }

    // Center the viewport
    int xOffset = (width - viewWidth) / 2;
    int yOffset = (height - viewHeight) / 2;

    glViewport(xOffset, yOffset, viewWidth, viewHeight);
    //glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    camera.ProcessMouseScroll(static_cast<float>(yoffset));
}