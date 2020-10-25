#include "functions.h"

//const char *vers = "0.1";//06.10.2019
//const char *vers = "0.2";//07.10.2019
//const char *vers = "0.3";//07.10.2019//Major changes : init timer with 100 ms period (use SIGALRM)
//const char *vers = "0.3.1";//07.10.2019//Minor changes : add all commands (with password)
//const char *vers = "0.4";//08.10.2019//Minor changes : add parser for device's answer
//const char *vers = "0.5";//09.10.2019//Minor changes : add new commands
//const char *vers = "0.5.1";//09.10.2019//Minor changes : add new commands+
//const char *vers = "0.6";//12.10.2019//Minor changes : edit sequence for data exchange
//const char *vers = "0.7";//13.10.2019//Minor changes : edit old commands and add new command
//const char *vers = "0.8";//15.10.2019//Minor changes in logging part
//const char *vers = "0.8.1";//15.10.2019//Minor changes : add parse error code to string value
//const char *vers = "0.9";//16.10.2019//Minor changes : add new param for text decoding
//                         // for example, --codePage=CP1251 (default CP866)
//                         // add new command
//const char *vers = "1.0";//18.10.2019//Major changes : escaping bytes DLE, ETX
//const char *vers = "1.1";//18.11.2019//Major changes : add news+
//const char *vers = "1.2";//19.11.2019//Major changes : add new commands
//const char *vers = "1.2.1";//19.11.2019//Minor changes : add new commands
//const char *vers = "1.3";//20.11.2019//Minor changes : add bintobcd convertor in make commands
//const char *vers = "1.3.1";//20.11.2019//Minor changes : convert password to bcd and add new commands
//const char *vers = "1.3.2";//21.11.2019//Minor changes : add new commands++
//const char *vers = "1.3.3";//21.11.2019//Minor changes : add one command (request from FN)
//const char *vers = "1.3.4";//21.11.2019//Minor changes : add new commands for FN
//const char *vers = "1.3.5";//22.11.2019//Minor changes : add new commands for FN
//const char *vers = "1.3.6";//22.11.2019//Minor changes : ++ new commands for FN
//const char *vers = "1.3.7";//23.11.2019//Minor changes : edit "get_text_attr" command - add tag's name
//const char *vers = "1.3.8";//23.11.2019//Minor changes : add "set_text_attr" command for string tags only
//const char *vers = "1.3.9";//25.11.2019//Minor changes : add new device status parser and new commands
//const char *vers = "1.4.0";//25.11.2019//Minor changes : add new commands (continue)
//const char *vers = "1.4.1";//25.11.2019//Minor changes : add new commands (continue+)
//const char *vers = "1.4.2";//25.11.2019//Minor changes : add new commands (continue++)
//const char *vers = "1.4.3";//26.11.2019//Minor changes : add new commands (continue+++)
//const char *vers = "1.4.4";//26.11.2019//Minor changes : add new commands (continue++++)
//const char *vers = "1.4.5";//26.11.2019//Minor changes : add new commands (continue+++++)
//const char *vers = "1.5";//27.11.2019//Major changes : fixed device bug: no DLE before ETX !
//const char *vers = "1.5.1";//28.11.2019//Minor changes +news
const char *vers = "1.6";//28.11.2019//Major changes in "enter_mode" command



static uint8_t tbl, pole;
static uint16_t column;

const char sep = '^';
const char *eol = "\r\n";

const char *codeCP1251 = "CP1251";
const char *codeCP866 = "CP866";
const char *codeUTF8   = "UTF-8";

char codePage[cp_size] = {0};

uint8_t dbg = LOG_OFF;

int fd = -1;
int fd_log = -1;

uint8_t QuitAll  = 0;
uint8_t SIGTERMs = 1;
uint8_t SIGINTs  = 1;
uint8_t SIGKILLs = 1;
uint8_t SIGSEGVs = 1;
uint8_t SIGABRTs = 1;

static uint32_t varta = 0;

const char *path_log = "";
const char *file_log = "custom_log.txt";

char device[sdef] = {0};

int max_size_log = 1024 * 32000;
int MaxLogLevel = LOG_DEBUG + 1;

uint32_t SPEED = 0;
uint16_t passwd = 0;

const int max_tag = 68;
static const s_tag_t tags[] = {//0-char, 1-uint8_t, 2-uint16_t, 3-uint32_t, 4-uint8_t[], 5-STLV(bytes), 6-VLN(bytes)
    {1001, 1, "Автоматический режим"},//Автоматический режим//Целое//byte//1
    {1002, 1, "Автономный режим"},//Автономный режим//Целое//byte//1
    {1005, 0, "Адрес оператора по переводу денежных средств"},//СтрокаASCII//256
    {1008, 0, "Номер телефона / эл.адрес покупателя"},//Строка//64
    {1009, 0, "Адрес (место) расчетов"},//Адрес (место) расчетов//СтрокаASCII//256
    {1010, 4, "Размер вознаграждения банковского агента"},//Целое//8//VLN
    {1012, 3, "Дата, время"},//Дата, время//ЦелоеUnixTime//4
    {1016, 0, "ИНН оператора по переводу денежных средств"},//Строка//12
    {1017, 0, "ИНН ОФД"},//ИНН ОФД//СтрокаASCII//12
    {1018, 0, "ИНН пользователя"},//ИНН пользователя//СтрокаASCII//12
    {1021, 0, "Кассир"},//Строка//64
    {1026, 0, "Наименование оператора по переводу денежных средств"},//Строка//64
    {1036, 0, "Номер автомата"},//Номер автомата//СтрокаASCII//12
    {1037, 0, "Регистрационный номер ККТ"},//Регистрационный номер ККТ//СтрокаASCII//20
    {1044, 0, "Операция банковского агента"},//Строка//24
    {1046, 0, "Наименование ОФД"},//Наименование ОФД//СтрокаASCII//256
    {1048, 0, "Наименование пользователя"},//Наименование пользователя//СтрокаASCII//256
    {1055, 1, "Применяемая система налогообложения"},//Целое//1//byte
    {1056, 1, "Признак шифрования"},//Признак шифрования//Целое//byte/1
    {1057, 1, "Признак платежного агента"},//Признак платежного агента//Целое//byte//1
    {1060, 0, "Адрес сайта ФНС"},//Строка//256
    {1062, 1, "Системы налогообложения"},//Системы налогообложения//Целое//byte//1
    {1073, 0, "Телефон платежного агента"},//Строка//19
    {1074, 0, "Телефон оператора по приему платежей"},//Строка//19//ASCII
    {1075, 0, "Телефона оператора по переводу денежных средств"},//Строка//19
    {1084, 5, "Дополнительный реквизит пользователя"},//Структура//328//STLV
    {1101, 1, "Код причины перерегистрации"},//Код причины перерегистрации//Целое//byte//1
    {1108, 1, "Признак расчетов в Интернете"},//Признак расчетов в Интернете//Целое//byte//1
    {1109, 1, "Признак услуги"},//Признак услуги//Целое//byte//1
    {1117, 0, "Адрес электронной почты отправителя чека"},//Строка//64
    {1126, 1, "Признак проведения лотереи"},//Признак проведения лотереи//Целое//byte//1
    {1162, 5, "Код товара"},//Структура//30//STLV
    {1171, 1, "Телефон поставщика"},//Целое//1//byte
    {1173, 1, "Тип коррекции"},//Целое//1//byte
    {1174, 5, "Основание для коррекции"},//Структура//292//STLV
    {1177, 0, "Наименование основания для коррекции"},//256//ASCII
    {1178, 3, "Дата документа основания для коррекции"},//4//unixtime
    {1179, 0, "Номер документа основания для коррекции"},//32//ASCII
    {1187, 0, "Место расчетов"},//Место расчетов//СтрокаASCII//256
    {1191, 0, "Дополнительный реквизит товара"},//Строка//64//ASCII
    {1192, 0, "Дополнительный реквизит чека"},//Строка//16
    {1193, 1, "Признак проведения азартных игр"},//Признак проведения азартных игр//Целое//byte//1
    {1197, 0, "Единица измерения товара"},//Строка//16//SCII
    {1203, 0, "ИНН кассира"},//Строка//12
    {1205, 4, "Код причины изменения сведения о ККТ"},//Код причины изменения сведения о ККТ//Целое//byte//4
    {1207, 1, "Признак торговли подакцизными товарами"},//Признак торговли подакцизными товарами//Целое//byte//1
    {1209, 1, "Номер версии ФФД"},//Номер версии ФФД//Целое//byte//1
    {1221, 1, "Признак установки принтера в автомате"},//Признак установки принтера в автомате//byte//1
    {1222, 1, "Признак агента по предмету расчета"},//Флаги//1//BIN
    {1223, 5, "Данные агента"},//Структура//512//STLV
    {1224, 5, "Данные поставщика"},//Структура//512//STLV
    {1225, 0, "Наименование поставщика"},//Строка//256//ASCII
    {1226, 0, "ИНН поставщика"},//Строка//256//Строка
    {1227, 0, "Покупатель (клиент)"},//Строка//256
    {1228, 0, "ИНН покупателя (клиента)"},//Целое//12//ASCII
    {1229, 6, "Акциз"},//Целое//6//VLN
    {1230, 0, "Код страны"},//Строка//3//ASCII
    {1231, 0, "Декларация"},//Строка//32//ASCII
    {15000,0, "Пользовательский реквизит 1"},//Строка//64//ASCII
    {15001,0, "Пользовательский реквизит 2"},//Строка//64//ASCII
    {15002,0, "Пользовательский реквизит 3"},//Строка//64//ASCII
    {15003,0, "Пользовательский реквизит 4"},//Строка//64//ASCII
    {15004,0, "Пользовательский реквизит 5"},//Строка//64//ASCII
    {15010,0, "Пользовательский буфер 1"},//Строка//64//ASCII
    {15011,0, "Пользовательский буфер 2"},//Строка//64//ASCII
    {15012,0, "Пользовательский буфер 3"},//Строка//64//ASCII
    {15013,0, "Пользовательский буфер 4"},//Строка//64//ASCII
    {15014,0, "Пользовательский буфер 5"}//Строка//64//ASCII
};


static const s_cmd_elem inCMD[total_inCMD] = {
    {0x3F, wait_ack_max_sec, "request_status"},//0//Запрос состояния ККТ
    {0x4C, wait_ack_max_sec, "line_print"},//1//Печать текстовых документов
    {0x75, wait_ack_max_sec, "end_of_print"},//2//Команда отрезать чек
    {0x47, wait_ack_max_sec, "beep"},//3//Звуковой сигнал
    {0xCE, wait_ack_max_sec, "restart_device"},//4//Рестарт ККТ
    {0x9A, wait_ack_max_sec, "shift_open"},//5//Открыть смену
    {0x6C, wait_ack_max_sec, "clishe_print"},//6//Печать клише чека//В клише возможна печать картинок и штрихкодов
    {0x73, wait_ack_max_sec, "buttom_print"},//7//Печать нижней части чека
    {0x7C, wait_ack_max_sec, "sh_code_print"},//8//Печать штрихкода по номеру
    {0x45, wait_ack_max_sec, "request_code_status"},//9//Запрос кода состояния ККТ
    {0x5A, 40,               "shift_close"},//10//Закрыть смену//Команда выводит на печать отчет о закрытии смены.
    {0x82, wait_ack_max_sec, "demo_print"},//11/Демонстрационная печать
    {0xA5, wait_ack_max_sec, "get_dev_type"},//12//Получить тип устройства
    {0xB3, wait_ack_max_sec, "get_printer_error"},//13//Получить последний код ошибки //актуальна только в режиме 7.150x
    {0x7D, wait_ack_max_sec, "get_bar_pic"},//14//Состояние массива штрихкодов и картинок // Barcode or Picture
    {0x9D, wait_ack_max_sec, "get_version"},//15//Получение версии
    {0xA4, wait_ack_max_sec, "get_stat_exch"},//16//Получить статус информационного обмена с ФН
    {0x92, wait_ack_max_sec, "open_chek"},//17//Открыть чек
    {0x59, wait_ack_max_sec, "cancel_chek"},//18//Аннулирование всего чека
    {0x49, wait_ack_max_sec, "coming_money"},//19//Внесение денег// <49h><Флаги(1)><Сумма(5)>
    {0x4F, wait_ack_max_sec, "payout_money"},//20//Выплата денег// <4Fh><Флаги(1)><Сумма(5)>
    {0x4A, 20,               "close_chek"},//21//Закрыть чек (со сдачей) <4Ah><Флаги(1)><Форма расчета(1)><Внесенная сумма(5)>
    {0xE6, 50,               "reg_item"},//22/Регистрация позиции <E6h><Флаги (1)><Наименование товара (64)><Цена (6)><Количество (5)>
                                        // <Тип (1)><Знак (1)><Размер(6)><Налог (1)><Секция (1)><ШК (16)> <Резерв (1)>
    {0x56, wait_ack_max_sec, "enter_mode"},//23//Вход в режим//<56h> <Режим(1)><Пароль(4)>
    {0x48, wait_ack_max_sec, "exit_mode"},//24//Выход из текущего режима//<48h>
    {0x4D, wait_ack_max_sec, "cash_request"},//25//Запрос наличных//Команда: <4Dh>
    {0x58, wait_ack_max_sec, "get_shift_money"},//26//Получение последнего сменного итога//Команда: <58h>
    {0x71, wait_ack_max_sec, "init_tables"},//27//Инициализация таблиц начальными значениями//Команда: <71h>
    {0x46, wait_ack_max_sec, "read_tables"},//28//Чтение таблицы//Команда: <46h><Таблица(1)><Ряд(2)><Поле(1)>
    {0x8D, 20,               "pic_print"},//29//Печать картинки по номеру//Команда:<8Dh><Принтер(1)><Номер(1)><Смещение(2)>
    {0xA4, wait_ack_max_sec, "shift_req_param"},//30//Запрос параметров текущей смены//Команда: <A4h><10h>
    {0xA4, wait_ack_max_sec, "get_stat_FN"},//31//Запрос статуса ФН//Команда: <A4h><30h>
    {0xA4, wait_ack_max_sec, "get_num_FN"},//32//Запрос номера ФН//Команда: <A4h><31h>//Ответ: <55h><Код Ошибки (1)><Номер ФН(16)>
    {0xA4, wait_ack_max_sec, "get_black_day"},//33//Запрос срока действия ФН//Команда: <A4h><32h>
    {0xA4, wait_ack_max_sec, "get_ver_FN"},//34//Запрос версии ФН//Команда: <A4h><33h>
    {0xA4, wait_ack_max_sec, "get_err_FN"},//35//Запрос последних ошибок ФН//Команда: <A4h><35h><Номер блока для чтения(1)>
    {0xA4, wait_ack_max_sec, "get_noack_FD"},//36//Запрос количества ФД, на которые нет квитанции (стр.90)//Команда: <A4h><42h>
    {0xE9, wait_ack_max_sec, "get_text_attr"},//37//Чтение реквизита//Команда: <E9h><Номер реквизита (2)><Номер блока (1)>
    {0xE8, wait_ack_max_sec, "set_text_attr"},//38//Запись реквизита//Команда:<E8h><Флаги(1)><Количество блоков(1)><Номер блока(1)><Данные реквизита(X)>
    {0x43, wait_ack_max_sec, "set_discount"},//39//Скидка//Команда: <43h><Флаги(1)><Область(1)><Тип(1)><Знак(1)><Размер(5)>//Ответ:<55h><Код ошибки(1)><Расширенный код ошибки (1)>
    {0xB8, wait_ack_max_sec, "reg_tax_chek"},//40//Регистрация налога на весь чек//Команда:<B8h><Флаги(1)><Область(1)><Тип(1)><Сумма(7)>//Ответ:<55h><Код ошибки(1)><Расширенный код ошибки(1)>
    {0x99, wait_ack_max_sec, "calc_by_chek"},//41//Расчет по чеку (стр.121)//Команда:<99h><Флаги(1)><Форма расчета(1)><Сумма(5)>//Ответ:<55h><Код Ошибки(1)><Остаток(5)><Сдача(5)>
    {0x9B, wait_ack_max_sec, "storno_calc_by_chek"},//42//Сторно расчета по чеку//Команда:<9Bh><Флаги(1)><Форма расчета(1)><Сумма(5)>//Ответ:<55h><Код Ошибки (1)><Остаток(5)><Сдача(5)>
    {0x67, wait_ack_max_sec, "begin_get_report"},//43//Начало снятия отчета//Команда:<67h><Тип Отчета(1)>//Ответ:<55h><Код Ошибки(1)><Расширенный код ошибки(1)>
    {0x77, wait_ack_max_sec, "general_cancel"},//44//Общее гашение//Команда:<77h>//Ответ:<55h><Код Ошибки(1)><Расширенный код ошибки(1)>
    {0x6B, 10,               "teh_clear"},//45//Технологическое обнуление ККТ//Команда:<6Bh>//Ответ:<55h><Код Ошибки(1)><Расширенный код ошибки(1)>
    {0x6D, wait_ack_max_sec, "set_prot_code"},//46//Ввод кода защиты ККТ//Команда:<6Dh><Номер(1)><Код(Х)>//Ответ:<55h><Код Ошибки(1)><(0)>
    {0x74, wait_ack_max_sec, "stat_prot_code"},//47//Запрос активизированности кода защиты ККТ//Команда:<74h><Номер(1)>//Ответ:<4Ch><Активизирован(1)>
    {0x91, 45,               "read_register"},//48//Считать регистр//Команда:<91h><Регистр(1)><Параметр1(1)><Параметр2(1)>//Ответ:<55h><Код Ошибки(1)><Значение(Х)>
    {0x95, wait_ack_max_sec, "reprint_last_doc"},//49//Повторная печать последнего документа//Команда:<95h>//Ответ:<55h><Код Ошибки(1)><Расширенный код ошибки(1)>
    {0xEE, wait_ack_max_sec, "reprint_report"},//50//Допечатать отчет//Команда:<EEh>//Ответ:<55h><Код ошибки(1)><Расширенный код ошибки(1)>
    {0x97, wait_ack_max_sec, "clear_buffer"},//51//Очистить буфер последнего документа//Команда:<97h>//Ответ:<55h><Код Ошибки(1)><Расширенный код ошибки(1)>
    {0xA6, 50,               "activation_FN"},//52//Активизация ФН//Команда:<A6h>//Ответ:<55h><Код Ошибки(0)><Расширенный код ошибки(1)>
    {0xA7, 20,               "close_FN"},//53//Закрытие архива ФН//Команда:<A7h>//Ответ:<55h><Код Ошибки(1)><Расширенный код ошибки(1)>
    {0xA8, 120,              "print_rep_reg"},//54//Печать итогов регистрации/перерегистрации ККТ//Команда:<A8h>//Ответ:<55h><Код Ошибки(1)><Расширенный код ошибки(1)>
    {0xAB, 120,              "print_doc_by_num"},//55//Печать документа по номеру//Команда:<ABh><Номер документа(5)>//Ответ:<55h><Код Ошибки(1)><Расширенный код ошибки(1)>
    {0xED, wait_ack_max_sec, "set_date_time"},//56//Программирование даты и времени//Команда:<EDh><День(1)><Месяц(1)><Год(1)><Час(1)><Минута(1)><Секунда(1)>//Ответ:<55h><Код ошибки(1)><Расширенный код ошибки(1)>

    {0x64, wait_ack_max_sec, "set_date"},//57//Программирование даты//Команда:<64h><День(1)><Месяц(1)><Год(1)>//Ответ:<55h><Код Ошибки(1)><Расширенный код ошибки(1)>
    {0x4B, wait_ack_max_sec, "set_time"},//58//Программирование времени//Команда:<4Bh><Час(1)><Минута(1)><Секунда(1)>//Ответ:<55h><Код Ошибки(1)><Расширенный код ошибки(1)>

    {0x50, wait_ack_max_sec, "write_tables"}//59//Программирование таблицы //<50h><Таблица(1)><Ряд(2)><Поле(1)><Значение (Х)>
};


const int max_param = 7;
const char *name_param[] = {
    "--dev=",
    "--cmd=",
    "--arg=",
    "--log=",
    "--speed=",
    "--password=",
    "--codepage="
};

const char *spd[] = {
    "4800",
    "9600",
    "38400",
    "57600",
    "115200"
};
const uint32_t ispd[] = {
    B4800,  //0000014
    B9600,  //0000015
    B38400, //0000017
    B57600, //0010001
    B115200 //0010002
};

