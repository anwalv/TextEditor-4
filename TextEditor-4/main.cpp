#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <windows.h>
#include <string>
using namespace std;
class Text {
private:
    char* text;
    int arraySize;
public:
    Text(int size = 100) {
        text = (char*)calloc(size, sizeof(int));
        arraySize = size;
    }
    ~Text() {
        free(text);
    };
    void TextReallocation(int currentSize, int inputLength) {
        arraySize = currentSize + inputLength + 2;
        text = (char*)realloc(text, arraySize * sizeof(char));
        if (text == NULL) {
            std::cout << "Memory reallocation failed.\n";
            return;
        }
    }
    char* getText() const {
        return text;
    }

    int getArraySize() const {
        return arraySize;
    }

    void setText(const char* newText) {
        strcpy(text, newText);
    }
};

typedef char* (*encrypt_ptr_t)(const char*, int);
typedef char* (*decrypt_ptr_t)(const char*, int);

class CaesarCipher {
private:
    HINSTANCE handle;
    encrypt_ptr_t encrypt_ptr;
    decrypt_ptr_t decrypt_ptr;

public:
    bool loadLibrary() {
        handle = LoadLibraryA("C:/Users/User/source/repos/caesarLib/caesar.dll");
        DWORD err = GetLastError();
        if (handle == nullptr || handle == INVALID_HANDLE_VALUE) {
            cout << "Lib not found." << endl;
            return false;
        }

        encrypt_ptr = (encrypt_ptr_t)GetProcAddress(handle, "encrypt");
        decrypt_ptr = (decrypt_ptr_t)GetProcAddress(handle, "decrypt");

        if (encrypt_ptr == nullptr || decrypt_ptr == nullptr) {
            cout << "Functions not found." << endl;
            FreeLibrary(handle);
            handle = nullptr;
            return false;
        }

        return true;
    }
    CaesarCipher() : handle(nullptr), encrypt_ptr(nullptr), decrypt_ptr(nullptr) {
        if (!loadLibrary()) {
            throw runtime_error("Failed to load Caesar cipher library.");
        }
    }

    ~CaesarCipher() {
        if (handle) {
            FreeLibrary(handle);
        }
    }

    char* encryptText(const char* message, int key) {
        if (encrypt_ptr) {
            return encrypt_ptr(message, key);
        }
        return nullptr;
    }

    char* decryptText(const char* message, int key) {
        if (decrypt_ptr) {
            return decrypt_ptr(message, key);
        }
        return nullptr;
    }
};

