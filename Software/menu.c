#include "menu.h"
#include "OLED.h"
#include "KEY.h"
#include "Motor.h"
#include "PWM.h"
#include "Delay.h"
#include "Sensor_Data.h"
#include "System_Data.h"
#include "Motor_Data.h"
#include "Timer.h"
#include <string.h>
#include <stdlib.h>

// 外部变量声明
extern Sensor_Packet sensor_packet;
extern System_Status_Packet system_status_packet;
extern Motor_Speed_Data motor_speed_data;

// 实时显示状态
static RealtimeDisplayState realtime_state = {0};
static uint8_t in_realtime_mode = 0;  // 是否处于实时显示模式

// 当前显示的菜单
static Menu *current_menu = NULL;
static uint8_t need_refresh = 1;  // 需要刷新显示标志

// 警告模式状态
static uint8_t warning_mode = 0;  // 是否处于警告模式
static uint32_t warning_start_time = 0;  // 警告开始时间

// 显示配置
#define DISPLAY_LINES 6           // 显示总行数
#define MENU_ITEMS_PER_PAGE 5     // 每页显示的菜单项数量
#define WARNING_DISPLAY_TIME 3000 // 警告显示时间（ms）

//辅助函数
static void IntToStr(uint16_t num, char *str)
{
    char temp[6];
    uint8_t i = 0, j = 0;
    // 处理0的情况
    if (num == 0) {
        str[0] = '0';
        str[1] = '\0';
        return;
    }
    // 提取各位数字
    while (num > 0) {
        temp[i++] = '0' + (num % 10);
        num /= 10;
    }
    // 反转字符串
    while (i > 0) {
        str[j++] = temp[--i];
    }
    str[j] = '\0';
}

