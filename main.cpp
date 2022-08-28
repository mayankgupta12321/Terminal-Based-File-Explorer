#include <bits/stdc++.h>
#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <ctime>
#include <termios.h>

#define cursorSelected "==>"
#define cursorUnselected "   "

using namespace std;

string homeDirectory;
string currentWorkingDirectory;
vector<vector<string>> dirInfo; // For Storing directory info (including sub-directory & Files)
struct termios original_termios; // Store original state of terminal
bool normalMode = true; // set to True if normal mode is on, else false
int rowIndex = 0; // cursor pointing to this index.
stack<string> backwardStack;
stack<string> forwardStack;

// Clears Forward Stack
void clearForwardStack() {
    while (!forwardStack.empty()) {
        forwardStack.pop();
    }
}

// Clears the Screen.
void clearScreen() {
    cout << "\033[H\033[2J\033[3J"; // ANSI escape Sequence to clear screen.
}

// Prints the Error & Exit from the Programme
void printError(string s) {
  cout << "Error : " << s << "\r\n";
  exit(1);
}

// Convert Seconds into date/time;
string GetTimeAndDate(unsigned long long sec) {
    char date[100];
    time_t seconds = (time_t)(sec);
    strftime(date, sizeof(date) - 1, "%d-%b-%Y %R ", localtime(&seconds));
    struct tm *tmmm = localtime(&seconds);

    return date;
}

// Convert fileSize from Bytes to B/KB/MB/GB/TB
string convertSize(unsigned long int fileSize) {
    unsigned long int kb = 1024;    //KB
    unsigned long int mb = kb * kb; //MB
    unsigned long int gb = mb * mb; //GB

    string convertedSize = "";
    if(fileSize >= gb) {
       convertedSize = to_string(llround(fileSize/gb)) + "GB";
    }
    else if(fileSize >= mb) {
       convertedSize = to_string(llround(fileSize/mb)) + "MB";
    }
    else if(fileSize >= kb) {
       convertedSize = to_string(llround(fileSize/kb)) + "KB";
    }
    else {
        convertedSize = to_string(fileSize) + "B";
    }

    return convertedSize;
}

// Check If given path is a directory
bool isDirectory(string path) {
    struct stat fileStat;
    stat(path.c_str(),&fileStat);
    return S_ISDIR(fileStat.st_mode);
}

// Fetching Current System User, and storing it in global variable systemUserName
void getHomeDirectory() {
    homeDirectory = "/home/" + (string)getpwuid(getuid())->pw_name + "/";
}

// for fetching File Info stats like git status(fileSize , username, groupname, last modified date, file permission)
vector<string> getFileInfo(string fileName, string filePath) {
    vector<string> fileInfo;

    string fullFileName = filePath + fileName;

    struct stat fileStat;
    if(stat(fullFileName.c_str(),&fileStat) < 0)   {
        cout << "File Not Exists \n";
        return {};
    }
    // cout << to_string(fileStat.st_size) << '\t';
    string fileSize = convertSize(fileStat.st_size);
    string fileUserName = getpwuid(fileStat.st_uid)->pw_name;
    string fileGroupName = getgrgid(fileStat.st_gid)->gr_name;

    string filePermission = ""; // To store permission 
    filePermission += (S_ISDIR(fileStat.st_mode)) ? 'd' : '-'; // If directory than 'd', else '-'
    filePermission += (fileStat.st_mode & S_IRUSR) ? 'r' : '-'; // If user has read permission, than 'r'
    filePermission += (fileStat.st_mode & S_IWUSR) ? 'w' : '-'; // If user has write permission, than 'w'
    filePermission += (fileStat.st_mode & S_IXUSR) ? 'x' : '-'; // If user has execute permission, than 'x'
    filePermission += (fileStat.st_mode & S_IRGRP) ? 'r' : '-'; // If group has read permission, than 'r'
    filePermission += (fileStat.st_mode & S_IWGRP) ? 'w' : '-'; // If group has write permission, than 'w'
    filePermission += (fileStat.st_mode & S_IXGRP) ? 'x' : '-'; // If group has execute permission, than 'x'
    filePermission += (fileStat.st_mode & S_IROTH) ? 'r' : '-'; // If other has read permission, than 'r'
    filePermission += (fileStat.st_mode & S_IWOTH) ? 'w' : '-'; // If other has write permission, than 'w'
    filePermission += (fileStat.st_mode & S_IXOTH) ? 'x' : '-'; // If other has execute permission, than 'x'
    
    string fileLastModifiedDate = GetTimeAndDate(fileStat.st_mtim.tv_sec); //last modified time

    fileInfo.push_back(fileName);               //fileInfo[0] = fileName
    fileInfo.push_back(filePath);               //fileInfo[1] = filePath
    fileInfo.push_back(fileSize);               //fileInfo[2] = fileSize
    fileInfo.push_back(fileUserName);           //fileInfo[3] = fileUserName
    fileInfo.push_back(fileGroupName);          //fileInfo[4] = fileGroupName
    fileInfo.push_back(filePermission);         //fileInfo[5] = filePermission
    fileInfo.push_back(fileLastModifiedDate);   //fileInfo[6] = fileLastModifiedDate

    return fileInfo;
}

