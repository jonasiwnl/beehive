#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <atomic>

#ifdef _WIN32
#include <WinSock2.h>
#else
#include <sys/socket.h>
#include <unistd.h> // close()
#endif

using std::cin, std::cerr;      // IO
using std::string, std::vector; // Containers
using std::thread, std::atomic; // Multithreading


constexpr uint16_t MAX_CONNECTIONS = 10;

struct StreamServer {
    atomic<bool> server_up_flag;
    int client_socket;
    vector<thread> handlers;
    string error_message;

    StreamServer(int client_socket_in): server_up_flag(true), client_socket(client_socket_in)
    {
        handlers.reserve(MAX_CONNECTIONS);
    }

    // Listen for new connections, accept them, and handle with a separate thread.
    void listen_and_accept()
    {
        thread quit_listener(&StreamServer::wait_for_quit_input, this);
        while (server_up_flag) {
            int client = accept(client_socket, NULL, NULL);
            if (client < 0) {
                error_message = "ERROR: couldn't accept connection.\n";
                server_up_flag = false;
                break;
            }

            // Spin up a thread for each client.
            handlers.emplace_back(thread(&StreamServer::handle_client, this, client));
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

bool run_server() {
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        cerr << "ERROR: couldn't create socket.\n";
        return false;
    }

    /* TODO 3. Listen for connections and accept them (as well as for interrupt) */
    StreamServer ss = StreamServer(client_socket);
    ss.listen_and_accept();
#ifdef _WIN32
    closesocket(client_socket);
#else
    close(client_socket);
#endif

    if (!ss.error_message.empty()) {
        cerr << ss.error_message;
        return false;
    }

    return true;
}
