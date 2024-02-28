#include "dberror.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "buffer_mgr.h"
#include "storage_mgr.h"

typedef struct PageFrameHandleNode {
    int FixCount;
    SM_PageHandle readContent;
    bool DirtyFlag;
    bool UsedFlag; // for clock strategy
    int FrameNum; 
    BM_PageHandle* bh; 
} PageFrameHandleNode;

int write_count=0,read_count=0;

RC initBufferPool(BM_BufferPool *const bm, const char *const pageFileName, const int numPages, ReplacementStrategy strategy,void *stratData) {
     

    // Allocate memory for the buffer pool and frame handles
  PageFrameHandleNode *pg = malloc(numPages * sizeof(PageFrameHandleNode));
    if (pg== NULL) {
        bm->mgmtData = NULL;
        return RC_FILE_NOT_FOUND;
    }
    else{
    // Initialize the frame handles
        bm->numPages = numPages;
        bm->strategy = strategy;
        bm->pageFile= (char *)pageFileName;
        for (int i = 0; i < numPages; i++) {

            BM_PageHandle *bh=MAKE_PAGE_HANDLE();
            pg[i].readContent = (SM_PageHandle)malloc(PAGE_SIZE);
            pg[i].bh = bh; 
            bh->pageNum = NO_PAGE;
            bh->data = NULL;
            pg[i].FrameNum = i;    
            pg[i].DirtyFlag = false;
            pg[i].FixCount = 0;
        }
        bm->mgmtData=pg;
    }
    return RC_OK;
}

RC shutdownBufferPool(BM_BufferPool *const bm) {
    // Flush all dirty pages to disk
    PageFrameHandleNode *pgFrame = (PageFrameHandleNode *)bm->mgmtData;
    bool isOK = true;

   int i = 0;

   // Iterate through all page frames in the buffer pool
   while (isOK && i < bm->numPages)
   {
      // Check if the page is dirty and not fixed
      if (pgFrame[i].DirtyFlag == 1 && pgFrame[i].FixCount == 0)
      {
         if (forcePage(bm, pgFrame[i].bh) != RC_OK)
         {
               isOK = false;
               RC_message = "Force flush was unsuccessful.";
         }
      }
      i++;
   }

   // Check if the buffer pool exists  
   if (bm->mgmtData == NULL)
   {
      RC_message = "Force flush was unsuccessful as buffer pool does not exist.";
      return RC_ERROR;
   }

   RC_message = "Force flush was successful.";
   return isOK ? RC_OK : RC_ERROR;
}


RC forceFlushPool(BM_BufferPool *const bm) {

    PageFrameHandleNode *pgFrame = (PageFrameHandleNode *)bm->mgmtData;
    if(bm->mgmtData==NULL){
        return RC_ERROR;
    }
    else{
    // Flush all dirty pages to disk
    for (int i = 0; i < bm->numPages; i++) {
        if (pgFrame->DirtyFlag==1 && pgFrame->FixCount==0) {
            forcePage(bm,pgFrame->bh);
            
        }
    }
    }

    return RC_OK;
}

// Buffer Manager Interface Access Pages
RC markDirty(BM_BufferPool *const bm, BM_PageHandle *const page) {
    PageFrameHandleNode *pgFrame = (PageFrameHandleNode *)bm->mgmtData;
    bool found = false;
    
    // Loop through the frames to find the matching page
    for (int index = 0; index < bm->numPages && !found; index++) {
        if (pgFrame[index].bh->pageNum == page->pageNum) {
            // Mark the page as dirty
            pgFrame[index].DirtyFlag = true;
            // Increment the count of dirty pages
            write_count++;
            found = true;
        }
    }
    
    // Set the appropriate message
    if (found) {
        RC_message = "Page marked as dirty.";
        return RC_OK;
    } else {
        RC_message = "Page not found in buffer pool.";
        return RC_FILE_NOT_FOUND;
    }
}

RC unpinPage(BM_BufferPool *const bm, BM_PageHandle *const page) {
    PageFrameHandleNode *pgFrame = (PageFrameHandleNode *)bm->mgmtData;
    bool unpinned = false;
    
    // Iterate through the frames to find the matching page
    for (int index = 0; index < bm->numPages && !unpinned; index++) {
        if (pgFrame[index].bh->pageNum == page->pageNum && pgFrame[index].FixCount > 0) {
            // Decrease the fix count to unpin the page
            pgFrame[index].FixCount--;
            unpinned = true;
        }
    }
    
    // Set the appropriate message
    if (unpinned) {
        RC_message = "Page unpinned successfully.";
        return RC_OK;
    } else {
        RC_message = "Unpin unsuccessful. Page number not found in the buffer pool or fix count is zero.";
        return RC_ERROR;
    }
}


RC forcePage (BM_BufferPool *const bm, BM_PageHandle *const page){
    return RC_OK;

}
RC pinPage (BM_BufferPool *const bm, BM_PageHandle *const page, 
		const PageNumber pageNum){

            return RC_OK;
        }

// Statistics Interface
PageNumber *getFrameContents (BM_BufferPool *const bm){
    PageNumber *pg=malloc(sizeof(PageNumber)*bm->numPages);
    PageFrameHandleNode *pgFrame = (PageFrameHandleNode *)bm->mgmtData;

    for (int i = 0; i < bm->numPages; i++) {
        // Check if the page number is NO_PAGE (-1)
        if (pgFrame[i].bh->pageNum == NO_PAGE) {
            pg[i] = NO_PAGE;
        } else {
            // Otherwise, assign the actual page number
            pg[i] = pgFrame[i].bh->pageNum;
        }
    }

    return pg;

}
bool *getDirtyFlags (BM_BufferPool *const bm){
    bool* dirtyFlags = malloc(bm->numPages * sizeof(bool));
    PageFrameHandleNode *pgFrame = (PageFrameHandleNode*)bm->mgmtData;

    if (dirtyFlags == NULL) {
        return NULL;
    }

    for (int i = 0; i < bm->numPages; i++) {
        dirtyFlags[i] = pgFrame[i].DirtyFlag;
    }

    return dirtyFlags;
}

int *getFixCounts (BM_BufferPool *const bm){
    int* fixCounts = malloc(bm->numPages * sizeof(int));
    PageFrameHandleNode *pgFrame = (PageFrameHandleNode*)bm->mgmtData;

    if (fixCounts == NULL) {
        return NULL;
    }

    for (int i = 0; i < bm->numPages; i++) {
        if(pgFrame[i].FixCount==-1){
            fixCounts[i]=0;
        }else{
            fixCounts[i] = pgFrame[i].FixCount;
        }
    }

    return fixCounts;
}

int getNumReadIO (BM_BufferPool *const bm)
{
    return  read_count;
}

int getNumWriteIO (BM_BufferPool *const bm){
    return  write_count;
}