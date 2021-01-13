#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/un.h>
#include <sys/resource.h>
#include <sys/msg.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include <netinet/in.h>
#include <termios.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <stdarg.h>
#include <syslog.h>
#include <endian.h>

#include <locale.h>

#include <iconv_hook/iconv.h>

//-----------------------------------------------------------------------
#define SET_TEST_MODE

#define _100ms 1
#define _200ms 2
#define _300ms 3
#define _400ms 4
#define _500ms 5
#define _600ms 6
#define _700ms 7
#define _800ms 8
#define _900ms 9
#define _1s _100ms * 10
#define _1s1 _1s + _100ms
#define _1s2 _1s + _200ms
#define _1s3 _1s + _300ms
#define _1s4 _1s + _400ms
#define _1s5 _1s + _500ms
#define _1s6 _1s + _600ms
#define _1s7 _1s + _700ms
#define _1s8 _1s + _800ms
#define _1s9 _1s + _900ms
#define _2s _1s * 2
#define _3s _1s * 3
#define _4s _1s * 4
#define _5s _1s * 5
#define _6s _1s * 6
#define _7s _1s * 7
#define _8s _1s * 8
#define _9s _1s * 9
#define _10s _1s * 10
#define _15s _1s * 15
#define _20s _1s * 20
#define _25s _1s * 25
#define _30s _1s * 30

#ifdef SET_TEST_MODE
    #define wait_ack_min_sec   _10s
#else
    #define wait_ack_min_sec   _2s
#endif
#define wait_ack_max_sec   _10s
#define wait_cmd_sec       _2s

//-----------------------------------------------------------------------

#define STX  0x02
#define ETX  0x03
#define EOT  0x04
#define ENQ  0x05
#define ACK  0x06
#define DLE  0x10
#define NAK  0x15
#define ESC  0x1B

#define buf_size         2048
#define sdef              512
#define TIME_STR_LEN       64
#define max_try             5
#define max_print_line     57
#define max_spd             6
#define max_len_from_dev 1024
#define tmp_size         1024
#define cp_size            32

#define total_inCMD        60//59//58

//-----------------------------------------------------------------------

#ifndef uint8_t
typedef unsigned char uint8_t;
#endif

#ifndef uint32_t
typedef unsigned int uint32_t;
#endif

#ifdef LOG_DEBUG
#undef LOG_DEBUG
#endif


typedef enum {
    RET_NONE_ERROR = -2,
    RET_MAJOR_ERROR = -1,
    RET_OK,//0
    RET_MINOR_ERROR,//1
    RET_TIMEOUT,//2
    RET_NAK,//3
    RET_ENQ,//4
    RET_EOT//5
} res_t;

typedef enum {
    LOG_OFF,//0
    LOG_ON,//1
    LOG_DEBUG,//2
    LOG_DUMP//3
} log_t;

