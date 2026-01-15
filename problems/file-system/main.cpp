#include<iostream>
#include<map>
#include<set>
#include<vector>

using namespace std;

class Node {
    map<string, Node*> children;
    bool isFile;
    string content;
public:
    Node(): isFile(false) {}

    bool keyExists(string &key) {return children.find(key) != children.end();}
    Node* getKey(string &key) {return children.at(key);}
    void setKey(string &key) {children[key] = new Node();}
    bool isfile() {return isFile;}
    void setFile() {isFile = true;}
    string getContent() {return content;}
    void setContent(string &s) {content += s;}
    map<string, Node*>& getChildren() {return children;}
};

class FileSystem {
    Node* root;

    vector<string> split(string &s) {
        vector<string> ans;
        string temp = "";

        for(char c: s) {
            if(c == '/') {
                if(temp != "") ans.push_back(temp);
                temp.clear();
            } else temp += c;
        } 

        if(temp.size() > 0) ans.push_back(temp);
        return ans;
    }

public:
    FileSystem(): root(new Node()) {}

    vector<string> ls(string &path) {
        vector<string> parts = split(path);
        
        Node* curr = root;
        for(string &part: parts) curr = curr->getKey(part);

        vector<string> ans;
        if(curr->isfile()) {
            // if it is a file
            ans.push_back(parts.back()); 
            return ans;
        }

        for(auto &[id, _]: curr->getChildren()) ans.push_back(id);
        return ans;
    }

    void mkdir(string &path) {
        vector<string> parts = split(path);
        
        Node* curr = root;
        for(string &part: parts) {
            if(!curr->keyExists(part)) curr->setKey(part);
            curr = curr->getKey(part);
        }

        return;
    }

    void addContentToFile(string &path, string &content) {
        vector<string> parts = split(path);
        string fileName = parts.back();

        Node* curr = root;
        for(string &part: parts) {
            if(!curr->keyExists(part)) curr->setKey(part);
            curr = curr->getKey(part);
        }

        curr->setFile();
        curr->setContent(content);
    }

    string readContentFromFile(string &path) {
        vector<string> parts = split(path);

        Node* curr = root;
        for(string &part: parts) curr = curr->getKey(part);

        return curr->getContent();
    }
};

int main() {
    cout << "Main:: file system\n";
    vector<string> input = {"FileSystem","ls","mkdir","addContentToFile","ls","readContentFromFile"};
    vector<vector<string>> ops = {{}, {"/"}, {"/a/b/c"}, {"/a/b/c/d","hello"}, {"/"}, {"/a/b/c/d"}};

    FileSystem fs;
    for(int i = 0; i < input.size(); i++) {
        if(input[i] == "ls") {
            vector<string> res = fs.ls(ops[i][0]);
            cout << "ls " << ops[i][0] << ": ";
            for(const string &s : res) {
                cout << s << " ";
            }
            cout << endl;
        } else if(input[i] == "mkdir") {
            fs.mkdir(ops[i][0]);
            cout << "mkdir " << ops[i][0] << endl;      
        } else if(input[i] == "addContentToFile") {
            fs.addContentToFile(ops[i][0], ops[i][1]);
            cout << "addContentToFile " << ops[i][0] << " " << ops[i][1] << endl;      
        } else if(input[i] == "readContentFromFile") {
            string content = fs.readContentFromFile(ops[i][0]);
            cout << "readContentFromFile " << ops[i][0] << ": " << content << endl;      
        }
    }
    return 0;
}