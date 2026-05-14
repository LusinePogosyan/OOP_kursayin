#ifndef LINKER_H
#define LINKER_H

#include <vector>
#include <string>
#include "ObjectFile.h"

class Linker {
public:
    // Փոխում ենք պարամետրը, որպեսզի այն ընդունի ObjectFile օբյեկտների վեկտոր
    void link(const std::vector<ObjectFile>& objects, const std::string& outputPath);
};

#endif