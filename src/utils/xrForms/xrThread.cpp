#include "../../xrCore/xrCore.h"
#include "cl_log.h"
#include "xrThread.h"

void	CThread::startup(void* P)
{
	CThread* T = (CThread*)P;

	if (T->thMessages)	clMsg("* THREAD #%d: Started.", T->thID);
	FPU::m64r();
	T->Execute();
	T->thCompleted = TRUE;
	if (T->thMessages)	clMsg("* THREAD #%d: Task Completed.", T->thID);
}

void	CThreadManager::start(CThread* T)
{
	R_ASSERT(T);
	threads.push_back(T);
	T->Start();
}

void CThreadManager::wait(u32 sleep_time)
{
	// Wait for completion
	char perf[1024];
	if (threads.empty())
		return;

	while (true)
	{
		Sleep(sleep_time);

		perf[0] = 0;
		float	sumProgress = 0;
		float	sumPerformance = 0;
		u32		sumComplete = 0;

		for (u32 ID = 0; ID < threads.size(); ID++)
		{
			if (threads[ID]->thCompleted)
			{
				sumProgress++;
				sumComplete++;
			}
			else
			{
				sumProgress += threads[ID]->thProgress;
			}

			sumPerformance += threads[ID]->thPerformance;

			char P[64];

			if (ID)
				xr_sprintf(P, "*%3.1f", threads[ID]->thPerformance);
			else
				xr_sprintf(P, " %3.1f", threads[ID]->thPerformance);

			xr_strcat(perf, P);
		}

		if (threads[0]->thMonitor)
		{
			Status("Performance: %3.1f :%s", sumPerformance, perf);
		}

		Progress(sumProgress / float(threads.size()));

		if (sumComplete == threads.size())
			break;
	}

	// Delete threads
	for (u32 thID = 0; thID < threads.size(); thID++)
		if (threads[thID]->thDestroyOnComplete)	xr_delete(threads[thID]);

	threads.clear();
}