//-----------------------------------------------------------------------

static const char *DevTypeToStr(uint8_t type);
static const char *ErrToStr(uint8_t err);
char *statExchSTR(uint8_t fl, char *st);
static const char *rdstatExchStr(uint8_t type);
uint64_t binToBCD(long long bin);
static const char *valStr(uint8_t t, uint16_t c, uint8_t p);
static const char *statFNStr(uint8_t bt);
static const char *curDocStr(uint8_t bt);
static const char *dataDocStr(uint8_t bt);
static const char *shiftStatStr(uint8_t bt);
char *alarmStr(uint8_t fl, char *st);
static const char *typeSwStr(uint8_t bt);
int8_t getTagType(uint16_t tg, char **nm);
char *statFlagsStr(uint8_t fl, char *st);
int8_t getTblType(uint8_t t, uint16_t c, uint8_t p, int *sl);

//-----------------------------------------------------------------------
static char *checkSTATUS(uint8_t cs)
{
    switch (cs) {
        case 0://#define checkCLOSED  0
            return "чек закрыт";
        case 1://#define checkIN      1
            return "чек прихода";
        case 2://#define checkRETIN   2
            return "чек возврата прихода";
        case 4://#define checkOUT     4
            return "чек расхода";
        case 5://#define checkRETOUT  5
            return "чек возврата расхода";
        case 7://#define checkEDITIN  7
            return "чек коррекции: приход";
        case 9://#define checkEDITOUT 9
            return "чек коррекции: расход";
    }
    return "???";
}
//-----------------------------------------------------------------------
static char *iface(uint8_t ifc)//Данный параметр обозначает тип интерфейса, по которому работает ККТ, и принимает значения:
{
    switch (ifc) {
        case 1:
            return "RS-232";
        case 4:
            return "USB";
        case 5:
            return "Bluetooth";
        case 6:
            return "Ethernet";
        case 7:
            return "WiFi";
    }
    return "???";
}
//-----------------------------------------------------------------------
uint32_t get100ms()
{
    return varta;
}
//----------------------------------------------------------------------
uint32_t get_timer_ms(uint32_t tm)
{
    return (get100ms() + tm);
}
//----------------------------------------------------------------------
int check_delay_ms(uint32_t tm)
{
    return (get100ms() >= tm ? 1 : 0);
}
//----------------------------------------------------------------------
uint8_t LRC(uint8_t *buf, int len)
{
uint8_t ret = 0;
uint8_t *uk = buf;

    while (len--) ret ^= *uk++;

    return ret;
}
//----------------------------------------------------------------------
char *ThisTime()
{
time_t ct = time(NULL);
char *arba = ctime(&ct);

    arba[strlen(arba) - 1] = 0;//remove '\n'

    return (arba);
}
//----------------------------------------------------------------------
char *TimeNowPrn(char *ts)
{
struct tm *ctimka;
struct timeval tvl;

    gettimeofday(&tvl, NULL);
    ctimka = localtime(&tvl.tv_sec);
    sprintf(ts, "%02d.%02d %02d:%02d:%02d.%03d | ",
                ctimka->tm_mday, ctimka->tm_mon + 1,
                ctimka->tm_hour, ctimka->tm_min, ctimka->tm_sec, (int)(tvl.tv_usec/1000));
    return ts;
}
//------------------------------------------------------------------------
void ToSysLogMsg(int LogLevel, const char * const Msg)
{
    if (LogLevel <= MaxLogLevel) syslog(LogLevel, "%s", Msg);
}
//-----------------------------------------------------------------------
uint8_t get_ack_wait(uint8_t cmd_idx)
{
    if (cmd_idx >= total_inCMD) return wait_ack_max_sec;

    return (inCMD[cmd_idx].wait);
}
//-----------------------------------------------------------------------
void print_msg(uint8_t dt, const char *fmt, ...)
{
size_t len = buf_size;
char dts[TIME_STR_LEN] = {0};
char *udt = TimeNowPrn(dts);

    if (dt) len += sizeof(dts);
    char *st = (char *)calloc(1, len + 1);
    if (st) {
        int dl = 0, sz;
        va_list args;

        if (dt) dl = sprintf(st, "%s", udt);
        sz = dl;

        va_start(args, fmt);
        sz += vsnprintf(st + dl, len - dl, fmt, args);
        va_end(args);

        if (dbg) printf("%s", st);
        if (fd_log) write(fd_log, st, strlen(st));

        struct stat sb;
        if (!fstat(fd_log, &sb)) {
            if (sb.st_size > max_size_log) {
                close(fd_log);
                fd_log = -1;
                char name[128] = {0};
                sprintf(name, "%u_%s", (uint32_t)time(NULL), udt);
                sprintf(dts,"%s%s", path_log, file_log);
                rename(dts, name);
                fd_log = open(dts, O_WRONLY | O_APPEND | O_CREAT, 0664); //create new file
                if (fd_log <= 0) sprintf(name, "%sVer.%s Can't open file %s (%d)\n", udt, vers, dts, fd_log);
                            else sprintf(name, "%sVer.%s Open new file %s (%d)\n", udt, vers, dts, fd_log);
                ToSysLogMsg(LOG_INFO, name);
            }
        }

        free(st);

    }

}
//-----------------------------------------------------------------------
void GetSignal_(int sig)
{
int out = 0;
char stx[64] = {0};

    switch (sig) {
        case SIGHUP:
            sprintf(stx, "\tSIGHUP : time %u.%u\n", varta / 10, varta);
        break;
        case SIGALRM://form Timer !!!
            varta++;
            return;
        case SIGKILL:
            if (SIGKILLs) {
                SIGKILLs = 0;
                strcpy(stx, "\tSignal SIGKILL\n");
                out = 1;
            }
        break;
        case SIGTERM:
            if (SIGTERMs) {
                SIGTERMs = 0;
                strcpy(stx, "\tSignal SIGTERM\n");
                out = 1;
            }
        break;
        case SIGINT:
            if (SIGINTs) {
                SIGINTs = 0;
                strcpy(stx, "\tSignal SIGINT\n");
                out = 1;
            }
        break;
        case SIGSEGV:
            if (SIGSEGVs) {
                SIGSEGVs = 0;
                strcpy(stx, "\tSignal SIGSEGV\n");
                out = 1;
            }
        break;
        case SIGABRT:
            if (SIGABRTs) {
                SIGABRTs = 0;
                strcpy(stx, "\tSignal SIGABRT\n");
                out = 1;
            }
        break;
            default : sprintf(stx, "\tUNKNOWN signal %d\n", sig);
    }

    if (strlen(stx)) print_msg(0, stx);

    if (out) QuitAll = out;
}
//-----------------------------------------------------------------------
int conv_text(const char *from, const char *to, char *in, char *out, int len)
{
int ret = 0;
size_t ilen = len;
size_t olen = sdef;
char buf[sdef] = {0};
char *uk = buf;

    iconv_t obj = iconv_open(to, from);//(codeCP866, codeUTF8);
    if (obj == (iconv_t)-1) {//Error !
        if (dbg != LOG_OFF) {
            if (errno == EINVAL)
                print_msg(1, "[%s] Conversion from '%s' to '%s' is not supported\n", __func__, from, to);//codeUTF8, codeCP866);
            else
                print_msg(1, "[%s] Iconv initialization failure:\n", __func__);
        }
    } else {//Ok
        if (iconv(obj, &in, &ilen, &uk, &olen) == (size_t)-1) {//Error !
            if (dbg != LOG_OFF) print_msg(1, "[%s] Error conversion from %u'%s' to %u'%s'\n", __func__, ilen, from, olen, to);
        } else {//Ok
            ret = (sizeof(buf) - olen) & 0xff;
            if (ret > 0) memcpy(out, buf, ret);
        }
    }

    iconv_close(obj);

    return ret;
}
//-----------------------------------------------------------------------
int parse_inCMD(char *in_cmd)
{
int ret = -1, i;

    for (i = 0; i < total_inCMD; i++) {
        if (!strncmp(in_cmd, inCMD[i].name, strlen(inCMD[i].name))) {
            ret = i;
            break;
        }
    }

    return ret;
}
//-----------------------------------------------------------------------
void setSPEED(uint8_t SpeedIndex)
{
    if (SpeedIndex < max_spd) {

        if (SPEED != ispd[SpeedIndex]) {

            SPEED = ispd[SpeedIndex];

            struct termios tio;

            tcgetattr(fd, &tio);
            cfmakeraw(&tio);//set RAW mode
            tio.c_cflag = CS8 | CLOCAL | CREAD | SPEED;
            tcflush(fd, TCIFLUSH);
            tcsetattr(fd, TCSANOW, &tio);

            if (dbg >= LOG_DEBUG) print_msg(1, "Set speed for device '%s' to %s 8N1 OK\n", device, spd[SpeedIndex]);
        }

    } else {
        if (dbg != LOG_OFF) print_msg(1, "Error speed index %u for device '%s'\n", SpeedIndex, device);
    }

}
//-----------------------------------------------------------------------
uint8_t findSPEED(char *param)
{
uint8_t byte = 4;//default speed = spd[1] -> 115200

    for (uint8_t i = 0; i < max_spd; i++) {
        if (!strncmp(param, spd[i], strlen(spd[i]))) {
            byte = i;
            break;
        }
    }

    return byte;
}
//-----------------------------------------------------------------------
int makeCMD(uint8_t *buf, int idx, char *arg, uint8_t *cb)
{
int ret = 0;
char one[sdef]  = {0};
uint8_t byte;

    if (cb) {
        *buf = *cb;//ENQ or EOT
        ret = 1;
    } else {
        switch (idx) {
            case CMD_REQUEST_STATUS:     //0// {0x3F, 10, "request_status"},//Запрос состояния ККТ
            case CMD_BEEP:               //3// {0x47, 10, "beep"},//Звуковой сигнал
            case CMD_CLISHE_PRINT:       //6// {0x6C, wait_ack_max_sec, "clishe_print"}//Печать клише чека
            case CMD_BUTTOM_PRINT:       //7// {0x73, wait_ack_max_sec, "buttom_print"}//Печать нижней части чека
            case CMD_REQUEST_CODE_STATUS://9// {0x45, wait_ack_max_sec, "request_code_status"}//Запрос кода состояния ККТ
            case CMD_SHIFT_CLOSE:        //10// {0x5A, wait_ack_max_sec, "shift_close"}//Закрыть смену
            case CMD_GET_DEV_TYPE:       //12// {0xA5, wait_ack_max_sec, "get_dev_type"}//Получить тип устройства
            case CMD_GET_PRINTER_ERROR:  //13// {0xB3, wait_ack_max_sec, "get_printer_error"}//Получить последний код ошибки
            case CMD_CANCEL_CHEK:        //18// {0x59, wait_ack_max_sec, "cancel_chek"// Команда аннулирует (отменяет) текущий открытый чек.
            case CMD_EXIT_MODE:          //23// {0x48, wait_ack_max_sec, "exit_mode"}//Выход из текущего режима//<48h>
            case CMD_CASH_REQUEST:       //25// {0x4D, wait_ack_max_sec, "cash_request"}//Запрос наличных//Команда: <4Dh>
            case CMD_SHIFT_MONEY:        //26// {0x58, wait_ack_max_sec, "get_shift_money"}//Получение последнего сменного итога//Команда: <58h>
            case CMD_INIT_TABLES:        //27// {0x71, wait_ack_max_sec, "init_tables"}//Инициализация таблиц начальными значениями//Команда: <71h>
            case CMD_GENERAL_CANCEL:     //44//"general_cancel"//Общее гашение//Команда:<77h>//Ответ:<55h><Код Ошибки(1)><Расширенный код ошибки(1)>
            case CMD_TEH_CLEAR:          //45//"teh_clear" - 45//Технологическое обнуление ККТ//Команда:<6Bh>//Ответ:<55h><Код Ошибки(1)><Расширенный код ошибки(1)>
            case CMD_REPRINT_LAST_DOC:   //49//"reprint_last_doc"//Повторная печать последнего документа//Команда:<95h>//Ответ:<55h><Код Ошибки(1)><Расширенный код ошибки(1)>
            case CMD_REPRINT_REPORT:     //50//"reprint_report"//Допечатать отчет//Команда:<EEh>//Ответ:<55h><Код ошибки(1)><Расширенный код ошибки(1)>
            case CMD_CLEAR_BUFFER:       //51//"clear_buffer"//Очистить буфер последнего документа//Команда:<97h>//Ответ:<55h><Код Ошибки(1)><Расширенный код ошибки(1)>
            case CMD_ACTIVATION_FN:      //52//"activation_FN"//52//Активизация ФН//Команда:<A6h>//Ответ:<55h><Код Ошибки(0)><Расширенный код ошибки(1)>
            case CMD_CLOSE_FN:           //53//"close_FN"}//52//Закрытие архива ФН//Команда:<A7h>//Ответ:<55h><Код Ошибки(1)><Расширенный код ошибки(1)>
            case CMD_PRINT_REP_REG:      //54//"print_rep_reg"}//53//Печать итогов регистрации/перерегистрации ККТ//Команда:<A8h>//Ответ:<55h><Код Ошибки(1)><Расширенный код ошибки(1)>
            {
                buf[ret++] = STX;
                byte = passwd >> 8;
                if ((byte == DLE) || (byte == ETX)) buf[ret++] = DLE;
                buf[ret++] = byte;
                byte = passwd & 0xff;
                if ((byte == DLE) || (byte == ETX)) buf[ret++] = DLE;
                buf[ret++] = byte;
                //memcpy(&buf[ret], &passwd, 2); ret += 2;
                buf[ret++] = inCMD[idx].cmd;
                buf[ret++] = ETX;
                buf[ret] = LRC(buf + 1, ret - 1);
                ret++;
            }
            break;
            case CMD_LINE_PRINT://1: // {0x4C, 10, "line_print"},//Печать текстовых документов
            case CMD_SHIFT_OPEN://5: // {0x9A, 10, "shift_open"},//Открыть смену
            {
                buf[ret++] = STX;
                byte = passwd >> 8;
                if ((byte == DLE) || (byte == ETX)) buf[ret++] = DLE;
                buf[ret++] = byte;
                byte = passwd & 0xff;
                if ((byte == DLE) || (byte == ETX)) buf[ret++] = DLE;
                buf[ret++] = byte;
                buf[ret++] = inCMD[idx].cmd;
                if (idx == CMD_SHIFT_OPEN) buf[ret++] = 0;
                int dl = strlen(arg);
                if (dl) {//from=codeUTF8, to=codeCP866
                    if (!strlen(codePage)) strcpy(codePage, codeCP866);
                    dl = conv_text(codeUTF8, (const char *)codePage, arg, one, dl);
                    if (dl > 0) {
                        if (dl > max_print_line) {
                            dl = max_print_line;
                            one[dl] = '\0';
                        }
                    }
                    memcpy(&buf[ret], one, dl); ret += dl;
                }
                buf[ret++] = ETX;
                buf[ret] = LRC(buf + 1, ret - 1);
                ret++;
            }
            break;
            case CMD_END_OF_PRINT://2:// {0x75, 10, "end_of_print"},//Команда отрезать чек
            case CMD_RESTART_DEVICE://4:// {0xCE, 10, "restart_device"}//Рестарт ККТ
            case CMD_GET_BAR_PIC://14:// {0x7D, 10, "get_bar_pic"}//Состояние массива штрихкодов и картинок // barcode or picture
            case CMD_GET_VERSION://15:// {0x9D, wait_ack_max_sec, "get_version"}//Получение версии
            {
                buf[ret++] = STX;
                byte = passwd >> 8;
                if ((byte == DLE) || (byte == ETX)) buf[ret++] = DLE;
                buf[ret++] = byte;
                byte = passwd & 0xff;
                if ((byte == DLE) || (byte == ETX)) buf[ret++] = DLE;
                buf[ret++] = byte;
                buf[ret++] = inCMD[idx].cmd;
                uint8_t bt = 1;//for barcode array// or //for cpu ккт
                if ((idx == CMD_GET_BAR_PIC) || (idx == CMD_GET_VERSION)) {//"get_bar_pic" or //"get_version"
                    if (strlen(arg)) {
                        if (idx == CMD_GET_BAR_PIC) {//14
                            if (!strcmp(arg, "picture")) bt = 2;//for picture array
                        } else {
                            if (!strcmp(arg, "boot")) {
                                buf[ret++] = DLE;
                                bt = 3;//for bootblock
                            }
                        }
                    }
                } else bt = 0;
                buf[ret++] = bt;
                buf[ret++] = ETX;
                buf[ret] = LRC(buf + 1, ret - 1);
                ret++;
            }
            break;
            case CMD_SH_CODE_PRINT://8:// {0x7C, wait_ack_max_sec, "sh_code_print"},//Печать штрихкода по номеру
            case CMD_DEMO_PRINT://11:// {0x82, wait_ack_max_sec, "demo_print"}//Демонстрационная печать
            {
                buf[ret++] = STX;
                byte = passwd >> 8;
                if ((byte == DLE) || (byte == ETX)) buf[ret++] = DLE;
                buf[ret++] = byte;
                byte = passwd & 0xff;
                if ((byte == DLE) || (byte == ETX)) buf[ret++] = DLE;
                buf[ret++] = byte;
                buf[ret++] = inCMD[idx].cmd;
                buf[ret++] = 1;//печатать на чековой ленте
                byte = (uint8_t)atoi(arg);//Номер печатаемой картинки из внутреннего массива штрихкодов.
                                          // или тип документа для команды "demo_print"
                if ((byte == DLE) || (byte == ETX)) buf[ret++] = DLE;
                buf[ret++] = byte;
                if (idx == CMD_DEMO_PRINT) buf[ret++] = 0;//для команды "demo_print"
                buf[ret++] = ETX;
                buf[ret] = LRC(buf + 1, ret - 1);
                ret++;
            }
            break;
            case CMD_GET_STAT_EXCH://{0xA4 0x20, wait_ack_max_sec, "get_stat_exch"}//Получить статус информационного обмена с ФН
            case CMD_SHIFT_REQ_PARAM://30//"shift_req_param"//Запрос параметров текущей смены//Команда: <A4h><10h>
            case CMD_GET_STAT_FN://31//"get_stat_FN"//Запрос статуса ФН//Команда: <A4h><30h>
            case CMD_GET_NUM_FN://32//"get_num_FN"//Запрос номера ФН//Команда: <A4h><31h>//Ответ: <55h><Код Ошибки (1)><Номер ФН(16)>
            case CMD_GET_BLACK_DAY://33//"get_black_day" - Запрос срока действия ФН //Команда: <A4h><32h>
            case CMD_GET_VER_FN://34//{0xA4, wait_ack_max_sec, "get_ver_FN"}//34//Запрос версии ФН//Команда: <A4h><33h>
            case CMD_GET_NOACK_FD://36//"get_noack_FD" - Запрос количества ФД, на которые нет квитанции (стр.90)//Команда: <A4h><42h>
            {
                buf[ret++] = STX;
                byte = passwd >> 8;
                if ((byte == DLE) || (byte == ETX)) buf[ret++] = DLE;
                buf[ret++] = byte;
                byte = passwd & 0xff;
                if ((byte == DLE) || (byte == ETX)) buf[ret++] = DLE;
                buf[ret++] = byte;
                buf[ret++] = inCMD[idx].cmd;
                     if (idx == CMD_SHIFT_REQ_PARAM) byte = 0x10;
                else if (idx == CMD_GET_STAT_EXCH) byte = 0x20;
                else if (idx == CMD_GET_STAT_FN) byte = 0x30;
                else if (idx == CMD_GET_NUM_FN) byte = 0x31;
                else if (idx == CMD_GET_BLACK_DAY) byte = 0x32;
                else if (idx == CMD_GET_VER_FN) byte = 0x33;
                else if (idx == CMD_GET_NOACK_FD) byte = 0x42;
                if ((byte == DLE) || (byte == ETX)) buf[ret++] = DLE;
                buf[ret++] = byte;
                buf[ret++] = ETX;
                buf[ret] = LRC(buf + 1, ret - 1);
                ret++;

            }
            break;
            case CMD_OPEN_CHEK://{0x92, wait_ack_max_sec,"open_chek"}//Открыть чек (стр.118) <92h><Флаги (1)><Тип чека (1)>
            {
                int len = 0, dl = strlen(arg);
                if (!dl) break;
                char *uki = strchr(arg, '^');
                if (!uki) break;
                //
                buf[len++] = STX;
                byte = passwd >> 8;
                if ((byte == DLE) || (byte == ETX)) buf[len++] = DLE;
                buf[len++] = byte;
                byte = passwd & 0xff;
                if ((byte == DLE) || (byte == ETX)) buf[len++] = DLE;
                buf[len++] = byte;
                buf[len++] = inCMD[idx].cmd;
                byte = 0;                        //flag: bit0=0 - выполнить операцию
                if (arg[0] != '0') byte |= 0x04; //flag: bit2=1 - не печатать чек
                buf[len++] = byte;
                dl = atoi(uki + 1);//тип чека
                if (!dl) break;
                if ((dl == DLE) || (dl == ETX)) buf[len++] = DLE;
                buf[len++] = (uint8_t)dl;
                buf[len++] = ETX;
                buf[len] = LRC(buf + 1, len - 1);
                //
                ret = len + 1;
            }
            break;
            case CMD_COMING_MONEY://19://{0x49, wait_ack_max_sec, "coming_money"}//Внесение денег <49h><Флаги(1)><Сумма(5)>
            case CMD_PAYOUT_MONEY://20://{0x4F, wait_ack_max_sec, "payout_money"}//Выплата денег// <4Fh><Флаги(1)><Сумма(5)>
            {
                int len = 0;
                if (!strlen(arg)) break;
                long long mn = atoll(arg);

                uint64_t money = binToBCD(mn);
                s_bit64_t *m64 = (s_bit64_t *)&money;
                //
                buf[len++] = STX;
                byte = passwd >> 8;
                if ((byte == DLE) || (byte == ETX)) buf[len++] = DLE;
                buf[len++] = byte;
                byte = passwd & 0xff;
                if ((byte == DLE) || (byte == ETX)) buf[len++] = DLE;
                buf[len++] = byte;
                buf[len++] = inCMD[idx].cmd;
                buf[len++] = 0;//flag: bit0=0 - выполнить операцию
                for (int i = 3; i < 8; i++) {
                    if ((m64->byte[i] == DLE) || (m64->byte[i] == ETX)) buf[len++] = DLE;
                    buf[len++] = m64->byte[i];
                }
                buf[len++] = ETX;
                buf[len] = LRC(buf + 1, len - 1);
                //
                ret = len + 1;
            }
            break;
            case CMD_CLOSE_CHEK://21://"close_chek"}// Закрыть чек (со сдачей) <4Ah><Флаги(1)><Форма расчета(1)><Внесенная сумма(5)>
            case CMD_CALC_BY_CHEK://41//"calc_by_chek"//Расчет по чеку//Команда:<99h><Флаги(1)><Форма расчета(1)><Сумма(5)>
            case CMD_STORNO_CALC_BY_CHEK://42//"storno_calc_by_chek"//Сторно расчета по чеку//Команда:<9Bh><Флаги(1)><Форма расчета(1)><Сумма(5)>
            {
                int len = 0, dl = strlen(arg);
                if (!dl) break;
                if (dl > (sdef - 1)) dl = sdef - 1;
                memcpy(one, arg, dl);
                char *uki = strchr(one, '^');
                if (!uki) break;
                long long mn = atoll(uki + 1);
                *uki = '\0';
                uint8_t fr = (uint8_t)atoi(one);
                if (!fr || (fr > 10)) break;
                //
                buf[len++] = STX;
                byte = passwd >> 8;
                if ((byte == DLE) || (byte == ETX)) buf[len++] = DLE;
                buf[len++] = byte;
                byte = passwd & 0xff;
                if ((byte == DLE) || (byte == ETX)) buf[len++] = DLE;
                buf[len++] = byte;
                buf[len++] = inCMD[idx].cmd;
                buf[len++] = 0;//flag: bit0=0 - выполнить операцию

                uint64_t fra = binToBCD((long long)fr);
                s_bit64_t *m64 = (s_bit64_t *)&fra;
                if ((m64->byte[7] == DLE) || (m64->byte[7] == ETX)) buf[len++] = DLE;
                buf[len++] = m64->byte[7];//Форма расчета

                uint64_t money = binToBCD(mn);
                m64 = (s_bit64_t *)&money;
                for (int i = 3; i < 8; i++) {
                    if ((m64->byte[i] == DLE) || (m64->byte[i] == ETX)) buf[len++] = DLE;
                    buf[len++] = m64->byte[i];
                }
                buf[len++] = ETX;
                buf[len] = LRC(buf + 1, len - 1);
                //
                ret = len + 1;

            }
            break;
            case CMD_REG_ITEM://22://{0xE6, wait_ack_max_sec, "reg_item"}//Регистрация позиции//Нужно задать 10 параметров
                                   // <E6h><Флаги (1)><Наименование товара (64)><Цена (6)><Количество (5)>
                                   // <Тип (1)><Знак (1)><Размер(6)><Налог (1)><Секция (1)><ШК (16)> <Резерв (1)>
            {
                int cnt = 0, len = 0, dl = strlen(arg);
                if (!dl) break;
                if (dl > (sdef - 1)) dl = sdef - 1;
                memcpy(one, arg, dl);

                char *uke = NULL;
                char *uks = &one[0];
                char *uki = strchr(uks, '^');//1
                if (!uki) break;
                cnt++;
                s_cmd_reg_item item;
                memset((uint8_t *)&item, 0, sizeof(s_cmd_reg_item));

                if (one[0] == '1') item.flags = 2;//set flags

                uks = uki + 1;//указатель на имя товара
                uke = strchr(uks, '^');//2
                if (!uke) break;

                char temp[sdef] = {0};
                char outtemp[128] = {0};
                long long mn = 0;
                uint64_t money = 0;
                s_bit64_t *m64 = NULL;
                dl = uke - uks; if (dl > 64) dl = 64;
                strncpy(temp, uks, dl);
                // from temp(utf-8) to outtemp(cp866)
                dl = conv_text(codeUTF8, (const char *)codeCP866, temp, outtemp, dl);
                if (dl > 0) {
                    if (dl > 64) {
                        dl = 64;
                        outtemp[dl] = '\0';
                    }
                    memcpy(&item.name[0], outtemp, dl);//имя
                }

                uks = uke + 1;//указатель на цену (BCD)
                uke = strchr(uks, '^');//3
                if (!uke) break;
                dl = uke - uks; if (dl > 15) dl = 15;
                strncpy(temp, uks, dl);//get price
                mn = atoll(temp);
                money = binToBCD(mn);
                m64 = (s_bit64_t *)&money;
                memcpy(&item.price[0], &m64->byte[sizeof(s_bit64_t) - sizeof(item.price)], sizeof(item.price));//set price

                uks = uke + 1;//указатель на количество (BCD)
                uke = strchr(uks, '^');//4
                if (!uke) break;
                dl = uke - uks; if (dl > 15) dl = 15;
                memset(temp, 0, sizeof(temp));
                strncpy(temp, uks, dl);//get total
                mn = atoll(temp);
                money = binToBCD(mn);
                m64 = (s_bit64_t *)&money;
                memcpy(&item.total[0], &m64->byte[sizeof(s_bit64_t) - sizeof(item.total)], sizeof(item.total));//set total

                uks = uke + 1;//указатель на тип
                uke = strchr(uks, '^');//5
                if (!uke) break;
                if (*uks == '1') item.type = 1;//0–процентная, тип 1–суммовая

                uks = uke + 1;//указатель на знак
                uke = strchr(uks, '^');//6
                if (!uke) break;
                if (*uks == '1') item.sign = 1;//0–скидка, знак 1–надбавка.

                uks = uke + 1;//указатель на размер (BCD)
                uke = strchr(uks, '^');//7
                if (!uke) break;
                dl = uke - uks; if (dl > 15) dl = 15;
                memset(temp, 0, sizeof(temp));
                strncpy(temp, uks, dl);//get size
                mn = atoll(temp);
                money = binToBCD(mn);
                m64 = (s_bit64_t *)&money;
                memcpy(&item.size[0], &m64->byte[sizeof(s_bit64_t) - sizeof(item.size)], sizeof(item.size));//set size

                uks = uke + 1;//указатель на налог
                uke = strchr(uks, '^');//8
                if (!uke) break;
                dl = uke - uks; if (dl > 15) dl = 15;
                memset(temp, 0, sizeof(temp));
                strncpy(temp, uks, dl);//get tax
                item.tax = (uint8_t)atoi(temp);//set tax

                uks = uke + 1;//указатель на секцию (BCD)
                uke = strchr(uks, '^');//9
                if (!uke) break;
                dl = uke - uks; if (dl > 15) dl = 15;
                memset(temp, 0, sizeof(temp));
                strncpy(temp, uks, dl);//get sect
                mn = atoll(temp);
                money = binToBCD(mn);
                m64 = (s_bit64_t *)&money;
                item.sect = m64->byte[7];//set sect

                uks = uki + 1;//указатель на строку
                dl = strlen(uks); if (dl > 16) dl = 16;
                memset(temp, 0, sizeof(temp));
                strncpy(temp, uks, dl);
                // from temp(utf-8) to outemp(cp866)
                dl = conv_text(codeUTF8, (const char *)codeCP866, temp, outtemp, dl);
                if (dl > 0) {
                    if (dl > 16) {
                        dl = 16;
                        outtemp[dl] = '\0';
                    }
                    memcpy(&item.str[0], outtemp, dl);//имя
                }
                //
                buf[len++] = STX;
                byte = passwd >> 8;
                if ((byte == DLE) || (byte == ETX)) buf[len++] = DLE;
                buf[len++] = byte;
                byte = passwd & 0xff;
                if ((byte == DLE) || (byte == ETX)) buf[len++] = DLE;
                buf[len++] = byte;
                buf[len++] = inCMD[idx].cmd;
                uint8_t *bt = (uint8_t *)&item;
                for (int i = 0; i < sizeof(s_cmd_reg_item); i++) {
                    if ((*(bt + i) == DLE) || (*(bt + i) == ETX)) buf[len++] = DLE;
                    buf[len++] = *(bt + i);
                }
                buf[len++] = ETX;
                buf[len] = LRC(buf + 1, len - 1);
                //
                ret = len + 1;
            }
            break;
            case CMD_ENTER_MODE://{0x56, wait_ack_max_sec, "enter_mode"},//Вход в режим//<56h> <Режим(1)><Пароль(4)>
            //Режим(1) - Двоичное число (00h .. FFh). Младшая тетрада – режим, старшая – подрежим (формат «Подрежим.Режим»).
            {
                int len = 0, dl = strlen(arg);
                if (!dl) break;
                if (dl > (sdef - 1)) dl = sdef - 1;
                memcpy(one, arg, dl);
                char *uki = strchr(one, '^');
                if (!uki) break;
                long long pwd = atoll(uki + 1);
                *uki = '\0';
                dl = strlen(one);
                uint8_t mode = 0;
                if (dl > 0) {
                    mode = (one[0] - 0x30) & 0x0f;//режим задан
                    if (dl > 1) mode |= ((one[1] - 0x30) << 4);//подрежим задан
                } else break;
                //
                buf[len++] = STX;
                byte = passwd >> 8;
                if ((byte == DLE) || (byte == ETX)) buf[len++] = DLE;
                buf[len++] = byte;
                byte = passwd & 0xff;
                if ((byte == DLE) || (byte == ETX)) buf[len++] = DLE;
                buf[len++] = byte;

                buf[len++] = inCMD[idx].cmd;

                if ((mode == DLE) || (mode == ETX)) buf[len++] = DLE;
                buf[len++] = mode;

                uint64_t bit64 = binToBCD(pwd);
                s_bit64_t *m64 = (s_bit64_t *)&bit64;
                for (int i = 4; i < 8; i++) {
                    if ((m64->byte[i] == DLE) || (m64->byte[i] == ETX)) buf[len++] = DLE;
                    buf[len++] = m64->byte[i];
                }

                buf[len++] = ETX;
                buf[len] = LRC(buf + 1, len - 1);
                //
                ret = len + 1;
            }
            break;
            case CMD_READ_TABLES://28//"read_tables"//Чтение таблицы//Команда: <46h><Таблица(1)><Ряд(2)><Поле(1)>
            {
                int len = 0, dl = strlen(arg);
                if (!dl) break;

                if (dl > (sdef - 1)) dl = sdef - 1;
                memcpy(one, arg, dl);
                char temp[sdef] = {0};
                char *uks = one, *uke = NULL;

                uke = strchr(uks, '^');
                if (!uke) break;
                dl = uke - uks; if (dl > 4) dl = 4;
                memset(temp, 0, sizeof(temp));
                strncpy(temp, uks, dl);//get table
                tbl = (uint8_t)atoi(temp);//  STATIC

                uks = uke + 1;
                uke = strchr(uks, '^');
                if (!uke) break;
                dl = uke - uks; if (dl > 4) dl = 4;
                memset(temp, 0, sizeof(temp));
                strncpy(temp, uks, dl);//get column
                column = (uint16_t)atoi(temp);//  STATIC

                uks = uke + 1;
                memset(temp, 0, sizeof(temp));
                strncpy(temp, uks, 2);//get pole
                pole = (uint8_t)atoi(temp);//  STATIC
                //
                buf[len++] = STX;
                byte = passwd >> 8;
                if ((byte == DLE) || (byte == ETX)) buf[len++] = DLE;
                buf[len++] = byte;
                byte = passwd & 0xff;
                if ((byte == DLE) || (byte == ETX)) buf[len++] = DLE;
                buf[len++] = byte;

                buf[len++] = inCMD[idx].cmd;

                if ((tbl == DLE) || (tbl == ETX)) buf[len++] = DLE;
                buf[len++] = tbl;

                byte = column >> 8;
                if ((byte == DLE) || (byte == ETX)) buf[len++] = DLE;
                buf[len++] = byte;
                byte = column & 0xff;
                if ((byte == DLE) || (byte == ETX)) buf[len++] = DLE;
                buf[len++] = byte;

                if ((pole == DLE) || (pole == ETX)) buf[len++] = DLE;
                buf[len++] = pole;

                buf[len++] = ETX;
                buf[len] = LRC(buf + 1, len - 1);
                //
                ret = len + 1;
            }
            break;
            case CMD_WRITE_TABLES://57//"write_tables"//Программирование таблицы //<50h><Таблица(1)><Ряд(2)><Поле(1)><Значение (Х)>
            {
                int len = 0, dl = strlen(arg);
                if (!dl) break;

                if (dl > (sdef - 1)) dl = sdef - 1;
                memcpy(one, arg, dl);
                char temp[sdef] = {0};
                char *uks = one, *uke = NULL;

                uke = strchr(uks, '^');
                if (!uke) break;
                dl = uke - uks; if (dl > 4) dl = 4;
                memset(temp, 0, sizeof(temp));
                strncpy(temp, uks, dl);// get table
                tbl = (uint8_t)atoi(temp);//

                uks = uke + 1;
                uke = strchr(uks, '^');
                if (!uke) break;
                dl = uke - uks; if (dl > 4) dl = 4;
                memset(temp, 0, sizeof(temp));
                strncpy(temp, uks, dl);// get column
                column = (uint16_t)atoi(temp);//

                uks = uke + 1;
                uke = strchr(uks, '^');
                if (!uke) break;
                dl = uke - uks; if (dl > 4) dl = 4;
                memset(temp, 0, sizeof(temp));
                strncpy(temp, uks, dl);// get pole
                pole = (uint8_t)atoi(temp);//

                int slen = 0, i;
                int8_t tp = getTblType(tbl, column, pole, &slen);
                if (tp != 0) break;//!!! now string type support only !!!

                uks = uke + 1;
                dl = strlen(uks);// get value
                memset(temp, 0, sizeof(temp));
                if (dl) {// convert text from codeUTF8 to codeCP866
                    dl = conv_text(codeUTF8, (const char *)codeCP866, uks, temp, dl);
                    if (dl > 0) {
                        if (dl > slen) {
                            dl = slen;
                            temp[dl] = '\0';
                        } else {
                            for (i = dl; i < slen; i++) strcat(temp, " ");
                        }
                    } else break;
                }
                dl = strlen(temp);
                //
                len = 0;
                buf[len++] = STX;
                byte = passwd >> 8;
                if ((byte == DLE) || (byte == ETX)) buf[len++] = DLE;
                buf[len++] = byte;
                byte = passwd & 0xff;
                if ((byte == DLE) || (byte == ETX)) buf[len++] = DLE;
                buf[len++] = byte;
                buf[len++] = inCMD[idx].cmd;
                // put number table to buffer
                if ((tbl == DLE) || (tbl == ETX)) buf[len++] = DLE;
                buf[len++] = tbl;
                // put number column to buffer
                byte = column >> 8;
                if ((byte == DLE) || (byte == ETX)) buf[len++] = DLE;
                buf[len++] = byte;
                byte = column & 0xff;
                if ((byte == DLE) || (byte == ETX)) buf[len++] = DLE;
                buf[len++] = byte;
                // put number pole to buffer
                if ((pole == DLE) || (pole == ETX)) buf[len++] = DLE;
                buf[len++] = pole;
                // put value to buffer
                if (tp) {
                    for (i = 0; i < dl; i++) {
                        if ((temp[i] == DLE) || (temp[i] == ETX)) buf[len++] = DLE;
                        buf[len++] = temp[i];
                    }
                } else {//in string value DLE and ETX bytes not present
                    memcpy(&buf[len], temp, dl);
                    len += dl;
                }
                buf[len++] = ETX;
                buf[len] = LRC(buf + 1, len - 1);
                //
                ret = len + 1;
            }
            break;
            case CMD_PIC_PRINT://29//"pic_print"}//Печать картинки по номеру//Команда:<8Dh><Принтер(1)><Номер(1)><Смещение(2)>
            {
                int len = 0, dl = strlen(arg);
                if (!dl) break;

                char *uks = arg;
                char *uke = strchr(uks, '^');
                if (!uke) break;

                dl = uke - uks; if (dl > 8) dl = 8;
                memset(one, 0, sizeof(one));
                strncpy(one, uks, dl);
                uint8_t pic_num = (uint8_t)atoi(one);//Номер(1)

                uks = uke + 1;
                dl = strlen(uks); if (dl > 8) dl = 8;
                memset(one, 0, sizeof(one));
                strncpy(one, uks, dl);
                uint16_t shi = (uint16_t)atoi(one);//Смещение(2)
                //
                buf[len++] = STX;
                byte = passwd >> 8;
                if ((byte == DLE) || (byte == ETX)) buf[len++] = DLE;
                buf[len++] = byte;
                byte = passwd & 0xff;
                if ((byte == DLE) || (byte == ETX)) buf[len++] = DLE;
                buf[len++] = byte;

                buf[len++] = inCMD[idx].cmd;

                buf[len++] = 1;//1 - печатать на чековой ленте

                if ((pic_num == DLE) || (pic_num == ETX)) buf[len++] = DLE;
                buf[len++] = pic_num;

                byte = shi >> 8;
                if ((byte == DLE) || (byte == ETX)) buf[len++] = DLE;
                buf[len++] = byte;
                byte = shi & 0xff;
                if ((byte == DLE) || (byte == ETX)) buf[len++] = DLE;
                buf[len++] = byte;

                buf[len++] = ETX;
                buf[len] = LRC(buf + 1, len - 1);
                //
                ret = len + 1;
            }
            break;
            case CMD_GET_ERR_FN://{0xA4, wait_ack_max_sec, "get_err_FN"}//35//Запрос последних ошибок ФН//Команда: <A4h><35h><Номер блока для чтения(1)>
            {
                int len = 0, dl = strlen(arg);
                if (!dl) break;
                if (strstr(arg, "empty")) break;
                //
                buf[len++] = STX;
                byte = passwd >> 8;
                if ((byte == DLE) || (byte == ETX)) buf[len++] = DLE;
                buf[len++] = byte;
                byte = passwd & 0xff;
                if ((byte == DLE) || (byte == ETX)) buf[len++] = DLE;
                buf[len++] = byte;

                buf[len++] = inCMD[idx].cmd;
                buf[len++] = 0x35;//next byte of command

                uint64_t blk = binToBCD(atoll(arg));
                s_bit64_t *m64 = (s_bit64_t *)&blk;
                if ((m64->byte[7] == DLE) || (m64->byte[7] == ETX)) buf[len++] = DLE;
                buf[len++] = m64->byte[7];//Номер блока для чтения

                buf[len++] = ETX;
                buf[len] = LRC(buf + 1, len - 1);
                //
                ret = len + 1;
            }
            break;
            case CMD_SET_TEXT_ATTR://38//"set_text attr" - Запись реквизита
                                   //Команда:<E8h><Флаги(1)><Количество блоков(1)><Номер блока(1)><Данные реквизита(X)>
                                   // "1^0^1^0^1008:body@abubariba.com"
            {
                int len = 0, dl = strlen(arg);
                if (!dl) break;

                char *uks = &arg[0];
                char *uke = strchr(uks, '^');
                if (!uke) break;

                uint8_t bit0 = 0, bit1 = 0;
                if (*uks == '1') bit0 = 1;//выводить на печать

                uks = uke + 1;//указатель на тип реквизита
                uke = strchr(uks, '^');
                if (!uke) break;

                if (*uks == '1') bit1 = 1;//пользовательский реквизит
                bit0 |= bit1;//Флаги

                uks = uke + 1;//указатель на Количество блоков
                uke = strchr(uks, '^');
                if (!uke) break;

                dl = uke - uks; if (dl > 4) dl = 4;
                memset(one, 0, sizeof(one));
                strncpy(one, uks, dl);
                uint8_t blks = (uint8_t)atoi(one);//Количество блоков

                uks = uke + 1;//указатель на Номер блока
                uke = strchr(uks, '^');
                if (!uke) break;

                dl = uke - uks; if (dl > 4) dl = 4;
                memset(one, 0, sizeof(one));
                strncpy(one, uks, dl);
                uint8_t nblk = (uint8_t)atoi(one);//Номер блока

                uks = uke + 1;//указатель на Данные реквизита
                uke = strchr(uks, '\0');
                if (!uke) break;

                dl = uke - uks; if (dl > sdef - 1) dl = sdef - 1;
                memset(one, 0, sizeof(one));
                strncpy(one, uks, dl);//Данные реквизита
                uks = one;
                uke = strchr(uks, ':');
                if (!uke) break;

                char temp[sdef] = {0};
                dl = uke - uks; if (dl > sdef - 1) dl = sdef - 1;
                strncpy(temp, one, dl);//tag number
                s_tlv tlv;
                tlv.tag = (uint16_t)atoi(temp);//Номер тега реквизита
                char *name = NULL;
                if (getTagType(tlv.tag, &name)) break;
                memset(temp, 0, sizeof(temp));
                strcpy(temp, uke + 1);
                tlv.len = (uint16_t)strlen(temp) & 0x1ff;//Длинна значения тега
                //temp - значение тега
                memset(one, 0, sizeof(one));
                // from cp866 to utf-8
                tlv.len = (uint16_t)conv_text(codeUTF8 ,(const char *)codeCP866, temp, one, tlv.len);
                if (!tlv.len) break;
                //
                uint8_t dat[tmp_size] = {0};
                dl = sizeof(s_tlv) + tlv.len;
                memcpy(dat, (uint8_t *)&tlv, sizeof(s_tlv));
                memcpy(dat + sizeof(s_tlv), one, tlv.len);
                //
                buf[len++] = STX;
                byte = passwd >> 8;
                if ((byte == DLE) || (byte == ETX)) buf[len++] = DLE;
                buf[len++] = byte;
                byte = passwd & 0xff;
                if ((byte == DLE) || (byte == ETX)) buf[len++] = DLE;
                buf[len++] = byte;

                buf[len++] = inCMD[idx].cmd;
                buf[len++] = bit0;//Флаги

                if ((blks == DLE) || (blks == ETX)) buf[len++] = DLE;
                buf[len++] = blks;//Количество блоков

                if ((nblk == DLE) || (nblk == ETX)) buf[len++] = DLE;
                buf[len++] = nblk;//Номер блока

                for (int i = 0; i < dl; i++) {
                    if ((dat[i] == DLE) || (dat[i] == ETX)) buf[len++] = DLE;
                    buf[len++] = dat[i];
                }

                buf[len++] = ETX;
                buf[len] = LRC(buf + 1, len - 1);
                //
                ret = len + 1;
            }
            break;
            case CMD_GET_TEXT_ATTR://37//"get_text_attr" - Чтение реквизита//Команда:<E9h><Номер реквизита(2)><Номер блока(1)>
            {
                int len = 0, dl = strlen(arg);
                if (!dl) break;

                char *uks = &arg[0];
                char *uke = strchr(uks, '^');
                if (!uke) break;

                dl = uke - uks; if (dl > 8) dl = 8;
                strncpy(one, uks, dl);
                uint16_t tag = (uint16_t)atoi(one);//Номер реквизита
                //tag = htons(tag);

                uks = uke + 1;//указатель на Номер блока
                uke = strchr(uks, '\0');
                if (!uke) break;

                dl = uke - uks; if (dl > 8) dl = 8;
                memset(one, 0, sizeof(one));
                strncpy(one, uks, dl);
                uint8_t nblk = (uint8_t)atoi(one);//Номер блока
                //
                buf[len++] = STX;
                byte = passwd >> 8;
                if ((byte == DLE) || (byte == ETX)) buf[len++] = DLE;
                buf[len++] = byte;
                byte = passwd & 0xff;
                if ((byte == DLE) || (byte == ETX)) buf[len++] = DLE;
                buf[len++] = byte;
                buf[len++] = inCMD[idx].cmd;
                //Номер реквизита
/**/
                byte = tag & 0xff;//>> 8;
                if ((byte == DLE) || (byte == ETX)) buf[len++] = DLE;
                buf[len++] = byte;
                byte = tag >> 8;//& 0xff;
                if ((byte == DLE) || (byte == ETX)) buf[len++] = DLE;
                buf[len++] = byte;
/**/
//                memcpy(&buf[len], (uint8_t *)&tag, 2); len += 2;
                //Номер блока
                if ((nblk == DLE) || (nblk == ETX)) buf[len++] = DLE;
                buf[len++] = nblk;
                buf[len++] = ETX;
                buf[len] = LRC(buf + 1, len - 1);
                //
                ret = len + 1;
            }
            break;
            case CMD_SET_DISCOUNT://39//"set_discount" - Скидка//Команда:<43h><Флаги(1)><Область(1)><Тип(1)><Знак(1)><Размер(5)>
            {
                int len = 0;
                if (!strlen(arg)) break;
                long long mn = atoll(arg);

                uint64_t money = binToBCD(mn);
                s_bit64_t *m64 = (s_bit64_t *)&money;//Размер(5)
                //
                buf[len++] = STX;
                byte = passwd >> 8;
                if ((byte == DLE) || (byte == ETX)) buf[len++] = DLE;
                buf[len++] = byte;
                byte = passwd & 0xff;
                if ((byte == DLE) || (byte == ETX)) buf[len++] = DLE;
                buf[len++] = byte;
                buf[len++] = inCMD[idx].cmd;
                buf[len++] = 0;//Флаги(1)//bit0=0 - выполнить операцию
                buf[len++] = 0;//Область(1)//Область применения команды//0 – на весь чек.
                buf[len++] = 1;//Тип(1)//Тип скидки. Проверяется только младший бит: 1 – суммовое
                buf[len++] = 0;//Знак(1)//0 – скидка
                for (int i = 3; i < 8; i++) {//
                    if ((m64->byte[i] == DLE) || (m64->byte[i] == ETX)) buf[len++] = DLE;
                    buf[len++] = m64->byte[i];
                }
                buf[len++] = ETX;
                buf[len] = LRC(buf + 1, len - 1);
                //
                ret = len + 1;
            }
            break;
            case CMD_REG_TAX_CHEK://40//"reg_tax_chek" - Регистрация налога на весь чек//Команда:<B8h><Флаги(1)><Область(1)><Тип(1)><Сумма(7)>
            {
                int len = 0, dl = strlen(arg);
                if (!dl) break;
                if (dl > (sdef - 1)) dl = sdef - 1;
                memcpy(one, arg, dl);
                char *uki = strchr(one, '^');
                if (!uki) break;
                long long mn = atoll(uki + 1);//Сумма(7)
                *uki = '\0';
                uint8_t type = (uint8_t)atoi(one);
                if (!type || (type > 6)) break;//Тип(1)
                //
                buf[len++] = STX;
                byte = passwd >> 8;
                if ((byte == DLE) || (byte == ETX)) buf[len++] = DLE;
                buf[len++] = byte;
                byte = passwd & 0xff;
                if ((byte == DLE) || (byte == ETX)) buf[len++] = DLE;
                buf[len++] = byte;
                buf[len++] = inCMD[idx].cmd;
                buf[len++] = 0;//Флаги(1)//bit0=0 - выполнить операцию
                buf[len++] = 0;//Область(1)//Область применения команды//0 – на весь чек.
                buf[len++] = type;//Тип(1)

                uint64_t money = binToBCD(mn);
                s_bit64_t *m64 = (s_bit64_t *)&money;
                for (int i = 1; i < 8; i++) {//Сумма(7)
                    if ((m64->byte[i] == DLE) || (m64->byte[i] == ETX)) buf[len++] = DLE;
                    buf[len++] = m64->byte[i];
                }
                buf[len++] = ETX;
                buf[len] = LRC(buf + 1, len - 1);
                //
                ret = len + 1;
            }
            break;
            case CMD_BEGIN_GET_REPORT://43//"begin_get_report" - Начало снятия отчета//Команда:<67h><Тип Отчета(1)>//Ответ:<55h><Код Ошибки(1)><Расширенный код ошибки(1)>
            {
                int len = 0;
                if (!strlen(arg)) break;
                long long mn = atoi(arg);

                uint64_t tr = binToBCD(mn);
                s_bit64_t *m64 = (s_bit64_t *)&tr;//Тип Отчета(1)
                //
                buf[len++] = STX;
                byte = passwd >> 8;
                if ((byte == DLE) || (byte == ETX)) buf[len++] = DLE;
                buf[len++] = byte;
                byte = passwd & 0xff;
                if ((byte == DLE) || (byte == ETX)) buf[len++] = DLE;
                buf[len++] = byte;
                buf[len++] = inCMD[idx].cmd;
                if ((m64->byte[7] == DLE) || (m64->byte[7] == ETX)) buf[len++] = DLE;
                buf[len++] = m64->byte[7];
                buf[len++] = ETX;
                buf[len] = LRC(buf + 1, len - 1);
                //
                ret = len + 1;
            }
            break;
            case CMD_SET_PROT_CODE://46//"set_prot_code" - Ввод кода защиты ККТ//Команда:<6Dh><Номер(1)><Код(Х)>
                                   //Ответ:<55h><Код Ошибки(1)><(0)>
                                   // "N^C", N=1..30, C:0000000000000000..9999999999999999
            {
                int len = 0, dl = strlen(arg);
                if (!dl) break;
                if (dl > (sdef - 1)) dl = sdef - 1;
                memcpy(one, arg, dl);
                char *uki = strchr(one, '^');
                if (!uki) break;
                long long pc = atoll(uki + 1);//Код(Х)
                *uki = '\0';
                uint8_t num = (uint8_t)atoi(one);
                if (!num || (num > 30)) break;//Номер(1)
                //
                buf[len++] = STX;

                byte = passwd >> 8;
                if ((byte == DLE) || (byte == ETX)) buf[len++] = DLE;
                buf[len++] = byte;
                byte = passwd & 0xff;
                if ((byte == DLE) || (byte == ETX)) buf[len++] = DLE;
                buf[len++] = byte;

                buf[len++] = inCMD[idx].cmd;

                if ((num == DLE) || (num == ETX)) buf[len++] = DLE;
                buf[len++] = num;

                uint64_t pwd = binToBCD(pc);
                s_bit64_t *m64 = (s_bit64_t *)&pwd;
                for (int i = 0; i < 8; i++) {//Код(X)
                    if ((m64->byte[i] == DLE) || (m64->byte[i] == ETX)) buf[len++] = DLE;
                    buf[len++] = m64->byte[i];
                }
                buf[len++] = ETX;
                buf[len] = LRC(buf + 1, len - 1);
                //
                ret = len + 1;
            }
            break;
            case CMD_STAT_PROT_CODE://47//"stat_prot_code" - Запрос активизированности кода защиты ККТ//Команда:<74h><Номер(1)>//Ответ:<4Ch><Активизирован(1)>
            {
                int len = 0, dl = strlen(arg);
                if (!dl) break;
                uint8_t num = (uint8_t)atoi(arg);
                if (!num || (num > 30)) break;//Номер(1)
                //
                buf[len++] = STX;

                byte = passwd >> 8;
                if ((byte == DLE) || (byte == ETX)) buf[len++] = DLE;
                buf[len++] = byte;
                byte = passwd & 0xff;
                if ((byte == DLE) || (byte == ETX)) buf[len++] = DLE;
                buf[len++] = byte;

                buf[len++] = inCMD[idx].cmd;

                if ((num == DLE) || (num == ETX)) buf[len++] = DLE;
                buf[len++] = num;

                buf[len++] = ETX;
                buf[len] = LRC(buf + 1, len - 1);
                //
                ret = len + 1;
            }
            break;
            case CMD_READ_REGISTER://48//"read_register"//Считать регистр//Команда:<91h><Регистр(1)><Параметр1(1)><Параметр2(1)>//Ответ:<55h><Код Ошибки(1)><Значение(Х)>
            {
                int len = 0, dl = strlen(arg);
                if (!dl) break;

                char *uks = &arg[0];
                char *uke = strchr(uks, '^');
                if (!uke) break;
                dl = uke - uks; if (dl > 4) dl = 4;
                memset(one, 0, sizeof(one));
                strncpy(one, uks, dl);
                uint8_t reg = (uint8_t)atoi(one);//Регистр(1)
                if (!reg || (reg > 71)) break;//Допустимые значения 1..71.

                uks = uke + 1;//указатель на параметр1
                uke = strchr(uks, '^');
                if (!uke) break;
                dl = uke - uks; if (dl > 4) dl = 4;
                memset(one, 0, sizeof(one));
                strncpy(one, uks, dl);
                uint8_t par1 = (uint8_t)atoi(one);//Параметр1(1)

                uks = uke + 1;//указатель на параметр2
                uke = strchr(uks, '\0');
                if (!uke) break;
                dl = uke - uks; if (dl > 4) dl = 4;
                memset(one, 0, sizeof(one));
                strncpy(one, uks, dl);
                uint8_t par2 = (uint8_t)atoi(one);//Параметр1(2)
                //
                buf[len++] = STX;

                byte = passwd >> 8;
                if ((byte == DLE) || (byte == ETX)) buf[len++] = DLE;
                buf[len++] = byte;
                byte = passwd & 0xff;
                if ((byte == DLE) || (byte == ETX)) buf[len++] = DLE;
                buf[len++] = byte;

                buf[len++] = inCMD[idx].cmd;

                if ((reg == DLE) || (reg == ETX)) buf[len++] = DLE;
                buf[len++] = reg;

                if ((par1 == DLE) || (par1 == ETX)) buf[len++] = DLE;
                buf[len++] = par1;

                if ((par2 == DLE) || (par2 == ETX)) buf[len++] = DLE;
                buf[len++] = par2;

                buf[len++] = ETX;
                buf[len] = LRC(buf + 1, len - 1);
                //
                ret = len + 1;
            }
            break;
            case CMD_PRINT_DOC_BY_NUM://55//"print_doc_by_num"//Печать документа по номеру//Команда:<ABh><Номер документа(5)>//Ответ:<55h><Код Ошибки(1)><Расширенный код ошибки(1)>
            {
                int len = 0;
                if (!strlen(arg)) break;
                long long nd = atoll(arg);

                uint64_t numDoc = binToBCD(nd);
                s_bit64_t *m64 = (s_bit64_t *)&numDoc;
                //
                buf[len++] = STX;
                byte = passwd >> 8;
                if ((byte == DLE) || (byte == ETX)) buf[len++] = DLE;
                buf[len++] = byte;
                byte = passwd & 0xff;
                if ((byte == DLE) || (byte == ETX)) buf[len++] = DLE;
                buf[len++] = byte;

                buf[len++] = inCMD[idx].cmd;

                for (int i = 3; i < 8; i++) {
                    if ((m64->byte[i] == DLE) || (m64->byte[i] == ETX)) buf[len++] = DLE;
                    buf[len++] = m64->byte[i];
                }

                buf[len++] = ETX;

                buf[len] = LRC(buf + 1, len - 1);
                //
                ret = len + 1;
            }
            break;
            case CMD_SET_DATE_TIME://56//"set_date_time"//Программирование даты и времени
                                   //Команда:<EDh><День(1)><Месяц(1)><Год(1)><Час(1)><Минута(1)><Секунда(1)>
                                   //Ответ:<55h><Код ошибки(1)><Расширенный код ошибки(1)>
            {
                int len = 0, dl = strlen(arg);
                if (!dl) break;
                uint8_t tmp[6] = {0};
                char *uks = &arg[0];
                char *uke = strchr(uks, '^');
                if (!uke) break;
                dl = uke - uks; if (dl > 4) dl = 4;
                memset(one, 0, sizeof(one));
                strncpy(one, uks, dl);
                uint8_t day = (uint8_t)atoi(one);//День(1)
                if (!day || (day > 31)) break;//Допустимые значения 01..31.
                tmp[len++] = day;

                uks = uke + 1;//указатель на Месяц(1)
                uke = strchr(uks, '^');
                if (!uke) break;
                dl = uke - uks; if (dl > 4) dl = 4;
                memset(one, 0, sizeof(one));
                strncpy(one, uks, dl);
                uint8_t mon = (uint8_t)atoi(one);//Месяц(1)
                if (!mon || (mon > 12)) break;//Допустимые значения 01..12
                tmp[len++] = mon;

                uks = uke + 1;//указатель на Год(1)
                uke = strchr(uks, '^');
                if (!uke) break;
                dl = uke - uks; if (dl > 4) dl = 4;
                memset(one, 0, sizeof(one));
                strncpy(one, uks, dl);
                uint8_t year = (uint8_t)atoi(one);//Год(1)
                if (year > 99) break;//Допустимые значения 00..99
                tmp[len++] = year;

                uks = uke + 1;//указатель на Час(1)
                uke = strchr(uks, '^');
                if (!uke) break;
                dl = uke - uks; if (dl > 4) dl = 4;
                memset(one, 0, sizeof(one));
                strncpy(one, uks, dl);
                uint8_t hour = (uint8_t)atoi(one);//Час(1)
                if (hour > 23) break;//Допустимые значения 00..23
                tmp[len++] = hour;

                uks = uke + 1;//указатель на Минута(1)
                uke = strchr(uks, '^');
                if (!uke) break;
                dl = uke - uks; if (dl > 4) dl = 4;
                memset(one, 0, sizeof(one));
                strncpy(one, uks, dl);
                uint8_t min = (uint8_t)atoi(one);//Минута(1)
                if (min > 59) break;//Допустимые значения 00..59
                tmp[len++] = min;

                uks = uke + 1;//указатель на Секунда(1)
                uke = strchr(uks, '\0');
                if (!uke) break;
                dl = uke - uks; if (dl > 4) dl = 4;
                memset(one, 0, sizeof(one));
                strncpy(one, uks, dl);
                uint8_t sec = (uint8_t)atoi(one);//Секунда(1)
                if (sec > 59) break;//Допустимые значения 00..59
                tmp[len++] = sec;
                //
                len = 0;
                buf[len++] = STX;
                byte = passwd >> 8;
                if ((byte == DLE) || (byte == ETX)) buf[len++] = DLE;
                buf[len++] = byte;
                byte = passwd & 0xff;
                if ((byte == DLE) || (byte == ETX)) buf[len++] = DLE;
                buf[len++] = byte;

                buf[len++] = inCMD[idx].cmd;

                uint64_t bit64 = 0;
                s_bit64_t *m64 = (s_bit64_t *)&bit64;
                for (uint8_t i = 0; i < 6; i++) {
                    bit64 = binToBCD(tmp[i]);
                    if ((m64->byte[7] == DLE) || (m64->byte[7] == ETX)) buf[len++] = DLE;
                    buf[len++] = m64->byte[7];
                }

                buf[len++] = ETX;

                buf[len] = LRC(buf + 1, len - 1);
                //
                ret = len + 1;
            }
            break;
            case CMD_SET_DATE://57//"set_date"//Программирование даты//Команда:<64h><День(1)><Месяц(1)><Год(1)>//Ответ:<55h><Код Ошибки(1)><Расширенный код ошибки(1)>
            {
                int len = 0, dl = strlen(arg);
                if (!dl) break;
                uint8_t tmp[3] = {0};
                char *uks = &arg[0];
                char *uke = strchr(uks, '^');
                if (!uke) break;
                dl = uke - uks; if (dl > 4) dl = 4;
                memset(one, 0, sizeof(one));
                strncpy(one, uks, dl);
                uint8_t day = (uint8_t)atoi(one);//День(1)
                if (!day || (day > 31)) break;//Допустимые значения 01..31.
                tmp[len++] = day;

                uks = uke + 1;//указатель на Месяц(1)
                uke = strchr(uks, '^');
                if (!uke) break;
                dl = uke - uks; if (dl > 4) dl = 4;
                memset(one, 0, sizeof(one));
                strncpy(one, uks, dl);
                uint8_t mon = (uint8_t)atoi(one);//Месяц(1)
                if (!mon || (mon > 12)) break;//Допустимые значения 01..12
                tmp[len++] = mon;

                uks = uke + 1;//указатель на Год(1)
                uke = strchr(uks, '\0');
                if (!uke) break;
                dl = uke - uks; if (dl > 4) dl = 4;
                memset(one, 0, sizeof(one));
                strncpy(one, uks, dl);
                uint8_t year = (uint8_t)atoi(one);//Год(1)
                if (year > 99) break;//Допустимые значения 00..99
                tmp[len++] = year;
                //
                len = 0;
                buf[len++] = STX;
                byte = passwd >> 8;
                if ((byte == DLE) || (byte == ETX)) buf[len++] = DLE;
                buf[len++] = byte;
                byte = passwd & 0xff;
                if ((byte == DLE) || (byte == ETX)) buf[len++] = DLE;
                buf[len++] = byte;

                buf[len++] = inCMD[idx].cmd;

                uint64_t bit64 = 0;
                s_bit64_t *m64 = (s_bit64_t *)&bit64;
                for (uint8_t i = 0; i < 3; i++) {
                    bit64 = binToBCD(tmp[i]);
                    if ((m64->byte[7] == DLE) || (m64->byte[7] == ETX)) buf[len++] = DLE;
                    buf[len++] = m64->byte[7];
                }

                buf[len++] = ETX;

                buf[len] = LRC(buf + 1, len - 1);
                //
                ret = len + 1;
            }
            break;
            case CMD_SET_TIME://58//"set_time"//57//Программирование времени//Команда:<4Bh><Час(1)><Минута(1)><Секунда(1)>//Ответ:<55h><Код Ошибки(1)><Расширенный код ошибки(1)>
            {
                int len = 0, dl = strlen(arg);
                if (!dl) break;
                uint8_t tmp[3] = {0};
                char *uks = &arg[0];//указатель на Час(1)
                char *uke = strchr(uks, '^');
                if (!uke) break;
                dl = uke - uks; if (dl > 4) dl = 4;
                memset(one, 0, sizeof(one));
                strncpy(one, uks, dl);
                uint8_t hour = (uint8_t)atoi(one);//Час(1)
                if (hour > 23) break;//Допустимые значения 00..23
                tmp[len++] = hour;

                uks = uke + 1;//указатель на Минута(1)
                uke = strchr(uks, '^');
                if (!uke) break;
                dl = uke - uks; if (dl > 4) dl = 4;
                memset(one, 0, sizeof(one));
                strncpy(one, uks, dl);
                uint8_t min = (uint8_t)atoi(one);//Минута(1)
                if (min > 59) break;//Допустимые значения 00..59
                tmp[len++] = min;

                uks = uke + 1;//указатель на Секунда(1)
                uke = strchr(uks, '\0');
                if (!uke) break;
                dl = uke - uks; if (dl > 4) dl = 4;
                memset(one, 0, sizeof(one));
                strncpy(one, uks, dl);
                uint8_t sec = (uint8_t)atoi(one);//Секунда(1)
                if (sec > 59) break;//Допустимые значения 00..59
                tmp[len++] = sec;
                //
                len = 0;
                buf[len++] = STX;
                byte = passwd >> 8;
                if ((byte == DLE) || (byte == ETX)) buf[len++] = DLE;
                buf[len++] = byte;
                byte = passwd & 0xff;
                if ((byte == DLE) || (byte == ETX)) buf[len++] = DLE;
                buf[len++] = byte;

                buf[len++] = inCMD[idx].cmd;

                uint64_t bit64 = 0;
                s_bit64_t *m64 = (s_bit64_t *)&bit64;
                for (uint8_t i = 0; i < 3; i++) {
                    bit64 = binToBCD(tmp[i]);
                    if ((m64->byte[7] == DLE) || (m64->byte[7] == ETX)) buf[len++] = DLE;
                    buf[len++] = m64->byte[7];
                }

                buf[len++] = ETX;

                buf[len] = LRC(buf + 1, len - 1);
                //
                ret = len + 1;
            }
            break;
        }
    }

    return ret;
}
//-----------------------------------------------------------------------
uint16_t getPASSWD(char *pass)
{
uint16_t ret;

    uint64_t bit64 = binToBCD(atoll(pass));
    s_bit64_t *m64 = (s_bit64_t *)&bit64;
    memcpy((uint8_t *)&ret, &m64->byte[6], 2);

    return htons(ret);
}
//-----------------------------------------------------------------------
int parse_ANSWER(uint8_t *buf, int len, int idx)
{
int ret = RET_NONE_ERROR;//-2;
static char one[sdef]  = {0};

    if (idx < total_inCMD) {
        int dl = len - 2;
        uint8_t iCRC = buf[len - 1];
        uint8_t cCRC = LRC(buf + 1, dl);
        if (iCRC != cCRC) {
            if (dbg != LOG_OFF) print_msg(1, "Error CRC: 0x%02X/0x%02X\n", iCRC, cCRC);
            return RET_MINOR_ERROR;//1;
        }
        uint8_t *data = (uint8_t *)calloc(1, dl + 1);
        if (!data) return ret;

        int i = 0, j = 0;
        while (i < dl) {
            if (buf[i] == DLE) i++;
            data[j++] = buf[i++];
        }
        dl = j;
        if (dbg > LOG_DEBUG) {
            char * stz = (char *)calloc(1, (dl << 2) + 64);
            if (stz) {
                sprintf(stz, "ackData after DLE filter (%d):", dl);
                for (i = 0; i < dl; i++) sprintf(stz+strlen(stz), " %02X", data[i]);
                print_msg(1, "%s\n", stz);
                free(stz);
            }
        }
        switch (idx) {
            case  CMD_REQUEST_STATUS://0:// {0x3F, 10, "request_status"},//Запрос состояния ККТ
            {
                if (dl < sizeof(s_ack_stat)) {
                    ret = RET_MINOR_ERROR;//1;
                } else {
                    ret = RET_OK;
                    if (dbg != LOG_OFF) {
                        s_ack_stat *ack_stat = (s_ack_stat *)(data + 1);
                        uint64_t itogo = 0; //memcpy(&itogo, &ack_stat->itogo[0],   5); itogo = htobe64(itogo); itogo >>= 24;
                        for (i = 0; i < 5; i++) {
                            itogo |= ack_stat->itogo[i];
                            if (i != 4) itogo <<= 8;
                        }
                        itogo = htobe64(itogo);
                        double summa = itogo;
                        char temp[tmp_size] = {0};
                        if (ack_stat->coma > 0) summa /= (1 << (ack_stat->coma&3));
                        print_msg(0, "\tack:0x%02X\n\tcassir:%u\n\tnum_in_holl:%u\n"
                                     "\tdataYMD:%02x.%02x.%02x timeHMS:%02x:%02x:%02x\n"
                                     "\tflags:0x%02X %s\n"
                                     "\tmanNUM:%u\n\tmodel:%u\n\tmode:%u.%u\n"
                                     "\tcheckNUM:%u\n\tshiftNUM:%u\n\tcheckSTAT:%u '%s'\n"
                                     "\titogo:%u/%.2f\n\tcoma:%u\n\tport:%u '%s'\n",
                                     //htons(ack_stat->pass),
                                     ack_stat->ack, ack_stat->cassir, ack_stat->num_in_holl,
                                     ack_stat->dataYMD[0], ack_stat->dataYMD[1], ack_stat->dataYMD[2],
                                     ack_stat->timeHMS[0], ack_stat->timeHMS[1], ack_stat->timeHMS[2],
                                     ack_stat->flags, statFlagsStr(ack_stat->flags, temp),
                                     ack_stat->manNUM, ack_stat->model, (ack_stat->mode & 0x0f), (ack_stat->mode >> 4),
                                     ack_stat->checkNUM, ack_stat->shiftNUM, ack_stat->checkSTAT, checkSTATUS(ack_stat->checkSTAT),
                                     itogo, summa, ack_stat->coma, ack_stat->port, iface(ack_stat->port));
                    }
                }
            }
            break;
            case CMD_LINE_PRINT://1:// {0x4C, 10, "line_print"},//Печать текстовых документов
            case CMD_END_OF_PRINT://2:// {0x75, 10, "end_of_print"},//Команда отрезать чек
            case CMD_BEEP://3:// {0x47, 10, "beep"},//Звуковой сигнал
            case CMD_RESTART_DEVICE://4:// {0xCE, 10, "restart_device"}//Рестарт ККТ
            case CMD_SHIFT_OPEN://5:// {0x9A, 10, "shift_open"},//Открыть смену
            case CMD_CLISHE_PRINT://6:// {0x6C, wait_ack_max_sec, "clishe_print"}//Печать клише чека
            case CMD_BUTTOM_PRINT://7:// {0x73, wait_ack_max_sec, "buttom_print"}//Печать нижней части чека
            case CMD_SH_CODE_PRINT://8:// {0x7C, wait_ack_max_sec, "sh_code_print"},//Печать штрихкода по номеру
            case CMD_SHIFT_CLOSE://10:// {0x5A, wait_ack_max_sec, "shift_close"}//Закрыть смену
            case CMD_DEMO_PRINT://11:// {0x82, wait_ack_max_sec, "demo_print"}//Демонстрационная печать
            case CMD_GET_PRINTER_ERROR://13:// {0xB3, wait_ack_max_sec, "get_printer_error"}//Получить последний код ошибки
            case CMD_OPEN_CHEK://17:// {0x92, wait_ack_max_sec,"open_chek"}//Открыть чек (стр.118)
            case CMD_CANCEL_CHEK://18:// {0x59, wait_ack_max_sec, "cancel_chek"// Команда аннулирует (отменяет) текущий открытый чек.
            case CMD_COMING_MONEY://19://{0x49, wait_ack_max_sec, "coming_money"}//Внесение денег <49h><Флаги(1)><Сумма(5)>
            case CMD_PAYOUT_MONEY://20://{0x4F, wait_ack_max_sec, "payout_money"}//Выплата денег// <4Fh><Флаги(1)><Сумма(5)>
            case CMD_CLOSE_CHEK://21://{0x4A, wait_ack_max_sec, "close_chek"}// Закрыть чек (со сдачей) <4Ah><Флаги(1)><Форма расчета(1)><Внесенная сумма(5)>
            case CMD_REG_ITEM://22://{0xE6, wait_ack_max_sec, "reg_item"}//Регистрация позиции//Нужно задать 10 параметров
            case CMD_ENTER_MODE://23://{0x56, wait_ack_max_sec, "enter_mode"},//Вход в режим//<56h> <Режим(1)><Пароль(4)>
            case CMD_EXIT_MODE://24://{0x48, wait_ack_max_sec, "exit_mode"}//Выход из текущего режима//<48h>
            case CMD_INIT_TABLES://27://{0x71, wait_ack_max_sec, "init_tables"}//Инициализация таблиц начальными значениями//Команда: <71h>
            case CMD_PIC_PRINT://29//"pic_print"}//Печать картинки по номеру//Команда:<8Dh><Принтер(1)><Номер(1)><Смещение(2)>
            case CMD_SET_DISCOUNT://39//"set_discount" - Скидка//Команда:<43h><Флаги(1)><Область(1)><Тип(1)><Знак(1)><Размер(5)>
            case CMD_REG_TAX_CHEK://40//"reg_tax_chek" - Регистрация налога на весь чек
            case CMD_BEGIN_GET_REPORT://43//"begin_get_report" - Начало снятия отчета//Команда:<67h><Тип Отчета(1)>
            case CMD_GENERAL_CANCEL://44//"general_cancel"//Общее гашение//Команда:<77h>
            case CMD_TEH_CLEAR://45//"teh_clear"//Технологическое обнуление ККТ//Команда:<6Bh>
            case CMD_SET_PROT_CODE://46//"set_prot_code" - Ввод кода защиты ККТ//Команда:<6Dh><Номер(1)><Код(Х)>
            case CMD_REPRINT_LAST_DOC://49//"reprint_last_doc"//Повторная печать последнего документа//Команда:<95h>
            case CMD_REPRINT_REPORT://50//"reprint_report"//Допечатать отчет//Команда:<EEh>
            case CMD_CLEAR_BUFFER://51//"clear_buffer"//Очистить буфер последнего документа//Команда:<97h>
            case CMD_ACTIVATION_FN://52//"activation_FN"}//52//Активизация ФН//Команда:<A6h>
            case CMD_CLOSE_FN://53//"close_FN"}//52//Закрытие архива ФН//Команда:<A7h>
            case CMD_PRINT_REP_REG://54//"print_rep_reg"}//53//Печать итогов регистрации/перерегистрации ККТ//Команда:<A8h>
            case CMD_PRINT_DOC_BY_NUM://55//"print_doc_by_num"//Печать документа по номеру//Команда:<ABh><Номер документа(5)>
            case CMD_SET_DATE_TIME://56//"set_date_time"//Программирование даты и времени//Команда:<EDh><День(1)><Месяц(1)><Год(1)><Час(1)><Минута(1)><Секунда(1)>
            case CMD_WRITE_TABLES://57//"write_tables"//Программирование таблицы //<50h><Таблица(1)><Ряд(2)><Поле(1)><Значение (Х)>
            case CMD_SET_DATE://57//"set_date"//Программирование даты//Команда:<64h><День(1)><Месяц(1)><Год(1)>
            case CMD_SET_TIME://58//"set_time"//57//Программирование времени//Команда:<4Bh><Час(1)><Минута(1)><Секунда(1)>
            {
                if (dl < sizeof(s_ack_errs)) {
                    ret = RET_MINOR_ERROR;//1;
                } else {
                    s_ack_errs *ack_errs = (s_ack_errs *)(data + 1);
                    if (dbg != LOG_OFF) print_msg(0, "\tack:0x%02X\n\terr(0x%02X):'%s'\n\text_err:0x%02X\n",
                                                     ack_errs->ack, ack_errs->err, ErrToStr(ack_errs->err), ack_errs->ext_err);
                    ret = (ack_errs->ext_err << 8) | ack_errs->err;
                }
            }
            break;
            case CMD_REQUEST_CODE_STATUS://9:// {0x45, wait_ack_max_sec, "request_code_status"}//Запрос кода состояния ККТ
            {
                if (dl < sizeof(s_ack_code_stat)) {
                    ret = RET_MINOR_ERROR;//1;
                } else {
                    char tmp[tmp_size];
                    s_ack_code_stat *ack_code_stat = (s_ack_code_stat *)(data + 1);
                    sprintf(tmp, "\tack:0x%02X\n\tmode:%u.%u\n\terr_flags:0x%02X\n",
                                  ack_code_stat->ack,
                                  (ack_code_stat->mode & 0x0f) , (ack_code_stat->mode >> 4),
                                  ack_code_stat->err_flags);
                    if (ack_code_stat->err_flags > 0) {
                        uint8_t byte = ack_code_stat->err_flags;
                        s_ack_code_stat_flags *flags = (s_ack_code_stat_flags *)&byte;
                        if (flags->paper) strcat(tmp, "\t\t0 Нет бумаги в принтере чеков\n");
                        if (flags->prn_link) strcat(tmp, "\t\t1 Нет связи с принтером\n");
                        if (flags->prn_hard_err) strcat(tmp, "\t\t2 Механическая ошибка принтера\n");
                        if (flags->cat_err) strcat(tmp, "\t\t3 Ошибка отрезчика\n");
                        if (flags->prn_temp_err) strcat(tmp, "\t\t4 Перегрев принтера\n");
                        if (flags->paper_err) strcat(tmp, "\t\t5 Ошибка бумаги\n");
                        if (flags->pres_err) strcat(tmp, "\t\t6 Ошибка презентора\n");
                        if (flags->paper_out) strcat(tmp, "\t\t7 Ошибка окончания бумаги\n");
                    }
                    if (dbg != LOG_OFF) print_msg(0, tmp);
                    ret = ack_code_stat->err_flags;
                }

            }
            break;
            case CMD_GET_DEV_TYPE://12://{0xA5, wait_ack_max_sec, "get_dev_type"}//Получить тип устройства
            {
                if (dl < sizeof(s_ack_dev_type)) {
                    ret = RET_MINOR_ERROR;//1;
                } else {
                    s_ack_dev_type *ack_dev_type = (s_ack_dev_type *)(data + 1);
                    ret = ack_dev_type->err;
                    int dln = dl - sizeof(s_ack_dev_type) - 1;
                    if (dl > 0) {
                        if (!strlen(codePage)) strcpy(codePage, codeCP866);
                        dl = conv_text((const char *)codePage, codeUTF8, (char *)(data + sizeof(s_ack_dev_type) + 1), one, dln);
                    }
                    if (dbg != LOG_OFF)
                        print_msg(0, "\terr(0x%02X):'%s'\n\tprot_ver:%u\n\ttype(%u):'%s'\n"
                                     "\tmodel:%u\n\tmode:%u.%u\n\tdev_ver:%x.%x%x.%x%x\n\tname:'%s'\n",
                                 ack_dev_type->err, ErrToStr(ack_dev_type->err), ack_dev_type->prot_ver,
                                 ack_dev_type->type, DevTypeToStr(ack_dev_type->type),
                                 ack_dev_type->model, ack_dev_type->mode[0] , ack_dev_type->mode[1],
                                 ack_dev_type->dev_ver[0], ack_dev_type->dev_ver[1],ack_dev_type->dev_ver[2],
                                 ack_dev_type->dev_ver[3], ack_dev_type->dev_ver[4], one);
                                 //dln, dl, one);
                }
            }
            break;
            case CMD_GET_BAR_PIC://14:// {0x7D, 10, "get_bar_pic"}//Состояние массива штрихкодов и картинок // barcode or picture
            {
                if (dl < sizeof(s_ack_bar_pic)) {
                    ret = RET_MINOR_ERROR;//1;
                } else {
                    s_ack_bar_pic *ack_bar_pic = (s_ack_bar_pic *)(data + 1);
                    if (dbg != LOG_OFF) print_msg(0, "\tack:0x%02X\n\terr(0x%02X):'%s'\n\tfree_bytes:%u\n\tlast_index:%u\n",
                                                     ack_bar_pic->ack, ack_bar_pic->err, ErrToStr(ack_bar_pic->err),
                                                     ntohl(ack_bar_pic->freeBytes), ack_bar_pic->lastIndex);
                    ret = ack_bar_pic->lastIndex;
                }
            }
            break;
            case CMD_GET_VERSION://15:// case 15:// {0x9D, wait_ack_max_sec, "get_version"}//Получение версии
            {
                if (dl < sizeof(s_ack_version)) {
                    ret = RET_MINOR_ERROR;//1;
                } else {
                    s_ack_version *ack_version = (s_ack_version *)(data + 1);
                    if (dbg != LOG_OFF) print_msg(0, "\tack:0x%02X\n\terr(0x%02X):'%s'\n\tver:%x.%x.%02x%02x.%x\n",
                                                     ack_version->ack, ack_version->err, ErrToStr(ack_version->err),
                                                     ack_version->ver, ack_version->sub_ver,
                                                     ack_version->build[0], ack_version->build[1], ack_version->lang);
                    ret = ack_version->err;
                }
            }
            break;
            case CMD_GET_STAT_EXCH://{0xA4, wait_ack_max_sec, "get_stat_exch"}//Получить статус информационного обмена с ФН
            {
                if (dl < sizeof(s_ack_req_stat_exch)) {
                    ret = RET_MINOR_ERROR;//1;
                } else {
                    char stz[sdef] = {0};
                    s_ack_req_stat_exch *ack_req_stat_exch = (s_ack_req_stat_exch *)(data + 1);
                    if (dbg != LOG_OFF) print_msg(0, "\tack:0x%02X\n\terr(0x%02X):'%s'\n"
                                                     "\tstat:%u %s\n\trd_stat:%u %s\n"
                                                     "\ttotalMsg:%u\n\tfirst:%u\n"
                                                     "\tdatetime:%02u.%02u.%02u %02u:%02u\n",
                                                     ack_req_stat_exch->ack, ack_req_stat_exch->err,
                                                     ErrToStr(ack_req_stat_exch->err),
                                                     ack_req_stat_exch->stat, statExchSTR(ack_req_stat_exch->stat, stz),
                                                     ack_req_stat_exch->rd_stat, rdstatExchStr(ack_req_stat_exch->rd_stat),
                                                     ack_req_stat_exch->totalMsg, ack_req_stat_exch->first,
                                                     ack_req_stat_exch->datetime[0], ack_req_stat_exch->datetime[1],
                                                     ack_req_stat_exch->datetime[2], ack_req_stat_exch->datetime[3],
                                                     ack_req_stat_exch->datetime[4]);
                    ret = ack_req_stat_exch->err;
                }

            }
            break;
            case CMD_CASH_REQUEST://{0x4D, wait_ack_max_sec, "cash_request"}//Запрос наличных//Команда: <4Dh>
            {
                if (dl < sizeof(s_ack_cash_request)) {
                    ret = RET_MINOR_ERROR;//1;
                } else {
                    s_ack_cash_request *cash = (s_ack_cash_request *)(data + 1);
                    sprintf(one, "\tack:0x%02X\n\tmoney:", cash->ack);
                    for (uint8_t i = 0; i < sizeof(cash->money); i++) sprintf(one+strlen(one), "%02X", cash->money[i]);
                    if (dbg != LOG_OFF) print_msg(0, "%s\n", one);
                }
            }
            break;
            case CMD_SHIFT_MONEY://{0x58, wait_ack_max_sec, "get_shift_money"}//Получение последнего сменного итога//Команда: <58h>
            {
                if (dl < sizeof(s_ack_get_shift_money)) {
                    ret = RET_MINOR_ERROR;//1;
                } else {
                    s_ack_get_shift_money *cash = (s_ack_get_shift_money *)(data + 1);
                    sprintf(one, "\tack:0x%02X\n\terr(0x%02X):'%s'\n\tin_money:", cash->ack, cash->err, ErrToStr(cash->err));
                    for (uint8_t i = 0; i < sizeof(cash->in_money); i++) sprintf(one+strlen(one), "%02X", cash->in_money[i]);
                    strcat(one, "\n\tout_money:");
                    for (uint8_t i = 0; i < sizeof(cash->out_money); i++) sprintf(one+strlen(one), "%02X", cash->out_money[i]);
                    if (dbg != LOG_OFF) print_msg(0, "%s\n", one);
                    ret = cash->err;
                }
            }
            break;
            case CMD_READ_TABLES://28//"read_tables"//Чтение таблицы//Команда: <46h><Таблица(1)><Ряд(2)><Поле(1)>
            {
                if (dl < 2) {
                    ret = RET_MINOR_ERROR;//1;
                } else {
                    char tmp[tmp_size];
                    sprintf(tmp, "\tack:0x%02X\n\terr(0x%02X):'%s'", data[1], data[2], ErrToStr(data[2]));
                    if (data[2] == 0) {
                        sprintf(tmp+strlen(tmp), "\n\t'%s' value(%d):", valStr(tbl, column, pole), dl - 3);
                        for (int i = 3; i < dl; i++) sprintf(tmp+strlen(tmp), " %02X", data[i]);
                    }
                    if (dbg != LOG_OFF) print_msg(0, "%s\n", tmp);
                    ret = data[2];
                }
            }
            break;
            case CMD_SHIFT_REQ_PARAM://30//"shift_req_param"//Запрос параметров текущей смены//Команда: <A4h><10h>
            {
                if (dl < sizeof(s_ack_shift_req_param)) {
                    ret = RET_MINOR_ERROR;//1;
                } else {
                    s_ack_shift_req_param *shift = (s_ack_shift_req_param *)(data + 1);
                    if (dbg != LOG_OFF) print_msg(0, "\tack:0x%02X\n\terr(0x%02X):'%s'\n\tshift_stat:%u\n\tshift_num:%u\n\tchek_num:%u\n",
                                 shift->ack, shift->err, ErrToStr(shift->err), shift->sstat, shift->snum, shift->cnum);
                    ret = shift->err;
                }
            }
            break;
            case CMD_GET_STAT_FN://31//"get_stat_FN"//Запрос статуса ФН//Команда: <A4h><30h>
            {
                if (dl < sizeof(s_ack_get_stat_FN)) {
                    ret = RET_MINOR_ERROR;//1;
                } else {
                    s_ack_get_stat_FN *sta = (s_ack_get_stat_FN *)(data + 1);
                    if (dbg != LOG_OFF) {
                        char temp[1024] = {0};
                        print_msg(0, "\tack:0x%02X\n\terr(0x%02X):'%s'\n"
                                     "\tstage_stat(%u):'%s'\n"
                                     "\tcur_doc(%u):'%s'\n"
                                     "\tdata_doc(%u):'%s'\n\tshift_stat(%u):'%s'\n"
                                     "\tflags(0x%02X):'%s'\n\tdatetime:%02u.%02u.%02u %02u:%02u\n"
                                     "\tnumFN:%.*s\n\tnumLastFD:%u\n",
                                     sta->ack, sta->err, ErrToStr(sta->err),
                                     sta->stage_stat, statFNStr(sta->stage_stat),
                                     sta->cur_doc, curDocStr(sta->cur_doc),
                                     sta->data_doc, dataDocStr(sta->data_doc),
                                     sta->shift_stat, shiftStatStr(sta->shift_stat),
                                     sta->flags, alarmStr(sta->flags, temp),
                                     sta->datetime[0], sta->datetime[1],
                                     sta->datetime[2], sta->datetime[3],
                                     sta->datetime[4], 16, sta->numFN, sta->numLastFD);
                    }
                    ret = sta->err;
                }
            }
            break;
            case CMD_GET_NUM_FN://32//"get_num_FN"//Запрос номера ФН//Команда: <A4h><31h>//Ответ: <55h><Код Ошибки (1)><Номер ФН(16)>
            {
                if (dl < sizeof(s_ack_get_num_FN)) {
                    ret = RET_MINOR_ERROR;//1;
                } else {
                    s_ack_get_num_FN *sta = (s_ack_get_num_FN *)(data + 1);
                    if (dbg != LOG_OFF)
                        print_msg(0, "\tack:0x%02X\n\terr(0x%02X):'%s'\n\tnumFN:%.*s\n",
                                     sta->ack, sta->err, ErrToStr(sta->err), 16, sta->numFN);
                    ret = sta->err;
                }
            }
            break;
            case CMD_GET_BLACK_DAY://33//"get_black_day" - Запрос срока действия ФН //Команда: <A4h><32h>
            {
                if (dl < sizeof(s_ack_get_black_day)) {
                    ret = RET_MINOR_ERROR;//1;
                } else {
                    s_ack_get_black_day *sta = (s_ack_get_black_day *)(data + 1);
                    if (dbg != LOG_OFF)
                        print_msg(0, "\tack:0x%02X\n\terr(0x%02X):'%s'\n"
                                     "\tvalidity:%04u.%02u.%02u\n\tleft_reg:%u\n\tdone_reg:%u\n",
                                     sta->ack, sta->err, ErrToStr(sta->err),
                                     sta->validity[0] + 2000, sta->validity[1], sta->validity[1],
                                     sta->left_reg, sta->done_reg);

                    ret = sta->err;
                }
            }
            break;
            case CMD_GET_VER_FN://34//{0xA4, wait_ack_max_sec, "get_ver_FN"}//34//Запрос версии ФН//Команда: <A4h><33h>
            {
                if (dl < sizeof(s_ack_get_ver_FN)) {
                    ret = RET_MINOR_ERROR;//1;
                } else {
                    s_ack_get_ver_FN *sta = (s_ack_get_ver_FN *)(data + 1);
                    if (dbg != LOG_OFF)
                        print_msg(0, "\tack:0x%02X\n\terr(0x%02X):'%s'\n\tverFN:%.*s\n\ttypeSW(%u):'%s'\n",
                                     sta->ack, sta->err, ErrToStr(sta->err), sizeof(sta->verFN), sta->verFN, sta->typeSW, typeSwStr(sta->typeSW));
                    ret = sta->err;
                }
            }
            break;
            case CMD_GET_ERR_FN://{0xA4, wait_ack_max_sec, "get_err_FN"}//35//Запрос последних ошибок ФН
            {
                if (dl < sizeof(s_ack_get_err_FN)) {
                    ret = RET_MINOR_ERROR;//1;
                } else {
                    s_ack_get_err_FN *sta = (s_ack_get_err_FN *)(data + 1);
                    uint8_t *uk = data + 1 + sizeof(s_ack_get_err_FN);
                    char temp[1024] = {0};
                    int data_len = dl - 1 - sizeof(s_ack_get_err_FN);
                    sprintf(temp, "\tack:0x%02X\n\terr(0x%02X):'%s'\n\tblks:%u\n\tdata(%u):\n",
                                  sta->ack, sta->err, ErrToStr(sta->err), sta->blks, data_len);
                    if (sta->blks) {
                        for (int i = 0; i < data_len; i++) sprintf(temp+strlen(temp), "%02X", *(uk + i));
                        strcat(temp, "\n");
                    }
                    if (dbg != LOG_OFF) print_msg(0, temp);
                    int f = open("get_err_FN.txt", O_WRONLY | O_CREAT | O_TRUNC, 0664);
                    if (f > 0) {
                        write(f, temp, strlen(temp));
                        close(f);
                    }
                    ret = sta->err;
                }
            }
            break;
            case CMD_GET_NOACK_FD://36//"get_noack_FD" - //Запрос количества ФД, на которые нет квитанции (стр.90)//Команда: <A4h><42h>
            {
                if (dl < sizeof(s_ack_get_noack_FD)) {
                    ret = RET_MINOR_ERROR;//1;
                } else {
                    s_ack_get_noack_FD *sta = (s_ack_get_noack_FD *)(data + 1);
                    if (dbg != LOG_OFF)
                        print_msg(0, "\tack:0x%02X\n\terr(0x%02X):'%s'\n\tnoackFD:%u\n",
                                     sta->ack, sta->err, ErrToStr(sta->err), sta->noackFD);
                    ret = sta->err;
                }
            }
            break;
            case CMD_SET_TEXT_ATTR://38//"set_text attr" - Запись реквизита//Ответ: <55h><Код ошибки (1)>
            {
                if (dl < sizeof(s_ack_set_text_attr)) {
                    ret = RET_MINOR_ERROR;//1;
                } else {
                    s_ack_set_text_attr *sta = (s_ack_set_text_attr *)(data + 1);
                    if (dbg != LOG_OFF)
                        print_msg(0, "\tack:0x%02X\n\terr(0x%02X):'%s'\n", sta->ack, sta->err, ErrToStr(sta->err));
                    ret = sta->err;
                }
            }
            break;
            case CMD_GET_TEXT_ATTR://37//"get_text_attr" - Чтение реквизита//Команда:<E9h><Номер реквизита(2)><Номер блока(1)>
            {
                if (dl < sizeof(s_ack_get_text_attr)) {
                    ret = RET_MINOR_ERROR;//1;
                } else {
                    s_ack_get_text_attr *sta = (s_ack_get_text_attr *)(data + 1);
                    uint8_t *uk = data + 1 + sizeof(s_ack_get_text_attr);
                    char temp[1024] = {0};
                    int data_len = dl - 1 - sizeof(s_ack_get_text_attr);
                    sprintf(temp, "\tack:0x%02X\n\terr(0x%02X):'%s'\n\tnblks:%u\n\tdata(%u):\n",
                                  sta->ack, sta->err, ErrToStr(sta->err), sta->nblk, data_len);
                    if (data_len > 0) {
                        char *name = NULL;
                        s_tlv *hdt = (s_tlv *)uk;
                        int slen = sizeof(s_tlv);
                        int8_t itype = getTagType(hdt->tag, &name);
                        sprintf(temp+strlen(temp), "\t  tag(%u):'%s'\n\t  len:%u\n", hdt->tag, name, hdt->len);
                        if (hdt->len & 0x1ff) {
                            if (!itype) {//tag has string type
                                char stx[512] = {0};
                                // from cp866 to utf-8
                                int idl = conv_text((const char *)codeCP866, codeUTF8, (char *)(uk + slen), stx, hdt->len);
                                //
                                sprintf(temp+strlen(temp),"\t  value:'%.*s'\n", idl, stx);
                            } else {//tag has binary type
                                strcat(temp, "\t  value:");
                                for (int i = slen; i < slen + hdt->len; i++) sprintf(temp+strlen(temp), "%02X", *(uk + i));
                                strcat(temp, "\n");
                            }
                        }
                    }
                    if (dbg != LOG_OFF) print_msg(0, temp);
                    ret = sta->err;
                }

            }
            break;
            case CMD_CALC_BY_CHEK://41//"calc_by_chek"//Расчет по чеку//Команда:<99h><Флаги(1)><Форма расчета(1)><Сумма(5)>
            case CMD_STORNO_CALC_BY_CHEK://42//"storno_calc_by_chek"//Сторно расчета по чеку//Команда:<9Bh><Флаги(1)><Форма расчета(1)><Сумма(5)>
            {
                if (dl < sizeof(s_ack_calc_by_chek)) {
                    ret = RET_MINOR_ERROR;//1;
                } else {
                    s_ack_calc_by_chek *cash = (s_ack_calc_by_chek *)(data + 1);
                    sprintf(one, "\tack:0x%02X\n\tbalance:", cash->ack);
                    for (uint8_t i = 0; i < sizeof(cash->balance); i++) sprintf(one+strlen(one), "%02X", cash->balance[i]);//Остаток(5)
                    strcat(one, "\n\tmoney:");
                    for (uint8_t i = 0; i < sizeof(cash->money); i++) sprintf(one+strlen(one), "%02X", cash->money[i]);//Сдача(5)
                    if (dbg != LOG_OFF) print_msg(0, "%s\n", one);
                }
            }
            break;
            case CMD_STAT_PROT_CODE://47//"stat_prot_code" - Запрос активизированности кода защиты ККТ//Команда:<74h><Номер(1)>
                                    //Ответ:<4Ch><Активизирован(1)>
            {
                if (dl < sizeof(s_ack_set_text_attr)) {
                    ret = RET_MINOR_ERROR;//1;
                } else {
                    s_ack_set_text_attr *sta = (s_ack_set_text_attr *)(data + 1);
                    if (dbg != LOG_OFF) {
                        if (!sta->err) strcpy(one, "Не активизирован");
                                  else strcpy(one, "Активизирован");
                        print_msg(0, "\tack:0x%02X\n\terr(0x%02X):'%s'\n", sta->ack, sta->err, one);
                    }
                    ret = sta->err;
                }
            }
            break;
            case CMD_READ_REGISTER://48//"read_register"//Считать регистр//Команда:<91h><Регистр(1)><Параметр1(1)><Параметр2(1)>//Ответ:<55h><Код Ошибки(1)><Значение(Х)>
            {
                if (dl < sizeof(s_ack_set_text_attr)) {
                    ret = RET_MINOR_ERROR;//1;
                } else {
                    s_ack_set_text_attr *sta = (s_ack_set_text_attr *)(data + 1);
                    if (dbg != LOG_OFF) {
                        char temp[tmp_size] = {0};
                        uint8_t *uk = data + 1 + sizeof(s_ack_set_text_attr);
                        int data_len = dl - 1 - sizeof(s_ack_set_text_attr);
                        sprintf(temp, "\tack:0x%02X\n\terr(0x%02X):'%s'\n\tvalue(%d):", sta->ack, sta->err, ErrToStr(sta->err), data_len);
                        for (int i = 0; i < data_len; i++) sprintf(temp+strlen(temp), "%02X", *(uk + i));
                        print_msg(0, "%s\n", temp);
                    }
                    ret = sta->err;
                }
            }
            break;
        }
        //
        if ((ret == RET_MINOR_ERROR) && (idx < total_inCMD)) {
            if (dl < sizeof(s_ack_errs)) {
                    ret = RET_MINOR_ERROR;//1;
                } else {
                    s_ack_errs *ack_errs = (s_ack_errs *)(data + 1);
                    if (dbg != LOG_OFF) print_msg(0, "\tack:0x%02X\n\terr(0x%02X):'%s'\n\text_err:0x%02X\n",
                                                     ack_errs->ack, ack_errs->err, ErrToStr(ack_errs->err), ack_errs->ext_err);
                    ret = (ack_errs->ext_err << 8) | ack_errs->err;
            }
        }
        //
        free(data);
    }

    return ret;
}
//-----------------------------------------------------------------------
int8_t getTagType(uint16_t tg, char **nm)
{
int8_t ret = -1;
char *adr = NULL;

    for (int i = 0; i < max_tag; i++) {
        if (tg == tags[i].tag) {
            ret = tags[i].type;
            adr = (char *)&tags[i].name[0];
            break;
        }
    }

    *nm = adr;
    return ret;
}
//-----------------------------------------------------------------------
static const char *DevTypeToStr(uint8_t type)
{
    switch (type) {
        case 0:
            return "Тип не определен";
        case 1:
            return "ККТ";
        case 2:
            return "Весы";
        case 3:
            return "Блок Memo Plus™";
        case 4:
            return "Принтер этикеток";
        case 5:
            return "Терминал сбора данных";
        case 6:
            return "Дисплей покупателя";
        case 7:
            return "Сканер штрихкода, PIN-клавиатура, ресторанная клавиатура";
    }

    return "Зарезервировано";
}
//-----------------------------------------------------------------------
static const char *ErrToStr(uint8_t err)
{
    switch (err) {
        case  0://
            return "Ошибок нет";
        case  8:
            return "Неверная цена (сумма)";
        case 10://0Ah
            return "Неверное количество";
        case 11://0Bh
            return "Переполнение счетчика наличности";
        case 12://0Ch
            return "Невозможно сторно последней операции";
        case 13://0Dh
            return "Сторно по коду невозможно";// (в чеке зарегистрировано меньшее количество товаров с указанным кодом)
        case 14://0Eh
            return "Невозможен повтор последней операции";
        case 15://0Fh
            return "Повторная скидка на операцию невозможна";
        case 16://10h
            return "Скидка/надбавка на предыдущую операцию невозможна";
        case 17://11h
            return "Неверный код товара";
        case 18://12h
            return "Неверный штрихкод товара";
        case 19://13h
            return "Неверный формат";
        case 20://14h
            return "Неверная длина";
        case 21://15h
            return "ККТ заблокирована в режиме ввода даты";
        case 22://16h
            return "Требуется подтверждение ввода даты";
        case 24://18h
            return "Нет больше данных для передачи ПО ККТ";
        case 25://19h
            return "Нет подтверждения или отмены регистрации прихода";
        case 26://1Ah
            return "Отчет с гашением прерван";// Вход в режим невозможен.
        case 27://1Bh
            return "Отключение контроля наличности невозможно";// (не настроены необходимые способы расчета).
        case 30://1Eh
            return "Вход в режим заблокирован";
        case 31://1Fh
            return "Проверьте дату и время";
        case 32://20h
            return "Дата и время в ККТ меньше чем в ФН";
        case 33://21h
            return "Невозможно закрыть архив";
        case 61://3Dh
            return "Товар не найден";
        case 62://3Eh
            return "Весовой штрихкод с количеством <>1.000";
        case 63://3Fh
            return "Переполнение буфера чека";
        case 64://40h
            return "Недостаточное количество товара";
        case 65://41h
            return "Сторнируемое количество больше проданного";
        case 66://42h
            return "Заблокированный товар не найден в буфере чека";
        case 67://43h
            return "Данный товар не продавался в чеке, сторно невозможно";
        case 70://46h
            return "Неверная команда от ККТ";
        case 102://66h
            return "Команда не реализуется в данном режиме ККТ";
        case 103://67h
            return "Нет бумаги";
        case 104://68h
            return "Нет связи с принтером чеков";
        case 105://69h
            return "Механическая ошибка печатающего устройства";
        case 106://6Ah
            return "Неверный тип чека";
        case 107://6Bh
            return "Нет больше строк картинки/штрихкода";
        case 108://6Ch
            return "Неверный номер регистра";
        case 109://6Dh
            return "Недопустимое целевое устройство";
        case 110://6Eh
            return "Нет места в массиве картинок/штрихкодов";
        case 111://6Fh
            return "Неверный номер картинки/штрихкода";// (картинка/штрихкод отсутствует)
        case 112://70h
            return "Сумма сторно больше, чем было получено данным способом расчета";
        case 113://71h
            return "Сумма не наличных платежей превышает сумму чека";
        case 114://72h
            return "Сумма платежей меньше суммы чека";
        case 115://73h
            return "Накопление меньше суммы возврата или аннулирования";
        case 117://75h
            return "Переполнение суммы платежей";
        case 118://76h
            return "Предыдущая операция не завершена";
        case 119://77h
            return "Ошибка GSM-модуля";
        case 122://7Ah
            return "Данная модель ККТ не может выполнить команду";
        case 123://7Bh
            return "Неверная величина скидки/надбавки";
        case 124://7Ch
            return "Операция после скидки/надбавки невозможна";
        case 125://7Dh
            return "Неверная секция";
        case 126://7Eh
            return "Неверная форма/способ расчета";
        case 127://7Fh
            return "Переполнение при умножении";
        case 128://80h
            return "Операция запрещена в таблице настроек";
        case 129://81h
            return "Переполнение итога чека";
        case 130://82h
            return "Открыт чек аннулирования – операция невозможна";
        case 132://84h
            return "Переполнение буфера контрольной ленты";
        case 134://86h
            return "Вносимая клиентом сумма меньше суммы чека";
        case 135://87h
            return "Открыт чек возврата – операция невозможна";
        case 136://88h
            return "Смена превысила 24 часа";
        case 137://89h
            return "Открыт чек прихода – операция невозможна";
        case 138://8Ah
            return "Переполнение ФП";
        case 140://8Ch
            return "Неверный пароль";
        case 141://8Dh
            return "Буфер контрольной ленты не переполнен";
        case 142://8Eh
            return "Идет обработка контрольной ленты";
        case 143://8Fh
            return "Обнуленная касса (повторное гашение невозможно)";
        case 145://91h
            return "Неверный номер таблицы";
        case 146://92h
            return "Неверный номер ряда";
        case 147://93h
            return "Неверный номер поля";
        case 148://94h
            return "Неверная дата";
        case 149://95h
            return "Неверное время";
        case 150://96h
            return "Сумма чека по секции меньше суммы сторно";
        case 151://97h
            return "Подсчет суммы сдачи невозможен";
        case 152://98h
            return "В ККТ нет денег для выплаты";
        case 154://9Ah
            return "Чек закрыт – операция невозможна";
        case 155://9Bh
            return "Чек открыт – операция невозможна";
        case 156://9Ch
            return "Смена открыта, операция невозможна";
        case 158://9Eh
            return "Заводской номер/MAC-адрес уже задан";
        case 159://9Fh
            return "Исчерпан лимит перерегистраций";
        case 162://A2h
            return "Неверный номер смены";
        case 163://A3h
            return "Неверный тип отчета";
        case 164://A4h
            return "Недопустимый пароль";
        case 165://A5h
            return "Недопустимый заводской номер ККТ";
        case 166://A6h
            return "Недопустимый РНМ";
        case 167://A7h
            return "Недопустимый ИНН";
        case 168://A8h
            return "ККТ не фискализирована";
        case 169://A9h
            return "Не задан заводской номер";
        case 170://AAh
            return "Нет отчетов";
        case 171://ABh
            return "Режим не активизирован";
        case 172://ACh
            return "Нет указанного чека в ЭЖ";
        case 173://ADh
            return "Нет больше записей в ЭЖ";
        case 174://AEh
            return "Некорректный код или номер кода защиты ККТ";
        case 175://AFh
            return "Отсутствуют данные в буфере ККТ";
        case 176://B0h
            return "Требуется выполнение общего гашения";
        case 177://B1h
            return "Команда не разрешена введенными кодами защиты ККТ";
        case 178://B2h
            return "Невозможна отмена скидки/надбавки";
        case 179://B3h
            return "Невозможно закрыть чек данным способом расчета";// (в чеке присутствуют операции без контроля наличных)
        case 180://B4h
            return "Неверный номер маршрута";
        case 181://B5h
            return "Неверный номер начальной зоны";
        case 182://B6h
            return "Неверный номер конечной зоны";
        case 183://B7h
            return "Неверный тип тарифа";
        case 184://B8h
            return "Неверный тариф";
        case 186://BAh
            return "Ошибка обмена с фискальным модулем";
        case 190://BЕh
            return "Необходимо провести профилактические работы";
        case 191://BFh
            return "Неверные номера смен в ККТ и ФН";
        case 200://C8h
            return "Нет устройства, обрабатывающего данную команду";
        case 201://C9h
            return "Нет связи с внешним устройством";
        case 202://CAh
            return "Ошибочное состояние ТРК";
        case 203://CBh
            return "Больше одной регистрации в чеке";
        case 204://CСh
            return "Ошибочный номер ТРК";
        case 205://CDh
            return "Неверный делитель";
        case 208://D0h
            return "Активизация данного ФН в составе данной ККТ невозможна";
        case 209://D1h
            return "Перегрев головки принтера";
        case 210://D2h
            return "Ошибка обмена с ФН на уровне интерфейса I2C";
        case 211://D3h
            return "Ошибка формата передачи ФН";
        case 212://D4h
            return "Неверное состояние ФН";
        case 213://D5h
            return "Неисправимая ошибка ФН";
        case 214://D6h
            return "Ошибка КС ФН";
        case 215://D7h
            return "Закончен срок эксплуатации ФН";
        case 216://D8h
            return "Архив ФН переполнен";
        case 217://D9h
            return "В ФН переданы неверная дата или время";
        case 218://DAh
            return "В ФН нет запрошенных данных";
        case 219://DBh
            return "Переполнение ФН (итог чека)";
        case 220://DCh
            return "Буфер переполнен";
        case 221://DDh
            return "Невозможно напечатать вторую фискальную копию";
        case 223://DFh
            return "Сумма налога больше суммы регистраций по чеку";// и/или итога или больше суммы регистрации
        case 224://E0h
            return "Начисление налога на последнюю операцию невозможно";
        case 225://E1h
            return "Неверный номер ФН";
        case 228://E4h
            return "Сумма сторно налога больше суммы зарегистрированного налога данного типа";
        case 230://E6h
            return "Операция невозможна, недостаточно питания";
        case 231://E7h
            return "Некорректное значение параметров команды ФН";
        case 232://E8h
            return "Превышение размеров TLV данных ФН";
        case 233://E9h
            return "Нет транспортного соединения ФН";
        case 234://EAh
            return "Исчерпан ресурс КС ФН";
        case 235://EBh
            return "Исчерпан ресурс хранения ФД для отправки ОФД";
        case 236://ECh
            return "Сообщение от ОФД не может быть принято ФН";
        case 237://EDh
            return "В ФН есть неотправленные ФД";
        case 238://EEh
            return "Запросить расширенный код ошибки в регистре 55";
        case 239://EFh
            return "Исчерпан ресурс Ожидания передачи сообщения в ФН";
        case 240://F0h
            return "Продолжительность смены ФН более 24 часов";
        case 241://F1h
            return "Неверная разница во времени между двумя операциями ФН";
        case 242://F2h
            return "Неверная команда";
        case 243://F3h
            return "Количество позиций превысило разрешенный лимит";
        case 244://F4h
            return "Отсутствуют данные в команде";
        case 245://F5h
            return "Продажа подакцизного товара";
    }

    return "???";
}
//----------------------------------------------------------------------
char *statExchSTR(uint8_t fl, char *st)
{
    if (st) {
        if (fl & 1)    strcat(st, "\n\t  Транспортное соединение установлено");//bit0
        if (fl & 2)    strcat(st, "\n\t  Есть сообщение для передачи в ОФД");//bit1
        if (fl & 4)    strcat(st, "\n\t  Ожидание ответного сообщения (квитанции) от ОФД");//bit2
        if (fl & 8)    strcat(st, "\n\t  Есть команда от ОФД");//bit3
        if (fl & 0x10) strcat(st, "\n\t  Изменились настройки соединения с ОФД");//bit4
        if (fl & 0x20) strcat(st, "\n\t  Ожидание ответа на команду от ОФД");//bit5
    }

    return st;
}
//----------------------------------------------------------------------
static const char *rdstatExchStr(uint8_t type)
{
    switch (type) {
        case 0:
            return "\n\t  Чтение сообщения для ОФД не производится";
        case 1:
            return "\n\t  Производится чтение сообщения для ОФД";
    }

    return "\n\t  ???";
}
//----------------------------------------------------------------------
uint64_t binToBCD(long long bin)
{
long long shi = 0;
uint64_t bcd = 0;

    while (bin > 0) {
        bcd |= (bin % 10) << (shi++ << 2);
        bin /= 10;
    }

   return htobe64(bcd);
}
//----------------------------------------------------------------------
static const char *valStr(uint8_t t, uint16_t c, uint8_t p)
{
    switch (t) {//bt tables
        case 2://table # 2
            switch (c) {//by column
                case 1:
                    switch (p) {//by pole
                        case 1 ://Номер ККТ в магазине//1 BIN//1..255 DEF: 1
                            return "Номер ККТ в магазине";
                        break;
                        case 23 ://Пароль доступа//2 BCD//0000 .. 9999 DEF: 0000
                            return "Пароль доступа";
                        break;
                        case 85 ://Протокол работы ККТ2//1 BIN//0 – АТОЛ 2.4, 2 – АТОЛ 3.0, DEF: 2
                            return "Протокол работы ККТ";
                        break;
                        case 87 ://MAC-адрес (Ethernet)//6 BIN//DEF: FF:FF:FF:FF:FF:FF
                            return "MAC-адрес (Ethernet)";
                        break;
                        case 88 ://IP-адрес (Ethernet)//4 BIN//DEF: 192.168.10.1
                            return "IP-адрес (Ethernet)";
                        break;
                        case 89 ://Маска подсети (Ethernet)//4 BIN//DEF: 255.255.255.0
                            return "Маска подсети (Ethernet)";
                        break;
                        case 90 ://Шлюз (Ethernet)//4 BIN//DEF: 192.168.10.0
                            return "Шлюз (Ethernet)";
                        break;
                        case 91 ://Порт (Ethernet)//2 BCD//0000..9999 DEF: 5555
                            return "Порт (Ethernet)";
                        break;
                        case 107 ://Динамический тип IP-адреса//1 BIN
                                  //0 – отключен (используется статический тип IP-адреса) 1 – включен DEF: 1
                            return "Динамический тип IP-адреса (off/on)";
                        break;
                    }//pole
                break;
            }//column
        break;
        case 3://Таблица 3 «Пароли кассиров и администраторов»
            switch (c) {//by column
                case 1: case 2: case 3: case 4: case 5: case 6: case 7: case 8: case 9: case 10: case 11: case 12:
                case 13: case 14: case 15: case 16: case 17: case 18: case 19: case 20: case 21: case 22: case 23:
                case 24: case 25: case 26: case 27: case 28:
                    switch (p) {//by pole
                        case 1 ://Пароли кассиров//4 BCD
                            return "Пароли кассиров";
                        break;
                        case 2 ://Имя кассира//57 CHAR
                            return "Имя кассира";
                        break;
                        case 3 ://ИНН кассира//12 ASCII
                            return "ИНН кассира";
                        break;
                    }
                break;
                case 29:
                    switch (p) {//by pole
                        case 1 ://Пароль администратора//4 BCD
                            return "Пароль администратора";
                        break;
                        case 2 ://Имя администратора//57 CHAR
                            return "Имя администратора";
                        break;
                        case 3 ://ИНН администратора//12 ASCII
                            return "ИНН администратора";
                        break;
                    }
                break;
                case 30:
                    switch (p) {//by pole
                        case 1 ://Пароль системного администратора//4 BCD
                            return "Пароль системного администратора";
                        break;
                        case 2 ://Имя системного администратора//57 CHAR
                            return "Имя системного администратора";
                        break;
                        case 3 ://ИНН системного администратора//12 ASCII
                            return "ИНН системного администратора";
                        break;
                    }
                break;
            }

        break;
        case 4://Таблица 4 «Параметры скидок и надбавок»
            switch (c) {//by column
                case 1 ://
                    switch (p) {//by pole
                        case 1 ://Режим работы скидок//1 BCD
                                //0 – скидки запрещены,1 – разрешены скидки на весь чек,
                                //2 – разрешены скидки на позицию, 3 – разрешены все скидки
                                //DEF: 3
                            return "Режим работы скидок";
                        break;
                    }
                break;
                case 2 ://
                    switch (p) {//by pole
                        case 1 ://Режим работы надбавок//1 BCD
                                //0 – надбавки запрещены, 1 – разрешены надбавки на весь чек
                                //2 – разрешены надбавки на позицию, 3 – разрешены все надбавки
                                //DEF: 3
                            return "Режим работы надбавок";
                        break;
                    }
                break;
            }
        break;
        case 6://Таблица 6 «Текст в чеке»
            switch (c) {//by column
                case 1 : case 2: case 3: case 4: case 5: case 6: case 7: case 8: case 9: case 10: case 11:
                case 12: case 13: case 14: case 15: case 16: case 17: case 18: case 19: case 20:
                    switch (p) {//by pole
                        case 1 ://Запрограммированное значение строки клише или рекламного текста//57 CHAR
                                //DEF:ряды 4..5, 9..20: 57 пробелов,ряд 1: СПАСИБО,ряд 2: ЗА ПОКУПКУ!
                                //ряды 3, 8: 57 символов «—», ряд 6: ТОРГОВЫЙ ОБЪЕКТ №1,ряд 7: ДОБРО ПОЖАЛОВАТЬ!
                            return "Значение строки клише или рекламного текста";
                        break;
                    }
                break;
            }
        break;
        case 7://Таблица 7 «Наименования секций»
            switch (c) {//by column
                case 1: case 2: case 3: case 4: case 5: case 6: case 7: case 8:
                case 9: case 10: case 11: case 12: case 13: case 14: case 15: case 16:
                    switch (p) {//by pole
                        case 1 ://Запрограммированное наименование секции//57 CHAR//ряд 1: СЕКЦИЯ 01
                                //Запрограммированное наименование секции//57 CHAR//ряд 1: СЕКЦИЯ 02
                                //...
                                //Запрограммированное наименование секции//57 CHAR//ряд 1: СЕКЦИЯ 16
                            return "Наименование секции";
                        break;
                        case 2 ://Номер налога//1 BCD//DEF: 1
                            return "Номер налога";
                        break;
                    }
                break;
            }
        break;
        case 9://Таблица 9 «Подключенное оборудование»
            switch (c) {//by column
                case 1:
                    switch (p) {//by pole
                        case 1 : //Тип устройства//1 BCD//DEF: 3
                            return "Тип устройства";
                        break;
                        case 2 : //Скорость устройства//1 BCD//1-1200,2-2400,3-4800,4-9600,5-14400,6-38400,7-57600,8-115200,9-19200,DEF: 8
                            return "Скорость устройства";
                        break;
                    }//pole
                break;
                case 2:
                    switch (p) {//by pole
                        case 1 : //Тип устройства//1 BCD//DEF: 3
                            return "Тип устройства";
                        break;
                        case 2 : //Скорость устройства//1 BCD//1-1200,2-2400,3-4800,4-9600,5-14400,6-38400,7-57600,8-115200,9-19200,DEF: 8
                            return "Скорость устройства";
                        break;
                    }//pole
                break;
                case 4:
                    switch (p) {//by pole
                        case 1 : //0 – ККТ является пассивным устройством по интерфейсу RS-232
                                 //4 – ККТ является пассивным устройством по интерфейсу USB
                                 //6 – ККТ является пассивным устройством по интерфейсу Ethernet
                                 //7 – ККТ является пассивным устройством по интерфейсу WiFi; DEF: 4
                            return "Тип устройства";
                        break;
                    }//pole

                break;
            }//column
        break;
        case 10://Таблица 10 «Защита ККТ»
            switch (c) {//by column
                case 1: case 2: case 3: case 4: case 5: case 6: case 7: case 8: case 9: case 10:
                case 11: case 12: case 13: case 14: case 15: case 16: case 17: case 18: case 19: case 20:
                case 21: case 22: case 23: case 24: case 25: case 26: case 27: case 28: case 29: case 30:
                    switch (p) {//by pole
                        case 1 ://Код защиты ККТ //8 BCD//0000000000000000 .. 9999999999999999  DEF: 0
                            return "Код защиты ККТ";
                        break;
                    }
                break;
            }//column
        break;
        case 12://Таблица 12 «Наименования форм и способов расчета»
            switch (c) {//by column
                case 1: case 2: case 3: case 4: case 5: case 6: case 7: case 8: case 9:
                    switch (p) {//by pole
                        case 1 ://c=1 : Наименование способа расчета 2..10//64 CHAR//DEF: Безналичными..ТИП 10
                            return "Способ расчета";
                        break;
                        case 2 ://Номер формы расчета//1 BIN
                            return "Номер формы расчета";
                        break;

                    }
                break;
            }//column
        break;
        case 13://Таблица 13 «Налоги»
            switch (c) {//by column
                case 1: case 2: case 3: case 4: case 5: case 6:
                    switch (p) {//by pole
                        case 1:
                            return "Название налога";//-//НДС 20%
                        break;
                        case 2:
                            return "Ставка налога";//2 BCD//20 00
                        break;
                        case 3:
                            return "Тег налога";//2 BIN(старший первым)//1102
                        break;
                    }
                break;
            }
        break;
        case 15://Таблица 15 «Наименования реквизитов»
            if ((c >= 2) && (c <= 100)) {
                if (p == 1) {
                    return "Наименование реквизита";//25 CHAR
                }
            }
        break;
        case 17://Таблица 17 «Настройка беспроводных соединений»
            switch (c) {
                case 1:
                    if (p == 1) return "Конфигурационная строка";//64 CHAR//DEF: m2m.beeline.ru
                break;
                case 2:
                    if (p == 1) return "Имя пользователя";//64 CHAR//DEF: beeline
                break;
                case 3:
                    if (p == 1) return "Пароль";//64 CHAR//DEF: beeline
                break;
                case 4:
                    if (p == 1) return "Режим WiFi";//64 BCD//1 – Станция, 2 – Точка доступа//DEF: 1
                break;
                case 5:
                    if (p == 1) return "Имя точки WiFi";//64 CHAR//DEF: Ssid
                break;
                case 6:
                    if (p == 1) return "Пароль к точке доступа";//64 CHAR/DEF: Pswd
                break;
                case 7:
                    if (p == 1) return "Номер канала WiFi в режиме точки доступа";//64 BCD/0..13//DEF: 5
                break;
                case 8:
                    if (p == 1) return "Тип шифрования";//64 BCD//0–Open,1–WEP,2-WPA,3–WPA2,4–WPA/WPA2//DEF: 4
                break;
            }
        break;
        case 19://Таблица 19 «Настройка соединения с ОФД»
            if (c == 1) {
                switch (p) {//by pole
                    case 1://Адрес ОФД//64 CHAR//DEF: 0.0.0.0
                        return "Адрес ОФД";
                    break;
                    case 2://Порт ОФД//2 BIN//(старший байт вперед)//DEF: 7777
                        return "Порт ОФД";
                    break;
                    case 3://DNS ОФД//4 BIN//DEF: 0.0.0.0
                        return "DNS ОФД";
                    break;
                    case 4://Канал обмена с ОФД//1 BIN
                           //1–EthernetOverUsb,2–Ethernet,3–Wifi,4–GSM модем,5–EthernetOver Transport//DEF: 1
                        return "Канал обмена с ОФД";
                    break;
                    case 8://Адрес сайта для проверки ФПД (для печати в чеке)//96 CHAR//DEF: «»(пустое поле)
                        return "Адрес сайта для проверки ФПД";
                    break;
                    case 9://Адрес личного кабинета (ЛК)//64 CHAR//DEF: service.atol.ruA/kkt
                        return "Адрес личного кабинета";
                    break;
                    case 10://Порт сервера диагностики//2 BIN(старший вперед)//DEF: 80
                        return "Порт сервера диагностики";
                    break;
                    case 11://Интервал посылок диагностических сообщений при сбое обмена с ЛК (в секундах)//2 BIN//(старший вперед)
                            //1...65535 (0001..FFFF)
                            //При установке значения 0000 считается, что установлено значение 60 минут.
                            //DEF: 60 секунд (003C)
                        return "Интервал посылок диагностических сообщений";
                    break;
                    case 12://Интервал ожидания квитанции (в мин.)//1 BIN//1..5//DEF: 5
                        return "Интервал ожидания квитанции (в мин.)";
                    break;
                    case 13://Интервал опроса ФН при сбоях ОФД – разрыве соединения сервером (в сек.)//1 BIN//5..120//DEF: 30
                        return "Интервал опроса ФН при сбоях ОФД (в сек.)";
                    break;
                    case 14://Код привязки пользователя//15 CHAR//DEF: «» пустое поле
                        return "Код привязки пользователя";
                    break;
                }
            }
        break;
        case 21://Таблица 21 «Заголовки реквизитов в ФД»
            switch (c) {
                case 1:
                    if (p == 1) return "Адрес расчетов";// (тег 1009)//57 CHARS//DEF: «Адрес:»
                break;
                case 2:
                    if (p == 1) return "Наименование пользователя";// (тег 1048)//57 CHARS//DEF: «Пользователь»
                break;
                case 3:
                    if (p == 1) return "Скидка на весь чек";//57 CHARS//DEF: «ОКРУГЛЕНИЕ»
                break;
                case 4:
                    if (p == 1) return "Скидка на позицию";//57 CHARS//DEF: «СКАДКА»
                break;
                case 5:
                    if (p == 1) return "Надбавка на позицию";//57 CHARS//DEF: «НАДБАВКА»
                break;
                case 6:
                    if (p == 1) return "Заголовок чека";//57 CHARS//DEF: «КАССОВЫЙ ЧЕК»
                break;
                case 7:
                    if (p == 1) return "Свободная строка 1 Таблицы 20";//57 CHARS//DEF: до 57-и символов «пробел»
                break;
                case 8:
                    if (p == 1) return "Свободная строка 2 Таблицы 20";//57 CHARS//DEF: до 57-и символов «пробел»
                break;
                case 9:
                    if (p == 1) return "Свободная строка 3 Таблицы 20";//57 CHARS//DEF: до 57-и символов «—»
                break;
                case 10:
                    if (p == 1) return "Свободная строка 4 Таблицы 20";//57 CHARS//DEF: до 57-и символов «—»
                break;
                case 11:
                    if (p == 1) return "Свободная строка 5 Таблицы 20";//57 CHARS//DEF: до 57-и символов «—»
                break;
                case 12:
                    if (p == 1) return "Свободная строка 6 Таблицы 20";//57 CHARS//DEF: до 57-и символов «—»
                break;
                case 13:
                    if (p == 1) return "Свободная строка 7 Таблицы 20";//57 CHARS//DEF: до 57-и символов «пробел»
                break;
                case 14:
                    if (p == 1) return "Свободная строка 8 Таблицы 20";//57 CHARS//DEF: до 57-и символов «пробел»
                break;
            }
        break;
        case 22://Таблица 22 «Автоматическое снятие отчетов с гашением»
            if (c == 1) {
                switch (p) {//by pole
                    case 1://Время снятия отчета с гашением//2 BCD//Формат: XXYY – где:XX – часы, YY - минуты//DEF: 0123
                        return "Время снятия отчета с гашением";
                    break;
                    case 2://Настройка снятия отчетов с гашением//1 BIN//0..3
                           //0–не снимать отчет с гашением в указанное время;1–снимать отчет в определенное время;
                           //2–снимать отчет с гашением по истечении 24 часов с момента открытия смены;
                           //3–снимать отчет с гашением в указанное время и по истечении 24 часов с момента открытия смены;
                           //DEF: 0
                        return "Настройка снятия отчетов с гашением";
                    break;
                }
            }
        break;
    }//tbl

    return "Unknown name";
}
//----------------------------------------------------------------------
int8_t getTblType(uint8_t t, uint16_t c, uint8_t p, int *sl)
{
int8_t ret = -1;
int slen = 0;
//0-char, 1-uint8_t, 2-uint16_t, 3-uint32_t, 4-uint8_t[], 5-STLV(bytes), 6-VLN(bytes),
//7-BCD[2], 8-BIN[6], 9-BIN[4], 10-BCD, 11-BCD[8], 12-BCD[4], 13-BIN[2]

    switch (t) {//bt tables
        case 2://Таблица 2 «Тип и режимы кассы»
            switch (c) {//by column
                case 1:
                    switch (p) {//by pole
                        case 1 ://Номер ККТ в магазине//1 BIN//1..255 DEF: 1
                        case 85 ://Протокол работы ККТ2//1 BIN//0 – АТОЛ 2.4, 2 – АТОЛ 3.0, DEF: 2
                        case 107 ://Динамический тип IP-адреса//1 BIN
                                  //0 – отключен (используется статический тип IP-адреса) 1 – включен DEF: 1
                            slen = 1;
                            ret = 1;
                        break;
                        case 23 ://Пароль доступа//2 BCD//0000 .. 9999 DEF: 0000
                        case 91 ://Порт (Ethernet)//2 BCD//0000..9999 DEF: 5555
                            slen = 2;
                            ret = 7;//return "Пароль доступа";////return "Порт (Ethernet)";
                        break;
                        case 87 ://MAC-адрес (Ethernet)//6 BIN//DEF: FF:FF:FF:FF:FF:FF
                            slen = 6;
                            ret = 8;//return "MAC-адрес (Ethernet)";
                        break;
                        case 88 ://IP-адрес (Ethernet)//4 BIN//DEF: 192.168.10.1
                        case 89 ://Маска подсети (Ethernet)//4 BIN//DEF: 255.255.255.0
                        case 90 ://Шлюз (Ethernet)//4 BIN//DEF: 192.168.10.0
                            slen = 4;
                            ret = 9;//return : "IP-адрес (Ethernet)"; "Маска подсети (Ethernet)"; "Шлюз (Ethernet)";
                        break;
                    }//pole
                break;
            }//column
        break;
        case 3://Таблица 3 «Пароли кассиров и администраторов»
            switch (c) {//by column
                case 1: case 2: case 3: case 4: case 5: case 6: case 7: case 8: case 9: case 10: case 11: case 12:
                case 13: case 14: case 15: case 16: case 17: case 18: case 19: case 20: case 21: case 22: case 23: case 24:
                case 25: case 26: case 27: case 28: case 29: case 30:
                    switch (p) {//by pole
                        case 1 ://c=1..28 : Пароли кассиров (соответственно с 1 по 28 кассиры)//4 BCD//00000000..99999999//DEF:1..28
                                //c=29    : Пароль администратора//4 BCD//00000000..99999999;//DEF: 29
                                //c=30    : Пароль системного администратора//4 BCD//00000000..99999999;//DEF: 30
                            slen = 4;
                            ret = 12;
                        break;
                        case 2 ://c=1..28 : Имя кассира//57 CHAR//DEF: «КАССИР ХХ», ХХ – номер ряда
                                //c=29    : Имя администратора//57 CHAR//DEF: «АДМИНИСТРАТОР»
                                //c=30    : Имя системного администратора//57 CHAR//DEF: «СИС. АДМИНИСТРАТОР»
                        case 3 ://c=1..28 : ИНН кассира//12 ASCII//DEF: 12 символов «пробел»
                                //c=29    : ИНН администратора//12 ASCII//DEF: 12 символов «пробел»
                                //c=30    : ИНН системного администратора//12 ASCII//DEF: 12 символов «пробел»
                            if (p == 3) slen = 12; else slen = 57;
                            ret = 0;
                        break;
                    }
                break;
            }
        break;
        case 4://Таблица 4 «Параметры скидок и надбавок»
            switch (c) {//by column
                case 1 ://
                    switch (p) {//by pole
                        case 1 ://Режим работы скидок//1 BCD
                                //0 – скидки запрещены,1 – разрешены скидки на весь чек,
                                //2 – разрешены скидки на позицию, 3 – разрешены все скидки
                                //DEF: 3
                            slen = 1;
                            ret = 10;
                        break;
                    }
                break;
                case 2 ://
                    switch (p) {//by pole
                        case 1 ://Режим работы надбавок//1 BCD
                                //0 – надбавки запрещены, 1 – разрешены надбавки на весь чек
                                //2 – разрешены надбавки на позицию, 3 – разрешены все надбавки
                                //DEF: 3
                            slen = 1;
                            ret = 10;
                        break;
                    }
                break;
            }
        break;
        case 6://Таблица 6 «Текст в чеке»
            switch (c) {//by column
                case 1 : case 2: case 3: case 4: case 5: case 6: case 7: case 8: case 9: case 10: case 11: case 12:
                case 13: case 14: case 15: case 16: case 17: case 18: case 19: case 20:
                    switch (p) {//by pole
                        case 1 ://Запрограммированное значение строки клише или рекламного текста//57 CHAR
                                //DEF:ряды 4..5, 9..20: 57 пробелов,ряд 1: СПАСИБО,ряд 2: ЗА ПОКУПКУ!
                                //ряды 3, 8: 57 символов «—», ряд 6: ТОРГОВЫЙ ОБЪЕКТ №1,ряд 7: ДОБРО ПОЖАЛОВАТЬ!
                            slen = 57;
                            ret = 0;
                        break;
                    }
                break;
            }
        break;
        case 7://Таблица 7 «Наименования секций»
            switch (c) {//by column
                case 1: case 2: case 3: case 4: case 5: case 6: case 7: case 8:
                case 9: case 10: case 11: case 12: case 13: case 14: case 15: case 16:
                    switch (p) {//by pole
                        case 1 ://Запрограммированное наименование секции//57 CHAR//ряд 1: СЕКЦИЯ 01
                                //Запрограммированное наименование секции//57 CHAR//ряд 1: СЕКЦИЯ 02
                                //...
                                //Запрограммированное наименование секции//57 CHAR//ряд 1: СЕКЦИЯ 16
                            slen = 57;
                            ret = 0;
                        break;
                        case 2 ://Номер налога//1 BCD//DEF: 1
                            slen = 1;
                            ret = 10;
                        break;
                    }
                break;
            }
        break;
        case 9://Таблица 9 «Подключенное оборудование»
            switch (c) {//by column
                case 1: case 2:
                    switch (p) {//by pole
                        case 1 : //Тип устройства//1 BCD//DEF: 3
                        case 2 : //Скорость устройства//1 BCD//1-1200,2-2400,3-4800,4-9600,5-14400,6-38400,7-57600,8-115200,9-19200,DEF: 8
                            slen = 1;
                            ret = 10;//return "Тип устройства";//return "Скорость устройства";
                        break;
                    }//pole
                break;
                case 4:
                    switch (p) {//by pole
                        case 1 : //0 – ККТ является пассивным устройством по интерфейсу RS-232
                                 //4 – ККТ является пассивным устройством по интерфейсу USB
                                 //6 – ККТ является пассивным устройством по интерфейсу Ethernet
                                 //7 – ККТ является пассивным устройством по интерфейсу WiFi; DEF: 4
                            slen = 1;
                            ret = 1;//return "Тип устройства";
                        break;
                    }//pole

                break;
            }//column
        break;
        case 10://Таблица 10 «Защита ККТ»
            switch (c) {//by column
                case 1: case 2: case 3: case 4: case 5: case 6: case 7: case 8: case 9: case 10:
                case 11: case 12: case 13: case 14: case 15: case 16: case 17: case 18: case 19: case 20:
                case 21: case 22: case 23: case 24: case 25: case 26: case 27: case 28: case 29: case 30:
                    switch (p) {//by pole
                        case 1 ://Код защиты ККТ //8 BCD//0000000000000000 .. 9999999999999999  DEF: 0
                            slen = 8;
                            ret = 11;//return "Код защиты ККТ";
                        break;
                    }
                break;
            }//column
        break;
        case 12://Таблица 12 «Наименования форм и способов расчета»
            switch (c) {//by column
                case 1: case 2: case 3: case 4: case 5: case 6: case 7: case 8: case 9:
                    switch (p) {//by pole
                        case 1 ://c=1 : Наименование способа расчета 2//64 CHAR//DEF: Безналичными
                                //c=2 : Наименование способа расчета 3//64 CHAR//DEF: ПРЕДВАРИТЕЛЬНАЯ ОПЛАТА (АВАНС)
                                //c=3 : Наименование способа расчета 4//64 CHAR//DEF: ПОСЛЕДУЮЩАЯ ОПЛАТА (КРЕДИТ)
                                //c=4 : Наименование способа расчета 5//64 CHAR//DEF: ИНАЯ ФОРМА ОПЛАТЫ
                                //c=5 : Наименование способа расчета 6//64 CHAR//DEF: ПЛАТ.КАРТОЙ
                                //c=6 : Наименование способа расчета 7//64 CHAR//DEF: ТАРОЙ
                                //c=7 : Наименование способа расчета 8//64 CHAR//DEF: КРЕДИТОМ
                                //c=8 : Наименование способа расчета 9//64 CHAR//DEF: ТИП 9
                                //c=9 : Наименование способа расчета 10//64 CHAR//DEF: ТИП 10
                            slen = 64;
                            ret = 0;
                        break;
                        case 2 ://Номер формы расчета//1 BIN
                                //1 – Наличными,2 – Безналичными,3 – Предварительная оплата (аванс),
                                //4 – Последующая оплата (кредит);5 – Иная форма оплаты.//DEF: 2
                            slen = 1;
                            ret = 1;
                        break;

                    }
                break;
            }//column
        break;
        case 15://Таблица 15 «Наименования реквизитов»
            if ((c >= 2) && (c <= 100)) {
                if (p == 1) {
                    slen = 25;
                    ret = 0;//Запрограммированное наименование реквизита//25 CHAR
                }
            }
        break;
        case 19://Таблица 19 «Настройка соединения с ОФД»
            if (c == 1) {
                switch (p) {//by pole
                    case 1://Адрес ОФД//64 CHAR//DEF: 0.0.0.0
                    case 9://Адрес личного кабинета (ЛК)//64 CHAR//DEF: service.atol.ruA/kkt
                        slen = 64;
                        ret = 0;
                    break;
                    case 8://Адрес сайта для проверки ФПД (для печати в чеке)//96 CHAR//DEF: «»(пустое поле)
                        slen = 96;
                        ret = 0;
                    break;
                    case 14://Код привязки пользователя//15 CHAR//DEF: «» пустое поле
                        slen = 15;
                        ret = 0;
                    break;
                    case 2://Порт ОФД//2 BIN//(старший байт вперед)//DEF: 7777
                    case 10://Порт сервера диагностики//2 BIN(старший вперед)//DEF: 80
                    case 11://Интервал посылок диагностических сообщений при сбое обмена с ЛК (в секундах)//2 BIN//(старший вперед)
                            //1...65535 (0001..FFFF)
                            //При установке значения 0000 считается, что установлено значение 60 минут.
                            //DEF: 60 секунд (003C)
                        slen = 2;
                        ret = 13;
                    break;
                    case 3://DNS ОФД//4 BIN//DEF: 0.0.0.0
                        slen = 4;
                        ret = 9;
                    break;
                    case 4://Канал обмена с ОФД//1 BIN
                           //1–EthernetOverUsb,2–Ethernet,3–Wifi,4–GSM модем,5–EthernetOver Transport//DEF: 1
                    case 12://Интервал ожидания квитанции (в мин.)//1 BIN//1..5//DEF: 5
                    case 13://Интервал опроса ФН при сбоях ОФД – разрыве соединения сервером (в сек.)//1 BIN//5..120//DEF: 30
                        slen = 1;
                        ret = 1;
                    break;
                }
            }
        break;
        case 21://Таблица 21 «Заголовки реквизитов в ФД»
            if ((c > 0) && (c < 15)) {
                if (p == 1) {
                    slen = 57;
                    ret = 0;//57 CHARS
                }
            }
        break;
        case 22://Таблица 22 «Автоматическое снятие отчетов с гашением»
            if (c == 1) {
                switch (p) {//by pole
                    case 1://Время снятия отчета с гашением//2 BCD//Формат: XXYY – где:XX – часы, YY - минуты//DEF: 0123
                        slen = 2;
                        ret = 7;
                    break;
                    case 2://Настройка снятия отчетов с гашением//1 BIN//0..3
                           //0–не снимать отчет с гашением в указанное время;1–снимать отчет в определенное время;
                           //2–снимать отчет с гашением по истечении 24 часов с момента открытия смены;
                           //3–снимать отчет с гашением в указанное время и по истечении 24 часов с момента открытия смены;
                           //DEF: 0
                        slen = 1;
                        ret = 1;
                    break;
                }
            }
        break;
    }//tbl

    if (ret != 0) ret = -1;//for string only !!!

    *sl = slen;

    return ret;
}
//----------------------------------------------------------------------
static const char *statFNStr(uint8_t bt)
{
    switch (bt) {
        case 0:
            return "Настройка ФН";
        case 1:
            return "готовность к активации";
        case 3:
            return "Фискальный режим ФН";
        case 7:
            return "Постфискальный режим (производится передача фискальных документов ОФД)";
        case 15:
            return "Доступ к архиву ФН";
    }

    return "???";
}
//----------------------------------------------------------------------
static const char *curDocStr(uint8_t bt)
{
    switch (bt) {
        case 0:
            return "Нет открытого документа";
        case 1:
            return "Отчёт о регистрации ККТ";
        case 2:
            return "Отчёт об открытии смены";
        case 4:
            return "Кассовый чек";
        case 8:
            return "Отчёт о закрытии смены";
        case 0x10:
            return "Отчёт о закрытии фискального режима";
        case 0x12:
            return "Отчет об изменении параметров регистрации ККТ в связи с заменой ФН";
        case 0x13:
            return "Отчет об изменении параметров регистрации ККТ";
        case 0x14:
            return "Кассовый чек коррекции";
        case 0x17:
            return "Отчет о текущем состоянии расчетов";
    }

    return "???";
}
//----------------------------------------------------------------------
static const char *dataDocStr(uint8_t bt)
{
    switch (bt) {
        case 0:
            return "Нет данных документа";
        case 1:
            return "Получены данные документа";
    }

    return "???";
}
//----------------------------------------------------------------------
static const char *shiftStatStr(uint8_t bt)
{
    switch (bt) {
        case 0:
            return "Смена закрыта";
        case 1:
            return "Смена открыта";
    }

    return "???";
}
//----------------------------------------------------------------------
char *alarmStr(uint8_t fl, char *st)
{
    if (st) {
        if (fl & 1)    strcat(st, "\n\t  Требуется срочная замена ФН");//bit0
        if (fl & 2)    strcat(st, "\n\t  Ресурс КС исчерпан (до окончания срока действия осталось 30 дней)");//bit1
        if (fl & 4)    strcat(st, "\n\t  Памяти ФН переполнена (архив ФН заполнен на 90%)");//bit2
        if (fl & 8)    strcat(st, "\n\t  Время ожидания ответа ОФД превышено");//bit3
        if (fl & 0x80) strcat(st, "\n\t  Критическая ошибка ФН");//bit7
    }

    return st;
}
//----------------------------------------------------------------------
static const char *typeSwStr(uint8_t bt)
{
    switch (bt) {
        case 0:
            return "Отладочная версия";
        case 1:
            return "Серийная версия";
    }

    return "???";
}
//----------------------------------------------------------------------
char *statFlagsStr(uint8_t fl, char *st)
{
    if (st) {
        if (fl & 1)    strcat(st, "\n\t  ККТ фискализирована");//bit0
        if (fl & 2)    strcat(st, "\n\t  Смена открыта");//bit1
        if (fl & 4)    strcat(st, "\n\t  Наличие бумаги в презенторе");//bit2
        if (fl & 8)    strcat(st, "\n\t  Датчик ЧЛ - есть бумага");//bit3
        if (fl & 0x20) strcat(st, "\n\t  Крышка открыта");//bit5
        if (fl & 0x40) strcat(st, "\n\t  ФН активизирован");//bit6
        if (fl & 0x80) strcat(st, "\n\t  Батарейка в ККТ отсутствует");//bit7
    }

    return st;
}
//----------------------------------------------------------------------