typedef enum {
    CMD_REQUEST_STATUS,     //0//"request_status",//Запрос состояния ККТ
    CMD_LINE_PRINT,         //1//"line_print",//Печать текстовых документов
    CMD_END_OF_PRINT,       //2//"end_of_print",//Команда отрезать чек
    CMD_BEEP,               //3//"beep",//Звуковой сигнал
    CMD_RESTART_DEVICE,     //4//"restart_device",//Рестарт ККТ
    CMD_SHIFT_OPEN,         //5//"shift_open",//Открыть смену
    CMD_CLISHE_PRINT,       //6//"clishe_print",//Печать клише чека//В клише возможна печать картинок и штрихкодов
    CMD_BUTTOM_PRINT,       //7//"buttom_print",//Печать нижней части чека
    CMD_SH_CODE_PRINT,      //8//"sh_code_print",//Печать штрихкода по номеру
    CMD_REQUEST_CODE_STATUS,//9//"request_code_status",//Запрос кода состояния ККТ
    CMD_SHIFT_CLOSE,        //10//"shift_close",//Закрыть смену//Команда выводит на печать отчет о закрытии смены.
    CMD_DEMO_PRINT,         //11//"demo_print",//Демонстрационная печать
    CMD_GET_DEV_TYPE,       //12//"get_dev_type",//Получить тип устройства
    CMD_GET_PRINTER_ERROR,  //13//"get_printer_error",//Получить последний код ошибки //актуальна только в режиме 7.15
    CMD_GET_BAR_PIC,        //14//"get_bar_pic",//Состояние массива штрихкодов и картинок // Barcode or Picture
    CMD_GET_VERSION,        //15//"get_version",//Получение версии
    CMD_GET_STAT_EXCH,      //16//"get_stat_exch"//Получить статус информационного обмена с ФН
    CMD_OPEN_CHEK,          //17//"open_chek"//Открыть чек
    CMD_CANCEL_CHEK,        //18//"cancel_chek"// Аннулирование всего чека
    CMD_COMING_MONEY,       //19//"coming_money"//Внесение денег
    CMD_PAYOUT_MONEY,       //20//"payout_money"//Выплата денег
    CMD_CLOSE_CHEK,         //21//"close_chek"}//Закрыть чек (со сдачей)
    CMD_REG_ITEM,           //22//"reg_item"//Регистрация позиции
    CMD_ENTER_MODE,         //23//"enter_mode"//Вход в режим
    CMD_EXIT_MODE,          //24//"exit_mode"//Выход из текущего режима
    CMD_CASH_REQUEST,       //25//"cash_request"//Запрос наличных
    CMD_SHIFT_MONEY,        //26//"get_shift_money"//Получение последнего сменного итога
    CMD_INIT_TABLES,        //27//"init_tables"//Инициализация таблиц начальными значениями
    CMD_READ_TABLES,        //28//"read_tables"//Чтение таблицы//Команда: <46h><Таблица(1)><Ряд(2)><Поле(1)>
    CMD_PIC_PRINT,          //29//"pic_print"//Печать картинки по номеру//Команда:<8Dh><Принтер(1)><Номер(1)><Смещение(2)>
    CMD_SHIFT_REQ_PARAM,    //30//"shift_req_param"//Запрос параметров текущей смены//Команда: <A4h><10h>
    CMD_GET_STAT_FN,        //31//"get_stat_FN"//Запрос статуса ФН//Команда: <A4h><30h>
    CMD_GET_NUM_FN,         //32//"get_num_FN"//Запрос номера ФН//Команда: <A4h><31h>//Ответ: <55h><Код Ошибки (1)><Номер ФН(16)>
    CMD_GET_BLACK_DAY,      //33//"get_black_day" - Запрос срока действия ФН //Команда: <A4h><32h>
    CMD_GET_VER_FN,         //34//"get_ver_FN" - Запрос версии ФН//Команда: <A4h><33h>
    CMD_GET_ERR_FN,         //35//"get_err_FN" - апрос последних ошибок ФН//Команда: <A4h><35h><Номер блока для чтения(1)>
    CMD_GET_NOACK_FD,       //36//"get_noack_FD" - Запрос количества ФД, на которые нет квитанции (стр.90)//Команда: <A4h><42h>
    CMD_GET_TEXT_ATTR,      //37//"get_text_attr" - Чтение реквизита//Команда:<E9h><Номер реквизита(2)><Номер блока(1)>
    CMD_SET_TEXT_ATTR,      //38//"set_text attr" - Запись реквизита//Команда:<E8h><Флаги(1)><Количество блоков(1)><Номер блока(1)><Данные реквизита(X)>
    CMD_SET_DISCOUNT,       //39//"set_discount" - Скидка//Команда:<43h><Флаги(1)><Область(1)><Тип(1)><Знак(1)><Размер(5)>//Ответ:<55h><Код ошибки(1)><Расширенный код ошибки(1)>
    CMD_REG_TAX_CHEK,       //40//"reg_tax_chek" - Регистрация налога на весь чек//Команда:<B8h><Флаги(1)><Область(1)><Тип(1)><Сумма(7)>//Ответ:<55h><Код ошибки(1)><Расширенный код ошибки(1)>
    CMD_CALC_BY_CHEK,       //41//"calc_by_chek" - Расчет по чеку//Команда:<99h><Флаги(1)><Форма расчета(1)><Сумма(5)>//Ответ:<55h><Код Ошибки(1)><Остаток(5)><Сдача(5)>
    CMD_STORNO_CALC_BY_CHEK,//42//"storno_calc_by_chek" - Сторно расчета по чеку//Команда:<9Bh><Флаги(1)><Форма расчета(1)><Сумма(5)>//Ответ:<55h><Код Ошибки (1)><Остаток(5)><Сдача(5)>
    CMD_BEGIN_GET_REPORT,   //43//"begin_get_report" - Начало снятия отчета//Команда:<67h><Тип Отчета(1)>//Ответ:<55h><Код Ошибки(1)><Расширенный код ошибки(1)>
    CMD_GENERAL_CANCEL,     //44//"general_cancel" - Общее гашение//Команда:<77h>//Ответ:<55h><Код Ошибки(1)><Расширенный код ошибки(1)>
    CMD_TEH_CLEAR,          //45//"teh_clear" - Технологическое обнуление ККТ//Команда:<6Bh>//Ответ:<55h><Код Ошибки(1)><Расширенный код ошибки(1)>
    CMD_SET_PROT_CODE,      //46//"set_prot_code" - Ввод кода защиты ККТ//Команда:<6Dh><Номер(1)><Код(Х)>//Ответ:<55h><Код Ошибки(1)><(0)>
    CMD_STAT_PROT_CODE,     //47//"stat_prot_code" - Запрос активизированности кода защиты ККТ//Команда:<74h><Номер(1)>//Ответ:<4Ch><Активизирован(1)>
    CMD_READ_REGISTER,      //48//"read_register"  - Считать регистр//Команда:<91h><Регистр(1)><Параметр1(1)><Параметр2(1)>//Ответ:<55h><Код Ошибки(1)><Значение(Х)>
    CMD_REPRINT_LAST_DOC,   //49//"reprint_last_doc"//Повторная печать последнего документа//Команда:<95h>//Ответ:<55h><Код Ошибки(1)><Расширенный код ошибки(1)>
    CMD_REPRINT_REPORT,     //50//"reprint_report"//Допечатать отчет//Команда:<EEh>//Ответ:<55h><Код ошибки(1)><Расширенный код ошибки(1)>
    CMD_CLEAR_BUFFER,       //51//"clear_buffer"//Очистить буфер последнего документа//Команда:<97h>//Ответ:<55h><Код Ошибки(1)><Расширенный код ошибки(1)>
    CMD_ACTIVATION_FN,      //52//"activation_FN"//Активизация ФН//Команда:<A6h>//Ответ:<55h><Код Ошибки(0)><Расширенный код ошибки(1)>
    CMD_CLOSE_FN,           //53//"close_FN"//Закрытие архива ФН//Команда:<A7h>//Ответ:<55h><Код Ошибки(1)><Расширенный код ошибки(1)>
    CMD_PRINT_REP_REG,      //54//"print_rep_reg"//Печать итогов регистрации/перерегистрации ККТ//Команда:<A8h>//Ответ:<55h><Код Ошибки(1)><Расширенный код ошибки(1)>
    CMD_PRINT_DOC_BY_NUM,   //55//"print_doc_by_num"//Печать документа по номеру//Команда:<ABh><Номер документа(5)>//Ответ:<55h><Код Ошибки(1)><Расширенный код ошибки(1)>
    CMD_SET_DATE_TIME,      //56//"set_date_time"//Программирование даты и времени//Команда:<EDh><День(1)><Месяц(1)><Год(1)><Час(1)><Минута(1)><Секунда(1)>//Ответ:<55h><Код ошибки(1)><Расширенный код ошибки(1)>

    CMD_SET_DATE,           //57//"set_date"//Программирование даты//Команда:<64h><День(1)><Месяц(1)><Год(1)>//Ответ:<55h><Код Ошибки(1)><Расширенный код ошибки(1)>
    CMD_SET_TIME,           //58//"set_time"//57//Программирование времени//Команда:<4Bh><Час(1)><Минута(1)><Секунда(1)>//Ответ:<55h><Код Ошибки(1)><Расширенный код ошибки(1)>


    CMD_WRITE_TABLES        //59//"write_tables"//Программирование таблицы //<50h><Таблица(1)><Ряд(2)><Поле(1)><Значение (Х)>
} cmd_t;



#pragma pack(push,1)
typedef struct {
    uint16_t tag;
    int8_t type;
    char name[56 << 1];
} s_tag_t;
#pragma pack(pop)


#pragma pack(push,1)
typedef struct {
    uint8_t byte[8];
} s_bit64_t;
#pragma pack(pop)


#pragma pack(push,1)
typedef struct {
    uint8_t flags;//bit0: 0–выполнить операцию, 1–режим проверки операции
                  //bit1: 0–проверять денежную наличность, 1–не проверять
    char name[64];//наименование товара
    uint8_t price[6];//цена
    uint8_t total[5];//количество
    uint8_t type;//тип 0–процентная, тип 1–суммовая
    uint8_t sign;//знак 0–скидка, знак 1–надбавка.
    uint8_t size[6];//размер
    uint8_t tax;//налог
    uint8_t sect;//секция : 00 .. 30
    uint8_t str[16];//Строка 16 символов
    uint8_t reserve;//=0
} s_cmd_reg_item;
#pragma pack(pop)


#pragma pack(push,1)
typedef struct {
    uint8_t cmd;
    uint8_t wait;
    char name[30];
} s_cmd_elem;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct {
    unsigned fis:1;//0-й – ККТ фискализирована (0 – нет, 1 – да);
    unsigned shift_open:1;//1-й – смена открыта (0 – нет, 1 – да);
    unsigned paper_yes:1;//2-й – наличие бумаги в презенторе (0 – нет, 1 – есть);
    unsigned paper_sensor:1;//3-й – датчик ЧЛ (0 – нет бумаги, 1 – есть бумага);
    unsigned none:1;//4-й – бит не используется;
    unsigned cover_sensor:1;//5-й – состояние датчика крышки (0 – крышка закрыта, 1 – крышка открыта);
    unsigned activ:1;//6-й – состояние ФН: 0 – не активизирован, 1 – активизирован);
    unsigned bat:1;//7-й – наличие батарейки в ККТ: 0 – установлена, 1 – отсутствует.
} s_ack_stat_flags;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct {
//    uint16_t pass;
    uint8_t ack;//0x44
    uint8_t cassir;//Кассир  00..30, формат BCD
    uint8_t num_in_holl;//Номер_в_зале 01..255, формат BIN
    uint8_t dataYMD[3];//Дата_YMD  Показания внутренних часов: 00-99, 01-12, 01-31 (00-2000, ..., 89-2089, 90-99 - запрещенные значения)
    uint8_t timeHMS[3];//Время_HMS 00..23, 00..59, 00..59 – показания внутренних часов ККТ.
    uint8_t flags;
    uint32_t manNUM;//Зав. Номер ККТ позволяют вводить восьмиразрядный заводской номер (00000000-00999999), но значащими считаются младшие шесть разрядов
    uint8_t model;//Модель 
    uint16_t none;//3.0
    uint8_t mode;//Режим работы Двоичное число (0x00-0xFF). Младшая тетрада–режим, старшая–подрежим (формат «Подрежим.Режим»).
    uint16_t checkNUM;//Номер чека 0000-9999 (нумерация сквозная в рамках одной смены), формат BCD. Содержит «номер последнего закрытого чека+1».
    uint16_t shiftNUM;//Номер смены 0000 .. 9999 (нумерация сквозная в рамках одной смены) - номер последней закрытой смены, а не текущей.
    uint8_t checkSTAT;//Состояние чека
    uint8_t itogo[5];//Сумма чека Сумма текущего чека 000000000 .. 4294967295 мде.
    uint8_t coma;//Десятичная точка 0..3 – положение десятичной точки во всех денежных величинах (кол-во разрядов справа от десятичной точки)
    uint8_t port;//Данный параметр обозначает тип интерфейса, по которому работает ККТ, и принимает значения:
} s_ack_stat;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct {
    uint8_t ack;//0x55
    uint8_t err;
    uint8_t ext_err;
} s_ack_errs;
#pragma pack(pop)


#pragma pack(push,1)
typedef struct {
    unsigned paper:1;//0-й бит: наличие бумаги: 0 – бумага есть, 1 – нет бумаги в принтере чеков;
    unsigned prn_link:1;//1-й бит: связь с принтером: 0 – установлена, 1 – нет связи с принтером;
    unsigned prn_hard_err:1;//2-й бит: механическая ошибка принтера: 0 – нет ошибок, 1 – механическая или невосстановимая ошибка принтера;
    unsigned cat_err:1;//3-й бит: ошибка отрезчика: 0 – нет ошибок, 1 – ошибка отрезчика (для ККТ ошибка отрезчика возможна в случае отключения отрезчика);
    unsigned prn_temp_err:1;//4-й бит: ошибка принтера (перегрев): 0 – нет ошибок принтера, 1 – восстановимая ошибка принтера (перегрев);
    unsigned paper_err:1;//5-й бит: ошибка бумаги: 0 – нет ошибок, 1 – замятие бумаги;
    unsigned pres_err:1;//6-й бит: ошибка презентора (только для принтера PPU-700): 0 – нет ошибки, 1 – ошибка презентора (либо в нем осталась бумага);
    unsigned paper_out:1;//7-й бит: ошибка окончания бумаги: 0 – нет ошибки, 1 – бумага заканчивается (сработал датчик наличия бумаги).
} s_ack_code_stat_flags;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct {
    uint8_t ack;//0x55
    uint8_t mode;
    uint8_t err_flags;
} s_ack_code_stat;
#pragma pack(pop)