// 新增实时参数显示函数
void Menu_DisplayRealtimeParams(void)
{
    OLED_Clear();
    
    // 显示标题
    OLED_ShowString(0, 0, "Real-Time Data", OLED_8X16);
    
    // 显示电池信息
    Menu_DisplayBatteryInfo();
    
    // 分割线
    OLED_ShowString(0, 16, "-----------------------------", OLED_6X8);
    
    uint8_t line = 3;  // 从第3行开始显示
    
    // 提前声明所有局部变量
    char bat_str[16];
    char ir_str[4];
    char enc_str[8];
    char pid_str[8];
    char page_str[4];
    uint16_t volt_int, volt_frac;
    
    // 根据当前页面显示不同参数
    switch(realtime_state.current_page) {
        case 0:  // 第1页：电池和电机状态
        {
            // 电池状态
            OLED_ShowString(0, line*8, "Bat:", OLED_6X8);
            IntToStr(system_status_packet.battery_level, bat_str);
            OLED_ShowString(24, line*8, bat_str, OLED_6X8);
            OLED_ShowString(30, line*8, "% (", OLED_6X8);
            // 显示电压（整数部分.小数部分）
            volt_int = (uint16_t)system_status_packet.battery_voltage;
            volt_frac = (uint16_t)((system_status_packet.battery_voltage - volt_int) * 100);
            IntToStr(volt_int, bat_str);
            OLED_ShowString(48, line*8, bat_str, OLED_6X8);
            OLED_ShowString(60, line*8, ".", OLED_6X8);
            IntToStr(volt_frac, bat_str);
            OLED_ShowString(66, line*8, bat_str, OLED_6X8);
            OLED_ShowString(72, line*8, "V)", OLED_6X8);
            line++;
            // 左电机目标速度
            OLED_ShowString(0, line*8, "L-Target:", OLED_6X8);
            IntToStr(motor_speed_data.left_target_speed, bat_str);
            OLED_ShowString(54, line*8, bat_str, OLED_6X8);
            line++;
            // 左电机实际速度
            OLED_ShowString(0, line*8, "L-Actual:", OLED_6X8);
            IntToStr(motor_speed_data.left_actual_speed, bat_str);
            OLED_ShowString(54, line*8, bat_str, OLED_6X8);
            line++;
            // 右电机目标速度
            OLED_ShowString(0, line*8, "R-Target:", OLED_6X8);
            IntToStr(motor_speed_data.right_target_speed, bat_str);
            OLED_ShowString(54, line*8, bat_str, OLED_6X8);
            line++;
            // 右电机实际速度
            OLED_ShowString(0, line*8, "R-Actual:", OLED_6X8);
            IntToStr(motor_speed_data.right_actual_speed, bat_str);
            OLED_ShowString(54, line*8, bat_str, OLED_6X8);
            line++;
            // 系统控制模式
            OLED_ShowString(0, line*8, "Mode:", OLED_6X8);
            switch(system_status_packet.system_control_mode) {
                case MODE_MANUAL: OLED_ShowString(30, line*8, "Manual", OLED_6X8); break;
                case MODE_AUTO: OLED_ShowString(30, line*8, "Auto", OLED_6X8); break;
                case MODE_BLUETOOTH: OLED_ShowString(30, line*8, "BT", OLED_6X8); break;
                default: OLED_ShowString(30, line*8, "Unknown", OLED_6X8); break;
            }
            line++;
            break;
        }
            
        case 1:  // 第2页：传感器数据
        {
            // 红外传感器
            OLED_ShowString(0, line*8, "IR:", OLED_6X8);
            for(int i = 0; i < 4; i++) {
                IntToStr(sensor_packet.ir_sensor_raw[i], ir_str);
                OLED_ShowString(18 + i*24, line*8, ir_str, OLED_6X8);
                if(i < 3) OLED_ShowString(30 + i*24, line*8, ",", OLED_6X8);
            }
            line++;
            
            // 左编码器
            OLED_ShowString(0, line*8, "L-Encoder:", OLED_6X8);
            IntToStr(sensor_packet.left_encoder_raw, enc_str);
            OLED_ShowString(60, line*8, enc_str, OLED_6X8);
            line++;
            
            // 右编码器
            OLED_ShowString(0, line*8, "R-Encoder:", OLED_6X8);
            IntToStr(sensor_packet.right_encoder_raw, enc_str);
            OLED_ShowString(60, line*8, enc_str, OLED_6X8);
            line++;
            
            // 转向状态
            OLED_ShowString(0, line*8, "Steer:", OLED_6X8);
            switch(system_status_packet.steering_state) {
                case STEERING_STRAIGHT: OLED_ShowString(36, line*8, "Straight", OLED_6X8); break;
                case STEERING_LEFT: OLED_ShowString(36, line*8, "Left", OLED_6X8); break;
                case STEERING_RIGHT: OLED_ShowString(36, line*8, "Right", OLED_6X8); break;
                case STEERING_SHARP_LEFT: OLED_ShowString(36, line*8, "S-Left", OLED_6X8); break;
                case STEERING_SHARP_RIGHT: OLED_ShowString(36, line*8, "S-Right", OLED_6X8); break;
                default: OLED_ShowString(36, line*8, "Unknown", OLED_6X8); break;
            }
            line++;
            
            // 路线状态
            OLED_ShowString(0, line*8, "Line:", OLED_6X8);
            if(system_status_packet.line_lost) {
                OLED_ShowString(30, line*8, "Lost", OLED_6X8);
            } else {
                OLED_ShowString(30, line*8, "On Track", OLED_6X8);
            }
            line++;
            
            // 蓝牙状态
            OLED_ShowString(0, line*8, "BT:", OLED_6X8);
            if(system_status_packet.bluetooth_connected) {
                OLED_ShowString(18, line*8, "Connected", OLED_6X8);
            } else {
                OLED_ShowString(18, line*8, "Disconnected", OLED_6X8);
            }
            line++;
            break;
        }
            
        case 2:  // 第3页：PID和错误状态
        {
            // 左电机PID输出
            OLED_ShowString(0, line*8, "L-PID Out:", OLED_6X8);
            IntToStr((int16_t)motor_speed_data.left_pid_output, pid_str);
            OLED_ShowString(60, line*8, pid_str, OLED_6X8);
            line++;
            
            // 右电机PID输出
            OLED_ShowString(0, line*8, "R-PID Out:", OLED_6X8);
            IntToStr((int16_t)motor_speed_data.right_pid_output, pid_str);
            OLED_ShowString(60, line*8, pid_str, OLED_6X8);
            line++;
            
            // 左电机误差
            OLED_ShowString(0, line*8, "L-Error:", OLED_6X8);
            IntToStr((int16_t)motor_speed_data.left_error, pid_str);
            OLED_ShowString(54, line*8, pid_str, OLED_6X8);
            line++;
            
            // 右电机误差
            OLED_ShowString(0, line*8, "R-Error:", OLED_6X8);
            IntToStr((int16_t)motor_speed_data.right_error, pid_str);
            OLED_ShowString(54, line*8, pid_str, OLED_6X8);
            line++;
            
            // 急停状态
            OLED_ShowString(0, line*8, "E-Stop:", OLED_6X8);
            if(system_status_packet.emergency_stop) {
                OLED_ShowString(42, line*8, "ACTIVE", OLED_6X8);
            } else {
                OLED_ShowString(42, line*8, "Inactive", OLED_6X8);
            }
            line++;
            break;
        }
    }
    
    // 显示页码指示器
    OLED_ShowString(110, 56, "P", OLED_6X8);
    IntToStr(realtime_state.current_page + 1, page_str);
    OLED_ShowString(116, 56, page_str, OLED_6X8);
    OLED_ShowString(122, 56, "/3", OLED_6X8);
    
    OLED_Update();
}

