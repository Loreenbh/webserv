/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   utils.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: tissad <tissad@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/03/08 19:49:21 by glions            #+#    #+#             */
/*   Updated: 2025/05/30 17:02:25 by tissad           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "utils.hpp"

std::vector<std::string> splitString(const std::string str, char sep)
{
    std::vector<std::string> values;
    std::stringstream ss(str);
    std::string value;

    while (std::getline(ss, value, sep))
    {
        value.erase(std::remove(value.begin(), value.end(), '\t'), value.end());
        if (value.size() > 0)
            values.push_back(value);
    }
    return (values);
}

bool isValidExtension(const std::string &filename, const std::string &extension)
{
    size_t i = 0;
    if (filename.at(0) == '.')
        i = 1;
    bool point = false;
    for (; i < filename.size(); i++)
    {
        if (point == false && filename.at(i) == '.')
            point = true;
        else if (point == true && filename.at(i) == '.')
            return (false);
    }
    std::size_t dotPos = filename.rfind('.');
    if (dotPos == std::string::npos || dotPos == 0 || dotPos == filename.length() - 1)
        return (false);
    std::string ext = filename.substr(dotPos + 1);
    if (extension == ext)
        return (true);
    return (false);
}

void cleanArgs(std::vector<std::string> *args)
{
    for (std::vector<std::string>::iterator it = args->begin(); it != args->end(); ++it)
    {
        if (it->size() == 0 || *it == "\t" || *it == "\n" || *it == "" || *it == " " || *it == "\r" || *it == "\v")
            args->erase(it);
    }
}

std::vector<std::string> readFile(std::ifstream &file)
{
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(file, line))
        lines.push_back(line);
    return (lines);
}

std::string toString(int value)
{
    std::ostringstream oss;
    oss << value;
    return (oss.str());
}

int toInt(const std::string &str)
{
    std::istringstream iss(str);
    int value;
    iss >> value;
    return (value);
}

// bool isFile(const std::string& path)
// {
//     struct stat info;
//     std::cout << "-----------------------------------.je rentre dans le file" << std::endl;
//     if (stat(path.c_str(), &info) != 0) {
//         std::cerr << "Erreur d'accès au fichier: " << path << std::endl;
//         return false;
//     }
//     return (info.st_mode & S_IFREG) != 0;
// }

int findTypePath(const std::string& path)
{
    struct stat info;
    if (stat(path.c_str(), &info) != 0) {
        if (errno == ENOENT)
            return (2);
        else if (errno == EACCES)
            return (3);
    }
    if ((info.st_mode & S_IRUSR) == 0)
        return (3);
    if ((info.st_mode & S_IFDIR) != 0)
        return (4);
    else if ((info.st_mode & S_IFREG) != 0)
        return (5);
    return (2);
}

bool isCGI(std::string path)
{
	if (path.find(".php") != std::string::npos)
		return (true);
	if (path.find("cgi-bin") != std::string::npos)
		return (true);
	return (false);
}

std::string getExtension(std::string path)
{
    std::string::size_type pos = path.find_last_of(".");
    if (pos == std::string::npos)
        return ("");
    
    std::string::size_type slashPos = path.find_last_of("/\\");
    if (slashPos != std::string::npos && pos < slashPos)
        return ("");
    return (path.substr(pos));
}

bool routeExists(std::vector<std::string> vec, std::string valeur) {
    if (std::find(vec.begin(), vec.end(), valeur) != vec.end())
        return (true);
    return (false);
}


int hexToInt(const std::string& hexStr) {
    int result;
    std::stringstream ss;

    ss << std::hex << hexStr;
    ss >> result;

    return result;
}