#include <iostream>
#include <windows.h>
#include <conio.h>
#include <vector>
#include <string>
#include <iomanip> 

#define ESC 27
#define UP 72
#define DOWN 80
#define ENTER 13
#define SPACE 32

using namespace std;

HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

enum ConsoleColor {
    Black,
    Blue,
    Green,
    Cyan,
    Red,
    Magenta,
    Brown,
    LightGray,
    DarkGray,
    LightBlue,
    LightGreen,
    LightCyan,
    LightRed,
    LightMagenta,
    Yellow,
    White
};

void setColor(unsigned fg, unsigned bg) {
    SetConsoleTextAttribute(hConsole, (WORD)((bg << 4) | fg));
}

void GoToXY(short x, short y)
{
    SetConsoleCursorPosition(hConsole, { x, y });
}

void ConsoleCursorVisible(bool show, short size)
{
    CONSOLE_CURSOR_INFO structCursorInfo;
    structCursorInfo.bVisible = show;
    structCursorInfo.dwSize = size;
    SetConsoleCursorInfo(hConsole, &structCursorInfo);
}

bool WaitForKey()
{
    setColor(Yellow, Black);
    cout << "\n_______________________________________________\n";
    cout << "\nНатисніть Space для повернення в головне меню.\n";
    cout << "Натисніть Esc для виходу.\n";
    while (true)
    {
        int key = _getch();
        if (key == SPACE)
        {
            return 0;
        }
        else if (key == ESC)
        {
            exit(0);
        }
    }
}

string GetLastErrorAsString()
{
    setColor(Red, Black);

    DWORD errorMessageID = ::GetLastError();
    if (errorMessageID == 0) {
        return string();
    }

    LPSTR messageBuffer = nullptr;

    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

    string message(messageBuffer, size);

    LocalFree(messageBuffer);

    return message;
}

// Отримати список служб
int GetSvcList()
{
    DWORD bytesNeeded = 0;
    DWORD numServices = 0;
    DWORD resumeHandle = 0;

    SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ENUMERATE_SERVICE);
    if (!hSCManager)
    {
        std::cout << "Помилка при відкритті диспетчера служб: " << GetLastErrorAsString();
        return WaitForKey();
    }

    EnumServicesStatus(hSCManager, SERVICE_WIN32, SERVICE_STATE_ALL, NULL, 0, &bytesNeeded, &numServices, &resumeHandle);

    std::vector<BYTE> enumBuffer(bytesNeeded);
    LPENUM_SERVICE_STATUS services = reinterpret_cast<LPENUM_SERVICE_STATUS>(enumBuffer.data());

    if (!EnumServicesStatus(hSCManager, SERVICE_WIN32, SERVICE_STATE_ALL, services, bytesNeeded, &bytesNeeded, &numServices, &resumeHandle))
    {
        std::cout << "Помилка при отриманні списку служб: " << GetLastErrorAsString();
        CloseServiceHandle(hSCManager);
        return WaitForKey();
    }

    setColor(Black, LightCyan);
    cout << setw(10) << left << "STATUS" << setw(22) << "NAME" << setw(87) << "DISPLAY" << endl;

    for (DWORD i = 0; i < numServices; i++)
    {
        std::cout << setw(10) << left;
        switch (services[i].ServiceStatus.dwCurrentState)
        {
        case SERVICE_STOPPED:
            setColor(LightRed, Black);
            std::cout << "Stopped";
            setColor(White, Black);
            break;
        case SERVICE_START_PENDING:
            setColor(Yellow, Black);
            std::cout << "Starting";
            setColor(White, Black);
            break;
        case SERVICE_STOP_PENDING:
            setColor(Yellow, Black);
            std::cout << "Stopping";
            setColor(White, Black);
            break;
        case SERVICE_RUNNING:
            setColor(LightGreen, Black);
            std::cout << "Running";
            setColor(White, Black);
            break;
        case SERVICE_CONTINUE_PENDING:
            setColor(Yellow, Black);
            std::cout << "Contin..";
            setColor(White, Black);
            break;
        case SERVICE_PAUSE_PENDING:
            setColor(Yellow, Black);
            std::cout << "Pausing";
            setColor(White, Black);
            break;
        case SERVICE_PAUSED:
            setColor(Yellow, Black);
            std::cout << "Paused";
            setColor(White, Black);
            break;
        default:
            setColor(Yellow, Black);
            std::cout << "Unknown";
            setColor(White, Black);
            break;
        }
        if (strlen(services[i].lpServiceName) > 20)
        {
            std::cout << setw(22) << left << string(services[i].lpServiceName).substr(0, 17).append("...");
        }
        else
        {
            std::cout << setw(22) << left << services[i].lpServiceName;
        }
        std::cout << services[i].lpDisplayName << endl;
    }

    CloseServiceHandle(hSCManager);

    return WaitForKey();
}

