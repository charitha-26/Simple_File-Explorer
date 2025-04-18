#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#define MAX_PATH 1024
#define LOG_FILE "file_explorer_log.txt"
#define COLOR_RED 12
#define COLOR_GREEN 10
#define COLOR_YELLOW 14
#define COLOR_BLUE 9
#define COLOR_RESET 7

void set_color(int color);
void list_files(const char *path);
void change_directory(const char *path);
void make_directory(const char *path);
void delete_file_or_directory(const char *path);
void rename_file_or_directory(const char *oldname, const char *newname);
void search_files(const char *path, const char *pattern);
void display_file_properties(const char *file);
void copy_file(const char *source, const char *destination);
void log_operation(const char *operation);
void clear_input_buffer();
void display_welcome_message();
void display_fun_quote();
void show_loading_animation(const char *action);
char* construct_full_path(const char *base_path, const char *relative_path);
int stristr(const char *str, const char *substr);

void set_color(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

void log_operation(const char *operation) {
    FILE *log = fopen(LOG_FILE, "a");
    if (log) {
        time_t now = time(NULL);
        char *time_str = ctime(&now);
        time_str[strlen(time_str)-1] = '\0'; // Remove newline
        fprintf(log, "[%s] %s\n", time_str, operation);
        fclose(log);
    }
}

void clear_input_buffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

char* construct_full_path(const char *base_path, const char *relative_path) {
    char *full_path = malloc(MAX_PATH);
    if (!full_path) return NULL;
    
    if (relative_path[0] == '\\' || (strlen(relative_path) > 1 && relative_path[1] == ':')) {
        strncpy(full_path, relative_path, MAX_PATH-1);
    } else {
        snprintf(full_path, MAX_PATH, "%s\\%s", base_path, relative_path);
    }
    
    full_path[MAX_PATH-1] = '\0';
    return full_path;
}

int stristr(const char *str, const char *substr) {
    while (*str) {
        const char *s = str;
        const char *sub = substr;
        while (*s && *sub && tolower(*s) == tolower(*sub)) {
            s++;
            sub++;
        }
        if (!*sub) return 1;
        str++;
    }
    return 0;
}

void list_files(const char *path) {
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    char fullpath[MAX_PATH];

    snprintf(fullpath, sizeof(fullpath), "%s\\*", path);

    hFind = FindFirstFile(fullpath, &findFileData);
    if (hFind == INVALID_HANDLE_VALUE) {
        set_color(COLOR_RED);
        perror("Could not open directory");
        set_color(COLOR_RESET);
        return;
    }

    set_color(COLOR_BLUE);
    printf("\nContents of %s:\n", path);
    set_color(COLOR_RESET);

    do {
        if (strcmp(findFileData.cFileName, ".") == 0 || strcmp(findFileData.cFileName, "..") == 0)
            continue;
        if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            set_color(COLOR_YELLOW);
            printf("[DIR]  %s\n", findFileData.cFileName);
            set_color(COLOR_RESET);
        } else {
            printf("      %s\n", findFileData.cFileName);
        }
    } while (FindNextFile(hFind, &findFileData) != 0);

    FindClose(hFind);
    log_operation("Listed files");
}

void change_directory(const char *path) {
    if (SetCurrentDirectory(path)) {
        set_color(COLOR_GREEN);
        printf("Directory changed successfully.\n");
        set_color(COLOR_RESET);
        log_operation("Changed directory");
    } else {
        set_color(COLOR_RED);
        perror("Failed to change directory");
        set_color(COLOR_RESET);
    }
}

void make_directory(const char *path) {
    if (CreateDirectory(path, NULL)) {
        set_color(COLOR_GREEN);
        printf("Directory created successfully.\n");
        set_color(COLOR_RESET);
        log_operation("Created directory");
    } else {
        set_color(COLOR_RED);
        perror("Failed to create directory");
        set_color(COLOR_RESET);
    }
}

