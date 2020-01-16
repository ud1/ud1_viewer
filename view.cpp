#include <GL/glew.h>
#include "nanovg.h"
#define NANOVG_GL3_IMPLEMENTATION
#include "nanovg.h"
#include "nanovg_gl.h"

#include "view.h"
#include <QWheelEvent>
#include <sstream>
#include <QDebug>

#include "shader.h"
#include "camera.h"
#include <set>
#include <QTimer>
#include <ctime>

bool yIsUp = true;

NVGcolor toColor(uint32_t color) {
    return nvgRGBA((color & 0xFF000000) >> 24, (color & 0x00FF0000) >> 16, (color & 0x0000FF00) >> 8, color & 0x000000FF);
}

struct MeshData {
    GLuint vao = 0;
    GLuint quadBufferObject = 0;

    std::vector<float> data;
    GLenum usage = GL_STATIC_DRAW;

    void buildVao()
    {
        if (vao == 0)
           glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        if (quadBufferObject == 0)
            glGenBuffers(1, &quadBufferObject);
        glBindBuffer(GL_ARRAY_BUFFER, quadBufferObject);


        glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), data.data(), usage); //formatting the data for the buffer
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glBindVertexArray(0);
    }

    void pushV(const V3 &v)
    {
        data.push_back(v.x);
        data.push_back(v.y);
        data.push_back(v.z);
    }
};

struct GridData: MeshData
{
    void init(const P &minP, const P &maxP, double hMin, double hMax, double /*cellSize*/)
    {
        double z = 0.0;
        if (hMin > 0.0)
            z = hMin;

        pushV(V3(minP.x, z, minP.y));
        pushV(V3(minP.x, z, maxP.y));

        pushV(V3(minP.x, z, maxP.y));
        pushV(V3(maxP.x, z, maxP.y));

        pushV(V3(maxP.x, z, maxP.y));
        pushV(V3(maxP.x, z, minP.y));

        pushV(V3(maxP.x, z, minP.y));
        pushV(V3(minP.x, z, minP.y));


        pushV(V3(minP.x, hMin, minP.y));
        pushV(V3(minP.x, hMax, minP.y));

        pushV(V3(minP.x, hMin, maxP.y));
        pushV(V3(minP.x, hMax, maxP.y));

        pushV(V3(maxP.x, hMin, maxP.y));
        pushV(V3(maxP.x, hMax, maxP.y));

        pushV(V3(maxP.x, hMin, minP.y));
        pushV(V3(maxP.x, hMax, minP.y));


        pushV(V3(minP.x, hMax, minP.y));
        pushV(V3(minP.x, hMax, maxP.y));

        pushV(V3(minP.x, hMax, maxP.y));
        pushV(V3(maxP.x, hMax, maxP.y));

        pushV(V3(maxP.x, hMax, maxP.y));
        pushV(V3(maxP.x, hMax, minP.y));

        pushV(V3(maxP.x, hMax, minP.y));
        pushV(V3(minP.x, hMax, minP.y));

        /*if (cellSize > 0)
        {
            for (double x = minP.x + cellSize; x < maxP.x; x += cellSize)
            {
                pushV(V3(x, z, minP.y));
                pushV(V3(x, z, maxP.y));
            }

            for (double y = minP.y + cellSize; y < maxP.y; y += cellSize)
            {
                pushV(V3(minP.x, z, y));
                pushV(V3(maxP.x, z, y));
            }
        }*/

        /*pushV(V3(-3000.0, 0, 0));
        pushV(V3(+3000.0, 0, 0));

        pushV(V3(0, -3000.0, 0));
        pushV(V3(0, +3000.0, 0));

        pushV(V3(0, 0, -3000.0));
        pushV(V3(0, 0, +3000.0));*/

        buildVao();
    }
};

struct DummyPointData: MeshData
{
    void init()
    {
        pushV(V3(0, 0, 0));
        buildVao();
    }
};

struct LineData : MeshData
{
    LineData() {
        usage = GL_STREAM_DRAW;
    }
};

void checkGLError(QString errP)
{
    GLenum err = glGetError();
    if (err != GL_NO_ERROR)
    {
        qDebug() << "GL error " << errP << ": " << err;
    }
}

void setColor(GLint uniform, uint32_t color)
{
    float r = ((color & 0xFF000000) >> 24) / 255.0f;
    float g = ((color & 0x00FF0000) >> 16) / 255.0f;
    float b = ((color & 0x0000FF00) >> 8) / 255.0f;
    float a = (color & 0x000000FF) / 255.0f;

    glUniform4f(uniform, r, g, b, a);
}

