// =============================================================================
// CST-310 · Topic 6 · topic_06_sphere_mesh
//
// What this shows
// ---------------
//   A procedurally generated sphere mesh, lit with Blinn-Phong shading. The
//   sphere's resolution (latitude/longitude divisions) is keyboard-controlled
//   1..9. Watch the silhouette get rounder as resolution increases.
//
//   The mesh is built in main() at startup using lat/long parameterization.
//   Each vertex carries a position and a normal; for a sphere centered at
//   the origin, the normal is just the normalized position vector.
// =============================================================================

#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <cmath>

static const int   WW = 900;
static const int   WH = 700;
static const char* TITLE = "CST-310 · Topic 6 · Sphere Mesh";

static float g_yaw = 0.4f, g_pitch = 0.3f, g_dist = 3.5f;
static bool g_drag = false; static double g_lastX = 0, g_lastY = 0;
static int g_subdiv = 4;       // 1..9 → 8..72 segments along longitude
static bool g_wireframe = false;
static GLuint g_vao = 0, g_vbo = 0, g_ebo = 0;
static GLsizei g_indexCount = 0;

// -----------------------------------------------------------------------------
// Build a sphere mesh by lat/long parameterization.
//
// stacks  = number of horizontal rings (along latitude). 1 stack = pole pair.
// slices  = number of vertical wedges (along longitude). 1 slice = a sliver.
//
// Each vertex is written as 6 floats: x, y, z, nx, ny, nz.
// Triangles are formed by connecting (stack, slice) → (stack, slice+1)
//                                      → (stack+1, slice) → (stack+1, slice+1)
// -----------------------------------------------------------------------------
static void build_sphere(int stacks, int slices, std::vector<float>& verts, std::vector<unsigned int>& idx) {
    verts.clear(); idx.clear();
    const float PI = 3.14159265358979323846f;

    for(int i = 0; i <= stacks; ++i) {
        float phi = PI * float(i) / stacks;       // 0 .. PI from north to south pole
        float sphi = std::sin(phi), cphi = std::cos(phi);
        for(int j = 0; j <= slices; ++j) {
            float theta = 2.0f * PI * float(j) / slices;
            float stheta = std::sin(theta), ctheta = std::cos(theta);
            float x = sphi * ctheta;
            float y = cphi;
            float z = sphi * stheta;
            // Unit sphere centered at origin: normal == position.
            verts.push_back(x); verts.push_back(y); verts.push_back(z);
            verts.push_back(x); verts.push_back(y); verts.push_back(z);
        }
    }
    int cols = slices + 1;
    for(int i = 0; i < stacks; ++i) {
        for(int j = 0; j < slices; ++j) {
            unsigned int a = i * cols + j;
            unsigned int b = (i + 1) * cols + j;
            unsigned int c = (i + 1) * cols + (j + 1);
            unsigned int d = i * cols + (j + 1);
            idx.push_back(a); idx.push_back(b); idx.push_back(c);
            idx.push_back(a); idx.push_back(c); idx.push_back(d);
        }
    }
}

static void rebuild_mesh_buffers() {
    int n = g_subdiv * 8;          // 8..72 slices
    int stacks = n / 2;            // half as many stacks as slices
    if(stacks < 3) stacks = 3;
    std::vector<float> verts;
    std::vector<unsigned int> idx;
    build_sphere(stacks, n, verts, idx);
    g_indexCount = static_cast<GLsizei>(idx.size());
    glBindVertexArray(g_vao);
    glBindBuffer(GL_ARRAY_BUFFER, g_vbo);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(float), verts.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, idx.size() * sizeof(unsigned int), idx.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    std::cout<<"subdiv="<<g_subdiv<<"  ("<<n<<" slices, "<<stacks<<" stacks, "<<g_indexCount/3<<" triangles)\n";
}

static std::string read_file(const std::string& p){ std::ifstream f(p); std::stringstream s; s<<f.rdbuf(); return s.str(); }
static GLuint compile(GLenum k,const std::string& src){ GLuint id=glCreateShader(k); const char* c=src.c_str(); glShaderSource(id,1,&c,nullptr); glCompileShader(id); GLint ok; glGetShaderiv(id,GL_COMPILE_STATUS,&ok); if(!ok){char L[2048]; glGetShaderInfoLog(id,2048,nullptr,L); std::cerr<<L<<"\n"; return 0;} return id; }
static GLuint link_prog(GLuint a,GLuint b){ GLuint p=glCreateProgram(); glAttachShader(p,a); glAttachShader(p,b); glLinkProgram(p); GLint ok; glGetProgramiv(p,GL_LINK_STATUS,&ok); if(!ok){char L[2048]; glGetProgramInfoLog(p,2048,nullptr,L); std::cerr<<L<<"\n"; return 0;} return p; }
static void fb_cb(GLFWwindow*,int w,int h){ glViewport(0,0,w,h); }
static void mb_cb(GLFWwindow* w,int b,int act,int){ if(b==GLFW_MOUSE_BUTTON_LEFT){ g_drag=(act==GLFW_PRESS); if(g_drag) glfwGetCursorPos(w,&g_lastX,&g_lastY);}}
static void cur_cb(GLFWwindow*,double x,double y){ if(!g_drag) return; double dx=x-g_lastX, dy=y-g_lastY; g_lastX=x; g_lastY=y; g_yaw+=static_cast<float>(dx)*0.008f; g_pitch+=static_cast<float>(dy)*0.008f; if(g_pitch>1.4f) g_pitch=1.4f; if(g_pitch<-1.4f) g_pitch=-1.4f; }
static void sc_cb(GLFWwindow*,double,double y){ g_dist*=(y>0?0.9f:1.1f); if(g_dist<1.5f) g_dist=1.5f; if(g_dist>10.0f) g_dist=10.0f; }
static void key_cb(GLFWwindow* w,int k,int,int act,int){
    if(act != GLFW_PRESS) return;
    if(k == GLFW_KEY_ESCAPE) glfwSetWindowShouldClose(w, GL_TRUE);
    if(k == GLFW_KEY_W){ g_wireframe = !g_wireframe; glPolygonMode(GL_FRONT_AND_BACK, g_wireframe ? GL_LINE : GL_FILL); }
    if(k >= GLFW_KEY_1 && k <= GLFW_KEY_9){ g_subdiv = k - GLFW_KEY_0; rebuild_mesh_buffers(); }
}

