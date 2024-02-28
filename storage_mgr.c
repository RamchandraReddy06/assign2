
#include "dberror.h"
#include "stdio.h"
#include "storage_mgr.h"
#include "test_helper.h"
#include <stdio.h>
#include <string.h>

// Global variables (consider moving to a struct for better encapsulation)
FILE *F_fileHandlePtr = NULL;
char *out_message = NULL;  // Initialize the out_message
RC ret_message;

// Function prototypes for better readability
RC createPageFile(char *fileName);
void handleFileError(char *error_message);

// Storage Manager Setup
void initStorageManager() {
    printf("In initStorageManager\n");
    F_fileHandlePtr=NULL;
}

RC createPageFile(char *fileName) {
    F_fileHandlePtr = fopen(fileName, "w+");

    if (F_fileHandlePtr == NULL) {
        handleFileError("Could not create the file.");
        return RC_FILE_NOT_FOUND;
    }

    // Fill the initial page with null characters
    char buffer[PAGE_SIZE] = {'\0'};
    if (fwrite(buffer, sizeof(char), PAGE_SIZE, F_fileHandlePtr) != PAGE_SIZE) {
        handleFileError("Error writing to file.");
        fclose(F_fileHandlePtr);
        return RC_FILE_NOT_FOUND;
    }

    fclose(F_fileHandlePtr);
    out_message = "Created the file successfully.";
    return RC_OK;
}

void handleFileError(char *error_message) {
    out_message = error_message;
    fclose(F_fileHandlePtr);  // Close the file if it was opened
}


long getFileSize(FILE *file) {
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);  // Rewind to beginning
    return size;
}

RC openPageFile(char *fileName, SM_FileHandle *fHandle) {
    F_fileHandlePtr = fopen(fileName, "r+");

    if (F_fileHandlePtr == NULL) {
        out_message = "File Not Found";
        return RC_FILE_NOT_FOUND;
    }
    if (fileName == NULL || fHandle == NULL) {
        return RC_FILE_NOT_FOUND; // Invalid file name or file handle
    }
   
    // Initialize file handle
    fHandle->fileName = fileName;
    fHandle->mgmtInfo = F_fileHandlePtr;

    // Calculate total number of pages
    long totalFileSize = getFileSize(F_fileHandlePtr);
    fHandle->totalNumPages = (int)(totalFileSize / PAGE_SIZE);
    
    // Set current page position to 0
    fHandle->curPagePos = 0;

    out_message = "Opened the file successfully";
    return RC_OK;
}



RC closePageFile(SM_FileHandle *fHandle) {
    if (fHandle->mgmtInfo == NULL) {
        out_message = "File Not Found";
        return RC_FILE_NOT_FOUND;
    }

     if (fHandle == NULL || fHandle->fileName == NULL) {
        return RC_FILE_NOT_FOUND; // Invalid file handle
    }

    FILE *pageFileToClose = fHandle->mgmtInfo;
    fHandle->mgmtInfo = NULL;  // Mark as closed before potential errors

    if (fclose(pageFileToClose) != 0) {
        // Handle potential errors during file close
        out_message = "Error closing file";
        return RC_FILE_NOT_FOUND;
    }

    out_message = "File Closed Successfully";
    return RC_OK;
}


extern RC destroyPageFile (char *fileName){
    // Attempt to open the file in read mode to check if it exists
    F_fileHandlePtr = fopen(fileName, "r");
    if (F_fileHandlePtr == NULL) {
        // File does not exist
        return RC_FILE_NOT_FOUND;
    }

    // Close the file
    int result = fclose(F_fileHandlePtr);
    if (result == 0) {
        // File closed successfully

        // Attempt to delete the file
        result = remove(fileName);
        if (result == 0) {
            // File deleted successfully
            return RC_OK;
        } else {
            // Failed to delete the file
            printError(RC_FILE_NOT_FOUND);
            return RC_FILE_NOT_FOUND;
        }
    } else {
        // Failed to close the file
        printError(RC_FILE_NOT_FOUND);
        return RC_FILE_NOT_FOUND;
    }
}