float transformY(double y, double h)
{
    if (yIsUp)
        return h - y;

    return y;
}

struct ViewData
{
    ViewData(View *view): view(view), gridShader("gridShader"), sphereShader("sphereShader"), diskShader("diskShader"), rectShader("rectShader"), cylInner025Shader("cylInner025Shader") {}

    View *view;
    NVGcontext* vg = nullptr;

    int wid = 1, heig = 1;
    double zoom = 1.0;
    P zoomCenter = P(0.5, 0.5);
    P mousePos = P(0.0, 0.0);
    P rulerStart;
    bool rulerStarted = false;

    bool mode3d = false;
    Shader gridShader, sphereShader, diskShader, rectShader, cylInner025Shader;
    Camera camera;
    GridData gridData;
    DummyPointData dummyPointData;
    LineData lineData;

    std::set<int> pressedKeys;
    V3 vel = V3(0.0f, 0.0f, 0.0f);

    std::vector<SObj> staticObjects;

    ~ViewData()
    {
        nvgDeleteGL3(vg);
    }

    void initialize()
    {
        glewExperimental = GL_TRUE;
        GLenum glew_status = glewInit();
        if (glew_status != 0)
        {
            qDebug() << "GLEW init ERROR " << glewGetErrorString(glew_status);
            return;
        }

        // Set up the rendering context, load shaders and other resources, etc.:
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

        vg = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES);
        //nvgCreateFont(vg, "sans", "Roboto-Regular.ttf");

        gridShader.buildShaderProgram("://shaders/grid_vs.glsl", "://shaders/grid_fs.glsl");
        sphereShader.buildShaderProgram("://shaders/sphere_vs.glsl", "://shaders/sphere_gs.glsl", "://shaders/sphere_fs.glsl");
        diskShader.buildShaderProgram("://shaders/disk_vs.glsl", "://shaders/disk_gs.glsl", "://shaders/disk_fs.glsl");
        rectShader.buildShaderProgram("://shaders/rect_vs.glsl", "://shaders/rect_gs.glsl", "://shaders/rect_fs.glsl");
        cylInner025Shader.buildShaderProgram("://shaders/cylinder025_vs.glsl", "://shaders/cylinder025_gs.glsl", "://shaders/cylinder025_fs.glsl");