int main(){
    if(!glfwInit()){ return 1; }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    GLFWwindow* win = glfwCreateWindow(WW, WH, TITLE, nullptr, nullptr);
    if(!win){ glfwTerminate(); return 1; }
    glfwMakeContextCurrent(win);
    glfwSetFramebufferSizeCallback(win, fb_cb);
    glfwSetKeyCallback(win, key_cb);
    glfwSetMouseButtonCallback(win, mb_cb);
    glfwSetCursorPosCallback(win, cur_cb);
    glfwSetScrollCallback(win, sc_cb);
    glfwSwapInterval(1);

    if(!gladLoadGL((GLADloadfunc)glfwGetProcAddress)){ return 1; }
    std::cout<<"OpenGL "<<glGetString(GL_VERSION)<<"\nGPU:    "<<glGetString(GL_RENDERER)<<"\n";
    std::cout<<"keys 1..9 = resolution, W = wireframe toggle, drag = orbit, Esc = close\n";

    int fbw,fbh; glfwGetFramebufferSize(win,&fbw,&fbh); glViewport(0,0,fbw,fbh);
    glEnable(GL_DEPTH_TEST);

    auto vs = read_file("shaders/sph.vert");
    auto fs = read_file("shaders/sph.frag");
    GLuint v = compile(GL_VERTEX_SHADER, vs);
    GLuint f = compile(GL_FRAGMENT_SHADER, fs);
    GLuint prog = link_prog(v, f);
    glDeleteShader(v); glDeleteShader(f);
    if(!prog){ glfwTerminate(); return 1; }

    GLint loc_m = glGetUniformLocation(prog, "uModel");
    GLint loc_v = glGetUniformLocation(prog, "uView");
    GLint loc_p = glGetUniformLocation(prog, "uProjection");
    GLint loc_nm = glGetUniformLocation(prog, "uNormalMatrix");
    GLint loc_ld = glGetUniformLocation(prog, "uLightDir");
    GLint loc_cp = glGetUniformLocation(prog, "uCamPos");

    glGenVertexArrays(1, &g_vao);
    glGenBuffers(1, &g_vbo);
    glGenBuffers(1, &g_ebo);
    rebuild_mesh_buffers();

    while(!glfwWindowShouldClose(win)){
        glClearColor(0.05f, 0.07f, 0.11f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::vec3 camPos(g_dist*std::cos(g_pitch)*std::sin(g_yaw),
                         g_dist*std::sin(g_pitch),
                         g_dist*std::cos(g_pitch)*std::cos(g_yaw));
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view  = glm::lookAt(camPos, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 proj  = glm::perspective(glm::radians(50.0f),
                                            static_cast<float>(fbw) / fbh,
                                            0.1f, 100.0f);
        glm::mat3 nm = glm::transpose(glm::inverse(glm::mat3(model)));

        float t = static_cast<float>(glfwGetTime());
        glm::vec3 lightDir = glm::normalize(glm::vec3(std::cos(t*0.4f), 0.5f, std::sin(t*0.4f)));

        glUseProgram(prog);
        glUniformMatrix4fv(loc_m, 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(loc_v, 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(loc_p, 1, GL_FALSE, glm::value_ptr(proj));
        glUniformMatrix3fv(loc_nm, 1, GL_FALSE, glm::value_ptr(nm));
        glUniform3fv(loc_ld, 1, glm::value_ptr(lightDir));
        glUniform3fv(loc_cp, 1, glm::value_ptr(camPos));

        glBindVertexArray(g_vao);
        glDrawElements(GL_TRIANGLES, g_indexCount, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(win);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &g_vao);
    glDeleteBuffers(1, &g_vbo);
    glDeleteBuffers(1, &g_ebo);
    glDeleteProgram(prog);
    glfwDestroyWindow(win);
    glfwTerminate();
    return 0;
}
