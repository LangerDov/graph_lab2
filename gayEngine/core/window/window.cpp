#include "windowmanager.h"
#include "window.h"
#include <GLFW/glfw3.h>
#include "debugging/log.h"
#include "matvec.h"
#include "graphics/render/shaders/IShader.h"
#include "files/files.h"

#include <time.h>

int glfw_state = GLFW_FALSE;

void CloseWinCallback(void* HWIN)
{
    Window* win = (Window*)glfwGetWindowUserPointer((GLFWwindow*)HWIN);
    if (!win)
        return;
    if (!win->on_tryclose.func)
        return;
    if (win->time > (time(NULL) - 1))
    {
        glfwSetWindowShouldClose((GLFWwindow*)HWIN, GLFW_FALSE);
        return;
    }
    if (win->on_tryclose.func(win->on_tryclose.data))
        glfwSetWindowShouldClose((GLFWwindow*)HWIN, GLFW_TRUE);
    else
        glfwSetWindowShouldClose((GLFWwindow*)HWIN, GLFW_FALSE);
}

bool bChangeWidth = false;
bool bRotate = false;
float fWidth = 1.f;
void KeyCallback(void* HWIN, int key, int scancode, int action, int mods)
{
    Window* win = (Window*)glfwGetWindowUserPointer((GLFWwindow*)HWIN);
    if (!win)
        return;

    if (key == GLFW_KEY_UP)
    {
        bChangeWidth = true;
        fWidth += 1.f;
    }
    if (key == GLFW_KEY_Q)
    {
        bRotate = true;
    }
}


void SizeCallback(void* window, int width, int height)
{
    Window* win = (Window*)glfwGetWindowUserPointer((GLFWwindow*)window);
    if (!win)
        return;
    if (win->on_chngsize.func)
    {
        resize_callback_data_t data;
        data.data = win->on_chngsize.data;
        data.height = height;
        data.width = width;
        win->on_chngsize.func(&data);
    }
    win->GetRenderInterface()->WaitForDevice();
    win->GetRenderInterface()->SetViewport(width, height);
}

static void CursorPosCallback(void* HWIN, double xpos, double ypos)
{
    Window* win = (Window*)glfwGetWindowUserPointer((GLFWwindow*)HWIN);
    if (!win)
        return;
    if (!win->on_crsr_move.func)
        return;
    static crsr_pos_callback_data_t arg;
    arg.data = win->on_tryclose.data;
    arg.x = xpos;
    arg.y = ypos;
    win->on_crsr_move.func(&arg);
    //Log(lDebug, "x %f  y %f %d", xpos, ypos, win);
}

void vulkan_debug(const char* pMessage)
{
    Log(lDebug, pMessage);
}