        camera.position = V3(0, 20, -40);
        dummyPointData.init();
    }

    void updateZoom()
    {
        P center = zoomCenter * P(wid, heig);
        P c1 = center - P(wid, heig) * 0.5 / zoom;
        P c2 = center + P(wid, heig) * 0.5 / zoom;

        zoomCenter = (c1 + c2) / 2.0 / P(wid, heig);

        nvgTransform(vg, zoom, 0, 0, zoom, -c1.x * zoom, -c1.y * zoom);
    }

    void setupCommonUniforms(Shader &shader, const M4 &modelMat)
    {
        if (shader.uniforms.count("MVP"))
        {
            M4 MVP = camera.getVP() * modelMat;
            glUniformMatrix4fv(shader.uniforms["MVP"], 1, GL_FALSE, &MVP[0][0]);

            checkGLError("uniform MVP");
        }

        if (shader.uniforms.count("M"))
        {
            glUniformMatrix4fv(shader.uniforms["M"], 1, GL_FALSE, &modelMat[0][0]);

            checkGLError("uniform M");
        }

        if (shader.uniforms.count("MINV"))
        {
            M4 MINV = glm::inverse(modelMat);
            glUniformMatrix4fv(shader.uniforms["MINV"], 1, GL_FALSE, &MINV[0][0]);

            checkGLError("uniform M");
        }

        if (shader.uniforms.count("VP"))
        {
            M4 VP = camera.getVP();
            glUniformMatrix4fv(shader.uniforms["VP"], 1, GL_FALSE, &VP[0][0]);

            checkGLError("uniform VP");
        }

        if (shader.uniforms.count("MV"))
        {
            M4 MV = camera.getMatrix();
            glUniformMatrix4fv(shader.uniforms["MV"], 1, GL_FALSE, &MV[0][0]);

            checkGLError("uniform MV");
        }

        if (shader.uniforms.count("PROJ"))
        {
            M4 proj = camera.getProj();
            glUniformMatrix4fv(shader.uniforms["PROJ"], 1, GL_FALSE, &proj[0][0]);

            checkGLError("uniform PROJ");
        }

        if (shader.uniforms.count("eyePosition"))
        {
            glUniform3fv(shader.uniforms["eyePosition"], 1, &camera.position[0]);

            checkGLError("uniform eyePosition");
        }
    }

    void render(const Frame *frame, int w, int h)
    {
        // Draw the scene:
        glClear(GL_COLOR_BUFFER_BIT);
        glClear(GL_DEPTH_BUFFER_BIT);

        if (!mode3d)
        {
            nvgBeginFrame(vg, wid, heig, 1);
            nvgSave(vg);
            updateZoom();

            nvgStrokeColor(vg, nvgRGB(0,0,0));
            nvgStrokeWidth(vg, 1 / zoom);
            nvgBeginPath(vg);
            nvgMoveTo(vg, 0, 0);
            nvgLineTo(vg, w, 0);
            nvgLineTo(vg, w, h);
            nvgLineTo(vg, 0, h);
            nvgLineTo(vg, 0, 0);
            nvgStroke(vg);

            for (const SObj &obj : staticObjects)
                renderObj2d(obj, h);

            if (frame)
            {
                std::lock_guard<std::recursive_mutex> guard(frame->mutex);

                for (const Obj& obj : frame->objs)
                {
                    for (const auto& p : obj.subObjs)
                    {
                        renderObj2d(p.second, h);
                    }
                }
            }

            if (rulerStarted)
            {

                P curPos = toLocalRelP(mousePos);
                double rad = rulerStart.dist(curPos);

                nvgStrokeColor(vg, toColor(0x000000ff));
                nvgBeginPath(vg);
                nvgEllipse(vg, rulerStart.x, rulerStart.y, rad, rad);
                nvgMoveTo(vg, rulerStart.x, rulerStart.y);
                nvgLineTo(vg, curPos.x, curPos.y);
                nvgStroke(vg);
            }

            nvgEndFrame(vg);
            nvgRestore(vg);
        }
        else
        {
            glDisable(GL_BLEND);
            processInput();

            glEnable(GL_DEPTH_TEST);

            if (gridData.data.size() > 0)
            {
                glBindVertexArray(gridData.vao);
                checkGLError("bind vao");

                glUseProgram(gridShader.program);

                checkGLError("use program");

                setupCommonUniforms(gridShader, M4(1.0f));

                if (gridShader.uniforms.count("color"))
                {
                    setColor(gridShader.uniforms["color"], 0x000000ff);

                    checkGLError("uniform color");
                }

                glDrawArrays(GL_LINES, 0, gridData.data.size() / 3);

                checkGLError("draw");

                glBindVertexArray(0);
            }

            checkGLError("bind vao 0");

            for (const SObj &obj : staticObjects)
                renderObj3d(obj);

            if (frame)
            {
                std::lock_guard<std::recursive_mutex> guard(frame->mutex);

                for (const Obj& obj : frame->objs)
                {
                    for (const auto& p : obj.subObjs)
                    {
                        renderObj3d(p.second);
                    }
                }
            }
        }
    }

    void renderObj2d(const SObj &sobj, double h)
    {
        std::string type = getStr("type", sobj);
        if (type == "circle")
        {
            P p = getP("p", sobj);
            double rad = getDouble("r", sobj);
            uint32_t color = getInt("c", sobj);

            nvgFillColor(vg, toColor(color));
            nvgBeginPath(vg);
            nvgEllipse(vg, p.x, transformY(p.y, h), rad, rad);
            nvgFill(vg);
        }
        if (type == "circumference")
        {
            P p = getP("p", sobj);
            double rad = getDouble("r", sobj);
            uint32_t color = getInt("c", sobj);

            nvgStrokeColor(vg, toColor(color));
            nvgBeginPath(vg);
            nvgEllipse(vg, p.x, transformY(p.y, h), rad, rad);
            nvgStroke(vg);
        }
        else if (type == "line")
        {
            P p1 = getP("p1", sobj);

            uint32_t color = getInt("c", sobj);

            nvgStrokeColor(vg, toColor(color));
            nvgBeginPath(vg);
            nvgMoveTo(vg, p1.x, transformY(p1.y, h));

            for (int i = 2; i < 10000; ++i)
            {
                std::ostringstream oss;
                oss << "p" << i;
                if (sobj.count(oss.str()))
                {
                    P pi = getP(oss.str(), sobj);
                    nvgLineTo(vg, pi.x, transformY(pi.y, h));
                }
                else
                {
                    break;
                }
            }
            nvgStroke(vg);
        }
        else if (type == "poly")
        {
            P p1 = getP("p1", sobj);

            uint32_t color = getInt("c", sobj);

            nvgFillColor(vg, toColor(color));
            nvgBeginPath(vg);
            nvgMoveTo(vg, p1.x, transformY(p1.y, h));

            for (int i = 2; i < 10000; ++i)
            {
                std::ostringstream oss;
                oss << "p" << i;
                if (sobj.count(oss.str()))
                {
                    P pi = getP(oss.str(), sobj);
                    nvgLineTo(vg, pi.x, transformY(pi.y, h));
                }
                else
                {
                    break;
                }
            }
            nvgFill(vg);
        }
        else if (type == "rects")
        {
            uint32_t color = getInt("c", sobj);
            nvgFillColor(vg, toColor(color));
            nvgStrokeColor(vg, toColor(color));

            double hw = getDouble("hw", sobj);
            if (hw == 0)
                hw = 1;

            double dw = getDouble("dw", sobj);
            if (dw == 0)
                dw = 0.45;

            for (int i = 0; i < 10000; ++i)
            {
                std::ostringstream oss;
                oss << "p" << i;
                if (sobj.count(oss.str()))
                {
                    P pi = getP(oss.str(), sobj);

                    nvgBeginPath(vg);
                    nvgMoveTo(vg, (pi.x - 0.5) * hw, transformY((pi.y - 0.5) * hw, h));
                    nvgLineTo(vg, (pi.x + 0.5) * hw, transformY((pi.y - 0.5) * hw, h));
                    nvgLineTo(vg, (pi.x + 0.5) * hw, transformY((pi.y + 0.5) * hw, h));
                    nvgLineTo(vg, (pi.x - 0.5) * hw, transformY((pi.y + 0.5) * hw, h));
                    nvgLineTo(vg, (pi.x - 0.5) * hw, transformY((pi.y - 0.5) * hw, h));
                    nvgStroke(vg);

                    nvgBeginPath(vg);
                    nvgMoveTo(vg, (pi.x - dw) * hw, transformY((pi.y - dw) * hw, h));
                    nvgLineTo(vg, (pi.x + dw) * hw, transformY((pi.y - dw) * hw, h));
                    nvgLineTo(vg, (pi.x + dw) * hw, transformY((pi.y + dw) * hw, h));
                    nvgLineTo(vg, (pi.x - dw) * hw, transformY((pi.y + dw) * hw, h));
                    nvgLineTo(vg, (pi.x - dw) * hw, transformY((pi.y - dw) * hw, h));
                    nvgFill(vg);
                }
                else
                {
                    break;
                }
            }
        }
    }

    void renderObj3d(const SObj &sobj)
    {
        glDisable(GL_POLYGON_OFFSET_FILL);

        std::string type = getStr("type", sobj);
        if (type == "sphere")
        {
            glEnable(GL_CULL_FACE);
            glCullFace(GL_FRONT);

            V3 p = getV3("p", sobj);
            double rad = getDouble("r", sobj);
            uint32_t color = getInt("c", sobj);
            V3 lightPos = getV3("lp", sobj);

            glBindVertexArray(dummyPointData.vao);
            glUseProgram(sphereShader.program);

            setupCommonUniforms(sphereShader, M4(1.0f));

            if (sphereShader.uniforms.count("position"))
            {
                glUniform3fv(sphereShader.uniforms["position"], 1, &p[0]);

                checkGLError("uniform position");
            }

            if (sphereShader.uniforms.count("radius"))
            {
                glUniform1f(sphereShader.uniforms["radius"], rad);

                checkGLError("uniform radius");
            }

            if (sphereShader.uniforms.count("color"))
            {
                setColor(sphereShader.uniforms["color"], color);

                checkGLError("uniform color");
            }

            if (sphereShader.uniforms.count("lightPos"))
            {
                glUniform3fv(sphereShader.uniforms["lightPos"], 1, &lightPos[0]);

                checkGLError("uniform lightPos");
            }

            glDrawArrays(GL_POINTS, 0, 1);

            glBindVertexArray(0);
        }
        else if (type == "disk")
        {
            glDisable(GL_CULL_FACE);
            glEnable(GL_POLYGON_OFFSET_FILL);
            glPolygonOffset(-1, -1);

            V3 p = getV3("p", sobj);
            V3 n = glm::normalize(getV3("n", sobj));
            double rad = getDouble("r", sobj);
            uint32_t color = getInt("c", sobj);

            glBindVertexArray(dummyPointData.vao);
            glUseProgram(diskShader.program);

            setupCommonUniforms(diskShader, M4(1.0f));

            if (diskShader.uniforms.count("position"))
            {
                glUniform3fv(diskShader.uniforms["position"], 1, &p[0]);

                checkGLError("uniform position");
            }

            if (diskShader.uniforms.count("normal"))
            {
                glUniform3fv(diskShader.uniforms["normal"], 1, &n[0]);

                checkGLError("uniform normal");
            }

            if (diskShader.uniforms.count("radius"))
            {
                glUniform1f(diskShader.uniforms["radius"], rad);

                checkGLError("uniform radius");
            }

            if (diskShader.uniforms.count("color"))
            {
                setColor(diskShader.uniforms["color"], color);

                checkGLError("uniform color");
            }

            glDrawArrays(GL_POINTS, 0, 1);

            glBindVertexArray(0);
        }
        else if (type == "rect")
        {
            glDisable(GL_CULL_FACE);

            V3 p = getV3("p", sobj);
            V3 ht1 = getV3("ht1", sobj);
            V3 ht2 = getV3("ht2", sobj);
            V3 gridShift = getV3("gridShift", sobj);
            uint32_t color = getInt("c", sobj);
            V3 lightPos = getV3("lp", sobj);

            glBindVertexArray(dummyPointData.vao);
            glUseProgram(rectShader.program);

            setupCommonUniforms(rectShader, M4(1.0f));

            if (rectShader.uniforms.count("position"))
            {
                glUniform3fv(rectShader.uniforms["position"], 1, &p[0]);

                checkGLError("uniform position");
            }

            if (rectShader.uniforms.count("ht1"))
            {
                glUniform3fv(rectShader.uniforms["ht1"], 1, &ht1[0]);

                checkGLError("uniform ht1");
            }

            if (rectShader.uniforms.count("ht2"))
            {
                glUniform3fv(rectShader.uniforms["ht2"], 1, &ht2[0]);

                checkGLError("uniform ht2");
            }

            if (rectShader.uniforms.count("gridShift"))
            {
                glUniform3fv(rectShader.uniforms["gridShift"], 1, &gridShift[0]);

                checkGLError("uniform gridShift");
            }

            if (rectShader.uniforms.count("color"))
            {
                setColor(rectShader.uniforms["color"], color);

                checkGLError("uniform color");
            }

            if (rectShader.uniforms.count("lightPos"))
            {
                glUniform3fv(rectShader.uniforms["lightPos"], 1, &lightPos[0]);

                checkGLError("uniform lightPos");
            }


            glDrawArrays(GL_POINTS, 0, 1);

            glBindVertexArray(0);
        }
        else if (type == "line3d")
        {
            uint32_t color = getInt("c", sobj);

            lineData.data.clear();

            for (int i = 1; i < 10000; ++i)
            {
                std::ostringstream oss;
                oss << "p" << i;
                if (sobj.count(oss.str()))
                {
                    V3 pi = getV3(oss.str(), sobj);
                    lineData.pushV(pi);
                }
                else
                {
                    break;
                }
            }

            if (!lineData.data.empty())
            {
                lineData.buildVao();

                glBindVertexArray(lineData.vao);
                checkGLError("bind vao");

                glUseProgram(gridShader.program);

                checkGLError("use program");

                setupCommonUniforms(gridShader, M4(1.0f));

                if (gridShader.uniforms.count("color"))
                {
                    setColor(gridShader.uniforms["color"], color);

                    checkGLError("uniform color");
                }

                glDrawArrays(GL_LINE_STRIP, 0, lineData.data.size() / 3);

                checkGLError("draw");

                glBindVertexArray(0);
            }
        }
        else if (type == "cyl_inner_025")
        {
            glEnable(GL_CULL_FACE);
            glCullFace(GL_FRONT);

            uint32_t color = getInt("c", sobj);
            V3 lightPos = getV3("lp", sobj);
            M4 modelMat = getM4("transf", sobj);

            glBindVertexArray(dummyPointData.vao);
            glUseProgram(cylInner025Shader.program);

            setupCommonUniforms(cylInner025Shader, modelMat);

            if (cylInner025Shader.uniforms.count("color"))
            {
                setColor(cylInner025Shader.uniforms["color"], color);

                checkGLError("uniform color");
            }

            if (cylInner025Shader.uniforms.count("lightPos"))
            {
                glUniform3fv(cylInner025Shader.uniforms["lightPos"], 1, &lightPos[0]);

                checkGLError("uniform lightPos");
            }

            glDrawArrays(GL_POINTS, 0, 1);

            glBindVertexArray(0);
        }
    }

    void processInput()
    {
        using namespace std;

        if (mode3d)
        {
            V3 accel = V3(0.0f, 0.0f, 0.0f);

            if (pressedKeys.count(Qt::Key_W))
                accel.y += 1.0;

            if (pressedKeys.count(Qt::Key_S))
                accel.y -= 1.0;

            if (pressedKeys.count(Qt::Key_D))
                accel.x += 1.0;

            if (pressedKeys.count(Qt::Key_A))
                accel.x -= 1.0;

            if (pressedKeys.count(Qt::Key_Space))
                accel.z += 1.0;

            if (pressedKeys.count(Qt::Key_Control))
                accel.z -= 1.0;

            if (!pressedKeys.count(Qt::Key_W) && !pressedKeys.count(Qt::Key_S))
            {
                int sgn = (vel.y < 0.0f) ? -1 : ((vel.y > 0.0f) ? 1 : 0);
                accel.y = -sgn;
            }

            if (!pressedKeys.count(Qt::Key_D) && !pressedKeys.count(Qt::Key_A))
            {
                int sgn = (vel.x < 0.0f) ? -1 : ((vel.x > 0.0f) ? 1 : 0);
                accel.x = -sgn;
            }

            if (!pressedKeys.count(Qt::Key_Space) && !pressedKeys.count(Qt::Key_Control))
            {
                int sgn = (vel.z < 0.0f) ? -1 : ((vel.z > 0.0f) ? 1 : 0);
                accel.z = -sgn;
            }

            float maxVel = 50.0f;

            accel *= maxVel;

            float dt = 0.016;
            V3 oldVel = vel;
            vel += accel * dt;

            if (!pressedKeys.count(Qt::Key_W) && !pressedKeys.count(Qt::Key_S) && vel.y * oldVel.y < 0.0f)
                vel.y = 0.0f;

            if (!pressedKeys.count(Qt::Key_D) && !pressedKeys.count(Qt::Key_A) && vel.x * oldVel.x < 0.0f)
                vel.x = 0.0f;

            if (!pressedKeys.count(Qt::Key_Space) && !pressedKeys.count(Qt::Key_Control) && vel.z * oldVel.z < 0.0f)
                vel.z = 0.0f;



            if (glm::length(vel) > maxVel)
                vel *= (maxVel / glm::length(vel));

            glm::vec3 dPos = vel * dt;
            if (pressedKeys.count(Qt::Key_Shift))
                dPos *= 10.0;

            dPos = camera.transform(dPos.y, dPos.x, dPos.z);
            camera.position += dPos;

            QString status = QString("Pos %1 %2 %3").arg(camera.position.x).arg(camera.position.y).arg(camera.position.z);
            view->updateStatusString(status);
        }
    }

    P toLocalP(const P &p) const
    {
        P res = p / P(wid, heig);
        return toLocalRelP(res);
    }

    P toLocalRelP(const P &relP) const
    {
        P res = (zoomCenter + (relP - P(0.5, 0.5))/zoom)*P(wid, heig);
        return res;
    }

    void onKeyEvent(bool press, QKeyEvent *event)
    {
        if (press)
            pressedKeys.insert(event->key());
        else
            pressedKeys.erase(event->key());
    }
};