extern RC readBlock (int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage){
    RC msg;
  // Check if the page number is valid
    F_fileHandlePtr=fopen(fHandle->fileName,"r");
     if (fHandle->mgmtInfo == NULL) {
        // Handle file not found
        out_message = "File Not Found.";
        msg=RC_FILE_NOT_FOUND;
    }

    if (pageNum < 0 || pageNum >= fHandle->totalNumPages) {
        // Handle invalid page number
        out_message = "Invalid page number.";
        msg= RC_READ_NON_EXISTING_PAGE;
    }
    if(fHandle==NULL || fHandle->fileName==NULL || fHandle->mgmtInfo==NULL)
    {
        printf("Null pointer error!\n");
        printError(RC_FILE_NOT_FOUND);
       // fclose(F_fileHandlePtr);
        msg= RC_FILE_NOT_FOUND;
    }
    if(F_fileHandlePtr==NULL){
        msg= RC_FILE_NOT_FOUND;
    }
    else{
         if (pageNum < 0 || pageNum >= fHandle->totalNumPages)
        {
            // Close the file and set it to NULL in the file table
            fclose(F_fileHandlePtr);
            msg= RC_READ_NON_EXISTING_PAGE;
        }
        else{
            /* Seeking to the required position in the file.*/
            // int seekResult=fseek(F_fileHandlePtr , pageNum*PAGE_SIZE , SEEK_SET );
            /* Reading data from that position into memory.*/
            // size_t readBytes=fread(memPage,sizeof(char),PAGE_SIZE,F_fileHandlePtr);
            fHandle->curPagePos=pageNum;
            fclose(F_fileHandlePtr);
            //return_value=RC_OK;
            msg= RC_OK;
        }
    }
    return msg;
}


int getBlockPos(SM_FileHandle *fHandle) {
      //checking if file exists
    if(fHandle==NULL || fHandle->fileName==NULL || fHandle->mgmtInfo==NULL)
    {
        printf("Null pointer error!\n");
        printError(RC_FILE_NOT_FOUND);
        return RC_FILE_NOT_FOUND;
    }
    else{
        return fHandle->curPagePos;  // Direct return for clarity
    }
}

int getPageByNumber(SM_FileHandle *fHandle, char *type) {
    switch (type[0]) {  // Optimize string comparison
        case 'l': return fHandle->totalNumPages - 1;  // Handle "last"
        case 'p': return fHandle->curPagePos - 1;    // Handle "prev"
        case 'c': return fHandle->curPagePos;       // Handle "curr"
        case 'n': return fHandle->curPagePos + 1;    // Handle "next"
        default: return 0;                           // Handle invalid type
    }
}

#define PAGE_TYPE_DEFAULT "default"
#define PAGE_TYPE_LAST "last"
#define PAGE_TYPE_PREV "prev"
#define PAGE_TYPE_CURR "curr"
#define PAGE_TYPE_NEXT "next"

RC readBlockByType(SM_FileHandle *fHandle, SM_PageHandle memPage, char *type) {
    if(fHandle==NULL){
        ret_message=RC_FILE_NOT_FOUND;
        return ret_message;
    }
    else{
        int page = getPageByNumber(fHandle, type);
        return readBlock(page, fHandle, memPage);
    }
}

RC readFirstBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
    if(fHandle==NULL){
        ret_message=RC_FILE_NOT_FOUND;
        return ret_message;
    }
    else{
        return readBlockByType(fHandle, memPage, PAGE_TYPE_DEFAULT);
    }
}

RC readLastBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
    if(fHandle==NULL){
        ret_message=RC_FILE_NOT_FOUND;
        return ret_message;
    }
    else{
        return readBlockByType(fHandle, memPage, PAGE_TYPE_LAST);
    }
}

RC readPreviousBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
    if(fHandle==NULL){
        ret_message=RC_FILE_NOT_FOUND;
        return ret_message;
    }
    else{
        return readBlockByType(fHandle, memPage, PAGE_TYPE_PREV);
    }
}

RC readNextBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
    if(fHandle==NULL){
        ret_message=RC_FILE_NOT_FOUND;
        return ret_message;
    }
    else{
        return readBlockByType(fHandle, memPage, PAGE_TYPE_NEXT);
    }
}

RC readCurrentBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
    if(fHandle==NULL){
        ret_message=RC_FILE_NOT_FOUND;
        return ret_message;
    }
    else{
        return readBlockByType(fHandle, memPage, PAGE_TYPE_CURR);
    }
}




