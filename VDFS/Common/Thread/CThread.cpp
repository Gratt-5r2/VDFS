#include <UnionAfx.h>

namespace Common 
{
  CThread::CThread () {
    hFunc    = NULL;
    dwThreadID  = NULL;
    hThread    = NULL;
  };

  CThread::CThread (HFUNC f) {
    Init (f);
  };

  void CThread::Init (HFUNC f) {
    hFunc = f;
    dwThreadID  = NULL;
    hThread    = NULL;
  };

  DWORD CThread::Detach (HMEM res) {
    hThread = CreateThread (NULL, NULL, (LPTHREAD_START_ROUTINE)hFunc, res, NULL, &dwThreadID);
    return dwThreadID;
  };

  void CThread::Break () {
    TerminateThread (hThread, 0);
  };

  void CThread::Suspend () {
    SuspendThread (hThread);
  };

  void CThread::Resume () {
    ResumeThread (hThread);
  };

  void CThread::SetPriority (EThreadPriority p) {
    SetThreadPriority (hThread, p);
  };

  EThreadPriority CThread::GetPriority () {
    return (EThreadPriority)GetThreadPriority (hThread);
  };

  HFUNC CThread::GetFunction () {
    return hFunc;
  };

  DWORD CThread::GetID () {
    return dwThreadID;
  };

  HANDLE CThread::GetHandle () {
    return hThread;
  };

  CThread::~CThread () {};

  CMutex::CMutex () {
    InitializeCriticalSection (&mutex);
  };

  void CMutex::Enter () {
    EnterCriticalSection (&mutex);
  };
  void CMutex::Leave () {
    LeaveCriticalSection (&mutex);
  };

  CMutex::~CMutex () {
    DeleteCriticalSection (&mutex);
  };

  CHandleMutex::CHandleMutex( CMutex& mutex ) {
    sourceMutex = &mutex;
    sourceMutex->Enter();
  }

  CHandleMutex::~CHandleMutex() {
    sourceMutex->Leave();
  }
}