View::View(QWidget *parent) : QOpenGLWidget(parent)
{
    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    format.setStencilBufferSize(8);
    format.setVersion(3, 3);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setSamples(10);
    setFormat(format); // must be called before the widget or its parent window gets shown

    viewData = new ViewData(this);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
}

View::~View()
{
    delete viewData;
}

void View::initializeGL()
{
    viewData->initialize();

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    timer->start(16);
}

void View::resizeGL(int w, int h)
{
    this->viewData->wid = w;
    this->viewData->heig = h;

    this->viewData->camera.aspectRatio = (double) w / (double) h;
}



void View::paintGL()
{
    std::shared_ptr<Frame> renderFrame = this->renderFrame;
    viewData->render(renderFrame.get(), w, h);

    if (glm::length(viewData->vel) > 0.001)
    {
        update();
    }
}

void View::wheelEvent(QWheelEvent *event)
{
    QPoint p = event->angleDelta();
    int w = p.y() / 15/8;

    P zoomPoint = viewData->zoomCenter + (viewData->mousePos - P(0.5, 0.5))/viewData->zoom;

    if (w > 0)
    {
        for (int i = 0; i < w; ++i)
            viewData->zoom *= 1.2;
    } else
    {
        for (int i = 0; i < -w; ++i)
            viewData->zoom /= 1.2;
    }

    if (viewData->zoom < 0.1)
        viewData->zoom = 0.1;

    viewData->zoomCenter = zoomPoint - (viewData->mousePos - P(0.5, 0.5))/viewData->zoom;

    update();
}

