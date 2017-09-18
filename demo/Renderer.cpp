/*
* Copyright (C) 2017 by Godlike
* This code is licensed under the MIT license (MIT)
* (http://opensource.org/licenses/MIT)
*/
#include <pegasus/Math.hpp>
#include <demo/Renderer.hpp>

#include <glbinding/Binding.h>

using namespace pegasus;
using namespace render;

void mesh::Allocate(Mesh& mesh)
{
	//Generate VBO and EBO
    glGenVertexArrays(1, &mesh.bufferData.vertexArrayObject);
    glGenBuffers(1, &mesh.bufferData.vertexBufferObject);
    glGenBuffers(1, &mesh.bufferData.elementBufferObject);

    //Initialize VBO and EBO
    glBindVertexArray(mesh.bufferData.vertexArrayObject);

    glBindBuffer(GL_ARRAY_BUFFER, mesh.bufferData.vertexBufferObject);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLdouble) * mesh.vertices.size(), &mesh.vertices.front(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.bufferData.elementBufferObject);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * mesh.indices.size(), &mesh.indices.front(), GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_DOUBLE, GL_FALSE, sizeof(GLdouble) * 3, nullptr);

    glBindVertexArray(0);
}

void mesh::Deallocate(Mesh& mesh)
{
    glDeleteVertexArrays(1, &mesh.bufferData.vertexArrayObject);
    glDeleteBuffers(1, &mesh.bufferData.vertexBufferObject);
    glDeleteBuffers(1, &mesh.bufferData.elementBufferObject);
}

mesh::Mesh mesh::Create(std::vector<GLdouble>&& vertices, std::vector<GLuint>&& indices)
{
    Mesh mesh{ std::move(vertices), std::move(indices) };
    Allocate(mesh);
    return mesh;
}

mesh::Mesh mesh::CreatePlane(glm::dvec3 normal, double distance, double length)
{
	Mesh mesh;

    glm::dvec3 const i = math::CalculateOrthogonalVector(normal) * (length / 2.0);
    glm::dvec3 const j = glm::normalize(glm::cross(i, normal)) * (length / 2.0);
    glm::dvec3 const k = normal * distance;

	mesh.vertices = {{
        ( i + j + k).x, ( i + j + k).y, ( i + j + k).z,
        (-i + j + k).x, (-i + j + k).y, (-i + j + k).z,
        ( i - j + k).x, ( i - j + k).y, ( i - j + k).z,
        (-i - j + k).x, (-i - j + k).y, (-i - j + k).z,
	}};
	mesh.indices = {{ 0, 1, 2, 1, 2, 3 }};
	Allocate(mesh);

	return mesh;
}

mesh::Mesh mesh::CreateSphere(double radius, uint32_t depth)
{
    //Initial hexahedron
    Mesh mesh;
    mesh.vertices = {{
         0, 0, radius, 0, radius, 0, radius, 0, 0, 0, -radius, 0, -radius, 0, 0, 0, 0, -radius,
    }};
    mesh.indices = {{
        0, 1, 2, 0, 2, 3, 0, 3, 4, 0, 1, 4,
        1, 2, 5, 2, 3, 5, 3, 4, 5, 1, 4, 5,
    }};

    //Subdevide
    for(uint32_t i = 0; i < depth; ++i)
    {
        for (int64_t j = mesh.indices.size() - 3; j >= 0; j-=3)
        {
            glm::u32vec3 indices(mesh.indices[j], mesh.indices[j + 1], mesh.indices[j + 2]);
            glm::dvec3 const a(mesh.vertices[indices[0]*3],mesh.vertices[indices[0]*3+1], mesh.vertices[indices[0]*3+2]);
            glm::dvec3 const b(mesh.vertices[indices[1]*3],mesh.vertices[indices[1]*3+1], mesh.vertices[indices[1]*3+2]);
            glm::dvec3 const c(mesh.vertices[indices[2]*3],mesh.vertices[indices[2]*3+1], mesh.vertices[indices[2]*3+2]);
            glm::dvec3 const ab = glm::normalize((a + b) / 2.0) * radius;
            glm::dvec3 const bc = glm::normalize((b + c) / 2.0) * radius;
            glm::dvec3 const ca = glm::normalize((c + a) / 2.0) * radius;

            GLuint const index = static_cast<GLuint>(mesh.vertices.size()) / 3 - 1;
            mesh.indices.insert(mesh.indices.end(), {
                indices[0], index + 1, index + 3,
                indices[1], index + 1, index + 2,
                indices[2], index + 2, index + 3,
                index + 1, index + 2, index + 3
            });
            mesh.indices.erase(mesh.indices.begin() + j, mesh.indices.begin() + j + 3);

            mesh.vertices.insert(mesh.vertices.end(), { ab.x, ab.y, ab.z, bc.x, bc.y, bc.z, ca.x, ca.y, ca.z });
        }
    }
    Allocate(mesh);

    return mesh;
}

