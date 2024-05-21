#include <iostream>
#include <filesystem>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#include "errors.hpp"

void error(int errorCode, std::string text);
void printHelp();
void init(std::string filename);
bool travelToEnd(int fd);
void makeTableFromCSV(std::string htmlFile, std::string csvFile, std::string tableName);

int main(int argc, char const *argv[])
{
    if(argc == 1)
        error(NO_ACTIONS, "No actions given: try 'htmlmaker help' for a list of commands.");

    if(argc == 2 && strcmp(argv[1], "help") == 0)
        printHelp();

    if(strcmp(argv[1], "init") == 0){
        if(argc != 3)
            error(INVALID_ARGUMENTS, "Wrong arguments used for command 'init': correct 'init' format -- htmlmaker init <filename>");
        
        init(argv[2]);
    }

    if(strcmp(argv[1], "table") == 0){
        if(argc != 5)
            error(INVALID_ARGUMENTS, "Wrong arguments used for command 'table': correct 'table' format -- htmlmaker table <filename> <csv_file_path> <table_name>");

        makeTableFromCSV(argv[2], argv[3], argv[4]);
    }

    return OK;
}


/**
 * @name error
 * 
 * @param errorCode Integer representing the error that occurred, and what the program will return on exit (check errors.hpp)
 * @param text Text that will be display as the error message
*/
void error(int errorCode, std::string text){
    std::cerr << "[ERROR " << errorCode << "]: " << text << std::endl;
    exit(errorCode);
}

void printHelp(){
    using namespace std;

    cout << "List of available actions:\n" << endl;
    cout << "Basic actions:" << endl;
    cout << "\t- htmlmaker help" << endl;
    cout << "\t- htmlmaker init <filename>\n" << endl;
    cout << "Tables:" << endl;
    cout << "\t- htmlmaker table <filename> <csv_file_path> <table_name>" << endl;

    exit(OK);
}

/**
 * @name init
 * 
 * @param filename Name that will be given to the new HTML file being initialized
*/
void init(std::string filename){
    //Create './output' dir if not existent
    std::filesystem::create_directory("./output");

    //Append output folder to filename
    std::string completeFilename = "./output/";
    completeFilename.append(filename);

    //Check if file with filename already exists, abort if so
    int fileExists = open(completeFilename.c_str(), O_RDONLY);
    if(fileExists >= 0)
        error(FILE_ALREADY_EXISTS, "Given filename already exists. If you wish to override it please delete it from ./output");
    close(fileExists);

    //Create new HTML file
    int fd = open(completeFilename.c_str(), O_WRONLY|O_CREAT);

    if(fd == -1)
        error(CANT_CREATE_FILE, "Unknown error. Couldn't create file");
    
    std::string initText = 
    "<!DOCTYPE html>\n<html lang=\"en\">\n\t<head>\n\t\t<meta charset=\"UTF-8\">\n\t\t<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n\t\t<title>Title</title>\n\t</head>\n\t<body>\n\n\t</body>\n</html>";

    write(fd, initText.c_str(), initText.length());

    std::cout << "HTML file '" << filename << "' succesfully created at ./output!" << std::endl;

    close(fd);
    exit(OK);
}

/**
 * @name travelToEnd
 * 
 * @param fd File descriptor of HTML file
 * 
 * @returns True if end of html file is found, false otherwise 
*/
bool travelToEnd(int fd){
    off_t position = lseek(fd, -7, SEEK_END);
    do{
        char buffer[8];
        read(fd, buffer, 7);
        buffer[7] = '\0';
        if(strcmp(buffer, "</body>") == 0){
            lseek(fd, -7, SEEK_CUR);
            return true;
        }
        position = lseek(fd, -8, SEEK_CUR);
    }while(position >= 0);

    return false;
}

/**
 * @name makeTableFromCSV
 * 
 * @param htmlFile Name of the HTML file being edited from the output folder
 * @param csvFile Path of the csv file to be turned into a table
 * @param tableName Name to be given to the table. Will be used as ID
*/
void makeTableFromCSV(std::string htmlFile, std::string csvFile, std::string tableName){
    //Open html file
    std::string completeHtmlFilename = "./output/";
    completeHtmlFilename.append(htmlFile);
    int htmlFd = open(completeHtmlFilename.c_str(), O_RDWR|O_APPEND);
    if(htmlFd == -1)
        error(FILE_DOESNT_EXIST, "Given HTML file doesn't exist at './output'. You can create it using htmlmaker init.");

    //Open csv file
    int csvFd = open(csvFile.c_str(), O_RDONLY);
    if(csvFd == -1)
        error(FILE_DOESNT_EXIST, "Given CSV file doesn't exist.");

    //Travel to the end of the html file
    if(!travelToEnd(htmlFd))
        error(EOF_NOT_FOUND,"Couldn't find the end of the HTML file given: no '</body>' tag");

    
}