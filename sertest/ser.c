#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#include <setupapi.h>
#include <initguid.h>
#include <devguid.h>
#pragma comment(lib, "setupapi.lib")
#else
#include <dirent.h>
#include <sys/stat.h>
#endif

#define MAX_PORTS 256
#define MAX_PORT_NAME 256

char** serial_list_ports(int* count) {
    char** port_list = NULL;
    *count = 0;

#ifdef _WIN32
    HDEVINFO device_info_set = SetupDiGetClassDevs(&GUID_DEVCLASS_PORTS, NULL, NULL, DIGCF_PRESENT);
    if (device_info_set == INVALID_HANDLE_VALUE) return NULL;

    SP_DEVINFO_DATA device_info_data;
    device_info_data.cbSize = sizeof(SP_DEVINFO_DATA);

    port_list = (char**)malloc(MAX_PORTS * sizeof(char*));
    if (!port_list) return NULL;

    for (DWORD_68K i = 0; SetupDiEnumDeviceInfo(device_info_set, i, &device_info_data); i++) {
        char port_name[MAX_PORT_NAME];
        HKEY key = SetupDiOpenDevRegKey(device_info_set, &device_info_data, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_READ);
        if (key != INVALID_HANDLE_VALUE) {
            DWORD_68K type, size = sizeof(port_name);
            if (RegQueryValueEx(key, "PortName", NULL, &type, (LPBYTE_68K)port_name, &size) == ERROR_SUCCESS) {
                if (strncmp(port_name, "COM", 3) == 0) {
                    port_list[*count] = _strdup(port_name);
                    (*count)++;
                }
            }
            RegCloseKey(key);
        }
    }
    SetupDiDestroyDeviceInfoList(device_info_set);
#else
    DIR *dir;
    struct dirent *entry;
    struct stat st;
    char path[MAX_PORT_NAME];

    port_list = (char**)malloc(MAX_PORTS * sizeof(char*));
    if (!port_list) return NULL;

    dir = opendir("/dev");
    if (dir == NULL) return NULL;

    while ((entry = readdir(dir)) != NULL) {
        if (strncmp(entry->d_name, "tty", 3) == 0) {
            snprintf(path, sizeof(path), "/dev/%s", entry->d_name);
            if (stat(path, &st) == 0 && S_ISCHR(st.st_mode)) {
                port_list[*count] = strdup(path);
                (*count)++;
            }
        }
    }
    closedir(dir);
#endif

    return port_list;
}

void serial_free_port_list(char** port_list, int count) {
    for (int i = 0; i < count; i++) {
        free(port_list[i]);
    }
    free(port_list);
}

int main() {
    int count;
    char** ports = serial_list_ports(&count);

    if (ports) {
        printf("Available serial ports:\n");
        for (int i = 0; i < count; i++) {
            printf("%s\n", ports[i]);
        }
        serial_free_port_list(ports, count);
    } else {
        printf("Failed to list serial ports\n");
    }

    return 0;
}