mesh::Mesh mesh::CreateBox(glm::dvec3 i, glm::dvec3 j, glm::dvec3 k)
{
    Mesh mesh;
    mesh.vertices = {{
        ( i + j + k).x, ( i + j + k).y, ( i + j + k).z,
        (-i + j + k).x, (-i + j + k).y, (-i + j + k).z,
        ( i - j + k).x, ( i - j + k).y, ( i - j + k).z,
        (-i - j + k).x, (-i - j + k).y, (-i - j + k).z,
        ( i + j - k).x, ( i + j - k).y, ( i + j - k).z,
        (-i + j - k).x, (-i + j - k).y, (-i + j - k).z,
        ( i - j - k).x, ( i - j - k).y, ( i - j - k).z,
        (-i - j - k).x, (-i - j - k).y, (-i - j - k).z,
    }};
    mesh.indices = {{
        1, 2, 0, 1, 2, 3,
        1, 4, 0, 1, 4, 5,
        2, 4, 0, 2, 4, 6,
        1, 7, 3, 1, 7, 5,
        2, 7, 3, 2, 7, 6,
        4, 7, 5, 4, 7, 6,
    }};
    Allocate(mesh);

    return mesh;
}

void mesh::Delete(Mesh& mesh)
{
    std::vector<GLuint>().swap(mesh.indices);
    std::vector<GLdouble>().swap(mesh.vertices);
    mesh.model = glm::dmat4();
    Deallocate(mesh);
}

shader::Shader shader::CompileShader(GLenum type, const GLchar sources[1])
{
    Shader result{glCreateShader(type), type};

    glShaderSource(result.handle, 1, &sources, nullptr);
    glCompileShader(result.handle);

    GLint succes = 0;
    glGetShaderiv(result.handle, GL_COMPILE_STATUS, &succes);
    result.valid = static_cast<bool>(succes);

    if (!result.valid)
    {
        result.info.resize(512);
        glGetShaderInfoLog(result.handle, result.info.size(), nullptr, &result.info.front());
    }

    return result;
}

void shader::DeleteShader(Shader const& shader)
{
    glDeleteShader(shader.handle);
}

shader::Program shader::MakeProgram(Program::Handles shaders)
{
    Program result{glCreateProgram(), shaders};

    glAttachShader(result.handle, shaders.vertexShader);
    if (shaders.tesselationControlShader) {
        glAttachShader(result.handle, shaders.tesselationControlShader);
    }
    if (shaders.tesselationEvaluationShader) {
        glAttachShader(result.handle, shaders.tesselationEvaluationShader);
    }
    if (shaders.geometryShader) {
        glAttachShader(result.handle, shaders.geometryShader);
    }
    glAttachShader(result.handle, shaders.fragmentShader);

    glLinkProgram(result.handle);

    GLint succes = 0;
    glGetProgramiv(result.handle, GL_LINK_STATUS, &succes);
    result.valid = static_cast<bool>(succes);

    if (!result.valid)
    {
        result.info.resize(512);
        glGetProgramInfoLog(result.handle, result.info.size(), nullptr, &result.info.front());
    }

    return result;
}

Camera::Camera()
    : speed(0.2f)
    , m_ratio(1)
    , m_position(0, 0, 0)
    , m_direction(1, 0, 0)
    , m_up(0, 1, 0)
    , m_view(1)
    , m_projection(1)
{
    UpdateView();
    UpdateProjection();
}

void Camera::SetRatio(float ratio)
{
    m_ratio = ratio;
    UpdateProjection();
}

void Camera::SetPosition(glm::vec3 position)
{
    m_position = position;
    UpdateView();
}

void Camera::SetDirection(glm::vec3 direction)
{
    m_direction = direction;
    UpdateView();
}

void Camera::SetUp(glm::vec3 up)
{
    m_up = up;
    UpdateView();
}

glm::vec3 Camera::GetPosition() const
{
    return m_position;
}

glm::vec3 Camera::GetDirection() const
{
    return m_direction;
}

glm::vec3 Camera::GetUp() const
{
    return m_up;
}