#pragma pack(push,1)
typedef struct {
    uint8_t err;//<Код Ошибки(1)>
    uint8_t prot_ver;//<Версия протокола(1)>
    uint8_t type;//<Тип(1)>
    uint8_t model;//<Модель(1)>
    uint8_t mode[2];//<Режим (2)>
    uint8_t dev_ver[5];//<Версия устройства (5)>
//    uint8_t name[];<Название (N)>
} s_ack_dev_type;
#pragma pack(pop)


#pragma pack(push,1)
typedef struct {
    uint8_t ack;//0x55
    uint8_t err;//<Код Ошибки(1)>
    uint32_t freeBytes;//<Свободно (4)>
    uint8_t lastIndex;//<Последний (1)>
} s_ack_bar_pic;
#pragma pack(pop)


#pragma pack(push,1)
typedef struct {
    uint8_t ack;//0x55
    uint8_t err;//<Код Ошибки(1)>
    uint8_t ver;//<Версия(1)>
    uint8_t sub_ver;//<ПодВерсия (1)>
    uint8_t lang;//<КодЯзыка(1)>
    uint8_t build[2];//<Сборка(2)>
} s_ack_version;
#pragma pack(pop)


#pragma pack(push,1)
typedef struct {
    uint8_t ack;//0x55
    uint8_t err;//Код Ошибки(1)
    uint8_t stat;//Статус информационного обмена(1)
    uint8_t rd_stat;//Состояние чтения сообщения для ОФД(1)
    uint16_t totalMsg;//Количество сообщений для передачи в ОФД(2)
    uint32_t first;//Номер документа для ОФД, первого в очереди(4)
    uint8_t datetime[5];//Дата-время первого в очереди документа для ОФД
} s_ack_req_stat_exch;
#pragma pack(pop)


#pragma pack(push,1)
typedef struct {
    uint8_t ack;//0x4D
    uint8_t money[7];//Сумма(7) – сумма наличных денег в денежном ящике ККТ, число формата BCD в пределах 00000000000000.. 99999999999999 мде.
} s_ack_cash_request;
#pragma pack(pop)


#pragma pack(push,1)
typedef struct {
    uint8_t ack;//0x55
    uint8_t err;//Код Ошибки(1)
    uint8_t in_money[7];//Сменный Итог Приходов (7)
    uint8_t out_money[7];//Сменный Итог Расходов (7)
} s_ack_get_shift_money;
#pragma pack(pop)


#pragma pack(push,1)
typedef struct {
    uint8_t ack;//0x55
    uint8_t err;//Код Ошибки(1)
    uint8_t sstat;//Состояние смены
    uint16_t snum;//Номер смены
    uint16_t cnum;//Номер чека
} s_ack_shift_req_param;
#pragma pack(pop)


#pragma pack(push,1)
typedef struct {
    uint8_t ack;//0x55
    uint8_t err;//Код Ошибки(1)
    uint8_t stage_stat;//Состояние фазы жизни
    uint8_t cur_doc;//Текущий документ
    uint8_t data_doc;//Данные документа
    uint8_t shift_stat;//Состояние смены
    uint8_t flags;//Флаги предупреждения
    uint8_t datetime[5];//Дата и время
    char numFN[16];//Номер ФН1
    uint32_t numLastFD;//Номер последнего ФД2
} s_ack_get_stat_FN;
#pragma pack(pop)