// Stores the directory/Files info into global vector dirInfo.
void getDirectoryInfo(string path) {
    dirInfo.clear();
    // cout << path.c_str() << "\n";
    vector<string> fileNames; // For storing names of files & sub-directories
    struct dirent *de;
    DIR *dr = opendir(path.c_str());
    if (dr == NULL)  // opendir returns NULL if couldn't open directory
    {
        cout << "Can't Open File Directory\n";
        return;
    }
     while ((de = readdir(dr)) != NULL){
            fileNames.push_back(de->d_name);
            // cout << de->d_name << '\t';
            // cout << _D_EXACT_NAMLEN(de) << '\t';
    }
    closedir(dr);

    sort(fileNames.begin() , fileNames.end());

    for(string fileName : fileNames) {

        vector<string> fileInfo = getFileInfo(fileName, path);
        dirInfo.push_back(fileInfo);
        // cout << v[1] << "\t" << v[2] << "\t" << v[3] << "\t" << v[4] << "\t" << v[5] << "\t" << v[6] << "\t" << v[0] << "\n";
        // cout<<"\033[21;35m"<<v[0]<<"\033[0m" << "  " << v[1] << "\n";
    }
  
}   

// Returns Parent Directory
string getParentDirectory(string path) {
    vector<string> strArr;
    stringstream ss (path);
    string item;
    while (getline (ss, item, '/')) {
        if(item == "") continue;
        strArr.push_back (item);
    }
    if(strArr.size() == 0 || strArr.size() == 1) {
        return "/";
    }
    else {
        string newDirectory = "/";
        for(int i = 0 ; i < strArr.size() - 1 ; i++) {
            newDirectory += strArr[i] + "/";
        }
        return newDirectory;
    }
    // cout << newDirectory << endl;
    // return "";
}

// Opens the file in default editor
void openFile(string filePath) {
    const char *file_or_url = filePath.c_str();
    pid_t pid = fork();
    if(pid == 0) {
        // execv("xdg-open", , (char *)0);
        execlp("xdg-open", "xdg-open", file_or_url, (char *)0);
        // execl("/usr/bin/xdg-open", "xdg-open", filePath.c_str());
        // exec("xdg-open explorer.cpp");
        // cout << "Child Created" << endl;
    }
}

