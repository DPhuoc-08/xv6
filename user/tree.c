#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
#include "user/user.h"

int maximumRecursionDepth = 999999;
int dOption = 0;

static int
isPositiveNumericString(const char *str) {
    if (str[0] == '\0') return 0;
    for (int i = 0; str[i] != '\0'; i++) {
        if (str[i] < '0' || str[i] > '9') {
            return 0;
        }
    }
    return 1;
}

static int
isCurrentOrParentDirectory(const char *directoryOrFileName)
{
    return strcmp(directoryOrFileName, ".") == 0 || strcmp(directoryOrFileName, "..") == 0;
}

static void
extractStringNameFromDirectoryEntry(char *outputStringBuffer, struct dirent *directoryEntry)
{
    memmove(outputStringBuffer, directoryEntry->name, DIRSIZ);
    outputStringBuffer[DIRSIZ] = '\0';
}

static int
concatenateParentAndChildPaths(char *outputPathBuffer, int outputBufferSize, const char *parentPath, const char *childName)
{
    int parentPathLength = strlen(parentPath);
    int childNameLength = strlen(childName);

    if (parentPathLength + 1 + childNameLength + 1 > outputBufferSize)
    {
        return -1;
    }

    memmove(outputPathBuffer, parentPath, parentPathLength);
    outputPathBuffer[parentPathLength] = '/';
    memmove(outputPathBuffer + parentPathLength + 1, childName, childNameLength);
    outputPathBuffer[parentPathLength + 1 + childNameLength] = '\0';

    return 0;
}

static void
formattedIndentPrint(int currentDepthLevel, int *isLastEntryAtDepthLevel)
{
    for (int depthIndex = 0; depthIndex < currentDepthLevel; depthIndex++)
    {
        if (isLastEntryAtDepthLevel[depthIndex])
        {
            printf("    ");
        }
        else
        {
            printf("│   ");
        }
    }
}

static void
recursiveTree(const char *currentPath, int currentDepthLevel, int *isLastEntryAtDepthLevel)
{
    if (currentDepthLevel >= maximumRecursionDepth)
    {
        return;
    }

    int directoryFileDescriptor = open(currentPath, O_RDONLY);
    if (directoryFileDescriptor < 0)
    {
        fprintf(2, "tree: cannot open %s\n", currentPath);
        return;
    }

    struct stat directoryStatusInfo;
    if (fstat(directoryFileDescriptor, &directoryStatusInfo) < 0)
    {
        fprintf(2, "tree: fstat failed %s\n", currentPath);
        close(directoryFileDescriptor);
        return;
    }

    if (directoryStatusInfo.type != T_DIR)
    {
        close(directoryFileDescriptor);
        return;
    }

    char currentValidName[DIRSIZ + 1];
    int hasPendingEntry = 0;
    struct dirent nextDirectoryEntry;

    while (read(directoryFileDescriptor, &nextDirectoryEntry, sizeof(nextDirectoryEntry)) == sizeof(nextDirectoryEntry))
    {
        if (nextDirectoryEntry.inum == 0)
        {
            continue;
        }

        char extractedChildName[DIRSIZ + 1];
        extractStringNameFromDirectoryEntry(extractedChildName, &nextDirectoryEntry);

        if (isCurrentOrParentDirectory(extractedChildName))
        {
            continue;
        }

        if (dOption)
        {
            char fullChildPath[512];
            if (concatenateParentAndChildPaths(fullChildPath, sizeof(fullChildPath), currentPath, extractedChildName) < 0)
            {
                continue;
            }

            struct stat childStatusInfo;
            if (stat(fullChildPath, &childStatusInfo) < 0)
            {
                continue;
            }

            if (childStatusInfo.type != T_DIR)
            {
                continue;
            }
        }

        if (hasPendingEntry)
        {
            formattedIndentPrint(currentDepthLevel, isLastEntryAtDepthLevel);
            printf("├── %s\n", currentValidName);

            char fullChildPath[512];
            if (concatenateParentAndChildPaths(fullChildPath, sizeof(fullChildPath), currentPath, currentValidName) == 0) {
                struct stat childStatusInfo;
                if (stat(fullChildPath, &childStatusInfo) == 0 && childStatusInfo.type == T_DIR) {
                    isLastEntryAtDepthLevel[currentDepthLevel] = 0;
                    recursiveTree(fullChildPath, currentDepthLevel + 1, isLastEntryAtDepthLevel);
                }
            }
        }

        strcpy(currentValidName, extractedChildName);
        hasPendingEntry = 1;
    }

    close(directoryFileDescriptor);

    if (hasPendingEntry) {
        formattedIndentPrint(currentDepthLevel, isLastEntryAtDepthLevel);
        printf("└── %s\n", currentValidName);

        char fullChildPath[512];
        if (concatenateParentAndChildPaths(fullChildPath, sizeof(fullChildPath), currentPath, currentValidName) == 0) {
            struct stat childStatusInfo;
            if (stat(fullChildPath, &childStatusInfo) == 0 && childStatusInfo.type == T_DIR) {
                isLastEntryAtDepthLevel[currentDepthLevel] = 1;
                recursiveTree(fullChildPath, currentDepthLevel + 1, isLastEntryAtDepthLevel);
            }
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc > 5)
    {
        fprintf(2, "usage: tree [path] [-L depth] [-d]\n");
        exit(1);
    }

    const char *startDirectoryPath = ".";

    for (int argumentIndex = 1; argumentIndex < argc; argumentIndex++)
    {
        if (strcmp(argv[argumentIndex], "-L") == 0)
        {
            if (argumentIndex + 1 < argc)
            {
                if (isPositiveNumericString(argv[argumentIndex + 1]))
                {
                    int tempInt = atoi(argv[argumentIndex + 1]);
                    maximumRecursionDepth = tempInt;
                    argumentIndex++;
                    continue;
                }
            }
            fprintf(2, "tree: invalid depth\n");
            fprintf(2, "usage: tree [path] [-L depth] [-d]\n");
            exit(1);
        }
        else if (strcmp(argv[argumentIndex], "-d") == 0)
        {
            dOption = 1;
        }
        else
        {
            startDirectoryPath = argv[argumentIndex];
        }
    }

    printf("%s\n", startDirectoryPath);

    int isLastEntryAtDepthLevel[128] = {0};

    recursiveTree(startDirectoryPath, 0, isLastEntryAtDepthLevel);

    exit(0);
}