void delete_file_or_directory(const char *path) {
    printf("Are you sure you want to delete '%s'? (y/n): ", path);
    char confirm = getchar();
    clear_input_buffer();
    if (confirm == 'y' || confirm == 'Y') {
        show_loading_animation("Deleting");

        if (RemoveDirectory(path) || DeleteFile(path)) {
            set_color(COLOR_GREEN);
            printf("Deleted successfully.\n");
            set_color(COLOR_RESET);
            log_operation("Deleted file/directory");
        } else {
            set_color(COLOR_RED);
            perror("Failed to delete");
            set_color(COLOR_RESET);
        }
    } else {
        set_color(COLOR_YELLOW);
        printf("Delete operation cancelled.\n");
        set_color(COLOR_RESET);
    }
}

void rename_file_or_directory(const char *oldname, const char *newname) {
    char current_path[MAX_PATH];
$getCurrentDirectory(MAX_PATH, current_path);
    char *full_oldpath = construct_full_path(current_path, oldname);
    char *full_newpath = construct_full_path(current_path, newname);

    if (!full_oldpath || !full_newpath) {
        set_color(COLOR_RED);
        printf("Memory allocation failed\n");
        set_color(COLOR_RESET);
        free(full_oldpath);
        free(full_newpath);
        return;
    }

    show_loading_animation("Renaming");

    if (MoveFile(full_oldpath, full_newpath)) {
        set_color(COLOR_GREEN);
        printf("Renamed successfully.\n");
        set_color(COLOR_RESET);
        log_operation("Renamed file/directory");
    } else {
        set_color(COLOR_RED);
        perror("Failed to rename");
        set_color(COLOR_RESET);
    }

    free(full_oldpath);
    free(full_newpath);
}

void search_files(const char *path, const char *pattern) {
    WIN32_FIND_DATA findFileData;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    char fullpath[MAX_PATH];
    
    snprintf(fullpath, sizeof(fullpath), "%s\\*", path);
    
    hFind = FindFirstFile(fullpath, &findFileData);
    if (hFind == INVALID_HANDLE_VALUE) {
        set_color(COLOR_RED);
        printf("Error searching directory: %s\n", path);
        set_color(COLOR_RESET);
        return;
    }

    do {
        if (strcmp(findFileData.cFileName, ".") == 0 || strcmp(findFileData.cFileName, "..") == 0)
            continue;

        char *full_item_path = construct_full_path(path, findFileData.cFileName);
        if (!full_item_path) continue;

        if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            search_files(full_item_path, pattern);
        } else if (stristr(findFileData.cFileName, pattern)) {
            set_color(COLOR_GREEN);
            printf("Found: %s\n", full_item_path);
            set_color(COLOR_RESET);
        }

        free(full_item_path);
    } while (FindNextFile(hFind, &findFileData) != 0);

    FindClose(hFind);
}

void display_file_properties(const char *file) {
    WIN32_FILE_ATTRIBUTE_DATA fileData;
    char current_path[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, current_path);
    char *full_path = construct_full_path(current_path, file);

    if (!full_path) {
        set_color(COLOR_RED);
        printf("Memory allocation failed\n");
        set_color(COLOR_RESET);
        return;
    }

    if (GetFileAttributesEx(full_path, GetFileExInfoStandard, &fileData)) {
        SYSTEMTIME systemTime;
        FileTimeToSystemTime(&fileData.ftLastWriteTime, &systemTime);
        set_color(COLOR_BLUE);
        printf("\nFile: %s\n", full_path);
        printf("Size: %lld bytes\n", ((long long)fileData.nFileSizeHigh << 32) + fileData.nFileSizeLow);
        printf("Last Modified: %02d/%02d/%04d %02d:%02d:%02d\n",
               systemTime.wMonth, systemTime.wDay, systemTime.wYear,
               systemTime.wHour, systemTime.wMinute, systemTime.wSecond);
        set_color(COLOR_RESET);
    } else {
        set_color(COLOR_RED);
        printf("Unable to fetch file properties\n");
        set_color(COLOR_RESET);
    }

    free(full_path);
}

void copy_file(const char *source, const char *destination) {
    char current_path[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, current_path);
    char *full_source = construct_full_path(current_path, source);
    char *full_dest = construct_full_path(current_path, destination);

    if (!full_source || !full_dest) {
        set_color(COLOR_RED);
        printf("Memory allocation failed\n");
        set_color(COLOR_RESET);
        free(full_source);
        free(full_dest);
        return;
    }

    show_loading_animation("Copying");

    if (CopyFile(full_source, full_dest, FALSE)) {
        set_color(COLOR_GREEN);
        printf("File copied successfully.\n");
        set_color(COLOR_RESET);
        log_operation("Copied file");
    } else {
        set_color(COLOR_RED);
        perror("Failed to copy file");
        set_color(COLOR_RESET);
    }

    free(full_source);
    free(full_dest);
}

