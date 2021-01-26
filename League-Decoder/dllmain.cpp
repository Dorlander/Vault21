// dllmain.cpp : Defines the entry point for the DLL application.
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>
#include <cstdint>
#include <string>
#include <process.h>
#include <vector>

struct LeagueDecryptData
{
	int totalSuccessDecrypted;
	int totalSuccess_PAGE_NOACCESS;
	int totalSuccess_EXCEPTION_CONTINUE_EXECUTION;
	int totalFailedDecrypted;
};

typedef BOOLEAN(__stdcall* t_RtlDispatchException)(PEXCEPTION_RECORD ExceptionRecord, PCONTEXT ContextRecord);
t_RtlDispatchException fn_RtlDispatchException;

uint8_t* find_signature(LPCSTR szModule, const char* szSignature) {
	auto module = GetModuleHandleA(szModule);
	static auto pattern_to_byte = [](const char* pattern) {
		auto bytes = std::vector<int>{};
		auto start = const_cast<char*>(pattern);
		auto end = const_cast<char*>(pattern) + strlen(pattern);

		for (auto current = start; current < end; ++current) {
			if (*current == '?') {
				++current;
				if (*current == '?')
					++current;
				bytes.push_back(-1);
			}
			else {
				bytes.push_back(strtoul(current, &current, 16));
			}
		}
		return bytes;
	};

	auto dosHeader = (PIMAGE_DOS_HEADER)module;
	auto ntHeaders = (PIMAGE_NT_HEADERS)((uint8_t*)module + dosHeader->e_lfanew);
	auto textSection = IMAGE_FIRST_SECTION(ntHeaders);

	auto sizeOfImage = textSection->SizeOfRawData;
	auto patternBytes = pattern_to_byte(szSignature);
	auto scanBytes = reinterpret_cast<uint8_t*>(module) + textSection->VirtualAddress;

	auto s = patternBytes.size();
	auto d = patternBytes.data();

	auto mbi = MEMORY_BASIC_INFORMATION{ 0 };
	uint8_t* next_check_address = 0;

	for (auto i = 0ul; i < sizeOfImage - s; ++i) {
		bool found = true;
		for (auto j = 0ul; j < s; ++j) {
			auto current_address = scanBytes + i + j;
			if (current_address >= next_check_address) {
				if (!VirtualQuery(reinterpret_cast<void*>(current_address), &mbi, sizeof(mbi)))
					break;

				if (mbi.Protect == PAGE_NOACCESS) {
					i += ((std::uintptr_t(mbi.BaseAddress) + mbi.RegionSize) - (std::uintptr_t(scanBytes) + i));
					i--;
					found = false;
					break;
				}
				else {
					next_check_address = reinterpret_cast<uint8_t*>(mbi.BaseAddress) + mbi.RegionSize;
				}
			}

			if (scanBytes[i + j] != d[j] && d[j] != -1) {
				found = false;
				break;
			}
		}
		if (found) {
			return &scanBytes[i];
		}
	}
	return nullptr;
}

uint8_t* find_RtlDispatchExceptionAddress()
{
	auto address = find_signature("ntdll.dll" , "E8 ? ? ? ? 0A C0");

	if (!address)
	{
		return nullptr;
	}

	address = address + *reinterpret_cast<uint32_t*>(address + 1) + 5;

	return address;
}

int IsMemoryDecrypted(PVOID Address)
{
	CONTEXT ctx;
	EXCEPTION_RECORD exr;
	MEMORY_BASIC_INFORMATION mbi;
	FLOATING_SAVE_AREA w64;
	memset(&mbi, 0, sizeof(mbi));
	VirtualQuery(Address, &mbi, sizeof(mbi));
	if (mbi.Protect != PAGE_NOACCESS)
	{
		return 1;
	}
	RtlCaptureContext(&ctx);
	memset(&exr, 0, sizeof(EXCEPTION_RECORD));
	memset(&w64, 0, sizeof(WOW64_FLOATING_SAVE_AREA));

#ifdef _WIN64
	ctx.Rip = reinterpret_cast<DWORD64>(Address);// (DWORD)FinishThread;
#else
	ctx.Eip = reinterpret_cast<DWORD>(Address);// (DWORD)FinishThread;
#endif // 

	ctx.ContextFlags = 0x1007F;
	ctx.SegCs = 0x23;
	ctx.SegDs = 0x2b;
	ctx.SegEs = 0x2b;
	ctx.SegFs = 0x53;
	ctx.SegGs = 0x2b;
	ctx.SegSs = 0x2b;
	exr.ExceptionAddress = Address;
	exr.NumberParameters = 2;
	exr.ExceptionCode = EXCEPTION_ACCESS_VIOLATION;
	exr.ExceptionInformation[1] = reinterpret_cast<DWORD>(Address);
	_EXCEPTION_POINTERS ei;
	ei.ContextRecord = &ctx;
	ei.ExceptionRecord = &exr;

	HMODULE ntdll = GetModuleHandleA("ntdll.dll");

	uint8_t* _RtlDispatchExceptionAddress = find_RtlDispatchExceptionAddress();

	if (!_RtlDispatchExceptionAddress)
		return 0;

	DWORD RtlDispatchExceptionAddr = (DWORD)(_RtlDispatchExceptionAddress);

	if (RtlDispatchExceptionAddr) {
		fn_RtlDispatchException = (t_RtlDispatchException)(RtlDispatchExceptionAddr);
		if (fn_RtlDispatchException(&exr, &ctx)) {
			return 2;
		}
	}

	return 0;
}

