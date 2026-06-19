// BufferOverflow.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iomanip>
#include <iostream>
#include <string>   // used to temporarily hold user input so its length can be validated before copying into a fixed buffer
#include <cstring> // used for bounded C‑style string functions that prevent writing past the end of the char array

// added to define a single, clear buffer size so bounds are consistent everywhere
constexpr size_t USER_BUFFER_SIZE = 20;

/*
    added helper function to apply a guarded copy pattern
    this function checks if a buffer overflow would occur before copying
    it returns true if the copy is safe and false if an overflow attempt is detected
*/
bool safe_copy_to_buffer(const std::string& input, char* destination, size_t buffer_size)
{
    // added pre-check to detect a potential overflow before modifying memory
    if (input.length() >= buffer_size)
    {
        return false; // notify the calling function that an overflow would occur
    }

    // added bounded copy so the destination buffer cannot be overrun
    strncpy_s(destination, buffer_size, input.c_str(), _TRUNCATE);
    return true;
}

int main()
{
    std::cout << "Buffer Overflow Example" << std::endl;

    // TODO: The user can type more than 20 characters and overflow the buffer, resulting in account_number being replaced -
    //  even though it is a constant and the compiler buffer overflow checks are on.
    //  You need to modify this method to prevent buffer overflow without changing the account_number
    //  variable, and its position in the declaration. It must always be directly before the variable used for input.
    //  You must notify the user if they entered too much data.

    const std::string account_number = "CharlieBrown42";
    char user_input[USER_BUFFER_SIZE] = { 0 };  // added initialization so the buffer always starts in a known safe state
    std::string last_safe_value;  // added to track the most recent valid input for accurate overflow reporting

    std::string safe_input;
    bool success;

    while (true) // loop until user decides to quit
    {
        std::cout << "\nEnter a value (or type 'exit' to quit): ";
        std::getline(std::cin, safe_input);

        if (safe_input == "exit") // allow user to end the program when they choose
        {
            std::cout << "Exiting program." << std::endl;
            break;
        }

        success = safe_copy_to_buffer(safe_input, user_input, USER_BUFFER_SIZE);

        if (!success)
        {
            std::cout << "Buffer overflow attempt detected and blocked." << std::endl;
            std::cout << "Last safe value: \"" << last_safe_value << "\"" << std::endl;  // use tracked safe value
        }
        else
        {
            std::cout << "You entered: " << user_input << std::endl;
            last_safe_value = user_input;  // update last safe value after successful input
        }

        // account number must remain unchanged even if an overflow attempt occurs
        std::cout << "Account Number = " << account_number << std::endl;
    }

    return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu