#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <iostream>

void framebuffer_size_callback(GLFWwindow *window, int width, int height);

void mouse_callback(GLFWwindow *window, double xpos, double ypos);

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

void processInput(GLFWwindow *window);

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

unsigned int loadCubemap(vector<std::string> faces);

unsigned int loadTexture(char const * path);

void renderQuad();

void renderQuadForBloom();

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
bool cursorEnabled = false;
bool blinnPhong = true;
bool gateClosed = false;
float heightScale = 0.0305f;
bool bloom = false;
bool bloomKeyPressed = false;
float exposure = 1.0f;

// camera
Camera camera(glm::vec3(-6.0f, 7.0f, -9.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main() {
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
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "UFO-observed village", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(false);


    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);

    // build and compile shaders
    // -------------------------
    Shader ourShader("resources/shaders/2.model_lighting.vs", "resources/shaders/2.model_lighting.fs");
    Shader skyboxShader("resources/shaders/skybox.vs", "resources/shaders/skybox.fs");
    Shader blendingShader("resources/shaders/blending.vs", "resources/shaders/blending.fs");
    Shader shader("resources/shaders/parallax_mapping.vs", "resources/shaders/parallax_mapping.fs");

    Shader blurShader("resources/shaders/blur.vs", "resources/shaders/blur.fs");
    Shader bloomFinalShader("resources/shaders/bloom_final.vs", "resources/shaders/bloom_final.fs");
    Shader ufoShader("resources/shaders/bloomSpotLight.vs", "resources/shaders/bloomSpotLight.fs");

    // skybox vertices
    float skyboxVertices[] = {
            // positions
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            -1.0f,  1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f,  1.0f
    };

    float transparentVertices[] = {
            // positions                     // texture Coords (swapped y coordinates because texture is flipped upside down)
            0.0f,  0.5f,  0.0f,  0.0f,  0.0f,
            0.0f, -0.5f,  0.0f,  0.0f,  1.0f,
            1.0f, -0.5f,  0.0f,  1.0f,  1.0f,

            0.0f,  0.5f,  0.0f,  0.0f,  0.0f,
            1.0f, -0.5f,  0.0f,  1.0f,  1.0f,
            1.0f,  0.5f,  0.0f,  1.0f,  0.0f
    };

    glm::vec3 pointLightPositions[] = {
            glm::vec3( 0.7f,  0.2f,  2.0f),
            glm::vec3( 2.3f, 2.0f, -4.0f),
            glm::vec3(-4.0f,  2.0f, -12.0f),
            glm::vec3( 0.0f,  0.0f, -3.0f)
    };

    // Skybox setup
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);

    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);

    // transparent VAO
    unsigned int transparentVAO, transparentVBO;
    glGenVertexArrays(1, &transparentVAO);
    glGenBuffers(1, &transparentVBO);
    glBindVertexArray(transparentVAO);
    glBindBuffer(GL_ARRAY_BUFFER, transparentVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(transparentVertices), transparentVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);

    // configure (floating point) framebuffers
    // ---------------------------------------
    unsigned int hdrFBO;
    glGenFramebuffers(1, &hdrFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
    // create 2 floating point color buffers (1 for normal rendering, other for brightness threshold values)
    unsigned int colorBuffers[2];
    glGenTextures(2, colorBuffers);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        // attach texture to framebuffer
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0);
    }
    // create and attach depth buffer (renderbuffer)
    unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
    // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering
    unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, attachments);
    // finally check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // ping-pong-framebuffer for blurring
    unsigned int pingpongFBO[2];
    unsigned int pingpongColorbuffers[2];
    glGenFramebuffers(2, pingpongFBO);
    glGenTextures(2, pingpongColorbuffers);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
        glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorbuffers[i], 0);
        // also check if framebuffers are complete (no need for depth buffer)
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Framebuffer not complete!" << std::endl;
    }
    // load textures
    // .............

    unsigned int transparentTexture = loadTexture(FileSystem::getPath("resources/textures/tree.png").c_str());
    unsigned int floorDiffuseMap = loadTexture(FileSystem::getPath("resources/textures/grass/diffuse.png").c_str());
    unsigned int floorSpecularMap = loadTexture(FileSystem::getPath("resources/textures/grass/specular.png").c_str());

    unsigned int pDiffuseMap = loadTexture(FileSystem::getPath("resources/textures/grassD.jpg").c_str());
    unsigned int pNormalMap = loadTexture(FileSystem::getPath("resources/textures/grassN.jpg").c_str());
    unsigned int pHeightMap = loadTexture(FileSystem::getPath("resources/textures/grassH.jpg").c_str());

    ourShader.use();
    ourShader.setInt("material.diffuse", 0);
    ourShader.setInt("material.specular", 1);

    ufoShader.use();
    ufoShader.setInt("material.diffuse", 0);
    ufoShader.setInt("material.specular", 1);

    shader.use();
    shader.setInt("material.diffuseMap", 0);
    shader.setInt("material.normalMap", 1);
    shader.setInt("material.depthMap", 2);

    // skybox textures
    vector<std::string> skyboxSides = {
            FileSystem::getPath("resources/textures/alps/right.tga"),
            FileSystem::getPath("resources/textures/alps/left.tga"),
            FileSystem::getPath("resources/textures/alps/up.tga"),
            FileSystem::getPath("resources/textures/alps/down.tga"),
            FileSystem::getPath("resources/textures/alps/back.tga"),
            FileSystem::getPath("resources/textures/alps/front.tga")
    };
    unsigned int cubemapTexture = loadCubemap(skyboxSides);
    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);

    // transparent vegetation locations
    // --------------------------------
    vector<glm::vec3> vegetation
            {
                    glm::vec3(7.0f, 2.5f, 12.5f),
                    glm::vec3(5.0f, 2.5f, 12.4f),
                    glm::vec3(3.0f, 2.5f, 12.5f),
                    glm::vec3(0.0f, 2.5f, 12.4f),
                    glm::vec3(-3.0f, 2.5f, 12.5f),
                    glm::vec3(-6.0f, 2.5f, 12.4f),
                    glm::vec3(-9.0f, 2.5f, 12.5f),
                    glm::vec3(-12.0f, 2.5f, 12.4f),
                    glm::vec3(-14.0f, 2.5f, 12.5f),
                    glm::vec3(7.0f, 2.5f, -12.5f),
                    glm::vec3(5.0f, 2.5f, -12.4f),
                    glm::vec3(3.0f, 2.5f, -12.5f),
                    glm::vec3(0.0f, 2.5f, -12.4f),
                    glm::vec3(-3.0f, 2.5f, -12.5f),
                    glm::vec3(-6.0f, 2.5f, -12.4f),
                    glm::vec3(-9.0f, 2.5f, -12.5f),
                    glm::vec3(-12.0f, 2.5f, -12.4f),
                    glm::vec3(-14.0f, 2.5f, -12.5f)
            };

    vector<glm::vec3> vegetationRotated =
            {
                    glm::vec3(12.5f, 2.5f, 15.0f),
                    glm::vec3(12.4f, 2.5f, 13.0f),
                    glm::vec3(12.5f, 2.5f, 10.0f),
                    glm::vec3(12.4f, 2.5f, 7.0f),
                    glm::vec3(12.5f, 2.5f, 4.0f),
                    glm::vec3(12.4f, 2.5f, 1.0f),
                    glm::vec3(12.5f, 2.5f, -2.0f),
                    glm::vec3(12.4f, 2.5f, -5.0f),
                    glm::vec3(12.5f, 2.5f, -8.0f),
                    glm::vec3(-12.5f, 2.5f, 15.0f),
                    glm::vec3(-12.4f, 2.5f, 13.0f),
                    glm::vec3(-12.5f, 2.5f, 10.0f),
                    glm::vec3(-12.4f, 2.5f, 7.0f),
                    glm::vec3(-12.5f, 2.5f, 4.0f),
                    glm::vec3(-12.4f, 2.5f, 1.0f),
                    glm::vec3(-12.5f, 2.5f, -2.0f),
                    glm::vec3(-12.4f, 2.5f, -5.0f),
                    glm::vec3(-12.5f, 2.5f, -8.0f)
            };

    // blending shader configuration
    // --------------------
    blendingShader.use();
    blendingShader.setInt("texture1", 0);

    // bloom shaders configuration
    // ---------------------------
    blurShader.use();
    blurShader.setInt("image", 0);
    bloomFinalShader.use();
    bloomFinalShader.setInt("scene", 0);
    bloomFinalShader.setInt("bloomBlur", 1);

    //before loading model
    stbi_set_flip_vertically_on_load(false);

    // load models
    // -----------
    Model ufoModel("resources/objects/ufo/Low_poly_UFO.obj");
    ufoModel.SetShaderTextureNamePrefix("material.");

    Model stallModel("resources/objects/proba/model/silo.obj");
    stallModel.SetShaderTextureNamePrefix("material.");

    Model hutModel("resources/objects/hut/woodshed.obj");
    hutModel.SetShaderTextureNamePrefix("material.");

    Model wellModel("resources/objects/well/well.obj");
    wellModel.SetShaderTextureNamePrefix("material.");

    Model fenceModel("resources/objects/fence/fence wood.obj");
    fenceModel.SetShaderTextureNamePrefix("material.");

    Model sheepModel("resources/objects/sheep/sheep01.obj");
    sheepModel.SetShaderTextureNamePrefix("material.");

    Model humanModel("resources/objects/human/human.obj");
    humanModel.SetShaderTextureNamePrefix("material.");

    // coords for models
    // -----------------
    vector <glm::vec3> stalls =
            {
                    glm::vec3(10.5f, 0.0f, 11.0f),
                    glm::vec3(10.5f, 0.0f, 6.0f),
                    glm::vec3(10.5f, 0.0f, 1.0f),
                    glm::vec3(10.5f, 0.0f, -4.0f),
                    glm::vec3(10.5f, 0.0f, -9.0f)
            };

    vector <glm::vec3> hutsRotated =
            {
                    glm::vec3(-9.5f, 0.0f, -12.0f),
                    glm::vec3(-9.5f, 0.0f, -9.25f),
                    glm::vec3(-9.5f, 0.0f, -6.5f),
                    glm::vec3(-9.5f, 0.0f, -3.75f),
                    glm::vec3(-9.5f, 0.0f, -1.0f),
                    glm::vec3(-9.5f, 0.0f, 1.75f),
                    glm::vec3(-9.5f, 0.0f, 4.5f),
                    glm::vec3(-9.5f, 0.0f, 7.25f),
                    glm::vec3(-9.5f, 0.0f, 10.0f)
            };

    vector <glm::vec3> huts =
            {
                    glm::vec3(-4.5f, 0.0f, -10.0f),
                    glm::vec3(-4.5f, 0.0f, -7.25f),
                    glm::vec3(-4.5f, 0.0f, -4.5f),
                    glm::vec3(-4.5f, 0.0f, -1.75f),
                    glm::vec3(-4.5f, 0.0f, 3.75f),
                    glm::vec3(-4.5f, 0.0f, 6.5f),
                    glm::vec3(-4.5f, 0.0f, 9.25f),
                    glm::vec3(-4.5f, 0.0f, 12.0f)
            };

    vector <glm::vec3> fences =
            {
                    glm::vec3(1.0f, 0.0f, -8.0f),
                    glm::vec3(2.35f, 0.0f, -8.0f),
                    glm::vec3(3.70f, 0.0f, -8.0f),
                    glm::vec3(5.05f, 0.0f, -8.0f),
                    glm::vec3(6.4f, 0.0f, -8.0f),
                    glm::vec3(1.0f, 0.0f, 8.3f),
                    glm::vec3(2.35f, 0.0f, 8.3f),
                    glm::vec3(3.70f, 0.0f, 8.3f),
                    glm::vec3(5.05f, 0.0f, 8.3f),
                    glm::vec3(6.4f, 0.0f, 8.3f)
            };

    vector <glm::vec3> fencesRotated =
            {
                    glm::vec3(0.3f, 0.0f, -7.15f),
                    glm::vec3(0.3f, 0.0f, -5.8f),
                    glm::vec3(0.3f, 0.0f, -4.45f),
                    glm::vec3(0.3f, 0.0f, -3.10f),
                    glm::vec3(0.3f, 0.0f, -1.75f),
                    glm::vec3(0.3f, 0.0f, -0.4f),
                    glm::vec3(0.3f, 0.0f, 0.95f),
                    glm::vec3(0.3f, 0.0f, 2.30f),
                    glm::vec3(0.3f, 0.0f, 3.65f),
                    glm::vec3(0.3f, 0.0f, 5.0f),
                    glm::vec3(0.3f, 0.0f, 6.35f),
                    glm::vec3(0.3f, 0.0f, 7.7f),
                    glm::vec3(7.12f, 0.0f, -7.15f),
                    glm::vec3(7.12f, 0.0f, -5.8f),
                    glm::vec3(7.12f, 0.0f, -4.45f),
                    glm::vec3(7.12f, 0.0f, -3.10f),
                    glm::vec3(7.12f, 0.0f, -1.75f),
                    glm::vec3(7.12f, 0.0f, 2.30f),
                    glm::vec3(7.12f, 0.0f, 3.65f),
                    glm::vec3(7.12f, 0.0f, 5.0f),
                    glm::vec3(7.12f, 0.0f, 6.35f),
                    glm::vec3(7.12f, 0.0f, 7.7f)
            };

    vector <glm::vec3> sheepsInside =
            {
                    glm::vec3(10.0f, 0.0f, 8.7f),
                    glm::vec3(11.0f, 0.0f, 9.5f),
                    glm::vec3(11.0f, 0.0f, -5.5f),
                    glm::vec3(10.0f, 0.0f, -6.2f),
                    glm::vec3(11.0f, 0.0f, -10.5f),
                    glm::vec3(10.0f, 0.0f, -11.2f)
            };

    vector <glm::vec3> sheepsOutside =
            {
                    glm::vec3(2.0f, 0.0f, 6.0f),
                    glm::vec3(3.0f, 0.0f, 7.0f),
                    glm::vec3(3.5f, 0.0f, 5.5f),
                    glm::vec3(5.5f, 0.0f, 7.5f),
                    glm::vec3(6.5f, 0.0f, 6.0f),
                    glm::vec3(3.5f, 0.0f, 4.0f),
                    glm::vec3(4.5f, 0.0f, 3.0f),
                    glm::vec3(2.0f, 0.0f, 4.0f),
                    glm::vec3(5.5f, 0.0f, -2.0f),
                    glm::vec3(1.0f, 0.0f, -5.0f),
                    glm::vec3(2.0f, 0.0f, -4.0f),
                    glm::vec3(2.5f, 0.0f, -5.5f),
                    glm::vec3(4.5f, 0.0f, -3.5f),
                    glm::vec3(5.5f, 0.0f, -5.0f),
                    glm::vec3(5.5f, 0.0f, -7.0f)
            };

    // draw in wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window)) {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        if(cursorEnabled)
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        else
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        // render
        // ------
        glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //render scene into floating point framebuffer
        // -----------------------------------------------BLOOM
        glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ufoShader.use();
        ufoShader.setVec3("viewPosition", camera.Position);
        ufoShader.setFloat("material.shininess", 16.0f);

        // spotlight
        ufoShader.setVec3("spotLight.position", glm::vec3(10 * cos(glfwGetTime()/2), 7.0f, 10 * sin(glfwGetTime()/2)));
        ufoShader.setVec3("spotLight.direction", glm::vec3(0.0f, -1.0f, 0.0f));
        ufoShader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
        ufoShader.setVec3("spotLight.diffuse", 0.1f, 0.1f, 0.1f);
        ufoShader.setVec3("spotLight.specular", 0.1f, 0.1f, 0.1f);
        ufoShader.setFloat("spotLight.constant", 1.0f);
        ufoShader.setFloat("spotLight.linear", 1.0f);
        ufoShader.setFloat("spotLight.quadratic", 1.0f);
        ufoShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
        ufoShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));

        // don't forget to enable shader before setting uniforms
        ourShader.use();
        ourShader.setVec3("viewPos", camera.Position);
        ourShader.setFloat("material.shininess", 16.0f);
        ourShader.setInt("blinnPhong", blinnPhong);

        // directional light
        ourShader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
        ourShader.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
        ourShader.setVec3("dirLight.diffuse", 0.1f, 0.1f, 0.1f);
        ourShader.setVec3("dirLight.specular", 0.1f, 0.1f, 0.1f);
        // point light 1
        ourShader.setVec3("pointLights[0].position", pointLightPositions[0]);
        ourShader.setVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
        ourShader.setVec3("pointLights[0].diffuse", 0.1f, 0.1f, 0.1f);
        ourShader.setVec3("pointLights[0].specular", 0.1f, 0.1f, 0.1f);
        ourShader.setFloat("pointLights[0].constant", 1.0f);
        ourShader.setFloat("pointLights[0].linear", 0.09);
        ourShader.setFloat("pointLights[0].quadratic", 0.032);
        // point light 2
        ourShader.setVec3("pointLights[1].position", pointLightPositions[1]);
        ourShader.setVec3("pointLights[1].ambient", 0.05f, 0.05f, 0.05f);
        ourShader.setVec3("pointLights[1].diffuse", 0.1f, 0.1f, 0.1f);
        ourShader.setVec3("pointLights[1].specular", 0.1f, 0.1f, 0.1f);
        ourShader.setFloat("pointLights[1].constant", 1.0f);
        ourShader.setFloat("pointLights[1].linear", 0.09);
        ourShader.setFloat("pointLights[1].quadratic", 0.032);
        // point light 3
        ourShader.setVec3("pointLights[2].position", pointLightPositions[2]);
        ourShader.setVec3("pointLights[2].ambient", 0.05f, 0.05f, 0.05f);
        ourShader.setVec3("pointLights[2].diffuse", 0.1f, 0.1f, 0.1f);
        ourShader.setVec3("pointLights[2].specular", 0.1f, 0.1f, 0.1f);
        ourShader.setFloat("pointLights[2].constant", 1.0f);
        ourShader.setFloat("pointLights[2].linear", 0.09);
        ourShader.setFloat("pointLights[2].quadratic", 0.032);
        // point light 4
        ourShader.setVec3("pointLights[3].position", pointLightPositions[3]);
        ourShader.setVec3("pointLights[3].ambient", 0.05f, 0.05f, 0.05f);
        ourShader.setVec3("pointLights[3].diffuse", 0.1f, 0.1f, 0.1f);
        ourShader.setVec3("pointLights[3].specular", 0.1f, 0.1f, 0.1f);
        ourShader.setFloat("pointLights[3].constant", 1.0f);
        ourShader.setFloat("pointLights[3].linear", 0.09);
        ourShader.setFloat("pointLights[3].quadratic", 0.032);

        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
                                                (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);

        ufoShader.setMat4("projection", projection);
        ufoShader.setMat4("view", view);
        // render the loaded models

        // ufo model
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model,
                               glm::vec3(10 * cos(glfwGetTime()/2), 7.0f, 10 * sin(glfwGetTime()/2))); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(0.05f));    // it's a bit too big for our scene, so scale it down
        ufoShader.setMat4("model", model);
        ufoModel.Draw(ufoShader);

        // stall model
        for(unsigned int i = 0; i < stalls.size(); i++)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, stalls[i]);
            model = glm::scale(model, glm::vec3(0.1f));
            ourShader.setMat4("model", model);
            stallModel.Draw(ourShader);
        }

        // hut model
        for(unsigned int i = 0; i < hutsRotated.size(); i ++)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, hutsRotated[i]);
            model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(0.005f));
            ourShader.setMat4("model", model);
            hutModel.Draw(ourShader);
        }

        for(unsigned int i = 0; i < huts.size(); i++){
            model = glm::mat4(1.0f);
            model = glm::translate(model, huts[i]);
            model = glm::scale(model, glm::vec3(0.005f));
            ourShader.setMat4("model", model);
            hutModel.Draw(ourShader);
        }

        // human model
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.009f));
        ourShader.setMat4("model", model);
        humanModel.Draw(ourShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-6.0f, 0.0f, 8.0f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.009f));
        ourShader.setMat4("model", model);
        humanModel.Draw(ourShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-8.0f, 0.0f, 8.0f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(-45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, glm::vec3(0.009f));
        ourShader.setMat4("model", model);
        humanModel.Draw(ourShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-8.0f, 0.0f, 6.5f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, glm::vec3(0.009f));
        ourShader.setMat4("model", model);
        humanModel.Draw(ourShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-6.0f, 0.0f, 6.5f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(135.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, glm::vec3(0.009f));
        ourShader.setMat4("model", model);
        humanModel.Draw(ourShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-6.0f, 0.0f, -6.5f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.009f));
        ourShader.setMat4("model", model);
        humanModel.Draw(ourShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-8.0f, 0.0f, -6.5f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(-45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, glm::vec3(0.009f));
        ourShader.setMat4("model", model);
        humanModel.Draw(ourShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-8.0f, 0.0f, -8.0f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, glm::vec3(0.009f));
        ourShader.setMat4("model", model);
        humanModel.Draw(ourShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-6.0f, 0.0f, -8.0f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(135.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, glm::vec3(0.009f));
        ourShader.setMat4("model", model);
        humanModel.Draw(ourShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-6.0f, 0.0f, 1.5f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.009f));
        ourShader.setMat4("model", model);
        humanModel.Draw(ourShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-8.0f, 0.0f, 1.5f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(-45.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, glm::vec3(0.009f));
        ourShader.setMat4("model", model);
        humanModel.Draw(ourShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-8.0f, 0.0f, -1.5f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, glm::vec3(0.009f));
        ourShader.setMat4("model", model);
        humanModel.Draw(ourShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-6.0f, 0.0f, -1.5f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(135.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, glm::vec3(0.009f));
        ourShader.setMat4("model", model);
        humanModel.Draw(ourShader);

        // well model
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(4.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.15f));
        ourShader.setMat4("model", model);
        wellModel.Draw(ourShader);

        // fence model
        for(unsigned int i = 0; i < fences.size(); i++)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, fences[i]);
            model = glm::scale(model, glm::vec3(0.8f));
            ourShader.setMat4("model", model);
            fenceModel.Draw(ourShader);
        }

        for(unsigned int i = 0; i < fencesRotated.size(); i ++) {
            model = glm::mat4(1.0f);
            model = glm::translate(model, fencesRotated[i]);
            model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(0.8f));
            ourShader.setMat4("model", model);
            fenceModel.Draw(ourShader);
        }

        if(!gateClosed)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(6.55f, 0.0f, 2.05f));
            model = glm::rotate(model, glm::radians(45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(0.8f));
            ourShader.setMat4("model", model);
            fenceModel.Draw(ourShader);

            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(6.65f, 0.0f, -1.7f));
            model = glm::rotate(model, glm::radians(-45.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(0.8f));
            ourShader.setMat4("model", model);
            fenceModel.Draw(ourShader);
        }
        else
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(7.12f, 0.0f, 0.95f));
            model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(0.8f));
            ourShader.setMat4("model", model);
            fenceModel.Draw(ourShader);

            model = glm::mat4(1.0f);
            model = glm::translate(model, glm::vec3(7.12f, 0.0f, -0.4f));
            model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(0.8f));
            ourShader.setMat4("model", model);
            fenceModel.Draw(ourShader);
        }
        // sheep model
        for(unsigned int i = 0; i < sheepsInside.size(); i++){
            model = glm::mat4(1.0f);
            model = glm::translate(model, sheepsInside[i]);
            model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(0.6f));
            ourShader.setMat4("model", model);
            sheepModel.Draw(ourShader);
        }

        for(unsigned int i = 0; i < sheepsOutside.size(); i++){
            model = glm::mat4(1.0f);
            model = glm::translate(model, sheepsOutside[i]);
            model = glm::rotate(model, glm::radians(15.0f * (float)pow(-1, i) * i), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(0.6f));
            ourShader.setMat4("model", model);
            sheepModel.Draw(ourShader);
        }

        // skybox shader setup
        // -----------
        glDepthFunc(GL_LEQUAL);  // change depth function so depth test passes when values are equal to depth buffer's content
        skyboxShader.use();
        view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // remove translation from the view matrix
        skyboxShader.setMat4("view", view);
        skyboxShader.setMat4("projection", projection);

        // render skybox cube
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS); // set depth function back to default

        // blending shader setup
        blendingShader.use();
        projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        view = camera.GetViewMatrix();
        model = glm::mat4(1.0f);
        blendingShader.setMat4("projection", projection);
        blendingShader.setMat4("view", view);

        // vegetation
        glBindVertexArray(transparentVAO);
        glBindTexture(GL_TEXTURE_2D, transparentTexture);
        for (unsigned int i = 0; i < vegetation.size(); i++)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, vegetation[i]);
            model = glm::scale(model, glm::vec3(7.0f));
            blendingShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }
        for (unsigned int i = 0; i < vegetationRotated.size(); i++)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, vegetationRotated[i]);
            model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(7.0f));
            blendingShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }


        // configure view/projection matrices
        glm::mat4 projection1 = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view1 = camera.GetViewMatrix();
        shader.use();
        shader.setMat4("projection", projection1);
        shader.setMat4("view", view1);

        // setup shader

        // directional light
        shader.setVec3("lightDir", -0.2f, -1.0f, -0.3f);
        shader.setVec3("dirLight.direction", -0.2f, -1.0f, -0.3f);
        shader.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
        shader.setVec3("dirLight.diffuse", 0.1f, 0.1f, 0.1f);
        shader.setVec3("dirLight.specular", 0.1f, 0.1f, 0.1f);
        // point light 1
        shader.setVec3("lightPos[0]", pointLightPositions[0]);
        shader.setVec3("pointLight[0].position", pointLightPositions[0]);
        shader.setVec3("pointLight[0].ambient", 0.05f, 0.05f, 0.05f);
        shader.setVec3("pointLight[0].diffuse", 0.1f, 0.1f, 0.1f);
        shader.setVec3("pointLight[0].specular", 0.1f, 0.1f, 0.1f);
        shader.setFloat("pointLight[0].constant", 1.0f);
        shader.setFloat("pointLight[0].linear", 0.09);
        shader.setFloat("pointLight[0].quadratic", 0.032);
        // point light 2
        shader.setVec3("lightPos[1]", pointLightPositions[1]);
        shader.setVec3("pointLight[1].position", pointLightPositions[1]);
        shader.setVec3("pointLight[1].ambient", 0.05f, 0.05f, 0.05f);
        shader.setVec3("pointLight[1].diffuse", 0.1f, 0.1f, 0.1f);
        shader.setVec3("pointLight[1].specular", 0.1f, 0.1f, 0.1f);
        shader.setFloat("pointLight[1].constant", 1.0f);
        shader.setFloat("pointLight[1].linear", 0.09);
        shader.setFloat("pointLight[1].quadratic", 0.032);
        // point light 3
        shader.setVec3("lightPos[2]", pointLightPositions[2]);
        shader.setVec3("pointLight[2].position", pointLightPositions[2]);
        shader.setVec3("pointLight[2].ambient", 0.05f, 0.05f, 0.05f);
        shader.setVec3("pointLight[2].diffuse", 0.1f, 0.1f, 0.1f);
        shader.setVec3("pointLight[2].specular", 0.1f, 0.1f, 0.1f);
        shader.setFloat("pointLight[2].constant", 1.0f);
        shader.setFloat("pointLight[2].linear", 0.09);
        shader.setFloat("pointLight[2].quadratic", 0.032);
        // point light 4
        shader.setVec3("lightPos[3]", pointLightPositions[3]);
        shader.setVec3("pointLight[3].position", pointLightPositions[3]);
        shader.setVec3("pointLight[3].ambient", 0.05f, 0.05f, 0.05f);
        shader.setVec3("pointLight[3].diffuse", 0.1f, 0.1f, 0.1f);
        shader.setVec3("pointLight[3].specular", 0.1f, 0.1f, 0.1f);
        shader.setFloat("pointLight[3].constant", 1.0f);
        shader.setFloat("pointLight[3].linear", 0.09);
        shader.setFloat("pointLight[3].quadratic", 0.032);

        // render parallax-mapped quad
        glm::mat4 model1 = glm::mat4(1.0f);
        model1 = glm::translate(model1, glm::vec3(0.0f, 0.0f, 0.0f));
        model1 = glm::rotate(model1, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        model1 = glm::scale(model1, glm::vec3(12.5f));
        shader.setMat4("model", model1);
        shader.setVec3("viewPos", camera.Position);
        shader.setInt("blinnPhong", blinnPhong);
        shader.setFloat("material.shininess", 1000.0f);
        shader.setFloat("heightScale", heightScale); // adjust with Q and E keys
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, pDiffuseMap);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, pNormalMap);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, pHeightMap);
        glEnable(GL_CULL_FACE);     // floor won't be visible if looked from bellow
        glCullFace(GL_BACK);
        renderQuad();
        glDisable(GL_CULL_FACE);


        // 2. blur bright fragments with two-pass Gaussian Blur
        // --------------------------------------------------
        bool horizontal = true, first_iteration = true;
        unsigned int amount = 10;
        blurShader.use();
        for (unsigned int i = 0; i < amount; i++)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
            blurShader.setInt("horizontal", horizontal);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, first_iteration ? colorBuffers[1] : pingpongColorbuffers[!horizontal]);  // bind texture of other framebuffer (or scene if first iteration)
            renderQuadForBloom();
            horizontal = !horizontal;
            if (first_iteration)
                first_iteration = false;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // 3. now render floating point color buffer to 2D quad and tonemap HDR colors to default framebuffer's (clamped) color range
        // --------------------------------------------------------------------------------------------------------------------------
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        bloomFinalShader.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[!horizontal]);
        bloomFinalShader.setInt("bloom", bloom);
        bloomFinalShader.setFloat("exposure", exposure);
        renderQuadForBloom();


        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        if (heightScale > 0.0f)
            heightScale -= 0.0005f;
        else
            heightScale = 0.0f;
    }
    else if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    {
        if (heightScale < 1.0f)
            heightScale += 0.0005f;
        else
            heightScale = 1.0f;
    }

    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && !bloomKeyPressed)
    {
        bloom = !bloom;
        bloomKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_RELEASE)
    {
        bloomKeyPressed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
    {
        if (exposure > 0.0f)
            exposure -= 0.001f;
        else
            exposure = 0.0f;
    }
    else if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
    {
        exposure += 0.001f;
    }
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
    if (firstMouse) {
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
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
    camera.ProcessMouseScroll(yoffset);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_F1 && action == GLFW_PRESS)
        cursorEnabled = !cursorEnabled;
    if (key == GLFW_KEY_G && action == GLFW_PRESS)
        gateClosed = !gateClosed;
    if (key == GLFW_KEY_M && action == GLFW_PRESS)
        blinnPhong = !blinnPhong;
}

