#pragma once

using namespace std;
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
    string type;
    bool used;
    bool declared;
    int nombreParams;
}FlagFonction;