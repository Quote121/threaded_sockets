// Networking headers
#ifdef _WIN32
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <Windows.h>
#endif

#include <string>
#include <memory>
#include <vector>
#include <mutex>
#include <cstring>
#include <future>

// Network specific macros
#define ISVALIDSOCKET(s) ((s) != INVALID_SOCKET)
#define CLOSESOCKET(s) closesocket(s)
#define GETSOCKETERRNO() (WSAGetLastError())

// 2^16 - 40. 
// 40 is the minimum size of the tcp packet
// 2 is our overhead of the packet 2 bytes for size
constexpr unsigned int MAXTCPPAYLOAD = 65535-40-2;

// Chat related
#define MAXALIASSIZE 10

struct Packet
{
    unsigned char msgType;
    std::string message;
};

namespace MessageType{

    enum MessageType: unsigned char
    {
        // Alias
        ALIASSET,   // Client <-> Server Requesting and Setting alias 
        ALIASACK,   // Server -> Client Accept alias. Sends accepted username
        ALIASDNY,   // Server -> Client Reject alias. Send reason why

        // General message
        MESSAGE,    // Client <-> Server

        // Server info
        CONNUSERS,  // Server -> Client Number of connected users

    };

    std::string GetMessageType(unsigned char msgByte);
}


class NetworkedUser
{
private:
    SOCKET m_userSocket;
    std::string m_alias;
    time_t connectionTime;
    char address[NI_MAXHOST];

public:
    NetworkedUser(SOCKET sock_in, std::string name_in, time_t time_in, const char addr_in[NI_MAXHOST]) : 
        m_userSocket(sock_in), m_alias(name_in), connectionTime(time_in)
    {
        std::strcpy(address, addr_in);
    }

    SOCKET m_GetUserSocket(void) const 
    {
        return m_userSocket;
    }

    std::string m_GetUserAlias(void) const 
    {
        return m_alias;
    }

    const char* m_GetUserAddress(void) const 
    {
        return address;
    }

};

// Shared pointer of NetworkedUser
using spNetworkedUser = std::shared_ptr<NetworkedUser>;

class NetworkHandler
{
private:
    struct addrinfo* peer_address;
    SOCKET m_serverSocket;

    std::mutex connectedUserVectorMutex;
    std::vector<spNetworkedUser> connectedUsers;

    std::mutex asyncConnectionJobsMutex;
    std::vector<std::future<void>> asyncConnectionJobs;


    /// @brief Handle new connection (accept) asyncronously
    /// @param userSocket socket handle
    /// @param address network address
    void m_AsyncNewConnectionHandle(SOCKET userSocket, const char address[NI_MAXHOST]);

public:

    /// @brief Attempt to add a new user. Will reject if the username is taken
    /// @param user The shared pointer of the new user we are attempting to add
    /// @return bool - success (failed means username taken)
    bool m_AttemptAddNetworkedUser(spNetworkedUser user);


    /// @brief Return a copy of the connected user vector
    /// @return vector copy of connected users
    std::vector<spNetworkedUser> m_GetNetworkedUsers(void);


    /// @brief Get the number of connected users (with registered usernames)
    /// @return int - number of connected users
    int m_GetNetworkedUsersCount(void);
    

    /// @brief Clear the connected user vector thread safe
    void m_ClearNetworkedUserVector(void);

    
    /// @brief Holds the future return of a std::asnyc
    /// @param job the std::async return to be passed
    void m_AddAsyncConnectionJob(std::future<void>&& job);


    /// @brief Sets any socket as either blocking or non-blocking
    /// @param blocking boolean either true for blocking or false for non-blocking
    void SetSocketBlocking(bool blocking, SOCKET socket);


    /// @brief Creates the socket and policies
    /// @param port port number string
    /// @return bool - success
    bool m_Create(std::string port);


    /// @brief will set the socket to listen to n amount of connections
    /// @param connections will set the number of users the server will allow to queue to connect at any one point
    /// @return bool - success
    bool m_Listen(int connections);


    /// @brief Accept any incoming connections
    /// @return bool - success
    bool m_Accept(void);


    /// @brief Check for a message from a user, if there is one we can recieve it
    /// @param connectedUser is the handle to check the connected user
    /// @param messageOut is the returned message, will only be set if method return true
    /// @return bool - if there was a message
    bool m_ReceiveMessage(spNetworkedUser connectedUser, std::string& messageOut);


    /// @brief Send a message to all connected users
    /// @param messageType Type of messsage
    /// @param Sender NetworkedUser struct of the users sending the message
    /// @param message string to send
    /// @return bool - success
    bool m_BroadcastMessage(unsigned char messageType, spNetworkedUser sender, std::string message);


    /// @brief Send message to certain connected user. The only method that will call Ws2 send()
    /// @param messageType Type of messsage
    /// @param recipient is who to send the message to
    /// @param message is the message
    /// @return bool - success
    bool m_Send(unsigned char messageType, SOCKET recipient, const std::string& message);


    /// @brief Wrapper for recv()
    /// @param sender Connected user struct to recieve the data (can be nullptr)
    /// @param senderSock Non-connected user to recieve the data like in accept (can be nullptr)
    /// @param incomingPacketOut Struct passed by ref. Will set the data if method return true
    /// @param blocking Flag for if the sockets are set as blocking
    /// @return bool - success
    bool m_Recv(spNetworkedUser sender, SOCKET* senderSock, Packet& incomingPacketOut, bool blocking);


    /// @brief Disconnect a connected user
    /// @param userToDisconnect Connected user struct
    /// @return bool - success
    bool m_DisconnectUser(const spNetworkedUser userToDisconnect);


    /// @brief Closes all connections and clean up
    /// @return bool - success
    bool m_Shutdown(void);
};
