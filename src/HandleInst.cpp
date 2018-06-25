#include <stdio.h>
#include <string>
#include <map>
#include "parser.hpp"
#include <limits.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <fcntl.h>

using namespace std;

typedef struct Response_t {
    string version;
    string code;
    string text;
    map<string, string> kvPairs;
} Response;

int SendResponse(Response response, FILE *out, char *buf) {
    string initLine = response.version + " " + response.code + " " + response.text + "\r\n";
    string kvPairsStr;
    for (std::map<string,string>::iterator cur = response.kvPairs.begin();cur != response.kvPairs.end();cur++) {
        kvPairsStr += cur->first + ": " + cur->second + "\r\n";
    }
    string responseStr = initLine + kvPairsStr + "\r\n";
    strcpy(buf, responseStr.c_str());
    if (fwrite(buf, sizeof(char), responseStr.size(), out) < responseStr.size())
    {
        return -1;
    } else {
        fflush(out);
        return 0;
    }
}

string GetAbsPath(const std::string &doc_root) {
    char *res = realpath(doc_root.c_str(), NULL);
    if (res != NULL) {
        string absPath = string(res);
        free(res);
        return absPath;
    } else {
        return "";
    }
}

string GetLastModified(string path)
{
    struct stat buf;
    if (stat(path.c_str(), &buf) < 0)
    {
        return "";
    }
    char buff[256];
    strftime(buff, 256, "%a, %e %b %Y %H:%M:%S GMT", gmtime(&buf.st_mtime));
    return string(buff);
}

ssize_t GetContentLen(string path) {
    struct stat buf;
    return stat(path.c_str(), &buf) == 0 ? buf.st_size : -1;
}

string GetContentType(string path) {
    ssize_t dotIndex = path.rfind(".");
    string suff = path.substr(dotIndex, path.size() - dotIndex);
    if (suff == ".jpg") {
        return "image/jpeg";
    } else if (suff == ".png") {
        return "image/png";
    } else if (suff == ".html") {
        return "text/html";
    }
    return "unknown";
}

bool Exist(string &path)
{
    struct stat buf;
    return (stat (path.c_str(), &buf) == 0);
}

bool Access(string path)
{
    struct stat buf;
    if (stat(path.c_str(), &buf) < 0) {
        return false;
    }
    else if (buf.st_mode & S_IROTH) {
        return true;
    } else {
        return false;
    }
}

int HandleInst(Instruction inst, int clntSocket, string &doc_root, FILE *out, char *buf) {
    Response response;
    response.version = "HTTP/1.1";
    response.kvPairs["Server"] = "localhost";
    
    if (inst.valid == false || !inst.kvPairs.count("Host")) { // == inst.kvPairs.end()
        response.code = "400";
        response.text = "Client Error";
        //SendResponse
        if (SendResponse(response, out, buf) == -1)
            return -1;
        fflush(out);
    } else {
        string pathFromURL = GetAbsPath(doc_root + "/" + inst.url);
        string pathFromROOT = GetAbsPath(doc_root);
        
        if (pathFromURL == pathFromROOT) { /* "/" -> "/index.html" */
            string pathFromURL = GetAbsPath(doc_root + string("/index.html"));
        }
        
#ifdef DEBUG
        cout << "==================\n";
        cout << "Handling request: \n";
        cout << "merged addr: " << doc_root + string("/") + req_header.url << "\n";
        cout << "abs_url: " << abs_url_path << "\n";
#endif
        
        if (!Exist(pathFromURL)) {
            response.code = "403";
            response.text = "Not Found";
        
            if (SendResponse(response, out, buf) == -1)
                return -1;
            fflush(out);
        } else if (!Access(pathFromURL)) {
            response.code = "403";
            response.text = "Forbidden";
            
            if (SendResponse(response, out, buf) == -1)
                return -1;
            fflush(out);
        } else if (pathFromURL != "" && pathFromURL.find(GetAbsPath(doc_root)) != 0) {
            response.code = "404";
            response.text = "Not Found";
         
            if (SendResponse(response, out, buf) == -1)
                return -1;
            fflush(out);
        } else {
            ssize_t len = GetContentLen(pathFromURL);
            response.code = "200";
            response.text = "OK";
            response.kvPairs["Last-Modified"] = GetLastModified(pathFromURL);
            response.kvPairs["Content-Type"] = GetContentType(pathFromURL);
            response.kvPairs["Content-Length"] = to_string(len);
    
            if (SendResponse(response, out, buf) == -1)
                return -1;
            fflush(out);
            int fd = open(pathFromURL.c_str(), O_RDONLY);
            if (sendfile(clntSocket, fd, NULL, len) < len) {
                return -1;
            }
            close(fd);
        }
    }
    
    if ((inst.kvPairs.find("Connection") != inst.kvPairs.end()) && (inst.kvPairs["Connection"] == "close")) {
        return -1;
    } else {
        return 0;
    }
}