// Створити нову службу
int CreateSvc(const char* serviceName, const char* displayName, const char* binaryPath)
{
    SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CREATE_SERVICE);
    if (!hSCManager)
    {
        std::cout << "Помилка при відкритті диспетчера служб: " << GetLastErrorAsString();
        return WaitForKey();
    }

    SC_HANDLE hService = CreateService(hSCManager, serviceName, displayName, SERVICE_ALL_ACCESS, SERVICE_WIN32_OWN_PROCESS, SERVICE_DEMAND_START, SERVICE_ERROR_NORMAL, binaryPath, NULL, NULL, NULL, NULL, NULL);

    if (!hService)
    {
        std::cout << "Помилка при створенні служби: " << GetLastErrorAsString();
        CloseServiceHandle(hSCManager);
        return WaitForKey();
    }

    std::cout << "Служба успішно створена." << std::endl;

    CloseServiceHandle(hService);
    CloseServiceHandle(hSCManager);

    return WaitForKey();
}

// Запустити службу
int StartSvc(const char* serviceName)
{
    SERVICE_STATUS ServiceStatus;

    SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (!hSCManager)
    {
        std::cout << "Помилка при відкритті диспетчера служб: " << GetLastErrorAsString();
        return WaitForKey();
    }

    SC_HANDLE hService = OpenService(hSCManager, serviceName, SERVICE_START);
    if (!hService)
    {
        std::cout << "Помилка при відкритті служби: " << GetLastErrorAsString();
        CloseServiceHandle(hSCManager);
        return WaitForKey();
    }

    if (!StartService(hService, 0, NULL))
    {
        std::cout << "Помилка при запуску служби: " << GetLastErrorAsString();
        CloseServiceHandle(hService);
        CloseServiceHandle(hSCManager);
        return WaitForKey();
    }

    std::cout << "Служба успішно запущена." << std::endl;

    CloseServiceHandle(hService);
    CloseServiceHandle(hSCManager);

    return WaitForKey();
}

// Зупинити службу
int StopSvc(const char* serviceName)
{
    SERVICE_STATUS ServiceStatus;

    SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (!hSCManager)
    {
        std::cout << "Помилка при відкритті диспетчера служб: " << GetLastErrorAsString();
        return WaitForKey();
    }

    SC_HANDLE hService = OpenService(hSCManager, serviceName, SERVICE_STOP);
    if (!hService)
    {
        std::cout << "Помилка при відкритті служби: " << GetLastErrorAsString();
        CloseServiceHandle(hSCManager);
        return WaitForKey();
    }

    if (!ControlService(hService, SERVICE_CONTROL_STOP, &ServiceStatus))
    {
        std::cout << "Помилка при зупинці служби: " << GetLastErrorAsString();
        CloseServiceHandle(hService);
        CloseServiceHandle(hSCManager);
        return WaitForKey();
    }

    std::cout << "Служба зупинена." << std::endl;

    CloseServiceHandle(hService);
    CloseServiceHandle(hSCManager);

    return WaitForKey();
}

