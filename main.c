//**********************************************************************
//
//                       Версия протокола 2
//
//**********************************************************************

#include "functions.h"

//-----------------------------------------------------------------------

int main (int argc, char *argv[])
{
uint8_t to_dev[buf_size];
uint8_t from_dev[buf_size];
char dev_name[sdef] = {0};
char dev_cmd[sdef] = {0};
char dev_password[sdef] = {0};
char dev_arg[buf_size] = {0};
char chap[(buf_size << 1) + 64];
char tmp[sdef];
int Vixod = 0, lenr = 0, lenr_tmp = 0, i, k = 1, rdy = 0, all_len = 0;
int ret = RET_OK, ackRet = RET_NONE_ERROR;;
int lens = 0, inc = -1, dl = 0;
struct timeval mytv;
fd_set Fds;
uint8_t byte = 0, last_byte, yes, rx_stat = 0, faza = 0, cnt = 0;
struct sigaction Act, OldAct;
char *uk = NULL;
uint32_t tmr, tmr_cmd, wait_ack_sec = wait_ack_min_sec;
uint8_t clr_done = 0;
uint8_t dev_spd = 4;//spd[dev_spd] = 115200 - default speed
struct itimerval itmr;



    setlocale(LC_ALL, "en_US.UTF-8");


    sprintf(tmp, "%s%s", path_log, file_log);
    fd_log = open(tmp, O_WRONLY | O_APPEND | O_CREAT, 0664);//open log file
    if (fd_log < 0) {
        ret = RET_MAJOR_ERROR;
        sprintf(chap, "[Ver.%s] %s Can't open %s file (return %d)\n", vers, TimeNowPrn(tmp), tmp, ret);
        ToSysLogMsg(LOG_INFO, chap);
        return ret;
    }
/*
    // make symlink if not present
    sprintf(tmp, "ls | grep %s >>/dev/null", file_log);
    if (system(tmp)) {
        sprintf(tmp, "ln -s %s%s ./%s", path_log, file_log, file_log);
        system(tmp);
    }
*/
    if (argc < 3) {// < 4
        ret = RET_MAJOR_ERROR;//-1;
        if (dbg != LOG_OFF) print_msg(1, "ERROR: you must enter incomming options. For example: ./custom --dev=/dev/ttyACM0 --cmd=beep --password=0000 --arg=empty (return %d)\n\n", ret);
        close(fd_log);
        return ret;
    }

    while (k < argc) {
        yes = 0;
        strcpy(chap, argv[k]);
        for (i = 0; i < max_param; i++) {
            if ((uk = strstr(chap, name_param[i]))) {
                uk += strlen(name_param[i]);
                switch (i) {
                    case 0://"--dev="
                        strcpy(dev_name, uk);
                        cnt++;
                        yes = 1;
                    break;
                    case 1://"--cmd="
                        strcpy(dev_cmd, uk);
                        cnt++;
                        yes = 1;
                    break;
                    case 2://"--arg="
                        strcpy(dev_arg, uk);
                        if (!strstr(uk, "empty")) {
                            dl = strlen(dev_arg);
                            if (dl > (sdef - 1)) {
                                dl = sdef - 1;
                                dev_arg[dl] = '\0';
                            }
                        }
                        yes = 1;
                    break;
                    case 3://"--log="
                             if (strstr(uk, "on"))    dbg = LOG_ON;
                        else if (strstr(uk, "debug")) dbg = LOG_DEBUG;
                        else if (strstr(uk, "dump"))  dbg = LOG_DUMP;
                        yes = 1;
                    break;
                    case 4://"--speed="
                        dev_spd = findSPEED(uk);
                        yes = 1;
                    break;
                    case 5://"--password="
                        strcpy(dev_password, uk);
                        passwd = getPASSWD(dev_password);
                        //cnt++;
                        yes = 1;
                    break;
                    case 6://"--codepage="
                        strncpy(codePage, uk, sizeof(codePage) - 1);
                        //cnt++;
                        yes = 1;
                    break;
                }
            }
            if (yes) break;
        }
        k++;
    }
    if (cnt < 2) {// < 3
        ret = RET_MAJOR_ERROR;//-1;
        sprintf(chap, "Error: you must enter incomming options. For example: ./custom --dev=/dev/ttyACM0 --cmd=beep --password=0000 --arg=empty (return %d)\n\n", ret);
        if (dbg != LOG_OFF) print_msg(1, chap);
        close(fd_log);
        ToSysLogMsg(LOG_INFO, chap);
        return ret;
    }
    strcpy(device, dev_name);

    // remove 0x0A 0x0D from dev_arg
    uk = strchr(dev_arg, '\n');
    if (uk) {
        *uk = '\0';
        uk = strchr(dev_arg, '\r');
        if (uk) *uk = '\0';
    }

    inc = parse_inCMD(dev_cmd);
    if (inc < 0) {
        ret = RET_MINOR_ERROR;//1;
        if (dbg != LOG_OFF) print_msg(1, "Error: Unknown incomming command '%s' (return %d)\n", dev_cmd, ret);
        close(fd_log);
        return ret;
    }
    if (inc == CMD_LINE_PRINT) {//1// "line_print"
        if (!strlen(dev_arg)) strcpy(dev_arg, eol);//if arg="" set arg="\r\n"
    }

    sprintf(chap, "[Ver.%s] Start custom with dev='%s' cmd[%d]='%s' arg(%lu)='%s' dbg=%u\n",
                 vers, dev_name, inc, dev_cmd, strlen(dev_arg), dev_arg, dbg);
    if (dbg != LOG_OFF) print_msg(1, chap);
    ToSysLogMsg(LOG_INFO, chap);


    fd = open(dev_name, O_RDWR , 0664);
    if (fd < 0) {
        ret = RET_MAJOR_ERROR;//-1;
        if (dbg != LOG_OFF) print_msg(1, "Error: Can't open device '%s' (return %d)\n", dev_name, ret);
        close(fd_log);
        return ret;
    }

    //--------------------  set Signals route function ------------------

    memset((uint8_t *)&Act,    0, sizeof(struct sigaction));
    memset((uint8_t *)&OldAct, 0, sizeof(struct sigaction));
    Act.sa_handler = &GetSignal_;
    Act.sa_flags   = 0;
    sigaction(SIGHUP,  &Act, &OldAct);
    sigaction(SIGSEGV, &Act, &OldAct);
    sigaction(SIGTERM, &Act, &OldAct);
    sigaction(SIGABRT, &Act, &OldAct);
    sigaction(SIGINT,  &Act, &OldAct);
    sigaction(SIGKILL, &Act, &OldAct);

    sigaction(SIGALRM, &Act, &OldAct);

    //  SET TIMER to 100ms
    itmr.it_value.tv_sec     = 0;
    itmr.it_value.tv_usec    = 100000;//100ms
    itmr.it_interval.tv_sec  = 0;
    itmr.it_interval.tv_usec = 100000;//100ms
    if (setitimer(ITIMER_REAL, &itmr, NULL) != 0) {
        ret = RET_MAJOR_ERROR;//-1;
        if (fd > 0) close(fd);
        if (dbg != LOG_OFF) print_msg(1, "Error: Can't start timer (return %d)\n", dev_name, ret);
        close(fd_log);
        return ret;
    }

    //-------------------------------------------------------------------

    setSPEED(dev_spd);//4-115200 // 3-57600 // 1-9600

    //-------------------------------------------------------------------

    byte = last_byte = 0;
    faza = cnt = 0;
    lens = 0;
    tmr = 0;
    tmr_cmd = get_timer_ms(_200ms);
    uint8_t cmdBYTE = ENQ;
    uint8_t *cmdUK = &cmdBYTE;

    //-------------------------------------------------------------------


    while (!Vixod) {

        switch (faza) {
            case 0:
                if (check_delay_ms(tmr_cmd)) {
                    clr_done = 1;
                    lens = makeCMD(to_dev, inc, dev_arg, cmdUK);
                    if (lens > 0) {//команда сформирована успешно
                        if (dbg >= LOG_DEBUG) {
                            sprintf(chap, "to device (%d): >", lens);
                            for (i = 0; i < lens; i++) sprintf(chap+strlen(chap), " %02X", to_dev[i]);
                            print_msg(1, "%s\n", chap);
                        }
                        if (write(fd, to_dev, lens) != lens) {
                            if (dbg != LOG_OFF) print_msg(1, "Error: Can't write to device '%s'\n", dev_name);
                            ret = RET_MINOR_ERROR;//1;
                            Vixod = 1;
                        } else {
                            faza = 1;
                            if (!cmdUK)
                                wait_ack_sec = (uint32_t)get_ack_wait(inc);
                            else
                                wait_ack_sec = wait_ack_min_sec;
                            tmr = get_timer_ms(wait_ack_sec);
                            tmr_cmd = 0;
                            cnt++;
                        }
                    } else {
                        if (dbg != LOG_OFF) print_msg(1, "Error: Can't make CMD for '%s':'%s'\n", dev_cmd, dev_arg);
                        ret = RET_MINOR_ERROR;//1;
                        Vixod = 1;
                    }
                }
            break;
            case 1://wait ack
                if (tmr) {
                    if (check_delay_ms(tmr)) {
                        if (dbg != LOG_OFF) print_msg(1, "Error: Timeout (%u sec) wait ack for '%s':'%s' \n", wait_ack_sec/10, dev_cmd, dev_arg);
                        ret = RET_TIMEOUT;//2;
                        Vixod = 1;
                    }
                }
            break;
        }//switch(faza)

        if (!Vixod) {
            FD_ZERO(&Fds);
            FD_SET(fd, &Fds);
            mytv.tv_sec  = 0;
            mytv.tv_usec = 20000;
            if (select(fd + 1, &Fds, NULL, NULL, &mytv) > 0) {
                if (FD_ISSET(fd, &Fds)) {// rx_event from my device
                    //
                    lenr_tmp = read(fd, &byte, 1);
                    //
                    if (lenr_tmp <= 0) {
                        if (dbg != LOG_OFF) print_msg(1, "Error: Can't read from device '%s'\n", dev_name);
                        ret = RET_MINOR_ERROR;//1;
                        break;
                    } else if (lenr_tmp > 0) {
                        if (dbg == LOG_DUMP) print_msg(0, " %02X", byte);
                        if (clr_done) {
                            from_dev[lenr++] = byte;

                            if (byte == STX) {
                                rx_stat = 1;
                            } else if ((byte == ACK) && (lenr == 1)) {//6
                                all_len = lenr;
                                ret = RET_OK;//0;
                            } else if ((byte == NAK) && (lenr == 1)) {//21=0x15
                                all_len = lenr;
                                ret = RET_NAK;//3;
                            } if ((byte == ENQ) && (lenr == 1)) {
                                all_len = lenr;
                                ret = RET_ENQ;//4;
                            } else if ( (byte == ETX) && (last_byte != DLE) ) {
                                all_len = lenr + 1;
                            } else if ((byte == EOT) && (lenr == 1)) {
                                all_len = lenr;
                                ret = RET_EOT;
                            }
                            last_byte = byte;
                        } else byte = 0;

                        //if (lenr >= buf_size - 1) rdy = 1;
                        if (lenr >= max_len_from_dev) rdy = 1;
                        else {
                            if (lenr == all_len) rdy = 1;
                        }
                    }
                    if (rdy) {
                        if (dbg == LOG_DUMP) print_msg(0, "\n");
                        tmr = 0;
                        memset(chap, 0, sizeof(chap));
                        if (dbg >= LOG_DEBUG) {
                            sprintf(chap, "from device (%d): <", lenr);
                            for (i = 0; i < lenr; i++) sprintf(chap+strlen(chap), " %02X", from_dev[i]);
                            print_msg(1, "%s\n", chap);
                        }
                        if (rx_stat) {
                            ackRet = parse_ANSWER(from_dev, lenr, inc);
                            to_dev[0] = ACK;
                            if (dbg >= LOG_DEBUG) print_msg(1, "to device (%d): > %02X\n", 1, to_dev[0]);
                            if (write(fd, to_dev, 1) != 1) {
                                    if (dbg != LOG_OFF) print_msg(1, "Error: Can't write to device '%s'\n", dev_name);
                                    ret = RET_MINOR_ERROR;//1;
                                    Vixod = 1;
                            } else {
                                    faza = 1;
                                    wait_ack_sec = wait_ack_min_sec;
                                    tmr = get_timer_ms(wait_ack_sec);
                                    ret = RET_NONE_ERROR;//1;
                            }
                            // Error data from device : no DLE before ETX
                            if (ackRet == RET_MINOR_ERROR) {//CRC Error !
                                ret = RET_MINOR_ERROR;//1;
                                Vixod = 1;
                            }
                        }

                        switch (ret) {
                            case RET_EOT://got EOT from device - session done
                                if (ackRet != RET_NONE_ERROR) ret = ackRet;//RET_OK;
                                Vixod = 1;
                            break;
                            case RET_NAK://got NAK from device
                                if (cnt < max_try) {
                                    faza = 0;
                                    tmr = 0;
                                    tmr_cmd = get_timer_ms(_200ms);
                                } else Vixod = 1;
                            break;
                            case RET_ENQ://got ENQ from device
                                to_dev[0] = ACK;
                                if (dbg >= LOG_DEBUG) print_msg(1, "to device (%d): > %02X\n", 1, to_dev[0]);
                                if (write(fd, to_dev, 1) != 1) {
                                    if (dbg != LOG_OFF) print_msg(1, "Error: Can't write to device '%s'\n", dev_name);
                                    ret = RET_MINOR_ERROR;//1;
                                    Vixod = 1;
                                } else {
                                    faza = 1;
                                    wait_ack_sec = wait_ack_min_sec;
                                    tmr = get_timer_ms(wait_ack_sec);
                                }
                            break;
                            case RET_OK://got ACK from device - > command is done
                                if (!cmdUK) {//sended STX-command
                                    to_dev[0] = EOT;
                                    if (dbg >= LOG_DEBUG) print_msg(1, "to device (%d): > %02X\n", 1, to_dev[0]);
                                    if (write(fd, to_dev, 1) != 1) {
                                        if (dbg != LOG_OFF) print_msg(1, "Error: Can't write to device '%s'\n", dev_name);
                                        ret = RET_MINOR_ERROR;//1;
                                        Vixod = 1;
                                    } else {//end of session, wait answer for command
                                        Vixod = 0;
                                        ret = RET_OK;
                                        faza = 1;
                                        wait_ack_sec = wait_ack_max_sec;//10sec
                                        tmr = get_timer_ms(wait_ack_sec);
                                        if (inc == 3) Vixod = 1;// "beep" - no wait answer for this command
                                    }
                                } else {
                                    cmdUK = NULL;
                                    faza = 0;
                                    tmr = 0;
                                    tmr_cmd = get_timer_ms(_200ms);
                                }
                            break;
                        }//switch (ret)

                        if (!Vixod) {
                            memset(from_dev, 0, sizeof(from_dev));
                            rdy = all_len = 0;
                            lenr = lenr_tmp = 0;
                            rx_stat = 0;
                            byte = 0;
                            last_byte = 0;
                        }
                    }//if (rdy)
                }//if (FD_ISSET(fd, &Fds))
            }//if (select
        }//if (!Vixod)

        if (QuitAll) { ret = RET_MINOR_ERROR; break; }
        //---------------------------------------------------
    }//while (!Vixod)


    sprintf(chap, "[Ver.%s] Stop custom (return 0x%X/%d)\n", vers, (uint32_t)ret, ret);
    if (dbg != LOG_OFF) print_msg(1, chap);

    ToSysLogMsg(LOG_INFO, chap);

    close(fd_log);

    return ret;

}