//test_menu
void Test_Motor(void)
{
	Motor_SetSpeed_Left(100);
	Motor_SetSpeed_Right(100);
	for (int i=0;i<5201314*2;i++){}
	Motor_SetSpeed_Left(0);
	Motor_SetSpeed_Right(0);
}
void Show_Debug_Menu(void)
{
	for (uint8_t x =0;x<4;x++)
	{
		OLED_ShowString(0,32+x*8,"Serial",OLED_6X8);
		OLED_ShowNum(0,32+x*8,x+1,1,OLED_6X8);
		OLED_ShowNum(48,32+x*8,sensor_packet.ir_sensor_raw[x],1,OLED_6X8);
	}
}

// 菜单初始化
void Menu_Init(void)
{
    // 创建菜单
    Menu *main_menu = Menu_Create("Main Menu");
    Menu *test_menu = Menu_Create("Test Mode");
    
    // 主菜单
    Menu_AddItem(main_menu, "Emergency Stop", MENU_ITEM_ACTION, NULL, Emergency_Stop_Execute);
    Menu_AddItem(main_menu, "Start Track", MENU_ITEM_SUBMENU, NULL, NULL);
    Menu_AddItem(main_menu, "Strat B_Control", MENU_ITEM_SUBMENU, NULL, NULL);
    Menu_AddItem(main_menu, "DEBUG", MENU_ITEM_REALTIME_PARAMS, NULL, NULL);
    Menu_AddItem(main_menu, "Test", MENU_ITEM_SUBMENU, test_menu, NULL);
    Menu_AddItem(main_menu, "RESET", MENU_ITEM_ACTION, NULL, NULL);
    
	//Test菜单
	Menu_AddItem(test_menu, "Motor Test",MENU_ITEM_ACTION,NULL,Test_Motor);
	
    current_menu = main_menu;
    need_refresh = 1;
    
    // 初始化实时显示状态
    realtime_state.current_page = 0;
    realtime_state.total_pages = 3;
    realtime_state.last_refresh = 0;
    in_realtime_mode = 0;
}

// 检查电池警告
void Menu_CheckBatteryWarning(void)
{
    // 检查是否需要显示低电量警告
    if (system_status_packet.low_battery_warning && !warning_mode) {
        warning_mode = 1;
        warning_start_time = Timer_GetSystemMillis();  // 使用您的计时器函数
        
        Emergency_Stop_Execute();
        
        need_refresh = 1;
    }
    
    // 检查是否可以退出警告模式（只有当电池电量恢复时）
    if (warning_mode) {
        if (!system_status_packet.low_battery_warning) {
            warning_mode = 0;
            need_refresh = 1;
        }
    }
}

// 显示低电量警告页面
static void Menu_DisplayWarningPage(void)
{
    OLED_Clear();
    // 显示警告标题
    OLED_ShowString(32, 8, "LOW POWER", OLED_8X16);
    
    // 显示警告信息
    OLED_ShowString(20, 24, "Battery Level Low!", OLED_6X8);
    OLED_ShowString(30, 32, "Please Charge!", OLED_6X8);
    
    // 显示当前电量
    char level_str[4];
    IntToStr(system_status_packet.battery_level, level_str);
        
    // 手动构建字符串
    OLED_ShowString(20, 40, "Current: ", OLED_6X8);
    OLED_ShowString(56, 40, level_str, OLED_6X8);
    OLED_ShowString(62, 40, "%", OLED_6X8);
    
	// 显示警告持续时间
	uint32_t warning_duration = Timer_GetSystemMillis() - warning_start_time;
	uint16_t seconds = warning_duration / 1000;
	char sec_str[4];

	IntToStr(seconds, sec_str);

    OLED_ShowString(0, 56, "Time sum: ", OLED_6X8);
	OLED_ShowString(60, 56, sec_str, OLED_6X8);
	OLED_ShowString(78, 56, "s", OLED_6X8);

	OLED_Update();
}