// Перезапустити службу
int RestartSvc(const char* serviceName)
{
    SERVICE_STATUS ServiceStatus;
    DWORD dwWaitTime;

    SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (!hSCManager)
    {
        std::cout << "Помилка при відкритті диспетчера служб: " << GetLastErrorAsString();
        return WaitForKey();
    }

    SC_HANDLE hService = OpenService(hSCManager, serviceName, SERVICE_START | SERVICE_STOP | SERVICE_QUERY_STATUS);
    if (!hService)
    {
        std::cout << "Помилка при відкритті служби: " << GetLastErrorAsString();
        CloseServiceHandle(hSCManager);
        return WaitForKey();
    }

    if (!QueryServiceStatus(hService, &ServiceStatus))
    {
        std::cout << "Помилка при отриманні статусу служби: " << GetLastErrorAsString();
        CloseServiceHandle(hService);
        CloseServiceHandle(hSCManager);
        return WaitForKey();
    }

    if (ServiceStatus.dwCurrentState != SERVICE_STOPPED)
    {
        if (!ControlService(hService, SERVICE_CONTROL_STOP, &ServiceStatus))
        {
            std::cout << "Помилка при зупинці служби: " << GetLastErrorAsString();
            CloseServiceHandle(hService);
            CloseServiceHandle(hSCManager);
            return WaitForKey();
        }
        else
        {
            dwWaitTime = ServiceStatus.dwWaitHint / 20;
            if (dwWaitTime < 1000)
                dwWaitTime = 1000;
            else if (dwWaitTime > 10000)
                dwWaitTime = 10000;

            Sleep(dwWaitTime);

            if (!StartService(hService, 0, NULL))
            {
                std::cout << "Помилка при запуску служби: " << GetLastErrorAsString();
                CloseServiceHandle(hService);
                CloseServiceHandle(hSCManager);
                return WaitForKey();
            }
            std::cout << "Служба успішно перезапущена." << std::endl;
        }
    }
    else
    {
        if (!StartService(hService, 0, NULL))
        {
            std::cout << "Помилка при запуску служби: " << GetLastErrorAsString();
            CloseServiceHandle(hService);
            CloseServiceHandle(hSCManager);
            return WaitForKey();
        }
        std::cout << "Служба успішно перезапущена." << std::endl;
    }

    CloseServiceHandle(hService);
    CloseServiceHandle(hSCManager);

    return WaitForKey();
}

// Призупинити службу
int PauseSvc(const char* serviceName)
{
    SERVICE_STATUS ServiceStatus;

    SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (!hSCManager)
    {
        std::cout << "Помилка при відкритті диспетчера служб: " << GetLastErrorAsString();
        return WaitForKey();
    }

    SC_HANDLE hService = OpenService(hSCManager, serviceName, SERVICE_PAUSE_CONTINUE);
    if (!hService)
    {
        std::cout << "Помилка при відкритті служби: " << GetLastErrorAsString();
        CloseServiceHandle(hSCManager);
        return WaitForKey();
    }

    if (!ControlService(hService, SERVICE_CONTROL_PAUSE, &ServiceStatus))
    {
        std::cout << "Помилка при призупиненні служби: " << GetLastErrorAsString();
        CloseServiceHandle(hService);
        CloseServiceHandle(hSCManager);
        return WaitForKey();
    }

    std::cout << "Служба призупинена." << std::endl;

    CloseServiceHandle(hService);
    CloseServiceHandle(hSCManager);

    return WaitForKey();
}

