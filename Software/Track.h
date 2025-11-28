#ifndef __TRACK_H
#define __TRACK_H

#include "stdint.h"
#include "system_data.h"

// 状态机初始化函数
void TrackingStateMachine_Init(void);

// 状态机更新函数，需要在主循环中定期调用，推荐20ms间隔
void TrackingStateMachine_Update(void);

// 获取当前跟踪状态
TrackingState_t Tracking_GetCurrentState(void);

#endif