RC writeBlock(int pageNum, SM_FileHandle *fHandle, SM_PageHandle memPage) {
    // RC ret_message = RC_OK; // Define the return message variable

    // Check if file handle is NULL
    if (fHandle->mgmtInfo == NULL) {
        // out_message = "File Not Found.";
        return RC_FILE_NOT_FOUND;
    }

    // Check if page number is invalid
    if (pageNum < 0 || pageNum >= fHandle->totalNumPages) {
        out_message = "File does not have that page number.";
        return RC_READ_NON_EXISTING_PAGE;
    }

    // Seek to the correct position
    if (fseek(fHandle->mgmtInfo, pageNum * PAGE_SIZE, SEEK_SET) != 0) {
        out_message = "Error seeking in file.";
        return RC_FILE_NOT_FOUND;
    }

    // Write the block
    size_t bytesWrittenToBlock = fwrite(memPage, sizeof(char), PAGE_SIZE, fHandle->mgmtInfo);
    if (bytesWrittenToBlock != PAGE_SIZE) {
        out_message = "Error writing to file.";
        return RC_WRITE_FAILED;
    }

    // Update file handle state and close the file
    fHandle->curPagePos = pageNum;
    fclose(fHandle->mgmtInfo);
    out_message = "Content written to File successfully.";
    return RC_OK;
}


RC writeCurrentBlock(SM_FileHandle *fHandle, SM_PageHandle memPage) {
    if (fHandle == NULL || fHandle->fileName == NULL) {
        return RC_FILE_NOT_FOUND; // Invalid file handle
    }

    // Check if the current page position is valid
    else if (fHandle->curPagePos < 0 || fHandle->curPagePos >= fHandle->totalNumPages * PAGE_SIZE) {
        return RC_READ_NON_EXISTING_PAGE; // Invalid current page position
    }
    else{
        int page = getPageByNumber(fHandle, "curr");
        return writeBlock(page, fHandle, memPage);
    }
}

RC appendEmptyBlock(SM_FileHandle *fHandle) {
    if (fHandle->mgmtInfo == NULL) {
        // Handle file not found
        out_message = "File Not Found";
        return RC_FILE_NOT_FOUND;
    }

    // Seek to the end of the file and write an empty block
    if (fseek(fHandle->mgmtInfo, 0, SEEK_END) != 0) {
        // Handle seek error
        out_message = "Error seeking in file. appendempty";
        return RC_FILE_NOT_FOUND;
    }

    char eBlock[PAGE_SIZE] = {'\0'};
    if (fwrite(eBlock, sizeof(char), PAGE_SIZE, fHandle->mgmtInfo) != PAGE_SIZE) {
        // Handle write error
        out_message = "Error writing to file.";
        return RC_WRITE_FAILED;
    }

    // Update file handle state
    fHandle->totalNumPages++;
    fHandle->curPagePos = fHandle->totalNumPages - 1;

    out_message = "Appended empty block at end of file Successfully and filled with null values";
    return RC_OK;
}

extern RC ensureCapacity (int numberOfPages, SM_FileHandle *fHandle) {
    // Check if the file handle is valid
    RC msg;
    if (fHandle == NULL || fHandle->fileName == NULL) {
        msg= RC_FILE_NOT_FOUND; // Invalid file handle
    }

    // Calculate the new file size
    int newFileSize = (fHandle->totalNumPages + numberOfPages) * PAGE_SIZE;

    // Open the file specified by the file name in write mode
    FILE *file_ptr = fopen(fHandle->fileName, "r+");
    if (file_ptr == NULL) {
        msg= RC_FILE_NOT_FOUND; // File not found
    }

    // Seek to the end of the file
    if (fseek(file_ptr, 0, SEEK_END) != 0) {
        fclose(file_ptr);
        msg= RC_FILE_NOT_FOUND; // File not found
    }

    // Check if the file size needs to be increased
    if (ftell(file_ptr) < newFileSize) {
        // Allocate memory for the new pages
        char *newFileData = (char*)malloc(newFileSize * sizeof(char));
        if (newFileData == NULL) {
            // Memory allocation failed
            fclose(file_ptr);
            msg= RC_WRITE_FAILED;
        }

        // Read the existing file data
        size_t bytes_read = fread(newFileData, sizeof(char), ftell(file_ptr), file_ptr);
        if (bytes_read != ftell(file_ptr)) {
            // Reading failed
            free(newFileData);
            fclose(file_ptr);
            msg= RC_READ_NON_EXISTING_PAGE;
        }

        // Close the file
        fclose(file_ptr);

        // Open the file in write mode
        file_ptr = fopen(fHandle->fileName, "w");
        if (file_ptr == NULL) {
            // File creation failed
            free(newFileData);
            msg= RC_FILE_NOT_FOUND;
        }

        // Write the new file data
        size_t bytes_written = fwrite(newFileData, sizeof(char), newFileSize, file_ptr);
        if (bytes_written != newFileSize) {
            // Writing failed
            free(newFileData);
            fclose(file_ptr);
            msg= RC_WRITE_FAILED;
        }

        // Free memory
        free(newFileData);

        // Update the total number of pages
        fHandle->totalNumPages += numberOfPages;
    }
    return msg;
}