class TextEditor {
private:
    Text* textObject;
    char clipboard[100];
    char* states[3];
    int numStates;
    int undoNum;
    int redoNum;
    int currentStateIndex;
    CaesarCipher* cipher;


public:
    TextEditor(int size = 100, const char* libraryName = "caesar.dll") {
        textObject = new Text(size);
        int arraySize = size;
        numStates = 0;
        undoNum = -1;
        redoNum = -1;
        currentStateIndex = -1;
        cipher = new CaesarCipher();

        for (int i = 0; i < 3; ++i) {
            states[i] = (char*)calloc(arraySize, sizeof(char));
        }
    }
    ~TextEditor() {
        delete textObject;
        for (int i = 0; i < 3; ++i) {
            if (states[i] != nullptr) {
                free(states[i]);
            }
        }
        delete cipher;
    }
    void SaveState() {
        if (numStates == 3) {
            free(states[0]);
            for (int i = 0; i < 2; ++i) {
                states[i] = states[i + 1];
            }
            states[2] = nullptr;
            numStates = 2;
        }

        int currentSize = strlen(textObject->getText());
        states[numStates] = (char*)calloc(currentSize + 1, sizeof(char));
        strncpy(states[numStates], textObject->getText(), currentSize);
        currentStateIndex = numStates;
        numStates++;
        undoNum = numStates - 1;
        redoNum = -1;
    }
    void AppendText() {
        char userInput[1000];
        std::cout << "Please, enter text:\n";
        std::cin.getline(userInput, 1000);
        int inputLength = strlen(userInput);
        int currentSize = strlen(textObject->getText());
        if (currentSize + inputLength >= textObject->getArraySize()) {
            textObject->TextReallocation(currentSize, inputLength);
        }
        strcat(textObject->getText(), userInput);
        std::cout << "Text was added successfully\n";
    }
    void StartNewLine() {
        strcat(textObject->getText(), "\n");
        std::cout << "A new line has started!\n";
    }
    void WriteToFile() {
        FILE* out_file = fopen("C:\\Users\\User\\CLionProjects\\untitled2\\my_text.txt", "w");
        if (out_file == NULL) {
            std::cout << "Error! Could not open file.\n";
            return;
        }
        fprintf(out_file, "%s", textObject->getText());
        fclose(out_file);
        std::cout << "The data was successfully written to the file.\n";
    }
    void LoadFromFile() {
        FILE* in_file = fopen("C:\\Users\\User\\CLionProjects\\untitled2\\my_text.txt", "r");
        if (in_file == NULL) {
            std::cout << "Error! Could not open file.\n";
            return;
        }
        char myText[1000];
        if (fgets(myText, sizeof(myText), in_file) != NULL) {
            textObject->setText(myText);
        }
        fclose(in_file);
    }
    void PrintText() {
        std::cout << "Your text: " << textObject->getText() << "\n";
    }
    void InsertText() {
        int lineNumber;
        int position;
        char userInput[1000];
        int inputLength;
        int currentSize;
        char answer = 'n';

        while (answer != 'y') {
            std::cout << "Enter the line number: ";
            std::cin >> lineNumber;
            std::cin.ignore();

            std::cout << "Enter position number: ";
            std::cin >> position;
            std::cin.ignore();

            int currentLine = 1;
            int currentPosition = 0;
            const char* currentText = textObject->getText();
            while (currentLine < lineNumber) {
                if (*currentText == '\n') {
                    currentLine++;
                }
                currentText++;
                currentPosition++;
            }
            position += currentPosition;
            std::cout << "Is this position correct? (y/n): ";
            std::cin >> answer;
            std::cin.ignore();
        }
        std::cout << "Please enter text to insert:\n";
        std::cin.getline(userInput, 1000);

        inputLength = strlen(userInput);
        currentSize = strlen(textObject->getText());

        if (currentSize + inputLength >= textObject->getArraySize()) {
            textObject->TextReallocation(currentSize, inputLength);
        }

        memmove(textObject->getText() + position + inputLength, textObject->getText() + position, currentSize - position + 1);
        strncpy(textObject->getText() + position, userInput, inputLength);

        std::cout << "Text was added successfully.\n";
    }
    void SearchSubstring() {
        char substring[1000];
        std::cout << "Enter the substring to search for: ";
        std::cin.getline(substring, sizeof(substring));

        char* ptr = textObject->getText();
        int position = 0;
        int subLength = strlen(substring);

        while ((ptr = strstr(ptr, substring))) {
            std::cout << "Substring found at position " << ptr - textObject->getText() + position << "\n";
            position = ptr - textObject->getText() + subLength;
            ptr += subLength;
        }
    }
    void DeleteText() {
        int lineNumber;
        int position;
        int numChar;
        char answer = 'n';
     

        while (answer != 'y') {
            std::cout << "Enter the line number: ";
            std::cin >> lineNumber;
            std::cin.ignore();

            std::cout << "Enter position number: ";
            std::cin >> position;
            std::cin.ignore();

            int currentLine = 1;
            int currentPosition = 0;
            char* ptr = textObject->getText();
            while (currentLine < lineNumber) {
                if (*ptr == '\n') {
                    currentLine++;
                }
                ptr++;
                currentPosition++;
            }
            position += currentPosition;
            displayTextWithCursor(position);
            std::cout << "Is this position correct? (y/n): ";
            std::cin >> answer;
            std::cin.ignore();
        }

        std::cout << "Please, enter number of characters to delete: ";
        std::cin >> numChar;
        std::cin.ignore();

        int textLength = strlen(textObject->getText());
        if (position + numChar > textLength) {
            numChar = textLength - position;
        }

        memmove(textObject->getText() + position, textObject->getText() + position + numChar, textLength - position - numChar + 1);

        std::cout << "Text was deleted successfully\n";
    }
    void Undo() {
        if (undoNum < 0) {
            std::cout << "No commands to undo\n";
            return;
        }

        strncpy(textObject->getText(), states[undoNum], textObject->getArraySize());
        currentStateIndex = undoNum;
        undoNum--;

        redoNum = min(redoNum + 1, numStates - 1);

        std::cout << "Undo performed successfully\n";
    }
    void Redo() {
        if (redoNum < 0 || redoNum >= numStates) {
            std::cout << "No states to redo\n";
            return;
        }

        strncpy(textObject->getText(), states[redoNum], textObject->getArraySize());
        currentStateIndex = redoNum;
        redoNum++;

        undoNum = min(undoNum + 1, numStates - 1);

        std::cout << "Redo performed successfully\n";
    }
    void ReplaceInsert() {
        int lineNumber;
        int position;
        char userInput[1000];
        int inputLength;
        int currentSize;
        char answer = 'n';

        while (answer != 'y') {
            std::cout << "Enter the line number: ";
            std::cin >> lineNumber;
            std::cin.ignore();

            std::cout << "Enter position number: ";
            std::cin >> position;
            std::cin.ignore();

            int currentLine = 1;
            int currentPosition = 0;
            const char* currentText = textObject->getText();
            while (currentLine < lineNumber) {
                if (*currentText == '\n') {
                    currentLine++;
                }
                currentText++;
                currentPosition++;
            }
            position += currentPosition;
            displayTextWithCursor(position);
            std::cout << "Is this position correct? (y/n): ";
            std::cin >> answer;
            std::cin.ignore();
        }

        std::cout << "Please, enter text to insert:\n";
        std::cin.getline(userInput, 1000);

        inputLength = strlen(userInput);
        currentSize = strlen(textObject->getText());

        if (position + inputLength > currentSize) {
            std::cout << "The position and length of input exceed text size.\n";
            return;
        }

        strncpy(textObject->getText() + position, userInput, inputLength);
        std::cout << "Text was inserted with replacement successfully\n";
    }
    void Cut() {
        int lineNumber;
        int position;
        int numChar;
        char answer = 'n';

        while (answer != 'y') {
            std::cout << "Enter the line number: ";
            std::cin >> lineNumber;
            std::cin.ignore();

            std::cout << "Enter position number: ";
            std::cin >> position;
            std::cin.ignore();

            int currentLine = 1;
            int currentPosition = 0;
            const char* currentText = textObject->getText();
            while (currentLine < lineNumber) {
                if (*currentText == '\n') {
                    currentLine++;
                }
                currentText++;
                currentPosition++;
            }
            position += currentPosition;
            displayTextWithCursor(position);
            std::cout << "Is this position correct? (y/n): ";
            std::cin >> answer;
            std::cin.ignore();
        }
        std::cout << "Please, enter number of characters to cut: \n";
        std::cin >> numChar;
        std::cin.ignore();
        int textLength = strlen(textObject->getText());
        if (position + numChar > textLength) {
            numChar = textLength - position;
        }
        strncpy(clipboard, textObject->getText() + position, numChar);
        clipboard[numChar] = '\0';
        memmove(textObject->getText() + position, textObject->getText() + position + numChar, textLength - position - numChar + 1);
        std::cout << "Text was cut successfully!\n";
    }
    void PasteText() {
        int lineNumber;
        int position;
        char answer = 'n';

        while (answer != 'y') {
            std::cout << "Enter the line number: ";
            std::cin >> lineNumber;
            std::cin.ignore();

            std::cout << "Enter position number: ";
            std::cin >> position;
            std::cin.ignore();

            int currentLine = 1;
            int currentPosition = 0;
            const char* currentText = textObject->getText();
            while (currentLine < lineNumber) {
                if (*currentText == '\n') {
                    currentLine++;
                }
                currentText++;
                currentPosition++;
            }
            position += currentPosition;
            displayTextWithCursor(position);
            std::cout << "Is this position correct? (y/n): ";
            std::cin >> answer;
            std::cin.ignore();
        }

        int bufferLength = strlen(clipboard);
        int currentSize = strlen(textObject->getText());

        if (position > currentSize) {
            std::cout << "Invalid position. Cannot paste outside the current text boundaries.\n";
            return;
        }

        if (currentSize + bufferLength >= textObject->getArraySize()) {
            textObject->TextReallocation(currentSize, bufferLength);
        }

        memmove(textObject->getText() + position + bufferLength, textObject->getText() + position, currentSize - position + 1);
        strncpy(textObject->getText() + position, clipboard, bufferLength);
        memset(clipboard, 0, sizeof(clipboard));

        std::cout << "Text was pasted successfully\n";
    }
    void CopyText() {
        int lineNumber;
        int position;
        int numChar;
        char answer = 'n';

        while (answer != 'y') {
            std::cout << "Enter the line number: ";
            std::cin >> lineNumber;
            std::cin.ignore();

            std::cout << "Enter position number: ";
            std::cin >> position;
            std::cin.ignore();

            int currentLine = 1;
            int currentPosition = 0;
            const char* currentText = textObject->getText();
            while (currentLine < lineNumber) {
                if (*currentText == '\n') {
                    currentLine++;
                }
                currentText++;
                currentPosition++;
            }
            position += currentPosition;
            displayTextWithCursor(position);
            std::cout << "Is this position correct? (y/n): ";
            std::cin >> answer;
            std::cin.ignore();
        }

        std::cout << "Please, enter number of characters: \n";
        std::cin >> numChar;
        std::cin.ignore();

        int textLength = strlen(textObject->getText());
        if (position + numChar > textLength) {
            numChar = textLength - position;
        }

        strncpy(clipboard, textObject->getText() + position, numChar);
        clipboard[numChar] = '\0';

        std::cout << "Text was copied to clipboard successfully\n";
    }
    void displayTextWithCursor(int position) {
        int length = strlen(textObject->getText());
        for (int i = 0; i <= length; ++i) {
            if (i == position) {
                std::cout << "|";
            }
            if (i < length) {
                std::cout << textObject->getText()[i];
            }
        }
        std::cout << std::endl;
    }
    void EncryptText(int key) {
        char* encryptedText = cipher->encryptText(textObject->getText(), key);
        if (encryptedText != nullptr) {
            textObject->setText(encryptedText);
            std::cout << "Text was sucsesfully encrypted. ";
        }
        else {
            std::cout << "It was error.\n";
        }
    }
    void DecryptText(int key) {
        char* decryptedText = cipher->decryptText(textObject->getText(), key);
        if (decryptedText != nullptr) {
            textObject->setText(decryptedText);
            std::cout << "Text was sucsesfully decrypted. " ;
        }
        else {
            std::cout << "It was error.\n";
        }
    }

};