void View::mouseMoveEvent(QMouseEvent *event)
{
    P oldPos = viewData->mousePos;
    viewData->mousePos = P(event->pos().x(), event->pos().y()) / P(this->viewData->wid, this->viewData->heig);

    if (viewData->mode3d)
    {
        if (event->buttons() & Qt::LeftButton)
        {
            P delta = (viewData->mousePos - oldPos) * P(this->viewData->wid, this->viewData->heig);
            viewData->camera.rotate(delta.x, delta.y);
            //qDebug() << viewData->camera.pitch << " " << viewData->camera.yaw;

            //update();
        }
    }
    else
    {
        if (event->buttons() & Qt::LeftButton)
        {
            P delta = viewData->mousePos - oldPos;
            viewData->zoomCenter -= delta / viewData->zoom;

            //update();
        }

        //update();

        P titlePos = (viewData->zoomCenter + (viewData->mousePos - P(0.5, 0.5))/viewData->zoom)*P(this->viewData->wid, this->viewData->heig);
        P clampedTitlePos = clampP(titlePos, P(0, 0), P(this->w, this->h));

        QString status = QString("Pos %1 %2 (%3 %4)").arg(clampedTitlePos.x).arg(transformY(clampedTitlePos.y, h)).arg(titlePos.x).arg(transformY(titlePos.y, h));

        if (viewData->rulerStarted)
        {
            P curPos = viewData->toLocalRelP(viewData->mousePos);
            double rad = viewData->rulerStart.dist(curPos);
            status += " dist: ";
            status += QString("%1").arg(rad);
        }
        updateStatusString(status);
    }
}