void show_loading_animation(const char *action) {
    set_color(COLOR_YELLOW);
    printf("\n%s... Please wait", action);
    int j;
    for ( j = 0; j < 5; j++) {
        printf(".");
        fflush(stdout);
        Sleep(500);
    }
    printf("\n");
    set_color(COLOR_RESET);
}

void display_welcome_message() {
    set_color(COLOR_BLUE);
    printf("\n(: Welcome to the Simple File Explorer :)\n");
    set_color(COLOR_RESET);
}

void display_fun_quote() {
    const char *quotes[] = {
        "Life is too short for bad coffee!",
        "There's no place like 127.0.0.1!",
        "Procrastination is the art of keeping up with yesterday.",
        "The best way to predict the future is to invent it!"
    };
    srand(time(NULL));
    int index = rand() % 4;
    set_color(COLOR_GREEN);
    printf("\n* Fun Quote: \"%s\"\n", quotes[index]);
    set_color(COLOR_RESET);
}

int main() {
    char current_path[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, current_path);

    int choice;
    char path[MAX_PATH], newname[MAX_PATH], pattern[MAX_PATH], dest[MAX_PATH];

    display_welcome_message();
    display_fun_quote();

    while (1) {
        printf("\nCurrent Directory: %s\n", current_path);
        printf("Menu:\n");
        printf("1. List files and directories\n");
        printf("2. Change directory\n");
        printf("3. Create a directory\n");
        printf("4. Delete file/directory\n");
        printf("5. Rename file/directory\n");
        printf("6. Search files\n");
        printf("7. View file properties\n");
        printf("8. Copy file\n");
        printf("9. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        clear_input_buffer();

        switch (choice) {
            case 1:
                list_files(current_path);
                break;
            case 2:
                printf("Enter directory to change: ");
                fgets(path, sizeof(path), stdin);
                path[strcspn(path, "\n")] = '\0';
                change_directory(path);
                GetCurrentDirectory(MAX_PATH, current_path);
                break;
            case 3:
                printf("Enter directory name to create: ");
                fgets(path, sizeof(path), stdin);
                path[strcspn(path, "\n")] = '\0';
                make_directory(path);
                break;
            case 4:
                printf("Enter file/directory name to delete: ");
                fgets(path, sizeof(path), stdin);
                path[strcspn(path, "\n")] = '\0';
                delete_file_or_directory(path);
                break;
            case 5:
                printf("Enter current file/directory name: ");
                fgets(path, sizeof(path), stdin);
                path[strcspn(path, "\n")] = '\0';
                printf("Enter new name: ");
                fgets(newname, sizeof(newname), stdin);
                newname[strcspn(newname, "\n")] = '\0';
                rename_file_or_directory(path, newname);
                break;
            case 6:
                printf("Enter search pattern: ");
                fgets(pattern, sizeof(pattern), stdin);
                pattern[strcspn(pattern, "\n")] = '\0';
                set_color(COLOR_BLUE);
                printf("\nSearching for '%s' in %s:\n", pattern, current_path);
                set_color(COLOR_RESET);
                search_files(current_path, pattern);
                log_operation("Searched files");
                break;
            case 7:
                printf("Enter file name to view properties: ");
                fgets(path, sizeof(path), stdin);
                path[strcspn(path, "\n")] = '\0';
                display_file_properties(path);
                break;
            case 8:
                printf("Enter source file: ");
                fgets(path, sizeof(path), stdin);
                path[strcspn(path, "\n")] = '\0';
                printf("Enter destination file: ");
                fgets(dest, sizeof(dest), stdin);
                dest[strcspn(dest, "\n")] = '\0';
                copy_file(path, dest);
                break;
            case 9:
                set_color(COLOR_YELLOW);
                printf("Exiting the File Explorer. Goodbye!\n");
                set_color(COLOR_RESET);
                return 0;
            default:
                set_color(COLOR_RED);
                printf("Invalid choice! Please try again.\n");
                set_color(COLOR_RESET);
        }
    }

    return 0;
}
