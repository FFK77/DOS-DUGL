#ifndef DWORKER_H_INCLUDED
#define DWORKER_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

// DWorker support =====================================================================

typedef void (*dworkerFunctionPointer)(void*, int);

unsigned int CreateDWorker(dworkerFunctionPointer workerFunction, void *workerData);
void RunDWorker(unsigned int dworkerID, bool WaitIfBusy);
void SetDWorkerDataPtr(unsigned int dworkerID, void *dataPtr);
void SetDWorkerFunction(unsigned int dworkerID, dworkerFunctionPointer workerFunction);
// Set DWorker thread priority: 0 highest - 3 lowest
void SetDWorkerPriority(int priority);
bool IsBusyDWorker(unsigned int dworkerID);
void WaitDWorker(unsigned int dworkerID);
bool WaitTimeOutDWorker(unsigned int dworkerID, unsigned int timeOut);
void DestroyDWorker(unsigned int dworkerID);
void DWorkerYield();

// Mutex
#define PDMutex void*
void *CreateDMutex();
void  DestroyDMutex(PDMutex DMutexPtr);
int  LockDMutex(PDMutex DMutexPtr);
int  UnlockDMutex(PDMutex DMutexPtr);
bool  TryLockDMutex(PDMutex DMutexPtr);

#ifdef __cplusplus
        }  // extern "C" {
#endif

#endif // DWORKER_H_INCLUDED