void View::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::RightButton)
    {
        viewData->rulerStart = viewData->toLocalP(P(event->pos().x(), event->pos().y()));
        viewData->rulerStarted = !viewData->rulerStarted;
        //update();
    }
}

void View::keyPressEvent(QKeyEvent *event)
{
    onKeyEvent(true, event);
}

void View::keyReleaseEvent(QKeyEvent *event)
{
    if (event->isAutoRepeat())
        return;

    onKeyEvent(false, event);
}

void View::updateZoom()
{
    viewData->updateZoom();
}

void View::onKeyEvent(bool press, QKeyEvent *event)
{
    viewData->onKeyEvent(press, event);
    update();

    Obj obj;
    if (press)
        obj.type = "keyPress";
    else
        obj.type = "keyRelease";

    obj.props["key"] = (uint32_t) event->key();
    obj.props["text"] = event->text().toUtf8().data();
    emit keyEvent(obj);
}

void View::updateStatusString(const QString &str)
{
    if (str != status)
    {
        status = str;
        emit statusChanged(str);
    }
}

void View::changeFrame(std::shared_ptr<Frame> renderFrame, int /*totalCount*/)
{
    this->renderFrame = renderFrame;
    if (!this->renderFrame)
        viewData->staticObjects.clear();

    //update();
}

void View::fieldSizeChange(int w, int h)
{
    this->w = w;
    this->h = h;
    viewData->mode3d = false;
}

void View::field3d(const P &minP, const P &maxP, double hMin, double hMax, double cellSize)
{
    makeCurrent();
    viewData->gridData.init(minP, maxP, hMin, hMax, cellSize);
    viewData->mode3d = true;
}

void View::addStaticObject(const SObj &sobj)
{
    viewData->staticObjects.push_back(sobj);
}

void View::settingsChanged()
{
    viewData->camera.reloadSettings();
}