int main() {
    TextEditor editor;
    CaesarCipher cipher();

    int command = 0;
    editor.SaveState();

    cout << "==================== Text editor ====================" << endl;
    cout << "Commands:\n"
        << "1. Append text.\n"
        << "2. Start a new line.\n"
        << "3. Save information to a file.\n"
        << "4. Load information from a file.\n"
        << "5. Print current text.\n"
        << "6. Insert text by line and symbol index.\n"
        << "7. Search text.\n"
        << "8. Delete text.\n"
        << "9. Undo.\n"
        << "10. Redo.\n"
        << "11. Cut text.\n"
        << "12. Copy text.\n"
        << "13. Paste text.\n"
        << "14. Insert with replacement.\n"
        << "15. Encrypt text.\n"
        << "16. Decrypt text.\n"
        << "17. Exit.\n";

    while (command != 17) {
        cout << "\nPlease, enter a number of command: ";
        cin >> command;
        cin.ignore();

        switch (command) {
        case 1:
            editor.AppendText();
            editor.SaveState();
            break;
        case 2:
            editor.StartNewLine();
            editor.SaveState();
            break;
        case 3:
            editor.WriteToFile();
            break;
        case 4:
            editor.LoadFromFile();
            break;
        case 5:
            editor.PrintText();
            break;
        case 6:
            editor.InsertText();
            editor.SaveState();
            break;
        case 7:
            editor.SearchSubstring();
            break;
        case 8:
            editor.DeleteText();
            editor.SaveState();
            break;
        case 9:
            editor.Undo();
            break;
        case 10:
            editor.Redo();
            break;
        case 11:
            editor.Cut();
            editor.SaveState();
            break;
        case 12:
            editor.CopyText();
            break;
        case 13:
            editor.PasteText();
            editor.SaveState();
            break;
        case 14:
            editor.ReplaceInsert();
            editor.SaveState();
            break;
        case 15: {
            int key;
            cout << "Enter encryption key: ";
            cin >> key;
            editor.EncryptText(key);
            editor.SaveState();
            break;
        }
        case 16: {
            int key;
            cout << "Enter decryption key: ";
            cin >> key;
            editor.DecryptText(key);
            editor.SaveState();
            break;
        }
        case 17:
            cout << "Exiting program. Goodbye!\n";
            break;
        default:
            cout << "Invalid command. Please enter a number between 1 and 17.\n";
            break;
        }
    }
    return 0;
}