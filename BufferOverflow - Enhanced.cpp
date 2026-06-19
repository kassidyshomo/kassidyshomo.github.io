#include <iostream>
#include <string>
#include <cstring>

// Defines a single consistent buffer size used across the program
constexpr size_t USER_BUFFER_SIZE = 20;

// Defines the maximum number of consecutive invalid attempts allowed
constexpr int MAX_INVALID_ATTEMPTS = 3;

/*
    Performs validated bounded copying into a fixed-size buffer.
    Prevents overflow before memory modification occurs and separates
    validation logic from the main program flow.
*/
bool safe_copy_to_buffer(const std::string& input, char* destination, size_t buffer_size)
{
    // Prevent overflow before modifying memory
    if (input.length() >= buffer_size)
    {
        return false;
    }

    // Safe bounded copy into fixed-size buffer
    strncpy_s(destination, buffer_size, input.c_str(), _TRUNCATE);

    return true;
}

int main()
{
    std::cout << "Buffer Overflow Prevention Example" << std::endl;

    /*
        Original artifact behavior:
        The original version demonstrated how unsafe user input could
        overflow a fixed-size character buffer and overwrite nearby memory.

        Enhancement improvements:
        - Input is validated before copying into memory
        - Unsafe input is blocked before overflow can occur
        - Validation logic is separated into a dedicated helper function
        - Invalid input attempts are tracked and controlled
        - Last valid input is preserved for safer program behavior
        - Memory integrity is maintained across all execution paths
    */

    const std::string account_number = "CharlieBrown42";

    // Fixed-size buffer representing the vulnerable memory region
    char user_input[USER_BUFFER_SIZE] = { 0 };

    // Tracks the most recent valid input for safe fallback reporting
    std::string last_safe_value;

    std::string safe_input;
    bool success;

    // Tracks consecutive invalid input attempts
    int attempt_count = 0;

    while (true)
    {
        std::cout << "\nEnter a value (or type 'exit' to quit): ";
        std::getline(std::cin, safe_input);

        // Allows safe exit
        if (safe_input == "exit")
        {
            std::cout << "Exiting program." << std::endl;
            break;
        }

        // Handles empty input
        if (safe_input.empty())
        {
            std::cout << "Input cannot be empty." << std::endl;

            attempt_count++;

            // Terminates the program after repeated invalid attempts
            if (attempt_count >= MAX_INVALID_ATTEMPTS)
            {
                std::cout << "Too many invalid attempts. Program terminating." << std::endl;
                break;
            }

            continue;
        }

        success = safe_copy_to_buffer(safe_input, user_input, USER_BUFFER_SIZE);

        if (!success)
        {
            std::cout << "Buffer overflow attempt detected and blocked." << std::endl;
            std::cout << "Last safe value: \"" << last_safe_value << "\"" << std::endl;

            attempt_count++;

            // Terminates the program after repeated overflow attempts
            if (attempt_count >= MAX_INVALID_ATTEMPTS)
            {
                std::cout << "Too many invalid attempts. Program terminating." << std::endl;
                break;
            }
        }
        else
        {
            std::cout << "You entered: " << user_input << std::endl;

            // Updates the most recent safe program state
            last_safe_value = user_input;

            // Resets invalid attempt tracking after successful validation
            attempt_count = 0;
        }

        std::cout << "Attempt count: " << attempt_count << std::endl;

        /*
            The account number remains unchanged throughout execution.
            This confirms that the enhancement successfully preserves
            memory integrity during overflow attempts.
        */
        std::cout << "Account Number = " << account_number << std::endl;
    }

    return 0;
}