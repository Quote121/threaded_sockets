#include <mutex>

// Characters
const char wholeBlockChar = 219;
const char lowerBlockChar = 220;
const char upperBlockChar = 223;

// Forward definition
class MessageHandler;

class Display
{
private:
    /// @brief Sets the cursor position in the terminal
    /// @param x_col set the x or column position
    /// @param y_row set the y or row position
    /// @return bool if the oporation was sucessful
    static bool s_GoToXY(const short x_col, const short y_row);


    /// @brief Internally called draw info method. To avoid deadlocks
    /// @brief The method can be called externally with s_DrawInfoDisplayMux
    /// @param messageHandlerHandle 
    static void s_DrawInfoDisplay(MessageHandler *messageHandlerHandle);


    /// @brief Helper function for the too small window message
    /// @param columns current columns in the window
    /// @param rows current rows in the window
    /// @return bool - is the window too small
    static bool s_IsWindowTooSmall(short columns, short rows);

public:

    static std::mutex s_writeToScreenMutex; // For drawing to screen via iostream


    /// @brief Sets the terminal into raw mode that disables echo and line input
    static void s_SetTerminalModeRaw(void);


    /// @brief Resets the terminal mode to re-enable echo and line input
    static void s_SetTerminalModeReset(void);


    /// @brief returns the max coordinate in the console (zero indexed)
    /// @param returns the value columns via ref
    /// @param returns the value rows via ref
    static void s_GetConsoleMaxCoords(short& columns, short& rows);


    /// @brief clears the current terminal screen
    static void s_ClearTerminal(void);


    /// @brief draws the display
    /// @param messageHandlerHandle handle to message handler
    static void s_Draw(MessageHandler* messageHandlerHandle);


    /// @brief Thread safe way to access std::cout
    /// @param Mutex for screen writing
    /// @param X position on screen
    /// @param Y position on screen
    /// @param msg - string reference to the message
    static void s_WriteToScreen(short x_col, short y_row, std::string& msg);


    /// @brief Write characters in the input field. Should only be used by messageHandler
    // @param msg - what to put into cout
    static void s_WriteToInputDisplay(std::string msg);
    static void s_WriteToInputDisplay(char c);


    /// @brief Clear the input field area
    static void s_ClearInputField(void);


    /// @brief Draw the messages in the display
    /// @param MessageHandler object to get the messages
    static void s_DrawMessageDisplay(MessageHandler* messageHandlerHandle);


    /// @brief Update the info section of the display
    /// @param MessageHandler object to get the messages
    static inline void s_DrawInfoDisplayMux(MessageHandler *messageHandlerHandle);
};

// Inline definition of s_DrawInfoDisplay with the draw mutex
inline void Display::s_DrawInfoDisplayMux(MessageHandler *messageHandlerHandle)
{
    std::lock_guard<std::mutex> lock(Display::s_writeToScreenMutex);
    s_DrawInfoDisplay(messageHandlerHandle);
}
