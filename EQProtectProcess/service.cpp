#include "includes.h"

bool ExistOtherService(SC_HANDLE service_manager) {
	DWORD spaceNeeded = 0;
	DWORD numServices = 0;
	if (!EnumServicesStatus(service_manager, SERVICE_DRIVER, SERVICE_STATE_ALL, NULL, 0, &spaceNeeded, &numServices, 0) && GetLastError() != ERROR_MORE_DATA) {

		return true;
	}
	spaceNeeded += sizeof(ENUM_SERVICE_STATUSA);
	LPENUM_SERVICE_STATUSA buffer = (LPENUM_SERVICE_STATUSA)new BYTE[spaceNeeded];

	if (EnumServicesStatus(service_manager, SERVICE_DRIVER, SERVICE_STATE_ALL, buffer, spaceNeeded, &spaceNeeded, &numServices, 0)) {
		for (DWORD i = 0; i < numServices; i++) {
			ENUM_SERVICE_STATUSA service = buffer[i];
			SC_HANDLE service_handle = OpenService(service_manager, service.lpServiceName, SERVICE_QUERY_CONFIG);
			if (service_handle) {
				LPQUERY_SERVICE_CONFIGA config = (LPQUERY_SERVICE_CONFIGA)new BYTE[8096]; //8096 = max size of QUERY_SERVICE_CONFIGA
				DWORD needed = 0;
				if (QueryServiceConfig(service_handle, config, 8096, &needed)) {
					if (strstr(config->lpBinaryPathName, "EQProtect")) {
						delete[] buffer;

						CloseServiceHandle(service_handle);
						return false;
					}
				}
				else {

				}
				CloseServiceHandle(service_handle);
			}

		}
		delete[] buffer;
		return false; //no equal services we can continue
	}
	delete[] buffer;

	return true;

}

bool StopAndRemove(const std::string& servicename)
{
	const SC_HANDLE sc_manager_handle = OpenSCManager(nullptr, nullptr, SC_MANAGER_CREATE_SERVICE);

	if (!sc_manager_handle)
		return false;

	const SC_HANDLE service_handle = OpenService(sc_manager_handle, servicename.c_str(), SERVICE_STOP | DELETE);

	if (!service_handle)
	{
		CloseServiceHandle(sc_manager_handle);
		return false;
	}

	SERVICE_STATUS status = { 0 };
	const bool result = ControlService(service_handle, SERVICE_CONTROL_STOP, &status) && DeleteService(service_handle);

	CloseServiceHandle(service_handle);
	CloseServiceHandle(sc_manager_handle);

	return result;
}

bool service::create(LPCSTR path) {
	const SC_HANDLE sc_manager_handle = OpenSCManager(nullptr, nullptr, SC_MANAGER_ALL_ACCESS);

	if (ExistOtherService(sc_manager_handle)) {
		StopAndRemove("EQProtect");
	}

	SC_HANDLE service_handle = CreateServiceA(sc_manager_handle, "EQProtect", "EQProtect", SERVICE_START | SERVICE_STOP | DELETE, SERVICE_KERNEL_DRIVER, SERVICE_DEMAND_START, SERVICE_ERROR_IGNORE, "C:\\Windows\\System32\\Drivers\\vuldriver.sys", nullptr, nullptr, nullptr,0,0);
	if (!service_handle)
	{

		service_handle = OpenService(sc_manager_handle, "EQProtect", SERVICE_START);

		if (!service_handle)
		{

			CloseServiceHandle(sc_manager_handle);
			return false;
		}
	}

	HKEY hKey;
	LPCTSTR sk = TEXT("SYSTEM\\CurrentControlSet\\Services\\EQProtect");
	LONG openRes = RegOpenKeyEx(HKEY_LOCAL_MACHINE, sk, 0, KEY_ALL_ACCESS, &hKey);
	LPCTSTR value = TEXT("SessionId");
	LPCTSTR data = "bruhmomentum\0";
	LONG setRes = RegSetValueEx(hKey, value, 0, REG_SZ, (LPBYTE)data, strlen(data) + 1);
	LONG closeOut = RegCloseKey(hKey);
	const bool result = StartServiceA(service_handle, 0, nullptr);
	CloseServiceHandle(service_handle);
	CloseServiceHandle(sc_manager_handle);
}

void service::close(const std::string driver_name)
{
	const SC_HANDLE sc_manager_handle = OpenSCManager(nullptr, nullptr, SC_MANAGER_CREATE_SERVICE);


	const SC_HANDLE service_handle = OpenService(sc_manager_handle, driver_name.c_str(), SERVICE_STOP | DELETE);

	if (!service_handle)
	{
		CloseServiceHandle(sc_manager_handle);
		return;
	}

	SERVICE_STATUS status = { 0 };
	const bool result = ControlService(service_handle, SERVICE_CONTROL_STOP, &status) && DeleteService(service_handle);
	CloseServiceHandle(service_handle);
	CloseServiceHandle(sc_manager_handle);
}