#include <iostream>
#include <limits>
#include <type_traits>
#include <cmath>
#include <typeinfo>

// Checks whether multiplication can be safely performed without exceeding numeric limits
template <typename T>
bool check_safe_multiplication(T value, unsigned long int steps)
{
    // No risk if no multiplication steps are performed
    if (steps == 0)
        return true;

    /*
        Floating point types are handled differently because they do not
        follow the same overflow rules as integer types
    */
    if constexpr (std::is_floating_point<T>::value)
        return true;

    /*
        Prevent overflow or underflow before multiplication occurs by checking
        against the numeric limits of the data type
    */
    if (value > 0)
    {
        if ((long double)steps > (long double)std::numeric_limits<T>::max() / value)
            return false;
    }
    else if (value < 0)
    {
        if ((long double)steps > (long double)std::numeric_limits<T>::min() / value)
            return false;
    }

    return true;
}

/*
    Performs safe addition using direct mathematical computation instead of iteration

    Original artifact behavior:
    The original implementation used loop-based addition to repeatedly accumulate values
    step-by-step, which increased computational complexity and demonstrated overflow through iteration.

    Enhancement improvements:
    - Replaced iterative logic with direct mathematical computation
    - Reduced time complexity from O(n) to O(1)
    - Introduced pre-check validation before performing multiplication
    - Uses long double for intermediate calculations to improve precision safety
    - Validates final result against numeric limits of the data type
    - Maintains support for both integer and floating-point types
*/
template <typename T>
bool add_numbers(T const& start, T const& increment, unsigned long int const& steps, T& out_result)
{
    // Validate multiplication before performing computation
    if (!check_safe_multiplication(increment, steps))
        return false;

    /*
        Floating point handling uses long double for safer intermediate calculation
    */
    if constexpr (std::is_floating_point<T>::value)
    {
        long double result = (long double)start +
            ((long double)increment * (long double)steps);

        // Ensure result is a valid finite number
        if (!std::isfinite(result))
            return false;

        out_result = (T)result;
        return true;
    }
    else
    {
        /*
            Integer handling uses long double to prevent overflow during calculation
        */
        long double result = (long double)start +
            ((long double)increment * (long double)steps);

        // Ensure result stays within valid numeric range for the type
        if (result > (long double)std::numeric_limits<T>::max() ||
            result < (long double)std::numeric_limits<T>::min())
            return false;

        out_result = (T)result;
        return true;
    }
}

/*
    Performs safe subtraction using direct mathematical computation instead of iteration

    This function mirrors the structure of the addition function and applies the same
    enhancement principles to subtraction logic.

    Enhancement improvements:
    - Replaced iterative subtraction with direct computation
    - Reduced complexity from O(n) to O(1)
    - Added pre-check validation before multiplication
    - Uses type-safe intermediate calculations using long double
    - Ensures final result remains within valid numeric bounds
*/
template <typename T>
bool subtract_numbers(T const& start, T const& decrement, unsigned long int const& steps, T& out_result)
{
    // Validate multiplication before performing subtraction
    if (!check_safe_multiplication(decrement, steps))
        return false;

    /*
        Floating point handling uses long double for intermediate precision safety
    */
    if constexpr (std::is_floating_point<T>::value)
    {
        long double result = (long double)start -
            ((long double)decrement * (long double)steps);

        // Ensure result is finite and valid
        if (!std::isfinite(result))
            return false;

        out_result = (T)result;
        return true;
    }
    else
    {
        /*
            Integer handling validates against numeric limits after computation
        */
        long double result = (long double)start -
            ((long double)decrement * (long double)steps);

        // Ensure result is within allowable type range
        if (result > (long double)std::numeric_limits<T>::max() ||
            result < (long double)std::numeric_limits<T>::min())
            return false;

        out_result = (T)result;
        return true;
    }
}