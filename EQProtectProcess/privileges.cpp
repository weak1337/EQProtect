#include "includes.h"

bool privs::raise_and_gethandle(LPCSTR priv, HANDLE* driverhandle) {
	if (ImpersonateSelf(SecurityImpersonation)) {
		HANDLE threadToken = nullptr;
		*driverhandle = threadToken;
		if (OpenThreadToken(GetCurrentThread(), TOKEN_ALL_ACCESS,
			TRUE, &threadToken)) {
			TOKEN_PRIVILEGES privileges;
			privileges.PrivilegeCount = 1;
			privileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
			if (LookupPrivilegeValue(nullptr, priv,
				&privileges.Privileges[0].Luid)) {
				if (AdjustTokenPrivileges(threadToken, FALSE, &privileges, 0,
					nullptr, nullptr)) {
					*driverhandle = CreateFileA("\\\\.\\bruhmomentum", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, 0, OPEN_EXISTING, 0, 0);
					PRIVILEGE_SET set;
					set.PrivilegeCount = 1;
					set.Privilege[0].Attributes = SE_PRIVILEGE_ENABLED;
					set.Privilege[0].Luid = privileges.Privileges[0].Luid;
					BOOL is_set;
					if (PrivilegeCheck(threadToken, &set, &is_set)) {
						CloseHandle(threadToken);
						RevertToSelf();
						if (is_set)
							return true;
					}

				}
			}
		}
	}
}