#pragma pack(push,1)
typedef struct {
    uint8_t ack;//0x55
    uint8_t err;//Код Ошибки(1)
    char numFN[16];//Номер ФН1
} s_ack_get_num_FN;
#pragma pack(pop)


#pragma pack(push,1)
typedef struct {
    uint8_t ack;//0x55
    uint8_t err;//Код Ошибки(1)
    uint8_t validity[3];//Срок действия ФН
    uint8_t left_reg;//Осталось перерегистраций
    uint8_t done_reg;//Сделано регистраций/перерегистраций
} s_ack_get_black_day;
#pragma pack(pop)


#pragma pack(push,1)
typedef struct {
    uint8_t ack;//0x55
    uint8_t err;//Код Ошибки(1)
    char verFN[16];//Версия ФН
    uint8_t typeSW;//Тип ПО ФН
} s_ack_get_ver_FN;
#pragma pack(pop)


#pragma pack(push,1)
typedef struct {
    uint8_t ack;//0x55
    uint8_t err;//Код Ошибки(1)
    uint8_t blks;//Количество блоков
} s_ack_get_err_FN;
#pragma pack(pop)


#pragma pack(push,1)
typedef struct {
    uint8_t ack;//0x55
    uint8_t err;//Код Ошибки(1)
    uint16_t noackFD;//Количество неподтверждённых ФД
} s_ack_get_noack_FD;
#pragma pack(pop)


#pragma pack(push,1)
typedef struct {
    uint8_t ack;//0x55
    uint8_t err;
} s_ack_set_text_attr;
#pragma pack(pop)


#pragma pack(push,1)
typedef struct {
    uint8_t ack;//0x55
    uint8_t err;//Код Ошибки(1)
    uint8_t nblk;//Количество блоков
//    uint8_t data[X];//Данные реквизита
} s_ack_get_text_attr;
#pragma pack(pop)


#pragma pack(push,1)
typedef struct {
    uint16_t tag;
    uint16_t len;
} s_tlv;
#pragma pack(pop)


#pragma pack(push,1)
typedef struct {
    uint8_t ack;//0x55
    uint8_t err;//Код Ошибки(1)
    uint8_t balance[5];//Остаток(5)
    uint8_t money[5];//Сдача(5)
} s_ack_calc_by_chek;
#pragma pack(pop)


//-----------------------------------------------------------------------

extern const char *vers;
extern const char *eol;

extern uint8_t dbg;

extern int fd;
extern int fd_log;

extern uint8_t QuitAll;
extern uint8_t SIGTERMs;
extern uint8_t SIGINTs;
extern uint8_t SIGKILLs;
extern uint8_t SIGSEGVs;
extern uint8_t SIGABRTs;

extern const char *path_log;
extern const char *file_log;

extern char device[sdef];

extern int max_size_log;
extern int MaxLogLevel;

extern uint32_t SPEED;
extern uint16_t passwd;

extern const int max_param;
extern const char *name_param[];

extern const uint32_t ispd[];

extern char codePage[cp_size];

//----------------------------------------------------------------------

extern uint32_t get_timer_ms(uint32_t tm);
extern int check_delay_ms(uint32_t tm);

extern char *ThisTime();
extern uint8_t LRC(uint8_t *buf, int len);

extern char *TimeNowPrn(char *ts);
extern void ToSysLogMsg(int LogLevel, const char * const Msg);
extern uint8_t get_ack_wait(uint8_t cmd_idx);
extern void print_msg(uint8_t dt, const char *fmt, ...);
extern uint32_t get100ms();
extern void GetSignal_(int sig);

extern uint8_t findSPEED(char *param);
extern int parse_inCMD(char *in_cmd);
extern void setSPEED(uint8_t SpeedIndex);
extern int makeCMD(uint8_t *buf, int idx, char *arg, uint8_t *cb);
extern uint16_t getPASSWD(char *pass);
extern int parse_ANSWER(uint8_t *buf, int len, int idx);

//----------------------------------------------------------------------