void WindowThread(void* data)
{
    Window* parent = (Window*)data;
    int count;
    GLFWmonitor** monitors = glfwGetMonitors(&count);
    void* monitor = NULL;
    glfwWindowHint(GLFW_CLIENT_API, GLFW_FALSE);
    if (parent->opennig_attr.resizable)
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    else
        glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    if (parent->opennig_attr.decorated)
        glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
    else
        glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
    if (parent->opennig_attr.floating)
        glfwWindowHint(GLFW_FLOATING, GLFW_TRUE);
    else
        glfwWindowHint(GLFW_FLOATING, GLFW_FALSE);
    if (parent->opennig_attr.focused)
        glfwWindowHint(GLFW_FOCUSED, GLFW_TRUE);
    else
        glfwWindowHint(GLFW_FOCUSED, GLFW_FALSE);
    if (parent->opennig_attr.focus_on_show)
        glfwWindowHint(GLFW_FOCUS_ON_SHOW, GLFW_TRUE);
    else
        glfwWindowHint(GLFW_FOCUS_ON_SHOW, GLFW_FALSE);
    if (parent->opennig_attr.fullscreen)
        monitor = monitors[parent->monitor];
    if (parent->opennig_attr.iconfied)
        glfwWindowHint(GLFW_AUTO_ICONIFY, GLFW_TRUE);
    else
        glfwWindowHint(GLFW_AUTO_ICONIFY, GLFW_FALSE);
    if (parent->opennig_attr.maximized)
        glfwWindowHint(GLFW_MAXIMIZED, GLFW_TRUE);
    else
        glfwWindowHint(GLFW_MAXIMIZED, GLFW_FALSE);
    if (parent->opennig_attr.centered_cursor)
        glfwWindowHint(GLFW_CENTER_CURSOR, GLFW_TRUE);
    else
        glfwWindowHint(GLFW_CENTER_CURSOR, GLFW_FALSE);
    if (parent->opennig_attr.transparent_framebuffer)
        glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
    else
        glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_FALSE);
    if (parent->opennig_attr.visible)
        glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
    else
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

    parent->HWIN = glfwCreateWindow(parent->width, parent->height, parent->title.Str(), (GLFWmonitor*)monitor, NULL);
    if (!parent->HWIN)
        return;
    glfwSetWindowUserPointer((GLFWwindow*)parent->HWIN, parent);
    glfwSetWindowCloseCallback((GLFWwindow*)parent->HWIN, (void (*)(GLFWwindow*))CloseWinCallback);
    glfwSetKeyCallback((GLFWwindow*)parent->HWIN, (void (*)(GLFWwindow*, int, int, int, int))KeyCallback);
    glfwSetCursorPosCallback((GLFWwindow*)parent->HWIN, (void(*)(GLFWwindow*,double,double))CursorPosCallback);
    glfwSetWindowSizeCallback((GLFWwindow*)parent->HWIN, (void(*)(GLFWwindow*, int, int))SizeCallback);
    // init (p)vulkan
    parent->pRender = CreateVulkanInterface();
    IRenderInterface* pRender = parent->pRender;

    if (!pRender->Init(parent->HWIN))
        return;

    IShader* pPipeline = CreateShader();
    shaderinit_t infostr = {};
    FILES::File pVertFile("shaders/vert.spv", read_, open_exist);
    uint32_t size = pVertFile.GetSize();
    char* pVert = new char[size + 1];
    memset(pVert, 0, sizeof(char) * (size + 1));
    bool test = pVertFile.Read(pVert, size, 0);
    infostr.pVertCode = pVert;
    infostr.uiVertSize = size;

    FILES::File pFragFile("shaders/frag.spv", read_, open_exist);
    size = pFragFile.GetSize();
    char* pFrag = new char[size + 1];
    memset(pFrag, 0, sizeof(char) * (size + 1));
    test = pFragFile.Read(pFrag, size, 0);
    infostr.pFragCode = pFrag;
    infostr.uiFragSize = size;
    infostr.pRenderInterface = pRender;
    infostr.pName = "test";

    if (!pPipeline->Init(&infostr))
        return;

    delete[] pFrag;
    delete[] pVert;
    pFragFile.Close();
    pVertFile.Close();

    IBuffer* pRamBuffer = pRender->CreateBuffer(1024, vertex_buffer_bit | transfer_src_bit, memory_ram);
    if (!pRamBuffer)
        return;

    IBuffer* pGpuBuffer = pRender->CreateBuffer(1024, vertex_buffer_bit | index_buffer_bit | transfer_dst_bit, memory_gpu);
    if (!pGpuBuffer)
        return;

    IBufferRegion* pRamRegion = pRamBuffer->AllocMemory(480);
    IBufferRegion* pGpuRegion = pGpuBuffer->AllocMemory(400);

    float vertices[] = {
        0.f, 1.f, 0.f, 0.f, 0.f, 1.f,
        -1.f, 0.f, 1.f, 0.f, 1.f, 0.f,
        1.f, 0.f, 1.f, 1.f, 0.f, 0.5f,
        -1.f, 0.f, -1.f, 1.f, 1.f, 0.f,
        1.f, 0.f, -1.f, 1.f, 0.f, 0.7f,

    };
    float vertices1[] = {
        100.f, 100.f, 0.f, 1.f, 0.f, 0.f,
        300.f, 100.f, 0.f, 1.f, 0.f, 0.f,
        300.f, 200.f, 0.f, 1.f, 0.f, 0.f,
    };
    uint32_t indices[] = { 0, 1, 2,0,2, 3, 0,3,4,0,4,1,2 };

    void* pDest = pRamRegion->Map();
    memcpy(pDest, vertices, sizeof(vertices));
    memcpy((void*)((size_t)pDest + sizeof(vertices)), indices, sizeof(indices));
    memcpy((void*)((size_t)pDest + sizeof(vertices) + sizeof(indices)), vertices1, sizeof(vertices1));

    pRender->BeginTransfer();
    pRender->CopyBuffer(pRamRegion, pGpuRegion, sizeof(vertices) + sizeof(indices) + sizeof(vertices1), 0, 0);
    pRender->EndTransfer();

    Matrix mvp = Matrix::Perspective(4.f / 3.f, 45.f, 0.1, 100.f) * Matrix::Camera(Vector(0.f, 0.f, -1.f), Vector(0.f, 0.f, 5.f)) * Matrix::Translate(Vector(0, 0, 0));
    float z =  5;
    float angle = 0;
    parent->block->lock = false;
    parent->rootgrhpcmp->InitDraw(parent->HWIN);
    parent->time = time(NULL);

    while (!glfwWindowShouldClose((GLFWwindow*)parent->HWIN))
    {
       mvp = Matrix::Perspective(2.f / 3.f, 45.f, 0.1, 100.f) *
            Matrix::Camera(Vector(0.f, 0.f, -1.f), Vector(0.f, 0.f, 5.f)) *
            Matrix::Translate(Vector(0, 0, 5)) *
            Matrix::RotateByAxis(Vector(0.f, 1.f, 0.f), angle);
        pRender->StartQueue();

        pPipeline->Bind(RENDERMODE_FILL);
        pPipeline->PushConstant((void*)mvp.Array(), sizeof(Matrix));
        pPipeline->DrawIndexed(pGpuRegion, 0, sizeof(vertices), 12);

        //pPipeline->Bind(RENDERMODE_WIREFRAME);
        //pPipeline->PushConstant((void*)mvp.Array(), sizeof(Matrix));
        //pPipeline->DrawIndexed(pGpuRegion, sizeof(vertices) + sizeof(indices), sizeof(vertices), 3);

        if (bChangeWidth)
        {
            bChangeWidth = false;
            z++;
           // mvp = Matrix::Perspective(4.f / 3.f, 45.f, 0.1, 100.f) * Matrix::Camera(Vector(0.f, 0.f, -1.f), Vector(0.f, 0.f, 5.f)) * Matrix::Translate(Vector(0, 0, z));
        }
        if (true)
        {
            bRotate = false;
            angle+=0.1f;
        }

        pRender->EndQueue();
        glfwPollEvents();
    }
    pRender->WaitForDevice();
    pPipeline->Shutdown();
    pGpuBuffer->FreeMemory(pGpuRegion);
    pRamBuffer->FreeMemory(pRamRegion);

    pRender->DeleteBuffer(pGpuBuffer);
    pRender->DeleteBuffer(pRamBuffer);

    pRender->Shutdown();
    glfwDestroyWindow((GLFWwindow*)parent->HWIN);
    parent->HWIN = NULL;
}