// 是否处于警告模式
uint8_t Menu_IsInWarningMode(void)
{
    return warning_mode;
}

// 处理菜单逻辑（在主循环中调用）
void Menu_Process(void)
{
    // 检查电池警告状态
    Menu_CheckBatteryWarning();
    
    // 如果处于警告模式，不处理任何按键
    if (warning_mode) {
        Menu_DisplayWarningPage();
        OLED_Update();
        return;
    }
    
    if (current_menu == NULL) return;
    
    // 检查是否处于实时参数显示模式
    if (in_realtime_mode) {
        // 实时模式下的按键处理
        if (Key_Check(MENU_KEY_BACK, KEY_DOWN)) {
            // 返回键退出实时模式
            in_realtime_mode = 0;
            need_refresh = 1;
            return;
        }
        else if (Key_Check(MENU_KEY_UP, KEY_DOWN)) {
            // 上键切换页面
            if (realtime_state.current_page > 0) {
                realtime_state.current_page--;
                need_refresh = 1;
            }
        }
        else if (Key_Check(MENU_KEY_DOWN, KEY_DOWN)) {
            // 下键切换页面
            if (realtime_state.current_page < realtime_state.total_pages - 1) {
                realtime_state.current_page++;
                need_refresh = 1;
            }
        }
        
        // 实时模式下的定时刷新（50ms周期）
        uint32_t current_time = Timer_GetSystemMillis();
        if (current_time - realtime_state.last_refresh >= 50) {
            need_refresh = 1;
            realtime_state.last_refresh = current_time;
        }
        
        if (need_refresh) {
            Menu_DisplayRealtimeParams();
            need_refresh = 0;
        }
        return;
    }
    
    // 原有的菜单处理逻辑
    // 检查按键事件
    if (Key_Check(MENU_KEY_UP, KEY_DOWN)) {
        // 上移
        if (current_menu->current_item->prev) {
            current_menu->current_item = current_menu->current_item->prev;
            need_refresh = 1;
        }
    }
    else if (Key_Check(MENU_KEY_DOWN, KEY_DOWN)) {
        // 下移
        if (current_menu->current_item->next) {
            current_menu->current_item = current_menu->current_item->next;
            need_refresh = 1;
        }
    }
    else if (Key_Check(MENU_KEY_ENTER, KEY_DOWN)) {
        // 确认键
        switch (current_menu->current_item->type) {
            case MENU_ITEM_SUBMENU:
                if (current_menu->current_item->submenu) {
                    current_menu->current_item->submenu->parent = current_menu;
                    current_menu = current_menu->current_item->submenu;
                    need_refresh = 1;
                }
                break;
                
            case MENU_ITEM_BACK:
                if (current_menu->parent) {
                    current_menu = current_menu->parent;
                    need_refresh = 1;
                }
                break;
                
            case MENU_ITEM_ACTION:
                if (current_menu->current_item->action) {
                    current_menu->current_item->action();
                    need_refresh = 1;  // 动作执行后可能需要刷新
                }
                break;
                
            case MENU_ITEM_REALTIME_PARAMS:
                in_realtime_mode = 1;
                realtime_state.current_page = 0;
                realtime_state.total_pages = 3;
                realtime_state.last_refresh = Timer_GetSystemMillis();
                need_refresh = 1;
                break;
                
            default:
                break;
        }
    }
    else if (Key_Check(MENU_KEY_BACK, KEY_DOWN)) {
        // 返回键
        if (current_menu->parent) {
            current_menu = current_menu->parent;
            need_refresh = 1;
        }
    }
    
    // 如果需要刷新显示
    if (need_refresh) {
        Menu_RefreshDisplay();
        need_refresh = 0;
    }
}

// 显示电池信息在右上角
void Menu_DisplayBatteryInfo(void)
{
	if (system_status_packet.battery_level <=20) OLED_ShowImage(100,0,30,16,VeryLow);
	else if (system_status_packet.battery_level <=40) OLED_ShowImage(100,0,30,16,Low);
	else if (system_status_packet.battery_level <=60) OLED_ShowImage(100,0,30,16,Mediem);
	else if (system_status_packet.battery_level <=80) OLED_ShowImage(100,0,30,16,High);
	else OLED_ShowImage(100,0,30,16,VeryHigh);
}