// Відновити служби
int ResumeSvc(const char* serviceName)
{
    SERVICE_STATUS ServiceStatus;

    SC_HANDLE hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_CONNECT);
    if (!hSCManager)
    {
        std::cout << "Помилка при відкритті диспетчера служб: " << GetLastErrorAsString();
        return WaitForKey();
    }

    SC_HANDLE hService = OpenService(hSCManager, serviceName, SERVICE_PAUSE_CONTINUE);
    if (!hService)
    {
        std::cout << "Помилка при відкритті служби: " << GetLastErrorAsString();
        CloseServiceHandle(hSCManager);
        return WaitForKey();
    }

    if (!ControlService(hService, SERVICE_CONTROL_CONTINUE, &ServiceStatus))
    {
        std::cout << "Помилка при відновленні служби: " << GetLastErrorAsString();
        CloseServiceHandle(hService);
        CloseServiceHandle(hSCManager);
        return WaitForKey();
    }

    std::cout << "Служба відновлена." << std::endl;

    CloseServiceHandle(hService);
    CloseServiceHandle(hSCManager);

    return WaitForKey();
}

int main()
{
    SetConsoleTitle("Service Control Manager");
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
    char open[MAX_PATH] = { 0 };
    HWND hwnd = 0;
    GetModuleFileNameA(NULL, open, sizeof(open));
    hwnd = FindWindow(NULL, open);
    SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
    SetLayeredWindowAttributes(hwnd, 0, (255 * 95) / 100, LWA_ALPHA);
    ConsoleCursorVisible(false, 100);

    string serviceName, displayName, binaryPath;
    string Menu[] = { "[1] Отримати список служб", "[2] Створити нову службу", "[3] Запустити службу", "[4] Зупинити службу", "[5] Перезапустити службу", "[6] Призупинити службу", "[7] Відновити службу" };
    size_t active_menu = 0;

    while (true)
    {
        GoToXY(0, 0);

        for (size_t i = 0; i < size(Menu); i++)
        {
            if (i == active_menu)
                setColor(Yellow, Black);
            else
                setColor(DarkGray, Black);

            cout << Menu[i] << endl;
        }

        setColor(Yellow, Black);
        cout << "\n_________________________________\n";
        wchar_t arrow[] = L"\nВикористовуйте \u2191\u2193 для навігації.\n";
        WriteConsoleW(hConsole, arrow, wcslen(arrow), NULL, NULL);
        cout << "Натисніть Enter для вибору.\n";
        cout << "Натисніть Esc для виходу.\n";

        switch (_getch())
        {
        case ESC:
            exit(0);
        case UP:
            if (active_menu > 0)
                active_menu--;
            break;
        case DOWN:
            if (active_menu < size(Menu) - 1)
                active_menu++;
            break;
        case ENTER:
            setColor(Green, Black);
            system("cls");
            switch (active_menu)
            {
            case 0:
                GetSvcList();
                system("cls");
                break;
            case 1:
                cout << "Введіть назву служби: ";
                getline(cin, serviceName);
                cout << "Введіть відображуване ім'я: ";
                getline(cin, displayName);
                cout << "Введіть шлях до файлу: ";
                getline(cin, binaryPath);
                CreateSvc(serviceName.c_str(), displayName.c_str(), binaryPath.c_str());
                system("cls");
                break;
            case 2:
                cout << "Введіть назву служби: ";
                getline(cin, serviceName);
                StartSvc(serviceName.c_str());
                system("cls");
                break;
            case 3:
                cout << "Введіть назву служби: ";
                getline(cin, serviceName);
                StopSvc(serviceName.c_str());
                system("cls");
                break;
            case 4:
                cout << "Введіть назву служби: ";
                getline(cin, serviceName);
                RestartSvc(serviceName.c_str());
                system("cls");
                break;
            case 5:
                cout << "Введіть назву служби: ";
                getline(cin, serviceName);
                PauseSvc(serviceName.c_str());
                system("cls");
                break;
            case 6:
                cout << "Введіть назву служби: ";
                getline(cin, serviceName);
                ResumeSvc(serviceName.c_str());
                system("cls");
                break;
            }
        }
    }
    return 0;
}
