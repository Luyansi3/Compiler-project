#pragma once
#include <string>

// Structure to hold information about a variable
typedef struct FlagVar
{
    int index;
    bool used;
    bool affected;
    std::string functionName; // store the name of the function where the variable is used
    std::string varName; // store the original name of the variable
}FlagVar;


//Strucutre to hold informatin about a function
typedef struct FlagFonction
{
    bool used;
    bool declared;
    int nombreParams;
}FlagFonction;