// Print Directory Info
void printDirInfo(string path) {
    clearScreen(); //Clearing the Screen before Printing.
    getDirectoryInfo(path);
    for(int dirIndex = 0 ; dirIndex < dirInfo.size() ; dirIndex++) {
        vector<string> fileInfo = dirInfo[dirIndex];
        // if(dirIndex==0) cout<<"\033[21;35m";
        if(normalMode == true && dirIndex == rowIndex) {
            // cout<<"\033[35m";
            cout << cursorSelected << " " << fileInfo[1] << "\t" << fileInfo[2] << "\t" << fileInfo[3] << "\t" << fileInfo[4] << "\t" << fileInfo[5] << "\t" << fileInfo[6] << "\t" << fileInfo[0] << "\r\n";
            // cout<<"\033[0m";
        }
        else {
            cout << cursorUnselected << " " << fileInfo[1] << "\t" << fileInfo[2] << "\t" << fileInfo[3] << "\t" << fileInfo[4] << "\t" << fileInfo[5] << "\t" << fileInfo[6] << "\t";
            // if(fileInfo[5][0] == 'd') cout<<"\033[32m";
            // else cout<<"\033[31m";
            cout << fileInfo[0] << "\r\n";
            // cout<<"\033[0m";
        }
        // if(dirIndex==0) cout<<"\033[0m";
    }
}

// Enables Raw Mode (Canonical Mode)
void disableRawMode() {
    if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &original_termios) == -1) printError("tcgetattr");
}

// Enables Raw Mode (Non-Canonical Mode)
void enableRawMode() {
    // tcflag_t c_iflag;		/* input mode flags */
    // tcflag_t c_oflag;		/* output mode flags */
    // tcflag_t c_cflag;		/* control mode flags */
    // tcflag_t c_lflag;		/* local mode flags */
    // cc_t c_line;			/* line discipline */
    // cc_t c_cc[NCCS];		/* control characters */
    // speed_t c_ispeed;		/* input speed */
    // speed_t c_ospeed;		/* output speed */
    if(tcgetattr(STDIN_FILENO, &original_termios) == -1) printError("tcgetattr");
    atexit(disableRawMode);

    struct termios raw_termios = original_termios;
    raw_termios.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    raw_termios.c_oflag &= ~(OPOST);
    raw_termios.c_cflag |= (CS8);
    raw_termios.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    if(tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw_termios) == -1) printError("tcsetattr");;
}

// Handle Escape Key
int readEscape() {
    char finalChar = '\0';
    char ch1;
    read(STDIN_FILENO, &ch1, 1);
    if(ch1 == 27) return readEscape();
    if(ch1 == 91) {
        char ch2;
        read(STDIN_FILENO, &ch2, 1);
        if(ch2 == 27) return readEscape();
        else if(ch2 == 'A' || ch2 == 'B' || ch2 == 'C' || ch2 == 'D') return ch2;
    }
    else if(ch1 == 'h' || ch1 == 'q' || ch1 == ':' || ch1 == 127 || ch1 == 13) return ch1;
    return -1;
}

