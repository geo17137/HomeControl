#ifndef MTR86_H
#define MTR86_H
#include <Arduino.h>
// Redéclare les fonction RTOS avec la syntaxe du noyau temps réel Mtr86
#define TACHE TaskHandle_t
#define TACHE_T TimerHandle_t

#define USE_TIMERS 1

inline  TACHE cree(TaskFunction_t pvTaskCode, const char* pcName, uint32_t usStackDepth, UBaseType_t uxPriority, void* pvParameters = NULL, TaskHandle_t* _pxCreatedTask = NULL) {
    TaskHandle_t pxCreatedTask;
    xTaskCreate(pvTaskCode, pcName, usStackDepth, pvParameters, uxPriority, &pxCreatedTask);
    return pxCreatedTask;
}

inline TACHE tache_id(const char *pcNameToQuery) {
    return xTaskGetHandle(pcNameToQuery);
}

inline TACHE_T t_cree(
    TimerCallbackFunction_t pxCallbackFunction,
    const TickType_t xTimerPeriod,
    const UBaseType_t uxAutoReload = pdFALSE,
    const char *const pcTimerName = NULL,
    void *const pvTimerID = NULL)
{
    return xTimerCreate
                 ( pcTimerName,
                   xTimerPeriod,
                   uxAutoReload,
                   pvTimerID,
                   pxCallbackFunction );
}

inline BaseType_t t_start( TimerHandle_t xTimer,
                            TickType_t xBlockTime = 0) {
  return xTimerStart(xTimer, xBlockTime);
}

inline BaseType_t t_stop(TimerHandle_t xTimer,
    TickType_t xBlockTime = 0) {
  return xTimerStop(xTimer, xBlockTime);
}

inline void supprime(TACHE id) {
   vTaskDelete( id );
}

inline void dort(const TickType_t xTicksToDelay) {
    if (xTicksToDelay == 0) 
      vTaskSuspend(xTaskGetCurrentTaskHandle());
    vTaskDelay(xTicksToDelay);
}

inline void suspend(TaskHandle_t xTaskToSuspend) {
    vTaskSuspend(xTaskToSuspend );
}

inline void reveille(TaskHandle_t xTaskToResume) {
    vTaskResume(xTaskToResume);
}

/*
 Recharge le timer
*/

inline BaseType_t setDelay(TimerHandle_t xTimer, TickType_t xNewPeriod, TickType_t xBlockTime = 100) {
    return xTimerChangePeriod(xTimer, xNewPeriod, xBlockTime) == pdPASS;
    
}

/*
  Temps restant avant execution
  TickType_t xTimerGetExpiryTime( TimerHandle_t xTimer );
*/
inline TickType_t dlyRestant(TimerHandle_t xTimer) {
    return xTimerGetExpiryTime(xTimer) - xTaskGetTickCount();
}

inline uint cvrtic(uint t) {
    return pdMS_TO_TICKS(t);
}
#endif