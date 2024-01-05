#pragma	once

#include <wx/wx.h>
#include <wx/cmdline.h>

/*
class LOG : public wxFrame
{
public:
	LOG(const wxString& title);
};
*/

class Main : public wxApp
{
public:
	virtual bool OnInit();
};

IMPLEMENT_APP(Main)

extern class i_lc_log
{
public:
	virtual void clMsg(LPCSTR msg) = 0;
	virtual void clLog(LPCSTR msg) = 0;
	virtual void Status(LPCSTR msg) = 0;
	virtual	void Progress(const float F) = 0;
	virtual	void Phase(LPCSTR phase_name) = 0;
} *lc_log;

void clMsg(const char* format, ...);
void Status(const char* format, ...);
void Progress(const float F);
void Phase(const char* phase_name);
void logThread(void *dummy);
void logCallback(LPCSTR c);
