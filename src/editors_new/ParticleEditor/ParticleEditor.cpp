// ParticleEditor.cpp : Определяет точку входа для приложения.
//
#include "stdafx.h"

void BeginRender()
{
#define D3DCOLOR_RGBA(r,g,b,a) D3DCOLOR_ARGB(a,r,g,b)
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    RDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
    RDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
    RDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
    D3DCOLOR clear_col_dx = D3DCOLOR_RGBA((int)(clear_color.x * clear_color.w * 255.0f), (int)(clear_color.y * clear_color.w * 255.0f), (int)(clear_color.z * clear_color.w * 255.0f), (int)(clear_color.w * 255.0f));
    RDevice->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clear_col_dx, 1.0f, 0);

    RDevice->BeginScene();
}

void EndRender()
{
    RDevice->EndScene();
    RDevice->Present(nullptr, nullptr, nullptr, nullptr);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow)
{
    if (!IsDebuggerPresent()) Debug._initialize(false);
    const char* FSName = "fs.ltx";
    //{
    //    if (strstr(GetCommandLineA(), "-soc_14") || strstr(GetCommandLineA(), "-soc_10004"))
    //    {
    //        FSName = "fs_soc.ltx";
    //    }
    //    else if (strstr(GetCommandLineA(), "-soc"))
    //    {
    //        FSName = "fs_soc.ltx";
    //    }
    //    else if (strstr(GetCommandLineA(), "-cs"))
    //    {
    //        FSName = "fs_cs.ltx";
    //    }
    //}
    Core._initialize("Patricle", ELogCallback, 1, FSName);

    Tools = xr_new<CParticleTool>();
    PTools = (CParticleTool*)Tools;
    UI = xr_new<CParticleMain>();
    UI->RegisterCommands();

    UIMainForm* MainForm = xr_new< UIMainForm>();
    ::MainForm = MainForm;
    UI->Push(MainForm, false);

    while (MainForm->Frame())
    {
        SDL_Event Event;
        while (SDL_PollEvent(&Event))
        {
            if (!UI->ProcessEvent(&Event))
                break;
        }

        UI->BeginFrame();
        RDevice->SetRenderTarget(0, RSwapchainTarget);

        UI->Draw();

        BeginRender();
        UI->EndFrame();
        EndRender();
    }

    xr_delete(MainForm);
    Core._destroy();
    return 0;
}
