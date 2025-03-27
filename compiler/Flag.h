#pragma once

// Structure to hold information about a variable
typedef struct FlagVar
{
    int index;
    bool used;
    bool affected;
}FlagVar;


//Strucutre to hold informatin about a function
typedef struct FlagFonction
{
    bool used;
    bool declared;
    int nombreParams;
}FlagFonction;