// 刷新显示
void Menu_RefreshDisplay(void)
{
    if (current_menu == NULL) return;
    
    // 如果处于警告模式，显示警告页面
    if (warning_mode) {
        Menu_DisplayWarningPage();
        return;
    }
    
    // 清屏
    OLED_Clear();
    
    // 显示菜单标题（第一行）
    OLED_ShowString(0, 0, current_menu->title, OLED_8X16);
    
    // 在右上角显示电池信息
    Menu_DisplayBatteryInfo();
    
    // ============ 添加字符分割线 ============
	// 在标题下方显示一行连续的横线字符
	OLED_ShowString(0, 16, "-----------------------------", OLED_6X8);
	// ============ 分割线结束 ============
    
    // 显示菜单项
    MenuItem *item = current_menu->items;
    uint8_t line = 3;  // 从第3行开始显示（因为标题占2行+分割线占1行）
    uint8_t displayed_count = 0;
    
    // 计算显示起始位置（实现滚动效果）
    uint8_t start_index = 0;
    MenuItem *temp = current_menu->items;
    uint8_t current_index = 0;
    
    // 找到当前选中项的位置
    while (temp && temp != current_menu->current_item) {
        temp = temp->next;
        current_index++;
    }
    
    // 计算起始显示位置
    if (current_index >= MENU_ITEMS_PER_PAGE) {
        start_index = current_index - MENU_ITEMS_PER_PAGE + 1;
    }
    
    // 跳转到起始显示位置
    item = current_menu->items;
    for (uint8_t i = 0; i < start_index && item; i++) {
        item = item->next;
    }
    
    // 显示菜单项
    while (item && displayed_count < MENU_ITEMS_PER_PAGE) {
        uint8_t y_pos = line * 8;  // 每行8像素
        
        // 判断是否为当前选中项
        if (item == current_menu->current_item) {
            OLED_ShowString(0, y_pos, ">", OLED_6X8);
            OLED_ShowString(6, y_pos, item->name, OLED_6X8);
            
            // 根据类型显示不同图标
            if (item->type == MENU_ITEM_SUBMENU) {
                OLED_ShowString(114, y_pos, "*", OLED_6X8);
            } else if (item->type == MENU_ITEM_ACTION) {
                OLED_ShowString(114, y_pos, "*", OLED_6X8);
            } else if (item->type == MENU_ITEM_BACK) {
                OLED_ShowString(114, y_pos, "*", OLED_6X8);
            }
        } else {
            OLED_ShowString(0, y_pos, " ", OLED_6X8);
            OLED_ShowString(6, y_pos, item->name, OLED_6X8);
        }
        
        item = item->next;
        line++;
        displayed_count++;
    }
    
    // 显示滚动指示器（如果有更多项）
    if (current_menu->item_count > MENU_ITEMS_PER_PAGE) {
        if (start_index > 0) {
            OLED_ShowString(114, 24, "^", OLED_6X8);  // 向上箭头
        }
        if (start_index + MENU_ITEMS_PER_PAGE < current_menu->item_count) {
            OLED_ShowString(114, 56, "v", OLED_6X8); // 向下箭头
        }
    }
    OLED_Update();
}

// 创建新菜单
Menu* Menu_Create(char *title)
{
    Menu *menu = (Menu*)malloc(sizeof(Menu));
    if (menu) {
        menu->title = title;
        menu->items = NULL;
        menu->current_item = NULL;
        menu->parent = NULL;
        menu->item_count = 0;
        menu->display_start = 0;
    }
    return menu;
}

// 添加菜单项
void Menu_AddItem(Menu *menu, char *name, MenuItemType type, 
                 Menu *submenu, void (*action)(void))
{
    if (menu == NULL) return;
    
    MenuItem *new_item = (MenuItem*)malloc(sizeof(MenuItem));
    if (new_item) {
        new_item->name = name;
        new_item->type = type;
        new_item->submenu = submenu;
        new_item->action = action;
        new_item->next = NULL;
        new_item->prev = NULL;
        
        // 添加到链表尾部
        if (menu->items == NULL) {
            menu->items = new_item;
            menu->current_item = new_item;  // 第一个项设为当前选中
        } else {
            MenuItem *last = menu->items;
            while (last->next) {
                last = last->next;
            }
            last->next = new_item;
            new_item->prev = last;
        }
        
        menu->item_count++;
    }
}

// 获取当前菜单
Menu* Menu_GetCurrentMenu(void)
{
    return current_menu;
}

// 获取当前菜单项
MenuItem* Menu_GetCurrentItem(void)
{
    return current_menu ? current_menu->current_item : NULL;
}

// 强制刷新菜单显示
void Menu_ForceRefresh(void)
{
    need_refresh = 1;
}