LeagueDecryptData decrypt(const wchar_t* szModule) {
	LeagueDecryptData ldd;
	ldd.totalFailedDecrypted = 0;
	ldd.totalSuccessDecrypted = 0;
	ldd.totalSuccess_PAGE_NOACCESS = 0;
	ldd.totalSuccess_EXCEPTION_CONTINUE_EXECUTION = 0;

	auto module = GetModuleHandle(szModule);

	auto dosHeader = (PIMAGE_DOS_HEADER)module;
	auto ntHeaders = (PIMAGE_NT_HEADERS)((uint8_t*)module + dosHeader->e_lfanew);
	auto textSection = IMAGE_FIRST_SECTION(ntHeaders);

	auto sizeOfImage = textSection->SizeOfRawData;
	auto scanBytes = reinterpret_cast<uint8_t*>(module) + textSection->VirtualAddress;


	auto mbi = MEMORY_BASIC_INFORMATION{ 0 };
	uint8_t* next_check_address = 0;

	bool isFirstRegion = true;
	for (auto i = 0; i < sizeOfImage; ++i) {

		auto current_address = scanBytes + i;
		if (current_address >= next_check_address) {
			if (!VirtualQuery(reinterpret_cast<void*>(current_address), &mbi, sizeof(mbi)))
				continue;

			if (mbi.Protect != PAGE_NOACCESS || isFirstRegion) {
				isFirstRegion = false;
				i += ((std::uintptr_t(mbi.BaseAddress) + mbi.RegionSize) - (std::uintptr_t(scanBytes) + i));
				i--;
				continue;
			}
			else {
				next_check_address = reinterpret_cast<uint8_t*>(mbi.BaseAddress) + mbi.RegionSize;
			}
		}
		int ret = IsMemoryDecrypted((PVOID)((DWORD)current_address));
		if (ret != 0) {
			if (ret == 1) {
				ldd.totalSuccess_PAGE_NOACCESS++;
			}
			else if (ret == 2) {
				ldd.totalSuccess_EXCEPTION_CONTINUE_EXECUTION++;
			}
			ldd.totalSuccessDecrypted++;
		}
		else {
			ldd.totalFailedDecrypted++;
		}
	}
	return ldd;
}

__declspec(safebuffers)DWORD WINAPI InitThread(LPVOID module)
{
	LeagueDecryptData ldd = decrypt(nullptr);
	MessageBoxA(0, (
		"totalFailedDecrypted: " + std::to_string(ldd.totalFailedDecrypted) + "\n" +
		"totalSuccessDecrypted: " + std::to_string(ldd.totalSuccessDecrypted) + "\n" +
		"totalSuccess_EXCEPTION_CONTINUE_EXECUTION: " + std::to_string(ldd.totalSuccess_EXCEPTION_CONTINUE_EXECUTION) + "\n" +
		"totalSuccess_PAGE_NOACCESS: " + std::to_string(ldd.totalSuccess_PAGE_NOACCESS) + "\n" +
		"USE LEAGUE DUMPER NOW! Do not close this MessageBox!"
		).c_str(), "LeagueNuke", 0);

	return 0;
}

void OnExit() noexcept;
uintptr_t initThreadHandle;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	DisableThreadLibraryCalls(hModule);
	if (dwReason == DLL_PROCESS_ATTACH)
	{
		std::atexit(OnExit);
		initThreadHandle = _beginthreadex(nullptr, 0, (_beginthreadex_proc_type)InitThread, hModule, 0, nullptr);

		FreeLibrary(hModule);
	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		OnExit();
	}

	return TRUE;
}

void OnExit() noexcept
{
	WaitForSingleObject((HANDLE)initThreadHandle, INFINITE);
	CloseHandle((HANDLE)initThreadHandle);
}