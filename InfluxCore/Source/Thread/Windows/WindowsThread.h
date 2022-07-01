#pragma once

#include "../../Platform/Windows/WindowsPlatform.h"
#include "../BaseThread.h"

namespace Influx
{
	/* WindowsThread serves as an Encapsulator of a windows::HANDLE that is retrieved on _ThreadEntryProc */
	/* WindowsThread is castable to LPVOID* => it only contains a HANDLE datamember */
	class WindowsThread : public BaseThread
	{
		HANDLE mWindowsHandle;

	public:
		/* Tell the Thread to Exit (Forcing is not recommended -> Leaks) */
		/* [Noimpl] */
		inline virtual void Kill(bool force = false) override
		{
			
		}

		/* Halts the caller until this thread is finished. */
		inline virtual void WaitForComplete() override final
		{
			::WaitForSingleObject(mWindowsHandle, INFINITE);
		}

		/* Tell the Thread to pause/resume */
		inline virtual void Suspend(bool shouldPause = true) override final
		{
			::SuspendThread(mWindowsHandle);
		}
	};
}


