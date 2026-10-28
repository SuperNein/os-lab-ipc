#include <windows.h>
#include <iostream>
#include <stdexcept>
#include <string>

#include "process.hpp"

Process::Process(
    const std::string& binary_path, 
    const std::string& output_file_path
) {
    SECURITY_ATTRIBUTES sa{};
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = nullptr;

    HANDLE file_fd = CreateFileA(
        output_file_path.c_str(),
        GENERIC_WRITE,
        FILE_SHARE_READ,
        &sa,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr
    );
    if (file_fd == INVALID_HANDLE_VALUE) {
        std::cerr << "CreateFileA error: " << GetLastError() << '\n';
        throw std::runtime_error("Failed to open output file: " + output_file_path);
    }

    HANDLE pipefd[2]{INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE};
    if (!CreatePipe(&pipefd[0], &pipefd[1], &sa, 0)) {
        std::cerr << "CreatePipe error: " << GetLastError() << '\n';
        CloseHandle(file_fd);
        throw std::runtime_error("Failed to create pipe");
    }

    if (!SetHandleInformation(pipefd[1], HANDLE_FLAG_INHERIT, 0)) {
        std::cerr << "SetHandleInformation error: " << GetLastError() << '\n';
        CloseHandle(file_fd);
        CloseHandle(pipefd[0]);
        CloseHandle(pipefd[1]);
        throw std::runtime_error("Failed to set handle information");
    }

    STARTUPINFOA si{};
    si.cb = sizeof(STARTUPINFOA);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = pipefd[0];
    si.hStdOutput = file_fd;
    si.hStdError = GetStdHandle(STD_ERROR_HANDLE);

    PROCESS_INFORMATION pi{};
    std::string cmd = binary_path;

    BOOL create_rc = CreateProcessA(
        nullptr,
        cmd.data(),
        nullptr,
        nullptr,
        TRUE,
        0,
        nullptr,
        nullptr,
        &si,
        &pi
    );

    if (!create_rc) {
        std::cerr << "CreateProcessA error: " << GetLastError() << '\n';
        CloseHandle(file_fd);
        CloseHandle(pipefd[0]);
        CloseHandle(pipefd[1]);
        throw std::runtime_error("Failed to create process");
    }

    this->pid = pi.hProcess;
    this->write_fd = pipefd[1];
    this->is_closed = false;

    CloseHandle(pi.hThread);
    CloseHandle(pipefd[0]);
    CloseHandle(file_fd);
}

void Process::send_line(const std::string& line) {
    if (is_closed || write_fd == INVALID_HANDLE_VALUE) {
        throw std::runtime_error("Attempted to write to a closed or invalid pipe");
    }

    std::string f_line = line;
    if (f_line.empty() || f_line.back() != '\n') {
        f_line += '\n';
    }

    DWORD bytes_written = 0;
    BOOL write_rc = WriteFile(
        write_fd,
        f_line.data(),
        static_cast<DWORD>(f_line.size()),
        &bytes_written,
        nullptr
    );

    if (!write_rc) {
        std::cerr << "WriteFile error: " << GetLastError() << '\n';
        throw std::runtime_error("Failed to write data to pipe");
    }
}

void Process::close_input() {
    if (!is_closed && write_fd != INVALID_HANDLE_VALUE) {
        if (!CloseHandle(write_fd)) {
            std::cerr << "CloseHandle write_fd error: " << GetLastError() << '\n';
        }
        write_fd = INVALID_HANDLE_VALUE;
        is_closed = true;
    }
}

void Process::wait() {
    if (pid != INVALID_HANDLE_VALUE && pid != nullptr) {
        if (WaitForSingleObject(pid, INFINITE) == WAIT_FAILED) {
            std::cerr << "WaitForSingleObject error: " << GetLastError() << '\n';
        }
        if (!CloseHandle(pid)) {
            std::cerr << "CloseHandle pid error: " << GetLastError() << '\n';
        }
        pid = INVALID_HANDLE_VALUE;
    }
}

Process::~Process() {
    close_input();
    wait();
}

unsigned long Process::get_pid() const {
    if (pid == INVALID_HANDLE_VALUE || pid == nullptr) {
        return 0;
    }
    return static_cast<unsigned long>(GetProcessId(pid));
}