Window::Window()
{
    width = height = 0;
    x = y = 0;

    rootgrhpcmp = nullptr;
    thrd_info = nullptr;
    HWIN = nullptr;
    RenderBlock = nullptr;
    block = nullptr;
    monitor = 0;
    pRender = nullptr;
    window_thread = nullptr;
}

Window::~Window()
{
}

void Window::Create()
{
    width = 800;
    height = 600;
    x = 0;
    y = 0;
    monitor = 0;
    title = "Title";
    on_tryclose.data = nullptr;
    on_tryclose.func = nullptr;
    window_thread = new CAsyncThread;
    thrd_info = new threadinfo_t();
    thrd_info->pFunc = WindowThread;
    thrd_info->pData = this;
    block = new Block;
    block->lock = true;
}
bool Window::Open(window_open_attr attr)
{
    opennig_attr = attr;
    window_thread->Start(thrd_info);
    block->Enter();
    return true;
}

bool Window::Close()
{
    return false;
}

void Window::Destroy()
{
}

bool Window::SetTitle(UString title)
{
    if (HWIN)
        glfwSetWindowTitle((GLFWwindow*)HWIN, title.Str());
    this->title = title;
    return true;
}

void Window::SetRoot(ViewPort* vp)
{
    bool lock = block->lock;
    block->lock = true;
    Sleep(1);
    rootgrhpcmp = vp;
    block->lock = lock;
}
void Window::SetOnTryClose(callback_t<bool> callback)
{
    on_tryclose = callback;
}

void Window::SetOnCursorMove(callback_t<void>callback)
{
    on_crsr_move = callback;
}

void Window::SetOnWindowResize(callback_t<void> callback)
{
    on_chngsize = callback;
}

void* Window::GetHWIN()
{
    return HWIN;
}

IRenderInterface* Window::GetRenderInterface()
{
    return pRender;
}

bind_handler Window::CreateBind(bind)
{
    return bind_handler();
}


bool InitWindowSubsys()
{
    glfw_state = glfwInit();
    glfw_state = InitVulkan(vulkan_debug);
    return glfw_state;
}

bool WindowSubsysIsWork()
{
    return glfw_state;
}

void DestroyWindowSubsys()
{
    DestroyVulkan();
    glfwTerminate();
    glfw_state = GLFW_FALSE;
}
