#pragma once

#include <mutex>
#include <iostream>
#include <Windows.h>
#include <Strsafe.h>
#include <process.h>
#include <unordered_set>

using namespace std;

class ReadFileChange
{
private:
	std::mutex mtx_lock;
public:
	struct HandleStock
	{
		HANDLE hIocp;
		HANDLE hDir;
		std::mutex mtx_lock;
	};

	struct MyOverlapped : public OVERLAPPED
	{
		FILE_NOTIFY_INFORMATION buf[1024];
	};

	ReadFileChange()
	{

	}
	virtual ~ReadFileChange()
	{

	}

	static unsigned WINAPI ConsumerThread(void* Comp)
	{
		HandleStock* hHandleStock = (HandleStock*)Comp;
		DWORD dwNumBytes = 0;
		DWORD dwCompKey = 0;
		LPOVERLAPPED lpOverlapped = NULL;

		//DWORD dwNotifyFilter = FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME |
		//	FILE_NOTIFY_CHANGE_ATTRIBUTES |/* FILE_NOTIFY_CHANGE_SIZE |*/
		//	FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION;

		DWORD dwNotifyFilter = FILE_NOTIFY_CHANGE_SIZE;
		DWORD dwBytesReturned = 0;

		while (true)
		{
			BOOL bComp = GetQueuedCompletionStatus(hHandleStock->hIocp,
				&dwNumBytes,
				(PULONG_PTR)&dwCompKey,
				&lpOverlapped,
				INFINITE);

			if (!dwCompKey || !lpOverlapped)
			{
				dwCompKey = 0;
				continue;
			}

			hHandleStock->mtx_lock.lock();
			MyOverlapped* ovr = (MyOverlapped*)lpOverlapped;
			FILE_NOTIFY_INFORMATION* pInfo = ovr->buf;
			cout << "=========================" << endl;
			do {
				/*char fileName[1024] = { 0, };
				int count = WideCharToMultiByte(
					CP_ACP, 0, pInfo->FileName, pInfo->FileNameLength / sizeof(WCHAR),
					fileName, _ARRAYSIZE(fileName) - 1, NULL, NULL);
				fileName[count] = '\0';*/

				cout << "NextEntryOffset : " << pInfo->NextEntryOffset << endl;
				std::wstring t(pInfo->FileName, pInfo->FileNameLength / sizeof(wchar_t));
				switch (pInfo->Action)
				{
				case FILE_ACTION_MODIFIED:
				{
					WCHAR temp[MAX_PATH] = { 0, };
					StringCbCopyNW(temp, sizeof(temp), pInfo->FileName, pInfo->FileNameLength);

					int len = WideCharToMultiByte(CP_ACP, 0, temp, -1, NULL, 0, NULL, NULL);
					string strMulti(len, 0);
					WideCharToMultiByte(CP_ACP, 0, temp, -1, &strMulti[0], len, NULL, NULL);

					cout << "File Modified : " << strMulti.c_str() << endl;
				}
				break;

				case FILE_ACTION_ADDED:
				case FILE_ACTION_REMOVED:
				case FILE_ACTION_RENAMED_NEW_NAME:
				case FILE_ACTION_RENAMED_OLD_NAME:
				default:
					break;
				}

				if (0 >= pInfo->NextEntryOffset) { break; }
				do
				{
					pInfo = (FILE_NOTIFY_INFORMATION*)((PBYTE)pInfo + pInfo->NextEntryOffset);
					std::wstring n(pInfo->FileName, pInfo->FileNameLength / sizeof(wchar_t));
					if (t != n || 0 >= pInfo->NextEntryOffset) { break; }
				} while (true);
			} while (true);

			ZeroMemory(ovr, sizeof(*ovr));
			if (0 == ReadDirectoryChangesW(hHandleStock->hDir, (LPVOID)ovr->buf, sizeof(ovr->buf), TRUE, dwNotifyFilter, &dwBytesReturned, ovr, NULL))
			{
				cout << "Error : " << GetLastError() << endl;
				//return 0;
			}

			hHandleStock->mtx_lock.unlock();
		}
	}