glm::mat4 Camera::GetView() const
{
    return m_view;
}

glm::mat4 Camera::GetProjection() const
{
    return m_projection;
}

void Camera::UpdateView()
{
    m_view = glm::lookAt(m_position, m_position + m_direction, m_up);
}

void Camera::UpdateProjection()
{
    m_projection = glm::perspective(glm::radians(90.0f), m_ratio, 0.1f, 100.0f);
}

Renderer& Renderer::GetInstance()
{
    static Renderer instace;
    return instace;
}

bool Renderer::IsValid() const
{
    return m_initialized && (GLFW_FALSE == glfwWindowShouldClose(m_window.pWindow));
}

void Renderer::RenderFrame()
{
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(m_program.handle);

    for (asset::Asset<mesh::Mesh>& mesh : m_meshes)
    {
        glBindVertexArray(mesh.data.bufferData.vertexArrayObject);
        glm::mat4 const mvp = m_camera.GetProjection() * m_camera.GetView() * mesh.data.model;
        glUniformMatrix4fv(m_modelUniformHandle, 1, GL_FALSE, glm::value_ptr(mvp));
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDrawElements(GL_TRIANGLES, mesh.data.indices.size(), GL_UNSIGNED_INT, nullptr);
    }

    glfwSwapBuffers(m_window.pWindow);
    glfwPollEvents();
}

Handle Renderer::MakeMesh()
{
    return asset::Make(m_meshes);
}

mesh::Mesh& Renderer::GetMesh(Handle id)
{
    return asset::Get(m_meshes, id);
}

void Renderer::RemoveMesh(Handle id)
{
    asset::Remove(m_meshes, id);
}

Renderer::Renderer()
    : m_initialized(false)
{
    InitializeGlfw();
    InitializeContext();
    InitializeCallbacks();
    InitializeShaderProgram();
}

Renderer::~Renderer()
{
    glfwTerminate();
}

void Renderer::InitializeGlfw()
{
    m_initialized = (GLFW_TRUE == glfwInit());
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4);
}

void Renderer::InitializeContext()
{
    m_window.pWindow = glfwCreateWindow(m_window.windowWidth, m_window.windowHeight, "Pegasus", nullptr, nullptr);
    glfwMakeContextCurrent(m_window.pWindow);
    glfwGetFramebufferSize(m_window.pWindow, &m_window.frameBufferWidth, &m_window.frameBufferHeight);
    glbinding::Binding::initialize();
    glViewport(0, 0, m_window.frameBufferWidth, m_window.frameBufferHeight);
}

void Renderer::InitializeCallbacks() const
{
    glfwSetWindowSizeCallback(m_window.pWindow, &Renderer::Resize);
    glfwSetKeyCallback(m_window.pWindow, &Renderer::KeyButton);
    glfwSetCursorPosCallback(m_window.pWindow, &Renderer::CursoreMove);
    glfwSetMouseButtonCallback(m_window.pWindow, &Renderer::MouseButton);
}

void Renderer::InitializeShaderProgram()
{
    shader::Shader const vertexShader{
        shader::CompileShader(GL_VERTEX_SHADER, m_pVertexShaderSources)
    };
    shader::Shader const fragmentShader{
        shader::CompileShader(GL_FRAGMENT_SHADER, m_pFragmentShaderSources)
    };
    if (!vertexShader.valid || !fragmentShader.valid)
    {
        m_initialized = false;
    }

    shader::Program::Handles const shaders{
        vertexShader.handle, 0, 0, 0, fragmentShader.handle
    };
    m_program = shader::MakeProgram(shaders);
    m_modelUniformHandle = glGetUniformLocation(m_program.handle, "model");
    if (-1 == m_modelUniformHandle)
    {
        m_initialized = false;
    }
}

void Renderer::Resize(GLFWwindow* window, int width, int height)
{
    Renderer& renderer = GetInstance();

    renderer.m_window.windowHeight = height;
    renderer.m_window.windowWidth = width;
    glfwSetWindowSize(window, width, height);
    glfwGetFramebufferSize(window, &renderer.m_window.frameBufferWidth, &renderer.m_window.frameBufferHeight);
    glViewport(0, 0, renderer.m_window.frameBufferWidth, renderer.m_window.frameBufferHeight);
    renderer.m_camera.SetRatio(float(renderer.m_window.frameBufferWidth) / float(renderer.m_window.frameBufferHeight));
}

