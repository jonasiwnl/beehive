#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <atomic>
#include <sys/socket.h>
#include "screencapture.cpp"

using std::cout, std::cin, std::cerr; // IO
using std::string, std::vector; // Containers
using std::thread, std::atomic; // Multithreading


constexpr uint16_t MAX_CONNECTIONS = 10;

struct ClientHandler {
    atomic<bool> server_up_flag;
    int client_socket;
    vector<thread> handlers;
    string error_message;

    ClientHandler(int client_socket_in): server_up_flag(true), client_socket(client_socket_in)
    {
        handlers.reserve(MAX_CONNECTIONS);
    }

    // Listen for new connections, accept them, and handle with a separate thread.
    void listen_and_accept()
    {
        thread quit_listener(&ClientHandler::wait_for_quit_input, this);
        while (server_up_flag) {
            int client = accept(client_socket, NULL, NULL);
            if (client < 0) {
                error_message = "ERROR: couldn't accept connection.\n";
                server_up_flag = false;
                break;
            }

            // Spin up a thread for each client.
            handlers.emplace_back(thread(&ClientHandler::handle_client, this, client));
        }
        quit_listener.join();
        cleanup_handler_threads();
    }

    // when the user types "q" or "quit", end execution of the server (stop streaming.)
    void wait_for_quit_input()
    {
        string input;
        while (server_up_flag) {
            cin >> input;
            if (input == "q" || input == "quit") server_up_flag = false;
        }
    }

    void handle_client(int client_socket)
    {
        string message = "What up\n";
        while (server_up_flag) {
            /* TODO (jonas): how can client disconnects be handled */
            send(client_socket, message.c_str(), message.size(), 0);
            sleep(1);
        }
    }

    void cleanup_handler_threads()
    {
        for (auto& handler : handlers)
            // These threads should be done executing, as server_up_flag is false
            // So join *should be* safe here
            handler.join();
    }
};

int main()
{
    /* TODO 1. Select a window to stream from */

    ScreenCapture capturer;
    bool success;

#ifdef __APPLE__
    success = capturer.osx_display_active_windows();
    if (!success) {
        cerr << "ERROR: couldn't select window to stream from.\n";
        return 1;
    }

    capturer.capture();
#endif

    return 0; // This is the end of video capture code. Stuff beyond is under construction

    /* TODO 2. Expose a port */
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        cerr << "ERROR: couldn't create socket.\n";
        return 1;
    }

    /* TODO 3. Listen for connections and accept them (as well as for interrupt) */
    ClientHandler cs = ClientHandler(client_socket);
    cs.listen_and_accept();
    close(client_socket);

    if (!cs.error_message.empty()) {
        cerr << cs.error_message;
        return 1;
    }

    return 0;
}
