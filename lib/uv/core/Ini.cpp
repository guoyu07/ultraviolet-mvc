/*~
 * Ultraviolet MVC
 * Copyright (C) 2010 Lost Mind Software
 *
 * Licensed under the MIT license: http://www.opensource.org/licenses/mit-license.php
 *
 * @version $Id$
 */

/** @file Ini.cpp
 * Ultraviolet Core Ini class definition.
 *
 * Read and parse an ini file
 */

#include <string>
#include <fstream>
#include <iostream>
#include <sstream>

#include "Ini.h"
#include "core/Strlib.h"

/**
 *  
 */
uv::Ini::Ini(std::string filename)
{
    this->filename = filename;
    
    this->readFile();
}

/**
 *  
 */
uv::Ini::Ini()
{
}

/**
 *  
 */
void uv::Ini::readFile()
{
    std::ifstream iniStream;

    iniStream.open(this->filename.c_str());

    if (iniStream.is_open()) {
        std::string line;

        while (!iniStream.eof()) {
            getline(iniStream, line);
            this->readLine(line);
        }
    } else {
        std::cerr << "Error: Cannot read ini file \""
            << this->filename.c_str()
            << "\"" << std::endl;
    }

    iniStream.close();
}

/**
 *  
 */
std::map<std::string, uv::iniPairs> uv::Ini::getPairs()
{
    return this->keyValuePairs;
}

/**
 *  
 */
int uv::Ini::readLine(std::string line)
{
    size_t length = line.size();
    int i;
    bool foundKey = false;
    std::string key;
    std::string value = "";
    std::string section = "default";
    std::string::size_type pos = 0;
    std::string::size_type charLen = std::string(" ").length();

    if (this->currentSection.length() > 0) {
        section = this->currentSection;
    }
    
    for (i = 0; i < length; i++) {
        if (foundKey == true && key.length() > 0) {
            // Find the comment start (if any)
            // and end parsing the value at that position.
            // FIXME: FLAW. If the value contains a semi-colon it will
            //   end the value at the semi-colon
            pos = line.find(";");

            if (std::string::npos == pos) {
                value = line.substr(i, length - i);
            } else {
                // there is a trailing comment
                value = line.substr(i, pos - i);
            }

            value = Strlib::trim(value);

            // If the value is enclosed in quotes, remove the quotes
            if (value.substr(0, 1) == "\""
                && value.substr(value.length() - charLen, 1) == "\""
            ) {
                value = value.substr(charLen, value.length() - charLen * 2);
            }

            this->setPair(key, value, section);
            return 0;
        }

        // Find a section heading
        if (line.at(i) == '[') {
            pos = line.find("]");

            if (std::string::npos == pos) {
                return 1;
            }

            section = line.substr(i + charLen, pos - i - charLen);
            section = Strlib::trim(section);

            // Store this section
            this->currentSection = section;

            return 0;
        }

        // Check for a comment
        if (line.at(i) == ';') {
            // is a comment; ignore this line
            return 1;
        }

        // Find an equals sign
        pos = line.find("=");

        // If no equals sign was found
        // or this line starts with an equals sign
        // ignore this line
        if (std::string::npos == pos
            || pos == i
        ) {
            return 1;
        }

        // If we haven't found the key yet, set the key
        // This prevents (whitespace) = value from being parsed
        if (foundKey == false) {
            key = line.substr(i, pos - i);
            key = Strlib::trim(key);
            i = pos;
            foundKey = true;
        }

        // If the line ends in an equals sign,
        // be sure to add this key with a blank value
        if (pos == length - charLen) {
            this->setPair(key, "", section);
        }
    }
}

/**
 *  
 */
void uv::Ini::setPair(std::string key, std::string value, std::string section)
{
    uv::iniPairs pair = this->keyValuePairs[section];
    pair[key] = value;
    this->keyValuePairs[section] = pair;
}

/**
 *  
 */
std::string uv::Ini::list()
{
    if (keyValuePairs.empty()) {
        return "Empty.";
    }

    int i;
    std::string out;

    // section iterator
    std::map<std::string, uv::iniPairs>::iterator curr, end;

    // iniPairs iterator
    uv::iniPairs::iterator pcurr, pend;

    for (curr = keyValuePairs.begin(), end = keyValuePairs.end(); curr != end; curr++) {
        out.append("SECTION [" + curr->first + "] =>\n");

        for (pcurr = curr->second.begin(), pend = curr->second.end(); pcurr != pend; pcurr++) {
            out.append(" [" + pcurr->first + "] => " + pcurr->second + "\n");
        }
    }

    return out;
}

/**
 *  
 */
std::string uv::Ini::get(std::string key, std::string section)
{
    return this->keyValuePairs[section][key];
}

/**
 *  
 */
uv::iniPairs uv::Ini::getSection(std::string section)
{
    return this->keyValuePairs[section];
}