void Renderer::KeyButton(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    Camera& camera = GetInstance().m_camera;
    glm::vec3 const left = glm::normalize(glm::cross(camera.GetUp(), camera.GetDirection()));
    glm::vec3 const up = glm::normalize(glm::cross(camera.GetDirection(), left));
    GLint const nextCursorMode = 
        (glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED)
            ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED;

    switch (key)
    {
        case GLFW_KEY_W:
            if (action == GLFW_PRESS || action == GLFW_REPEAT) {
                camera.SetPosition(camera.GetPosition() + (camera.GetDirection() * camera.speed));
            }
            break;
        case GLFW_KEY_S:
            if (action == GLFW_PRESS || action == GLFW_REPEAT) {
                camera.SetPosition(camera.GetPosition() + (-camera.GetDirection() * camera.speed));
            }
            break;
        case GLFW_KEY_D:
            if (action == GLFW_PRESS || action == GLFW_REPEAT) {
                camera.SetPosition(camera.GetPosition() + (-left * camera.speed));
            }
            break;
        case GLFW_KEY_A:
            if (action == GLFW_PRESS || action == GLFW_REPEAT) {
                camera.SetPosition(camera.GetPosition() + (left * camera.speed));
            }
            break;
        case GLFW_KEY_SPACE:
            if (action == GLFW_PRESS || action == GLFW_REPEAT) {
                camera.SetPosition(camera.GetPosition() + (up * camera.speed));
            }
            break;
        case GLFW_KEY_LEFT_SHIFT:
            if (action == GLFW_PRESS || action == GLFW_REPEAT) {
                camera.SetPosition(camera.GetPosition() + (-up * camera.speed));
            }
            break;
        case GLFW_KEY_R:
            camera.SetDirection(glm::vec3(1, 0, 0));
            break;
        case GLFW_KEY_C:
            if (action == GLFW_RELEASE) {
                glfwSetInputMode(window, GLFW_CURSOR, nextCursorMode);
            }
            break;
        default:
            break;
    }
}

void Renderer::CursoreMove(GLFWwindow* window, double xpos, double ypos)
{
    Camera& camera = GetInstance().m_camera;

    static double lastX = 0;
    static double lastY = 0;

    float const sensitivity = 0.1;
    float const xoffset = (xpos - lastX) * sensitivity;
    float const yoffset = (lastY - ypos) * sensitivity;
    lastX = xpos;
    lastY = ypos;

    static double yaw = 0;
    static double pitch = 0;
    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f) {
        pitch = 89.0f;
    }
    if (pitch < -89.0f) {
        pitch = -89.0f;
    }

    glm::vec3 const direction(
        glm::cos(glm::radians(yaw)) * glm::cos(glm::radians(pitch)),
        glm::sin(glm::radians(pitch)),
        glm::sin(glm::radians(yaw)) * glm::cos(glm::radians(pitch))
    );

    camera.SetDirection(glm::normalize(direction));
}

void Renderer::MouseButton(GLFWwindow* window, int button, int action, int mods)
{
    Controls& controls = GetInstance().m_controls;

    switch (button)
    {
        case GLFW_MOUSE_BUTTON_LEFT:
            controls.leftMousePressed = GLFW_RELEASE != action;
            break;
        case GLFW_MOUSE_BUTTON_RIGHT:
            controls.rightMousePressed = GLFW_RELEASE != action;
            break;
        default:
            break;
    }
}

primitive::Plane::Plane(Renderer& renderer, glm::mat4 model, glm::dvec3 normal, double distance)
    : m_isInitialized(true)
    , m_pRenderer(&renderer)
    , m_handle(m_pRenderer->MakeMesh())
    , m_normal(normal)
    , m_distance(distance)
    , m_sideLength(1.0)
{
    mesh::Mesh& mesh = m_pRenderer->GetMesh(m_handle);
    mesh = mesh::CreatePlane(m_normal, m_distance, m_sideLength);
    mesh.model = model;
}

primitive::Plane::Plane(Plane&& other) noexcept
{
    *this = std::move(other);
}

primitive::Plane& primitive::Plane::operator=(Plane&& other) noexcept
{
    swap(*this, other);
    other.m_isInitialized = false;
    return *this;
}

primitive::Plane::~Plane()
{
    if (m_isInitialized)
    {
        mesh::Mesh& mesh = m_pRenderer->GetMesh(m_handle);
        mesh::Delete(mesh);
        m_pRenderer->RemoveMesh(m_handle);
    }
}

