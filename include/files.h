#ifndef __FILE_H
#define __FILE_H
#include <ArduinoOTA.h>
#include <string>
// #include <LITTLEFS.h>
// #define LittleFS_ LITTLEFS
#include <LittleFS.h>
#define LittleFS_ LittleFS

class FileLittleFS {
  char path[32];
  File file;
  public:
    FileLittleFS(const char *name);
    bool open(const char *mode);
    //~FileLittleFS() {free(path); }
    // void    listDir();
    static boolean exist(const char* name);
    boolean exist();
    void    purge(int size);
    String  readFile();
    void    readFile(char* buffer);
    size_t  fileSize();
    void    writeFile(const char * message);
    boolean writeFile(const char *message, const char *mode);
    boolean writeFile(String message, const char *mode);
    void    close();
    static  void rmDir(const char *dirName);
    static  void rmFile(const char *fileName);
    static  void mkDir(const char* dirName);
    unsigned size();
    void    rmFile();
};

#endif