#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fs.h"
#include "kernel/fcntl.h"
#include "user/user.h"

int summaryOnlyOption = 0;
int allFilesOption = 0;

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
    int childNameLength  = strlen(childName);

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

static long
calculateDiskUsage(const char *currentPath, int isRootPath)
{
    int fileDescriptor = open(currentPath, O_RDONLY);
    if (fileDescriptor < 0)
    {
        fprintf(2, "du: cannot open %s\n", currentPath);
        return 0;
    }

    struct stat fileStatusInfo;
    if (fstat(fileDescriptor, &fileStatusInfo) < 0)
    {
        fprintf(2, "du: fstat failed %s\n", currentPath);
        close(fileDescriptor);
        return 0;
    }

    if (fileStatusInfo.type == T_FILE)
    {
        close(fileDescriptor);
        long fileSize = fileStatusInfo.size;

        int shouldPrintThisEntry = 0;
        if (summaryOnlyOption)
        {
            if (isRootPath) shouldPrintThisEntry = 1;
        }
        else
        {
            if (allFilesOption || isRootPath) shouldPrintThisEntry = 1;
        }

        if (shouldPrintThisEntry)
        {
            printf("%d\t%s\n", (int)fileSize, currentPath);
        }
        return fileSize;
    }

    if (fileStatusInfo.type == T_DIR)
    {
        long accumulatedTotalSize = 0;
        struct dirent nextDirectoryEntry;

        while (read(fileDescriptor, &nextDirectoryEntry, sizeof(nextDirectoryEntry)) == sizeof(nextDirectoryEntry))
        {
            if (nextDirectoryEntry.inum == 0) continue;

            char extractedChildName[DIRSIZ + 1];
            extractStringNameFromDirectoryEntry(extractedChildName, &nextDirectoryEntry);

            if (isCurrentOrParentDirectory(extractedChildName)) continue;

            char fullChildPath[512];
            if (concatenateParentAndChildPaths(fullChildPath, sizeof(fullChildPath), currentPath, extractedChildName) < 0) continue;

            accumulatedTotalSize += calculateDiskUsage(fullChildPath, 0);
        }
        close(fileDescriptor);

        int shouldPrintThisEntry = 0;
        if (summaryOnlyOption)
        {
            if (isRootPath) shouldPrintThisEntry = 1;
        }
        else
        {
            shouldPrintThisEntry = 1;
        }

        if (shouldPrintThisEntry)
        {
            printf("%d\t%s\n", (int)accumulatedTotalSize, currentPath);
        }
        return accumulatedTotalSize;
    }

    close(fileDescriptor);
    return 0;
}

int main(int argc, char *argv[])
{
    if (argc > 4)
    {
        fprintf(2, "usage: du [path] [-a] [-s]\n");
        exit(1);
    }

    const char *startDirectoryPath = ".";

    for (int argumentIndex = 1; argumentIndex < argc; argumentIndex++)
    {
        if (strcmp(argv[argumentIndex], "-a") == 0)
        {
            allFilesOption = 1;
        }
        else if (strcmp(argv[argumentIndex], "-s") == 0)
        {
            summaryOnlyOption = 1;
        }
        else
        {
            startDirectoryPath = argv[argumentIndex];
        }
    }

    if (allFilesOption && summaryOnlyOption)
    {
        fprintf(2, "du: cannot both summarize and show all entries\n");
        exit(1);
    }

    calculateDiskUsage(startDirectoryPath, 1);
    exit(0);
}
