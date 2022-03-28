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

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
bool cursorEnabled = false;

// camera
Camera camera(glm::vec3(0.0f, 3.0f, 0.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

struct PointLight {
    glm::vec3 position;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

PointLight pointLight;
glm::vec3 backpackPosition = glm::vec3(0.0f);
float backpackScale = 1.0f;

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

    // floor plain coordinates
    float floorVertices[] = {
            // positions          // normals          // texture coords
            0.5f,  0.5f,  0.0f,  0.0f, 0.0f, -1.0f,  1.0f,  1.0f,  // top right
            0.5f, -0.5f,  0.0f,  0.0f, 0.0f, -1.0f,  1.0f,  0.0f,  // bottom right
            -0.5f, -0.5f,  0.0f,  0.0f, 0.0f, -1.0f,  0.0f,  0.0f,  // bottom left
            -0.5f,  0.5f,  0.0f,  0.0f, 0.0f, -1.0f,  0.0f,  1.0f   // top left
    };

    // floor vertices for use in EBO
    unsigned int floorIndices[] = {
            0, 1, 3,  // first Triangle
            1, 2, 3   // second Triangle
    };

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

    // Floor setup
    unsigned int floorVAO, floorVBO, floorEBO;
    glGenVertexArrays(1, &floorVAO);
    glGenBuffers(1, &floorVBO);
    glGenBuffers(1, &floorEBO);

    glBindVertexArray(floorVAO);
    glBindBuffer(GL_ARRAY_BUFFER, floorVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(floorVertices), floorVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, floorEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(floorIndices), floorIndices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6*sizeof(float)));
    glEnableVertexAttribArray(2);

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

    // load textures
    // .............

    unsigned int transparentTexture = loadTexture(FileSystem::getPath("resources/textures/tree.png").c_str());
    unsigned int floorDiffuseMap = loadTexture(FileSystem::getPath("resources/textures/grass/diffuse.png").c_str());
    unsigned int floorSpecularMap = loadTexture(FileSystem::getPath("resources/textures/grass/specular.png").c_str());
    ourShader.use();
    ourShader.setInt("material.texture_diffuse1", 0);
    ourShader.setInt("material.texture_specular1", 1);

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
                    glm::vec3(5.0f, 2.5f, 12.5f),
                    glm::vec3(3.0f, 2.5f, 12.5f),
                    glm::vec3(0.0f, 2.5f, 12.5f),
                    glm::vec3(-3.0f, 2.5f, 12.5f),
                    glm::vec3(-6.0f, 2.5f, 12.5f),
                    glm::vec3(-9.0f, 2.5f, 12.5f),
                    glm::vec3(-12.0f, 2.5f, 12.5f),
                    glm::vec3(-14.0f, 2.5f, 12.5f),
                    glm::vec3(7.0f, 2.5f, -12.5f),
                    glm::vec3(5.0f, 2.5f, -12.5f),
                    glm::vec3(3.0f, 2.5f, -12.5f),
                    glm::vec3(0.0f, 2.5f, -12.5f),
                    glm::vec3(-3.0f, 2.5f, -12.5f),
                    glm::vec3(-6.0f, 2.5f, -12.5f),
                    glm::vec3(-9.0f, 2.5f, -12.5f),
                    glm::vec3(-12.0f, 2.5f, -12.5f),
                    glm::vec3(-14.0f, 2.5f, -12.5f)
            };

    vector<glm::vec3> vegetationRotated =
            {
                    glm::vec3(12.5f, 2.5f, 15.0f),
                    glm::vec3(12.5f, 2.5f, 13.0f),
                    glm::vec3(12.5f, 2.5f, 10.0f),
                    glm::vec3(12.5f, 2.5f, 7.0f),
                    glm::vec3(12.5f, 2.5f, 4.0f),
                    glm::vec3(12.5f, 2.5f, 1.0f),
                    glm::vec3(12.5f, 2.5f, -2.0f),
                    glm::vec3(12.5f, 2.5f, -5.0f),
                    glm::vec3(12.5f, 2.5f, -8.0f),
                    glm::vec3(-12.5f, 2.5f, 15.0f),
                    glm::vec3(-12.5f, 2.5f, 13.0f),
                    glm::vec3(-12.5f, 2.5f, 10.0f),
                    glm::vec3(-12.5f, 2.5f, 7.0f),
                    glm::vec3(-12.5f, 2.5f, 4.0f),
                    glm::vec3(-12.5f, 2.5f, 1.0f),
                    glm::vec3(-12.5f, 2.5f, -2.0f),
                    glm::vec3(-12.5f, 2.5f, -5.0f),
                    glm::vec3(-12.5f, 2.5f, -8.0f)
            };

    // blending shader configuration
    // --------------------
    blendingShader.use();
    blendingShader.setInt("texture1", 0);

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
    // lights configuration
    pointLight.position = glm::vec3(4.0f, 4.0, 0.0);
    pointLight.ambient = glm::vec3(0.1, 0.1, 0.1);
    pointLight.diffuse = glm::vec3(0.6, 0.6, 0.6);
    pointLight.specular = glm::vec3(1.0, 1.0, 1.0);

    pointLight.constant = 1.0f;
    pointLight.linear = 0.09f;
    pointLight.quadratic = 0.012f;



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

        // don't forget to enable shader before setting uniforms
        ourShader.use();
        pointLight.position = glm::vec3(0.0, 2.0f, 0.0);
        ourShader.setVec3("pointLight.position", pointLight.position);
        ourShader.setVec3("pointLight.ambient", pointLight.ambient);
        ourShader.setVec3("pointLight.diffuse", pointLight.diffuse);
        ourShader.setVec3("pointLight.specular", pointLight.specular);
        ourShader.setFloat("pointLight.constant", pointLight.constant);
        ourShader.setFloat("pointLight.linear", pointLight.linear);
        ourShader.setFloat("pointLight.quadratic", pointLight.quadratic);
        ourShader.setVec3("viewPosition", camera.Position);
        ourShader.setFloat("material.shininess", 32.0f);

        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom),
                                                (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        ourShader.setMat4("projection", projection);
        ourShader.setMat4("view", view);

        // render the loaded models

        // ufo model
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model,
                               glm::vec3(10 * cos(glfwGetTime()/2), 7.0f, 10 * sin(glfwGetTime()/2))); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(0.05f));    // it's a bit too big for our scene, so scale it down
        ourShader.setMat4("model", model);
        ufoModel.Draw(ourShader);

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
        model = glm::translate(model, glm::vec3(-8.0f, 0.0f, 5.0f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, glm::vec3(0.009f));
        ourShader.setMat4("model", model);
        humanModel.Draw(ourShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-6.0f, 0.0f, 5.0f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(135.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        model = glm::scale(model, glm::vec3(0.009f));
        ourShader.setMat4("model", model);
        humanModel.Draw(ourShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-6.0f, 0.0f, -5.0f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.009f));
        ourShader.setMat4("model", model);
        humanModel.Draw(ourShader);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(-8.0f, 0.0f, -5.0f));
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

        for(unsigned int i = 0; i < fencesRotated.size(); i ++){
            model = glm::mat4(1.0f);
            model = glm::translate(model, fencesRotated[i]);
            model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            model = glm::scale(model, glm::vec3(0.8f));
            ourShader.setMat4("model", model);
            fenceModel.Draw(ourShader);
        }

        // TODO: add gate closing/opening on some key
        // closed coords: (swapped)
        // glm::vec3(7.12f, 0.0f, -0.4f),
        // glm::vec3(7.12f, 0.0f, 0.95f),
        // closed angle: 90
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

        // material properties
        ourShader.setFloat("material.shininess", 256.0f);

        // world transformation
        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::scale(model, glm::vec3(25.0f));
        ourShader.setMat4("model", model);

        // bind diffuse map
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, floorDiffuseMap);

        // bind specular map
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, floorSpecularMap);

        // render floor
        glBindVertexArray(floorVAO);
        glEnable(GL_CULL_FACE);     // floor won't be visible if looked from bellow
        glCullFace(GL_BACK);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
        glDisable(GL_CULL_FACE);


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