void handleKeyPressesInNormalMode() {
    while(true) {
        char finalChar = '\0';
        char ch;
        read(STDIN_FILENO, &ch, 1);
        if(ch == 'h' || ch == 'q' || ch == ':' || ch == 127 || ch == 13) {
            finalChar = ch;
        }
        // Escape Character
        else if (ch == 27) {
            char tempChar = readEscape();
            if(tempChar != -1) finalChar = tempChar;
        }

        // Quit - Close the Application
        if(finalChar == 'q') { 
            clearScreen();
            // cout << "Thanks for using File Explorer.\r\n";
            exit(1);
        }
        // : - Enter the Command Mode 
        else if(finalChar == ':') {
            cout << "\033[31m" << "Command Mode is in Implementation Phase.\r\n" << "\033[0m";
        }
        
        // h - Open Home Directory.
        else if(finalChar == 'h') {
            rowIndex = 0;
            clearForwardStack();
            backwardStack.push(currentWorkingDirectory);
            currentWorkingDirectory = homeDirectory;
            printDirInfo(currentWorkingDirectory);
            // cout << "Home\r\n";
        }
        
        // Beckspace - Going to Parent Directory
        else if(finalChar == 127) {
            string newDirectory = getParentDirectory(currentWorkingDirectory);
            if(newDirectory != currentWorkingDirectory) {
                clearForwardStack();
                backwardStack.push(currentWorkingDirectory);
            }
            rowIndex = 0;
            currentWorkingDirectory = newDirectory;
            printDirInfo(currentWorkingDirectory);
            // cout << "BackSpace\r\n";
        }
        
        // Enter - Open the Directory/File
        else if(finalChar == 13) {
            // cout << fileName;
            if(dirInfo[rowIndex][0] == ".") {}
            else if (dirInfo[rowIndex][0] == "..") {
                string newDirectory = getParentDirectory(currentWorkingDirectory);
                if(newDirectory != currentWorkingDirectory) {
                    clearForwardStack();
                    backwardStack.push(currentWorkingDirectory);
                }
                rowIndex = 0;
                currentWorkingDirectory = newDirectory;
                printDirInfo(currentWorkingDirectory);
            }
            else if(isDirectory(dirInfo[rowIndex][1] + dirInfo[rowIndex][0])) {
                clearForwardStack();
                backwardStack.push(currentWorkingDirectory);
                string newDirectory = dirInfo[rowIndex][1] + dirInfo[rowIndex][0] + "/";
                rowIndex = 0;
                currentWorkingDirectory = newDirectory;
                printDirInfo(currentWorkingDirectory);
            }
            else {
                openFile(dirInfo[rowIndex][1] + dirInfo[rowIndex][0]);
            }
            // cout << "Enter\r\n";
        }
        
        // Up Key - Scroll Up
        else if(finalChar == 'A') {
            if(rowIndex > 0) rowIndex--;
            printDirInfo(currentWorkingDirectory);
            // cout << "Up\r\n";
        }
        // Down Key - Scroll Down
        else if(finalChar == 'B') {
            if(rowIndex < dirInfo.size() - 1) rowIndex++;
            printDirInfo(currentWorkingDirectory);
        }

        // Right Key - For Next Directory
        else if(finalChar == 'C') {
            if(!forwardStack.empty()) {
                backwardStack.push(currentWorkingDirectory);
                currentWorkingDirectory = forwardStack.top();
                forwardStack.pop();
                printDirInfo(currentWorkingDirectory);
            }
        }

        // Left Key - For Previous Visited Directory
        else if(finalChar == 'D') {
            if(!backwardStack.empty()) {
                forwardStack.push(currentWorkingDirectory);
                currentWorkingDirectory = backwardStack.top();
                backwardStack.pop();
                printDirInfo(currentWorkingDirectory);
            }
        }
    }
}

void testCode() {
    
    // cout << "Hi\n";
    // for(int i = 0; i < 10 ; i++) 
    // cout << editorReadKey() << "\r\n";
    // return;
    // char c;
    // while (read(STDIN_FILENO, &c, 1) == 1 && c!='q') {
    //     // if(c=='b' || c == 'e') printError("nikal");
    //     if (iscntrl(c)) {
    //         cout << (int)c << "\r\n";
    //     } 
    //     else {
    //         cout << (int)c << " - " << c << "\r\n";
    //     }
    // }
}

int main(int argc, char **argv)
{
    // testCode();
    
    getHomeDirectory();
    currentWorkingDirectory = get_current_dir_name();
    if(currentWorkingDirectory[currentWorkingDirectory.size() - 1] != '/') currentWorkingDirectory += '/';
    enableRawMode();
    printDirInfo(currentWorkingDirectory);
    handleKeyPressesInNormalMode();

    // printDirInfo(currentWorkingDirectory);

    // cout << isDirectory("/") << endl;
    // printDirInfo("/home/mayank/Desktop/IIITH Courses/AOS/Assignment/Assignment1/2022201012/");
    // cout << homeDirectory << endl;
    // cout << "\033[7m" << "Mayank Gupta" << "\n";
    // printDirInfo("./");
    
    
    // getDirectoryInfo("./");
    
    return 0;
}