#pragma once

#include <string>

#ifdef _WIN32
    #include <windows.h>
    using pipe_handle_t = HANDLE;
    using proc_handle_t = HANDLE;
#else
    #include <unistd.h>
    #include <sys/types.h>
    using pipe_handle_t = int;
    using proc_handle_t = pid_t;
#endif

class Process {
public:
    Process(
        const std::string& binary_path, 
        const std::string& output_file_path
    );

    ~Process();

    Process(const Process&) = delete;
    Process& operator=(const Process&) = delete;

    void send_line(const std::string& line);
    void close_input();
    void wait();

    unsigned long get_pid() const;

private:
    pipe_handle_t write_fd;
    proc_handle_t pid;
    bool is_closed;
};