	void CheckAsync()
	{
		HANDLE hIocp = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

		TCHAR t[MAX_PATH];
		GetCurrentDirectory(MAX_PATH, t);
		std::string path(t);
		path.append("\\Test");
		HANDLE hDir = CreateFile(
			path.c_str(),
			FILE_LIST_DIRECTORY,
			FILE_SHARE_WRITE | FILE_SHARE_READ | FILE_SHARE_DELETE,
			NULL,
			OPEN_EXISTING,
			FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED,
			NULL);

		::CreateIoCompletionPort(hDir, hIocp, (DWORD)29, 0);

		MyOverlapped* ovr = new MyOverlapped;
		ZeroMemory(ovr, sizeof(*ovr));
		//DWORD dwNotifyFilter = FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME |
		//	FILE_NOTIFY_CHANGE_ATTRIBUTES |/* FILE_NOTIFY_CHANGE_SIZE |*/
		//	FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION;

		DWORD dwNotifyFilter = FILE_NOTIFY_CHANGE_SIZE;
		DWORD dwBytesReturned = 0;

		if (0 == ReadDirectoryChangesW(hDir, (LPVOID)ovr->buf, sizeof(ovr->buf), TRUE, dwNotifyFilter, &dwBytesReturned, ovr, NULL))
		{
			cout << "Error : " << GetLastError() << endl;
			return;
		}

		SYSTEM_INFO sysInfo;
		GetSystemInfo(&sysInfo);

		HandleStock* hHandleStock = new HandleStock;
		hHandleStock->hIocp = hIocp;
		hHandleStock->hDir = hDir;

		HANDLE* hList = new HANDLE[sysInfo.dwNumberOfProcessors];
		for (int i = 0; i < sysInfo.dwNumberOfProcessors; ++i)
		{
			hList[i] = (HANDLE)_beginthreadex(NULL, 0, ConsumerThread, (void*)hHandleStock, 0, NULL);
		}

		if (WAIT_FAILED == ::WaitForMultipleObjects(sysInfo.dwNumberOfProcessors, hList, TRUE, INFINITE))
		{
			cout << "";
		}
	}

	void Check()
	{
		TCHAR t[MAX_PATH];
		GetCurrentDirectory(MAX_PATH, t);
		std::string path(t);
		path.append("\\Test");
		HANDLE hDir = CreateFile(
			path.c_str(),
			FILE_LIST_DIRECTORY,
			FILE_SHARE_WRITE | FILE_SHARE_READ | FILE_SHARE_DELETE,
			NULL,
			OPEN_EXISTING,
			FILE_FLAG_BACKUP_SEMANTICS,
			NULL);

		DWORD dwBytesReturned = 0;
		//DWORD dwNotifyFilter = FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_DIR_NAME |
		//	FILE_NOTIFY_CHANGE_ATTRIBUTES |/* FILE_NOTIFY_CHANGE_SIZE |*/
		//	FILE_NOTIFY_CHANGE_LAST_WRITE | FILE_NOTIFY_CHANGE_CREATION;

		DWORD dwNotifyFilter = FILE_NOTIFY_CHANGE_SIZE;
		std::unordered_set<std::string> _check;
		while (true)
		{
			FILE_NOTIFY_INFORMATION strFileNotifyInfo[1024];
			if (0 == ReadDirectoryChangesW(hDir, (LPVOID)&strFileNotifyInfo, sizeof(strFileNotifyInfo), TRUE, dwNotifyFilter, &dwBytesReturned, NULL, NULL))
			{
				cout << "Error : " << GetLastError() << endl;
				break;
			}
			else
			{
				mtx_lock.try_lock();
				cout << "=========================" << endl;
				auto pVal = (FILE_NOTIFY_INFORMATION*)strFileNotifyInfo;
				do {
					cout << "NextEntryOffset : " << pVal->NextEntryOffset << endl;

					std::wstring t;
					switch (pVal->Action)
					{
					case FILE_ACTION_MODIFIED:
					{
						/*WCHAR temp[MAX_PATH] = { 0, };
						StringCbCopyNW(temp, sizeof(temp), pVal->FileName, pVal->FileNameLength);
						t.append(pVal->FileName, pVal->FileNameLength);*/

						t.append(pVal->FileName, pVal->FileNameLength / sizeof(wchar_t));
						int len = WideCharToMultiByte(CP_ACP, 0, t.c_str(), -1, NULL, 0, NULL, NULL);
						string strMulti(len, 0);
						WideCharToMultiByte(CP_ACP, 0, t.c_str(), -1, &strMulti[0], len, NULL, NULL);
						cout << "File Modified : " << strMulti.c_str() << endl;
					}
					break;

					case FILE_ACTION_ADDED:
					case FILE_ACTION_REMOVED:
					case FILE_ACTION_RENAMED_NEW_NAME:
					case FILE_ACTION_RENAMED_OLD_NAME:
					default:
						break;
					}

					if (0 >= pVal->NextEntryOffset) { break; }
					do
					{
						pVal = (FILE_NOTIFY_INFORMATION*)((PBYTE)pVal + pVal->NextEntryOffset);
						std::wstring n(pVal->FileName, pVal->FileNameLength / sizeof(wchar_t));
						if (t != n) { break; }
					} while (true);
				} while (true);
				mtx_lock.unlock();
			}
		}
	}
};
