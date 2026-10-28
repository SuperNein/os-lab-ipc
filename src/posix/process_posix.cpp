#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <cstdio>
#include <stdexcept>

#include "process.hpp"

Process::Process(
        const std::string& binary_path, 
        const std::string& output_file_path
    ) {
        int file_fd = open(
            output_file_path.c_str(), 
            O_WRONLY | O_CREAT | O_TRUNC, 
            0644
        );

        if (file_fd == -1) {
            perror("open");
            throw std::runtime_error("Failed to open output file: " + output_file_path);
        }
        
        int pipefd[2]{};
        
        if (pipe(pipefd) == -1) {
            perror("pipe");
            close(file_fd);
            
            throw std::runtime_error("Failed to create pipe");
        }

        pid_t fork_rc = fork();
        
        if (fork_rc == -1) {
            // failed
            perror("fork");

            close(pipefd[0]);
            close(pipefd[1]);

            throw std::runtime_error("Failed to fork");
        } else if (fork_rc == 0) {
            // child
            if (dup2(pipefd[0], STDIN_FILENO) == -1) {
                perror("dup2 stdin");
                _exit(EXIT_FAILURE);
            }

            if (dup2(file_fd, STDOUT_FILENO) == -1) {
                perror("dup2 stdout");
                _exit(EXIT_FAILURE);
            }

            close(file_fd);
            close(pipefd[0]);
            close(pipefd[1]);

            execl(
                binary_path.c_str(), 
                binary_path.c_str(), 
                static_cast<char*>(nullptr)
            );

            perror("execl");
            _exit(127);
        } else {
            // parent
            this->pid = fork_rc;
            this->write_fd = pipefd[1];
            this->is_closed = false;

            close(pipefd[0]);
            close(file_fd);
        }
}

void Process::send_line(const std::string& line) {
    if (is_closed || write_fd == -1) {
        throw std::runtime_error("Attempted to write to a closed or invalid pipe");
    }

    std::string f_line = line;
    
    if (f_line.empty() || f_line.back() != '\n') {
        f_line += '\n';
    }

    ssize_t bytes_written = write(write_fd, f_line.data(), f_line.size());
    
    if (bytes_written == -1) {
        perror("write");
        throw std::runtime_error("Failed to write data to pipe");
    }
}

void Process::close_input() {
    if (!is_closed && write_fd != -1) {
        if (close(write_fd) == -1) {
            perror("close write_fd");
        }

        write_fd = -1;
        is_closed = true;
    }
}

void Process::wait() {
    if (pid > 0) {
        if (waitpid(pid, nullptr, 0) == -1) {
            perror("waitpid");
        }

        pid = -1;
    }
}

Process::~Process() {
    close_input();
    wait();
}

unsigned long Process::get_pid() const {
    return static_cast<unsigned long>(pid);
}