unsigned int loadCubemap(vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

unsigned int loadTexture(char const * path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format = GL_RED;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, (GLint)format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

// renders a 1x1 quad in NDC with manually calculated tangent vectors
// ------------------------------------------------------------------
unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
    if (quadVAO == 0)
    {
        // positions
        glm::vec3 pos1(-1.0f,  1.0f, 0.0f);
        glm::vec3 pos2(-1.0f, -1.0f, 0.0f);
        glm::vec3 pos3( 1.0f, -1.0f, 0.0f);
        glm::vec3 pos4( 1.0f,  1.0f, 0.0f);
        // texture coordinates
        glm::vec2 uv1(0.0f, 1.0f);
        glm::vec2 uv2(0.0f, 0.0f);
        glm::vec2 uv3(1.0f, 0.0f);
        glm::vec2 uv4(1.0f, 1.0f);
        // normal vector
        glm::vec3 nm(0.0f, 0.0f, 1.0f);

        // calculate tangent/bitangent vectors of both triangles
        glm::vec3 tangent1, bitangent1;
        glm::vec3 tangent2, bitangent2;
        // triangle 1
        // ----------
        glm::vec3 edge1 = pos2 - pos1;
        glm::vec3 edge2 = pos3 - pos1;
        glm::vec2 deltaUV1 = uv2 - uv1;
        glm::vec2 deltaUV2 = uv3 - uv1;

        float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        tangent1.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent1.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent1.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
        tangent1 = glm::normalize(tangent1);

        bitangent1.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        bitangent1.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        bitangent1.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
        bitangent1 = glm::normalize(bitangent1);

        // triangle 2
        // ----------
        edge1 = pos3 - pos1;
        edge2 = pos4 - pos1;
        deltaUV1 = uv3 - uv1;
        deltaUV2 = uv4 - uv1;

        f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

        tangent2.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
        tangent2.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
        tangent2.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);
        tangent2 = glm::normalize(tangent2);


        bitangent2.x = f * (-deltaUV2.x * edge1.x + deltaUV1.x * edge2.x);
        bitangent2.y = f * (-deltaUV2.x * edge1.y + deltaUV1.x * edge2.y);
        bitangent2.z = f * (-deltaUV2.x * edge1.z + deltaUV1.x * edge2.z);
        bitangent2 = glm::normalize(bitangent2);


        float quadVertices[] = {
                // positions            // normal         // texcoords  // tangent                          // bitangent
                pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
                pos2.x, pos2.y, pos2.z, nm.x, nm.y, nm.z, uv2.x, uv2.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,
                pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent1.x, tangent1.y, tangent1.z, bitangent1.x, bitangent1.y, bitangent1.z,

                pos1.x, pos1.y, pos1.z, nm.x, nm.y, nm.z, uv1.x, uv1.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
                pos3.x, pos3.y, pos3.z, nm.x, nm.y, nm.z, uv3.x, uv3.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z,
                pos4.x, pos4.y, pos4.z, nm.x, nm.y, nm.z, uv4.x, uv4.y, tangent2.x, tangent2.y, tangent2.z, bitangent2.x, bitangent2.y, bitangent2.z
        };
        // configure plane VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(8 * sizeof(float)));
        glEnableVertexAttribArray(4);
        glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)(11 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
}

unsigned int quadVAO1 = 0;
unsigned int quadVBO1;
void renderQuadForBloom()
{
    if (quadVAO1 == 0)
    {
        float quadVertices[] = {
                // positions        // texture Coords
                -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
                -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
                1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
                1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup plane VAO
        glGenVertexArrays(1, &quadVAO1);
        glGenBuffers(1, &quadVBO1);
        glBindVertexArray(quadVAO1);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO1);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO1);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}