void primitive::swap(Plane& lhs, Plane& rhs) noexcept
{
    std::swap(lhs.m_isInitialized, rhs.m_isInitialized);
    std::swap(lhs.m_pRenderer, rhs.m_pRenderer);
    std::swap(lhs.m_handle, rhs.m_handle);
    std::swap(lhs.m_normal, rhs.m_normal);
    std::swap(lhs.m_distance, rhs.m_distance);
    std::swap(lhs.m_sideLength, rhs.m_sideLength);
}

glm::dvec3 primitive::Plane::GetNormal() const
{
    return m_normal;
}

double primitive::Plane::GetDistance() const
{
    return m_distance;
}

void primitive::Plane::SetModel(glm::mat4 const& model) const
{
    mesh::Mesh& mesh = m_pRenderer->GetMesh(m_handle);
    mesh.model = model;
}

glm::mat4 primitive::Plane::GetModel() const
{
    mesh::Mesh const& mesh = m_pRenderer->GetMesh(m_handle);
    return mesh.model;
}

primitive::Sphere::Sphere(Renderer& renderer, glm::mat4 model, double radius, glm::dvec3 color)
    : m_isInitialized(true)
    , m_pRenderer(&renderer)
    , m_handle(m_pRenderer->MakeMesh())
    , m_radius(radius)
{
    mesh::Mesh& mesh = m_pRenderer->GetMesh(m_handle);
    mesh = mesh::CreateSphere(m_radius, 3);
    mesh.model = model;
}

primitive::Sphere::Sphere(Sphere&& other) noexcept
{
    *this = std::move(other);
}

primitive::Sphere& primitive::Sphere::operator=(Sphere&& other) noexcept
{
    swap(*this, other);
    other.m_isInitialized = false;
    return *this;
}

primitive::Sphere::~Sphere()
{
    if (m_isInitialized)
    {
        mesh::Mesh& mesh = m_pRenderer->GetMesh(m_handle);
        mesh::Delete(mesh);
        m_pRenderer->RemoveMesh(m_handle);
    }
}

void primitive::swap(Sphere& lhs, Sphere& rhs) noexcept
{
    std::swap(lhs.m_isInitialized, rhs.m_isInitialized);
    std::swap(lhs.m_pRenderer, rhs.m_pRenderer);
    std::swap(lhs.m_handle, rhs.m_handle);
    std::swap(lhs.m_radius, rhs.m_radius);
}

double primitive::Sphere::GetRadius() const
{
    return m_radius;
}

void primitive::Sphere::SetModel(glm::mat4 const& model) const
{
    mesh::Mesh& mesh = m_pRenderer->GetMesh(m_handle);
    mesh.model = model;
}

glm::mat4 primitive::Sphere::GetModel() const
{
    mesh::Mesh const& mesh = m_pRenderer->GetMesh(m_handle);
    return mesh.model;
}

primitive::Box::Box(Renderer& renderer, glm::mat4 model, Axes axes, glm::dvec3 color)
    : m_isInitialized(true)
    , m_pRenderer(&renderer)
    , m_handle(m_pRenderer->MakeMesh())
{
    mesh::Mesh& mesh = m_pRenderer->GetMesh(m_handle);
    mesh = mesh::CreateBox(axes.i, axes.j, axes.k);
    mesh.model = model;
}

primitive::Box::Box(Box&& other) noexcept
{
    *this = std::move(other);
}

primitive::Box& primitive::Box::operator=(Box&& other) noexcept
{
    swap(*this, other);
    other.m_isInitialized = false;
    return *this;
}

primitive::Box::~Box()
{
    if (m_isInitialized)
    {
        mesh::Mesh& mesh = m_pRenderer->GetMesh(m_handle);
        mesh::Delete(mesh);
        m_pRenderer->RemoveMesh(m_handle);
    }
}

void primitive::swap(Box& lhs, Box& rhs) noexcept
{
    std::swap(lhs.m_isInitialized, rhs.m_isInitialized);
    std::swap(lhs.m_pRenderer, rhs.m_pRenderer);
    std::swap(lhs.m_handle, rhs.m_handle);
    std::swap(lhs.m_axes, rhs.m_axes);
}

primitive::Box::Axes primitive::Box::GetAxes() const
{
    return m_axes;
}

void primitive::Box::SetModel(glm::mat4 const& model) const
{
    mesh::Mesh& mesh = m_pRenderer->GetMesh(m_handle);
    mesh.model = model;
}

glm::mat4 primitive::Box::GetModel() const
{
	mesh::Mesh const& mesh = m_pRenderer->GetMesh(m_handle);
	return mesh.model;
}
