#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <cstring>
#include <getopt.h>
using std::cerr;

void perror_exit(const char* msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int main(int argc, char* argv[]) {
    const char* filename = nullptr;
    const char* message = nullptr;
    bool clear = false;

    // Parse command line options
    int opt;
    while ((opt = getopt(argc, argv, "c")) != -1) {
        switch (opt) {
            case 'c':
                clear = true;
                break;
            default:
                cerr << "Usage: " << argv[0] << " [-c] <filename> <message>\n";
                return EXIT_FAILURE;
        }
    }

    // Check for remaining arguments
    if (optind + 2 != argc) {
        cerr << "Usage: " << argv[0] << " [-c] <filename> <message>\n";
        return EXIT_FAILURE;
    }

    filename = argv[optind];
    message = argv[optind + 1];

    // Check if the file exists and its permissions
    struct stat statbuf;
    if (stat(filename, &statbuf) == -1) {
        // File does not exist, create it
        int fd = open(filename, O_CREAT | O_WRONLY, 0000); // No permissions
        if (fd == -1) {
            perror_exit("Failed to create file");
        }
        close(fd);
    } else if (statbuf.st_mode & (S_IRWXU | S_IRWXG | S_IRWXO)) {
        cerr << "Error: File has permissions. Cannot proceed.\n";
        return EXIT_FAILURE;
    }

    // Temporarily change permissions to allow writing
    if (chmod(filename, S_IWUSR) == -1) {
        perror_exit("Failed to change file permissions");
    }

    // Open the file for writing
    int fd = open(filename, O_WRONLY | (clear ? O_TRUNC : O_APPEND));
    if (fd == -1) {
        perror_exit("Failed to open file for writing");
    }

    // Write the message to the file
    if (write(fd, message, strlen(message)) == -1) {
        perror_exit("Failed to write to file");
    }
    if (write(fd, "\n", 1) == -1) {
        perror_exit("Failed to write newline to file");
    }

    // Close the file
    if (close(fd) == -1) {
        perror_exit("Failed to close file");
    }

    // Revoke permissions
    if (chmod(filename, 0000) == -1) {
        perror_exit("Failed to revoke file permissions");
    }

    return EXIT_SUCCESS;
}

